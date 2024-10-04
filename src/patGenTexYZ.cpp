#include <mutex>
#include <thread>

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>

#include <conio.h>

#include <common.h>
#include <iniFile.h>

#include <pattern.h>
#include <patternsTiled.h>

#include <trackball.h>
#include <vertexBuffer.h>

#ifdef _WIN32
#include <Windows.h>
#include <gl/wglew.h>
#endif

// OpenGL / glew Headers
//#define GL3_PROTOTYPES 1
#include <GL/glew.h>

#ifdef USE_GLUT
#include <GL/freeglut.h>
#else
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>
#endif

#include <imgui/imgui.h>
#include <imgui/imgui_impl_freeglut.h>
#include <imgui/imgui_impl_opengl2.h>

#include <nlohmann/json.hpp>

#include <glm/gtc/type_ptr.hpp>

using namespace TextileUX;

#define KEY_RETURN	13
#define KEY_DC1		17
#define KEY_ESC		27

#ifdef WIN32
#define WINDOW_DECORATION_OFFSET_X 8
#define WINDOW_DECORATION_OFFSET_Y 31
#else
//TODO
#error put decoration sizes of other GUI systems here
#endif

#ifdef USE_GLUT
int glutWindowID = -1;
#else
SDL_Window *mainWindow = nullptr;
SDL_GLContext glContext = nullptr;
#endif

int windowX = 100;
int windowY = 100;

int windowWidth = 768;
int windowHeight = 512;

float ar = 1.0f;

double timeAccu = 0.0;

glm::vec4 clearColor;

bool drawGrid = true;
int gridCount = 3;
float gridSpacing = 0.25f;
std::vector<VertexBuffer*> grids( 3 );

PatternParamsBase *params = nullptr;
TiledPattern *pattern = nullptr;
int stitchProgress = 0;

bool drawCrosshair = false;
glm::vec3 crosshairColor( 0.75f, 0.75f, 0.75f );

glm::vec2 mousePos;

glm::vec3 defaultTrackballT = zero();
float defaultTrackballR = 0.0f;
float defaultTrackballSE = -2.5;
Trackball2D trackball( defaultTrackballT, defaultTrackballR, defaultTrackballSE );

IniFile *ini = nullptr;
char imGuiIniFileName[1024];

std::string statusString( "ready" );

Unit unit = U_MM;

namespace TextileUX
{
	extern double globalTime;
}

bool quit = false;
bool cleaned = false;

bool uiActive = true;

//have to use flag to keep track of ImGui init state.
bool imGuiInitialized = false;

void save()
{
	if( pattern )
	{
		if( pattern->save() )
			std::cout << "successfully saved pattern to disc" << std::endl;
		else
			std::cerr << "failed to save pattern to disc" << std::endl;
	}
}

void clearPattern()
{
	safeDelete( params );
	safeDelete( pattern );

	stitchProgress = 0;
}

void createGrids()
{
	int p = pow( 2, grids.size() - 1 );
	float size = p * 10 * gridSpacing;

	std::cout << "p: " << p << ", size: " << size << std::endl;

	for( int i = 0; i < grids.size(); i++ )
	{
		safeDelete( grids[i] );

		int subdivisions = ( 10 * pow( 2, i ) ) - 1;
		grids[i] = VertexBuffer::createGrid( glm::vec2( size ), subdivisions, glm::vec3( 0.2f, 0.3f, 0.5f ) * 0.5f );
	}
}

void motion( int x, int y )
{
	bool imGuiHandled = ( ImGui::GetIO().WantCaptureMouse && uiActive );

	mousePos.x = x;
	mousePos.y = y;

#ifdef USE_GLUT
	if( uiActive )
		ImGui_ImplFreeGLUT_MotionFunc( x, y );
#endif

	if( !imGuiHandled )
		trackball.onMotion( x, y );
}

void passiveMotion( int x, int y )
{
	bool imGuiHandled = ( ImGui::GetIO().WantCaptureMouse && uiActive );

	mousePos.x = x;
	mousePos.y = y;

#ifdef USE_GLUT
	if( uiActive )
		ImGui_ImplFreeGLUT_MotionFunc( x, y );
#endif

	if( !imGuiHandled )
		trackball.onPassiveMotion( x, y );
}

void mouse( int button, int state, int x, int y )
{
	bool imGuiHandled = ( ImGui::GetIO().WantCaptureMouse && uiActive );

#ifdef USE_GLUT
	if( uiActive )
		ImGui_ImplFreeGLUT_MouseFunc( button, state, x, y );
#endif

	if( !imGuiHandled )
		trackball.onMouse( button, state, x, y );

	if( button == 0 )
	{
		if( state )
			drawCrosshair = false;
		else if( !imGuiHandled )
			drawCrosshair = true;
	}
}

void mouseWheel( int wheel, int direction, int x, int y )
{
	bool imGuiHandled = ( ImGui::GetIO().WantCaptureMouse && uiActive );

#ifdef USE_GLUT
	if( uiActive )
		ImGui_ImplFreeGLUT_MouseWheelFunc( wheel, direction, x, y );
#endif

	if( !imGuiHandled )
		trackball.onMouseWheel( wheel, direction, x, y );
}

#ifdef USE_GLUT
void keyDown( unsigned char key, int x, int y )
#else
void keyDown( int key )
#endif
{
	bool imGuiHandled = ( ImGui::GetIO().WantCaptureKeyboard && uiActive );

	if( !imGuiHandled )
		if( trackball.onKeyDown( key, x, y ) )
			return;

#ifdef USE_GLUT
	if( uiActive )
		ImGui_ImplFreeGLUT_KeyboardFunc( key, x, y );

	int mods = glutGetModifiers();
	switch( key )
	{
	case KEY_RETURN:
	case KEY_ESC:
		stitchProgress = 0;
		break;
	case KEY_DC1:
		if( !( mods & GLUT_ACTIVE_SHIFT ) && !( mods & GLUT_ACTIVE_ALT ) )
			quit = true;
		break;
	}
#else
	switch( key )
	{
	case SDLK_SPACE:
		break;
	case SDLK_PLUS:
	case SDLK_KP_PLUS:
		break;
	case SDLK_MINUS:
	case SDLK_KP_MINUS:
		break;
	case SDLK_RETURN:
	case SDLK_ESCAPE:
		stitchProgress = 0;
		break;
	}
#endif
}

void keyUp( unsigned char key, int x, int y )
{
	bool imGuiHandled = ( ImGui::GetIO().WantCaptureKeyboard && uiActive );

	if( !imGuiHandled )
		if( trackball.onKeyUp( key, x, y ) )
			return;

#ifdef USE_GLUT
	if( uiActive )
		ImGui_ImplFreeGLUT_KeyboardUpFunc( key, x, y );
#endif
}

void specialDown( int key, int x, int y )
{
	bool imGuiHandled = ( ImGui::GetIO().WantCaptureKeyboard && uiActive );

	if( !imGuiHandled )
		if( trackball.onSpecialDown( key, x, y ) )
			return;

#ifdef USE_GLUT
	if( uiActive )
		ImGui_ImplFreeGLUT_SpecialFunc( key, x, y );

	switch( key )
	{
	case GLUT_KEY_F2:
		save();
		break;
	case GLUT_KEY_LEFT:
		if( stitchProgress )
			stitchProgress--;
		break;
	case GLUT_KEY_RIGHT:
		stitchProgress++;
		break;
	}
#endif
}

void specialUp( int key, int x, int y )
{
	bool imGuiHandled = ( ImGui::GetIO().WantCaptureKeyboard && uiActive );

	if( !imGuiHandled )
		if( trackball.onSpecialUp( key, x, y ) )
			return;

#ifdef USE_GLUT
	if( uiActive )
		ImGui_ImplFreeGLUT_SpecialUpFunc( key, x, y );

	switch( key )
	{
	}
#endif
}

void update()
{
	static auto prevTime = std::chrono::system_clock::now();
	auto currentTime = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = currentTime - prevTime;
	prevTime = currentTime;

	double dt = diff.count();
	timeAccu += dt;
	::globalTime += dt;

	//update potential temporal stuff here
}

void display()
{
#ifdef USE_GLUT
	windowX = glutGet( (GLenum) GLUT_WINDOW_X ) - WINDOW_DECORATION_OFFSET_X;
	windowY = glutGet( (GLenum) GLUT_WINDOW_Y ) - WINDOW_DECORATION_OFFSET_Y;
#else
	SDL_GetWindowSize( mainWindow, &windowWidth, &windowHeight );
#endif

	glViewport( 0, 0, windowWidth, windowHeight );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( -1.0f, 1.0f, -1.0f / ar, 1.0f / ar );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glPushAttrib( GL_ALL_ATTRIB_BITS );
	{
		glPushMatrix();
		{
			glMultMatrixf( glm::value_ptr( trackball.mat() ) );

			if( drawGrid )
				for( int i = 0; i < grids.size(); i++ )
				{
					glLineWidth( grids.size() - i );
					grids[i]->draw();
				}

			if( pattern )
			{
				pattern->draw();

				if( stitchProgress > 0 )
				{
					glPushAttrib( GL_ALL_ATTRIB_BITS );
					{
						glEnable( GL_COLOR_MATERIAL );
						glDisable( GL_DEPTH_TEST );

						if( stitchProgress > pattern->getTotalStitchCount() )
							stitchProgress = pattern->getTotalStitchCount();

						if( stitchProgress )
						{
							//TODO: adapt this code to support multiple lower and multiple upper traces

							/*
							int lower = min<int>( stitchProgress, pattern->getTrace().getStitchCount() );
							int upper = max<int>( 0, stitchProgress - lower );

							if( lower )
							{
								std::vector<glm::vec3> verts( lower );
								std::copy( pattern->getTrace().getStitches().begin(), pattern->getTrace().getStitches().begin() + lower, verts.begin() );

								glColor3f( 1, 0, 0 );
								glLineWidth( 2.0f );

								glBegin( GL_LINE_STRIP );
								{
									for( auto& v : verts )
										glVertex3fv( glm::value_ptr( v ) );
								}
								glEnd();

								glPointSize( 3.0f );

								glBegin( GL_POINTS );
								{
									for( auto& v : verts )
										glVertex3fv( glm::value_ptr( v ) );
								}
								glEnd();

								if( lower == stitchProgress )
								{
									glPointSize( 5.0f );
									glColor3f( 1, 1, 1 );

									glBegin( GL_POINTS );
									glVertex3fv( glm::value_ptr( verts.back() ) );
									glEnd();
								}
							}

							if( upper )
							{
								std::vector<glm::vec3> verts( upper );
								std::copy( pattern->getTrace2().getStitches().begin(), pattern->getTrace2().getStitches().begin() + upper, verts.begin() );

								glColor3f( 1, 0, 0 );
								glLineWidth( 2.0f );

								glBegin( GL_LINE_STRIP );
								{
									for( auto& v : verts )
										glVertex3fv( glm::value_ptr( v ) );
								}
								glEnd();

								glPointSize( 3.0f );

								glBegin( GL_POINTS );
								{
									for( auto& v : verts )
										glVertex3fv( glm::value_ptr( v ) );
								}
								glEnd();

								glPointSize( 5.0f );
								glColor3f( 1, 1, 1 );

								glBegin( GL_POINTS );
								glVertex3fv( glm::value_ptr( verts.back() ) );
								glEnd();
							}
							*/
						}
					}
					glPopAttrib();
				}
			}
		}
		glPopMatrix();
	}
	glPopAttrib();

	static auto prevTime = std::chrono::system_clock::now();
	auto currentTime = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = currentTime - prevTime;
	prevTime = currentTime;

	double dt = diff.count();

	float mouseX_normalized = mousePos.x / windowWidth * 2.0f - 1.0f;
	float mouseY_normalized = -( mousePos.y / windowHeight * 2.0f - 1.0f );

	if( drawCrosshair )
	{
		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();

			glMatrixMode( GL_MODELVIEW );
			glLoadIdentity();

			glPushMatrix();
			{
				glEnable( GL_COLOR_MATERIAL );
				glDisable( GL_TEXTURE_2D );
				glDisable( GL_DEPTH_TEST );

				glLineWidth( 1 );
				glColor3fv( glm::value_ptr( crosshairColor ) );

				glBegin( GL_LINES );
				{
					glVertex3f( -1.0f, mouseY_normalized, 0.0f );
					glVertex3f(  1.0f, mouseY_normalized, 0.0f );

					glVertex3f( mouseX_normalized, -1.0f, 0.0f );
					glVertex3f( mouseX_normalized,  1.0f, 0.0f );
				}
				glEnd();
			}
			glPopMatrix();
		}
		glPopAttrib();
	}

	if( pattern )
	{
		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glMatrixMode( GL_PROJECTION );
			glPushMatrix();
			{
				glLoadIdentity();
				gluOrtho2D( 0, windowWidth, windowHeight, 0 );

				glEnable( GL_COLOR_MATERIAL );
				glDisable( GL_TEXTURE_2D );
				glDisable( GL_DEPTH_TEST );

				char tempStr[128];


				int x0 = 10;
				int y0 = ( uiActive ? 35 : 15 );

				glColor4f( 1, 1, 1, 1 );

				sprintf( tempStr, "%s [%s]", pattern->getName().c_str(), pattern->getSizeString().c_str() );
				printText( tempStr, x0, y0 );
				y0 += 15;

				glColor4fv( glm::value_ptr( Pattern::Color ) );

				sprintf( tempStr, "lower (x%d):", pattern->getTraceCount() );
				printText( tempStr, x0, y0 );
				y0 += 15;

				sprintf( tempStr, "  vert:   %d", (int)pattern->getTrace().getVertexCount() );
				printText( tempStr, x0, y0 );
				y0 += 15;

				sprintf( tempStr, "  stitches: %d", (int) pattern->getTrace().getStitchCount() );
				printText( tempStr, x0, y0 );
				y0 += 15;

				sprintf( tempStr, "  runlength: %.3f", pattern->getTrace().getRunLength() );
				printText( tempStr, x0, y0 );
				y0 += 15;

				glColor4fv( glm::value_ptr( DoublePattern::Color2 ) );

				sprintf( tempStr, "upper (x%d):", pattern->getTrace2Count() );
				printText( tempStr, x0, y0 );
				y0 += 15;

				sprintf( tempStr, "  vert:   %d", (int) pattern->getTrace2().getVertexCount() );
				printText( tempStr, x0, y0 );
				y0 += 15;

				sprintf( tempStr, "  stitches: %d", (int) pattern->getTrace2().getStitchCount() );
				printText( tempStr, x0, y0 );
				y0 += 15;

				sprintf( tempStr, "  runlength: %.3f", pattern->getTrace2().getRunLength() );
				printText( tempStr, x0, y0 );
				y0 += 15;


				glColor4f( 1, 1, 1, 1 );

				printText( "total:", x0, y0 );
				y0 += 15;

				sprintf( tempStr, "  stitches: %d", pattern->getTotalStitchCount() );
				printText( tempStr, x0, y0 );
				y0 += 15;

				sprintf( tempStr, "  runlength: %.3f", pattern->getTotalRunLength() );
				printText( tempStr, x0, y0 );
				y0 += 15;
			}
			glPopMatrix();
		}
		glPopAttrib();
	}

	if( uiActive )
	{
		ImGui_ImplOpenGL2_NewFrame();

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2( (float) windowWidth, (float) windowHeight );
		if( io.DeltaTime < std::numeric_limits<float>::epsilon() )
			io.DeltaTime = (float) dt;
		ImGui::NewFrame();

		if( ImGui::BeginMainMenuBar() )
		{
			if( ImGui::BeginMenu( "File" ) )
			{
				if( ImGui::MenuItem( "Save", "F2" ) )
				{
					save();
				}
				if( ImGui::MenuItem( "Exit", "Ctrl+Q" ) )
				{
					quit = true;
				}
				ImGui::EndMenu();
			}

			if( ImGui::BeginMenu( "Pattern" ) )
			{
				if( ImGui::MenuItem( "Diamond Zigzag" ) )
				{
					clearPattern();

					params = new DiamondZigZagTiled::PatternParams();
					pattern = new DiamondZigZagTiled();
					if( pattern->build( params ) )
					{
						pattern->setUnit( unit );
						statusString = "created Diamond Zigzag";
					}
					else
					{
						statusString = "ERROR: creating Diamond Zigzag failed";
						clearPattern();
					}
				}
				if( ImGui::MenuItem( "Diamond Spiral" ) )
				{
					clearPattern();

					params = new DiamondSpiralTiled::PatternParams();
					pattern = new DiamondSpiralTiled();
					if( pattern->build( params ) )
					{
						pattern->setUnit( unit );
						statusString = "created Diamond Spiral";
					}
					else
					{
						statusString = "ERROR: creating Diamond Spiral failed";
						clearPattern();
					}
				}
				if( ImGui::MenuItem( "Meander" ) )
				{
					clearPattern();

					params = new MeanderTiled::PatternParams();
					pattern = new MeanderTiled();
					if( pattern->build( params ) )
					{
						pattern->setUnit( unit );
						statusString = "created Meanderl";
					}
					else
					{
						statusString = "ERROR: creating Meander failed";
						clearPattern();
					}
				}
				if( ImGui::MenuItem( "Antenna" ) )
				{
					clearPattern();

					params = new AntennaTiled::PatternParams();
					pattern = new AntennaTiled();
					if( pattern->build( params ) )
					{
						pattern->setUnit( unit );
						statusString = "created Antenna";
					}
					else
					{
						statusString = "ERROR: creating Antenna failed";
						clearPattern();
					}
				}
				if( ImGui::MenuItem( "Flower" ) )
				{
					clearPattern();

					params = new FlowerTiled::PatternParams();
					pattern = new FlowerTiled();
					if( pattern->build( params ) )
					{
						pattern->setUnit( unit );
						statusString = "created Flower";
					}
					else
					{
						statusString = "ERROR: creating Flower failed";
						clearPattern();
					}
				}
				ImGui::Separator();
				if( ImGui::MenuItem( "Clear pattern" ) )
				{
					clearPattern();
				}
				ImGui::EndMenu();
			}

			if( ImGui::BeginMenu( "Scale" ) )
			{
				for( int i = 0; i < (int) U_COUNT; i++ )
				{
					if( i == U_IN )
						continue;	//TODO: implement inch in save() methods of patterns

					if( ImGui::RadioButton( unitToString( (Unit)i ), ( unit == i ) ) )
					{
						unit = (Unit)i;
						if( pattern )
							pattern->setUnit( unit );
					}
				}

				ImGui::EndMenu();
			}

			if( ImGui::BeginMenu( "View" ) )
			{
				if( ImGui::MenuItem( "Grid", nullptr, drawGrid ) )
					drawGrid = !drawGrid;

				float f = gridSpacing;
				{
					ScopedImGuiDisable disable( !drawGrid );
					ImGui::PushItemWidth( 100 );
					if( ImGui::InputFloat( "Grid spacing", &f, 0.01 ) )
					{
						gridSpacing = f;
						createGrids();
					}
					ImGui::PopItemWidth();
				}

				if( ImGui::MenuItem( "Reset view" ) )
					trackball = Trackball2D( defaultTrackballT, defaultTrackballR, defaultTrackballSE );

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		ImGui::Begin( "info" );
		{
			glm::vec3 worldPos( glm::inverse( trackball.mat() ) * glm::vec4( mouseX_normalized, mouseY_normalized / ar, 0.0f, 1.0f ) );

			const char *us = unitToString( unit );

			ImGui::Text( "pos" );
			ImGui::Text( "x: %.2f %s", worldPos.x, us );
			ImGui::Text( "y: %.2f %s", worldPos.y, us );

			ImGui::Text( "screen" );
			ImGui::Text( "x: %4dpx", (int)mousePos.x );
			ImGui::Text( "y: %4dpx", (int)mousePos.y );
		}
		ImGui::End();

		ImGui::Begin( "params" );
		{
			if( pattern && params )
				if( params->drawUI() )
					pattern->build( params );
		}
		ImGui::End();

		ImGui::SetNextWindowPos( ImVec2( 0, windowHeight - 25 ) );
		ImGui::SetNextWindowSize( ImVec2( windowWidth, 25 ) );
		ImGui::Begin( "##status", nullptr,
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings );

		ImGui::Text( "%s", statusString.c_str() );
		ImGui::End();

		ImGui::Render();

		ImGui_ImplOpenGL2_RenderDrawData( ImGui::GetDrawData() );
	}
	
#ifdef USE_GLUT
	glutSwapBuffers();
#else
	SDL_GL_SwapWindow( mainWindow );
#endif

	checkForGLError();
}

#ifdef USE_GLUT
void idle()
{
	if( quit )
		exit( 0 );

	update();

	glutPostRedisplay();

	GLenum err = glGetError();
	if( err != GL_NO_ERROR )
		std::cerr << "openGL error: " << glewGetErrorString( err );
}
#endif

void moved( int x, int y )
{
	windowX = x;
	windowY = y;
}

void sizeChanged( int width, int height )
{
//#if _DEBUG
//	std::cout << "sizechanged: " << width << ", " << height << std::endl;
//#endif

#ifdef USE_GLUT
	windowWidth = width;
	windowHeight = height;

	ar = (float) windowWidth / windowHeight;

	ImGui_ImplFreeGLUT_ReshapeFunc( width, height );
#endif
}

bool ctrlHandler( DWORD fdwCtrlType )
{
	switch( fdwCtrlType )
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		quit = true;
		return true;
	}

	return false;
}

void initImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void) io;

	std::string temp = getAppName() + ".imgui.ini";
	strcpy_s( imGuiIniFileName, temp.c_str() );

	io.IniFilename = imGuiIniFileName;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplFreeGLUT_Init();
	ImGui_ImplOpenGL2_Init();

	ImGuiStyle& style = ImGui::GetStyle();

	float alpha = 1.0f;
	// https://gist.github.com/dougbinks/8089b4bbaccaaf6fa204236978d165a9#file-imguiutils-h-L9-L93
	// light style from Pacome Danhiez (user itamago) https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
	style.Alpha = 1.0f;
	style.FrameRounding = 3.0f;
	style.Colors[ImGuiCol_Text] = ImVec4( 0.00f, 0.00f, 0.00f, 1.00f );
	style.Colors[ImGuiCol_TextDisabled] = ImVec4( 0.60f, 0.60f, 0.60f, 1.00f );
	style.Colors[ImGuiCol_WindowBg] = ImVec4( 0.94f, 0.94f, 0.94f, 0.94f );
	style.Colors[ImGuiCol_ChildBg] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
	style.Colors[ImGuiCol_PopupBg] = ImVec4( 1.00f, 1.00f, 1.00f, 0.94f );
	style.Colors[ImGuiCol_Border] = ImVec4( 0.00f, 0.00f, 0.00f, 0.39f );
	style.Colors[ImGuiCol_BorderShadow] = ImVec4( 1.00f, 1.00f, 1.00f, 0.10f );
	style.Colors[ImGuiCol_FrameBg] = ImVec4( 1.00f, 1.00f, 1.00f, 0.94f );
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4( 0.26f, 0.59f, 0.98f, 0.40f );
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4( 0.26f, 0.59f, 0.98f, 0.67f );
	style.Colors[ImGuiCol_TitleBg] = ImVec4( 0.96f, 0.96f, 0.96f, 1.00f );
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4( 1.00f, 1.00f, 1.00f, 0.51f );
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4( 0.82f, 0.82f, 0.82f, 1.00f );
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4( 0.86f, 0.86f, 0.86f, 1.00f );
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4( 0.98f, 0.98f, 0.98f, 0.53f );
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4( 0.69f, 0.69f, 0.69f, 1.00f );
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4( 0.59f, 0.59f, 0.59f, 1.00f );
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4( 0.49f, 0.49f, 0.49f, 1.00f );
	style.Colors[ImGuiCol_CheckMark] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
	style.Colors[ImGuiCol_SliderGrab] = ImVec4( 0.24f, 0.52f, 0.88f, 1.00f );
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
	style.Colors[ImGuiCol_Button] = ImVec4( 0.26f, 0.59f, 0.98f, 0.40f );
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
	style.Colors[ImGuiCol_ButtonActive] = ImVec4( 0.06f, 0.53f, 0.98f, 1.00f );
	style.Colors[ImGuiCol_Header] = ImVec4( 0.26f, 0.59f, 0.98f, 0.31f );
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4( 0.26f, 0.59f, 0.98f, 0.80f );
	style.Colors[ImGuiCol_HeaderActive] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4( 1.00f, 1.00f, 1.00f, 0.50f );
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4( 0.26f, 0.59f, 0.98f, 0.67f );
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4( 0.26f, 0.59f, 0.98f, 0.95f );
	style.Colors[ImGuiCol_Tab] = ImVec4( 0.26f, 0.59f, 0.98f, 0.31f );
	style.Colors[ImGuiCol_TabHovered] = ImVec4( 0.26f, 0.59f, 0.98f, 0.80f );
	style.Colors[ImGuiCol_TabActive] = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4( 0.26f, 0.59f, 0.98f, 0.31f );
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4( 0.26f, 0.59f, 0.98f, 0.80f );
	style.Colors[ImGuiCol_PlotLines] = ImVec4( 0.39f, 0.39f, 0.39f, 1.00f );
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4( 1.00f, 0.43f, 0.35f, 1.00f );
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4( 0.90f, 0.70f, 0.00f, 1.00f );
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4( 1.00f, 0.60f, 0.00f, 1.00f );
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4( 0.26f, 0.59f, 0.98f, 0.35f );
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4( 0.26f, 0.59f, 0.98f, 0.80f );
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4( 0.06f, 0.53f, 0.98f, 1.00f );
	//style.Colors[ImGuiCol_NavHighlight] = ImVec4(  );
	//style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(  );
	//style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(  );
	//style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(  );

	style.WindowRounding = 0;
	style.FrameRounding = 0;
	style.PopupRounding = 0;

	for( int i = 0; i <= ImGuiCol_COUNT; i++ )
	{
		ImVec4& col = style.Colors[i];
		if( col.w < 1.00f )
		{
			col.x *= alpha;
			col.y *= alpha;
			col.z *= alpha;
			col.w *= alpha;
		}
	}

	//have to use flag to avoid crash it shutdown, in case we did not make it until here.
	imGuiInitialized = true;
}

void initGL( int argc, char **argv )
{
#ifdef USE_GLUT

	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowPosition( windowX, windowY );
	glutInitWindowSize( windowWidth, windowHeight );
	glutWindowID = glutCreateWindow( "EmbroiderAid" );

	glutDisplayFunc( display );
	glutReshapeFunc( sizeChanged );
	glutIdleFunc( idle );
	glutMotionFunc( motion );
	glutPassiveMotionFunc( passiveMotion );
	glutMouseFunc( mouse );
	glutMouseWheelFunc( mouseWheel );
	glutKeyboardFunc( keyDown );
	glutKeyboardUpFunc( keyUp );
	glutSpecialFunc( specialDown );
	glutSpecialUpFunc( specialUp );

	GLenum glErr = glewInit();
	if( glErr != GLEW_OK )
	{
		std::stringstream sstr;
		sstr << "Error initializing glew: \"" << glewGetErrorString( glErr ) << "\"";
		throw std::runtime_error( sstr.str() );
	}

#else
	if( SDL_Init( SDL_INIT_VIDEO ) )
	{
		std::cerr << "SDL error: " << SDL_GetError();
		return;
	}

	if( TTF_Init() )
	{
		std::cerr << "TTF error: " << TTF_GetError();
		return;
	}

	// Create our window centered at 512x512 resolution
	mainWindow = SDL_CreateWindow(
		"EmbroiderAid",
		windowX, windowY,
		windowWidth, windowHeight,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);

	/*
	renderer = SDL_CreateRenderer( mainWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
	if( !renderer )
		std::cerr << "unable to create renderer" << std::endl;
		*/

	SDL_SetWindowTitle( mainWindow, "EmbroiderAid" );

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	glContext = SDL_GL_CreateContext( mainWindow );

	// This makes our buffer swap syncronized with the monitor's vertical refresh
	SDL_GL_SetSwapInterval( 1 );

	// Init GLEW
	// Apparently, this is needed for Apple. Thanks to Ross Vander for letting me know
#ifndef __APPLE__
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	/*
	font = TTF_OpenFont( "courier.ttf", 12 );
	if( !font )
		std::cerr << "could not open font: " << TTF_GetError() << std::endl;
	*/

#endif

#ifdef _WIN32
	//enable v-sync
	wglSwapIntervalEXT( 1 );
#endif

	glPointSize( 4 );
	glLineWidth( 1 );

	clearColor = glm::vec4( 0.0f, 0.1f, 0.2f, 1.0f );
	glClearColor( clearColor[0], clearColor[1], clearColor[2], clearColor[3] );

	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );
	glDisable( GL_COLOR_MATERIAL );
	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );

	initImGui();

	moved( windowX, windowY );
	sizeChanged( windowWidth, windowHeight );

	createGrids();
}

void readIni()
{
	ini = new IniFile( getAppName() + ".ini" );
	if( !ini->read() )
		std::cerr << "<warning> failed to read window.ini" << std::endl;
	else
	{
		IniFile::Section* w = ini->tryGet( "window" );
		if( w )
		{
			w->tryGet<int>( "posX", windowX );
			w->tryGet<int>( "posY", windowY );

			w->tryGet<int>( "width", windowWidth );
			w->tryGet<int>( "height", windowHeight );
		}

		IniFile::Section* c = ini->tryGet( "trackball" );
		if( c )
		{
			glm::vec3 t;
			float r = 0;
			float se = 0;
			c->tryGet<glm::vec3>( "t", t );
			//c->tryGet<float>( "r", r );
			c->tryGet<float>( "se", se );

			trackball = Trackball2D( t, r, se, true );
		}

		IniFile::Section* a = ini->tryGet( "app" );
		if( a )
		{
			a->tryGet<bool>( "grid", drawGrid );
			a->tryGet<float>( "spacing", gridSpacing );
		}
	}
}

void saveIni()
{
	if( ini )
	{
		( *ini )["window"].set( "posX", windowX );
		( *ini )["window"].set( "posY", windowY );

		( *ini )["window"].set( "width", windowWidth );
		( *ini )["window"].set( "height", windowHeight );

		( *ini )["trackball"].set( "t", trackball.getTranslation() );
		//( *ini )["trackball"].set( "r", trackball.getRotation() );
		( *ini )["trackball"].set( "se", trackball.getScaleExp() );

		( *ini )["app"].set( "grid", drawGrid );
		( *ini )["app"].set( "spacing", gridSpacing );

		if( !ini->write() )
			std::cerr << "<error> failed to write window.ini" << std::endl;
	}
}

void cleanup()
{
	if( cleaned )
		return;
	cleaned = true;

	clearPattern();

	for( auto &it : grids )
		safeDelete( it );

#ifdef USE_GLUT

	//have to use flag to keep track of ImGui init state and to avoid crash.
	if( imGuiInitialized )
	{
		ImGui_ImplOpenGL2_Shutdown();
		ImGui_ImplFreeGLUT_Shutdown();
		ImGui::DestroyContext();
	}

	glutExit();
#endif


#ifdef USE_GLUT
	glutWindowID = -1;
#else
	if( glContext )
	{
		SDL_GL_DeleteContext( glContext );
		glContext = nullptr;
	}

	if( mainWindow )
	{
		SDL_DestroyWindow( mainWindow );
		mainWindow = nullptr;
	}

	TTF_Quit();
	SDL_Quit();
#endif

	saveIni();
	safeDelete( ini );
}


void __cdecl onExit()
{
	std::cerr << "on exit called" << std::endl;

	cleanup();
}

void __cdecl onTerminate()
{
	std::cerr << "process terminated" << std::endl;
	abort();
}

int main( int argc, char **argv )
{
	atexit( onExit );
	std::set_terminate( onTerminate );

	srand( 0 );

	try
	{
		if( !SetConsoleCtrlHandler( (PHANDLER_ROUTINE) ctrlHandler, true ) )
			throw std::runtime_error( "registering ctrl-handler failed" );

		disbleQuickEditMode();

		setArg0( argv[0] );

		readIni();
		initGL( argc, argv );

#ifdef USE_GLUT
		glutMainLoop();
#else
		while( !quit )
		{
			SDL_Event event;
			while( SDL_PollEvent( &event ) )
			{
				if( event.type == SDL_QUIT )
					quit = true;

				if( event.type == SDL_WINDOWEVENT )
				{
					switch( event.window.event )
					{
					case SDL_WINDOWEVENT_SHOWN:
						break;
					case SDL_WINDOWEVENT_HIDDEN:
						break;
					case SDL_WINDOWEVENT_EXPOSED:
						break;
					case SDL_WINDOWEVENT_MOVED:
						moved( event.window.data1, event.window.data2 );
						break;
					case SDL_WINDOWEVENT_RESIZED:
						break;
					case SDL_WINDOWEVENT_SIZE_CHANGED:
						sizeChanged( event.window.data1, event.window.data2 );
						break;
					case SDL_WINDOWEVENT_MINIMIZED:
						break;
					case SDL_WINDOWEVENT_MAXIMIZED:
						break;
					case SDL_WINDOWEVENT_RESTORED:
						break;
					case SDL_WINDOWEVENT_ENTER:
						break;
					case SDL_WINDOWEVENT_LEAVE:
						break;
					case SDL_WINDOWEVENT_FOCUS_GAINED:
						break;
					case SDL_WINDOWEVENT_FOCUS_LOST:
						break;
					case SDL_WINDOWEVENT_CLOSE:
						break;
#if SDL_VERSION_ATLEAST(2, 0, 5)
					case SDL_WINDOWEVENT_TAKE_FOCUS:
						break;
					case SDL_WINDOWEVENT_HIT_TEST:
						break;
#endif
					default:
						break;
					}
				}

				if( event.type == SDL_KEYDOWN )
					keyDown( event.key.keysym.sym );
			}

			update();
			display();

			GLenum err = glGetError();
			if( err != GL_NO_ERROR )
				std::cerr << "openGL error: " << glewGetErrorString( err );
		}
#endif

		cleanup();
	}
	catch( std::exception &e )
	{
		std::cerr << "caught exception: " << e.what() << std::endl;

		cleanup();

#ifdef _DEBUG
#ifdef _WIN32
		std::cout << "press ENTER" << std::endl;
		getchar();
#endif
#endif
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
