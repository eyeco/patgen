#include <mutex>
#include <thread>

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <experimental/filesystem>

#include <conio.h>

#include <cxxopts.hpp>

#include <common.h>
#include <iniFile.h>

#include <pattern.h>
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

std::vector<VertexBuffer*> grids( 3 );

Pattern *pattern = nullptr;
int stitchProgress = 0;

Trackball2D trackball( zero(), 0.0f, -2.5 );

namespace TextileUX
{
	extern double globalTime;

	struct StartupConfig
	{
	private:
		template<typename T>
		static std::string toString( const T &t )
		{
			std::stringstream sstr;
			sstr << t;

			return sstr.str();
		}

		template<>
		static std::string toString<bool>( const bool &t )
		{
			return std::string( t ? "true" : "false" );
		}

	public:
		StartupConfig()
		{}

		static bool readPA( int argc, char **argv, StartupConfig &cfg )
		{
			//generic options: 
			cxxopts::Options options( argv[0], " command line options" );
			options
				.positional_help( "[optional args]" )
				.show_positional_help();

			options.add_options()
				//TODO: incorporate version, run RCStamp tool as pre build step and generate include file with version number constants
				//http://www.codeproject.com/KB/dotnet/build_versioning.aspx
				//		( "version,v",										"print version string" )
				( "h,help", "print help" )
				;

			//positional options:

			try
			{
				cxxopts::ParseResult result = options.parse( argc, argv );

				//generic options
				if( result.count( "help" ) )
				{
					std::cout << options.help( { 
						""//, 
						//"OSC" 
					} ) << std::endl;

					return false;
				}

			}
			catch( std::exception &e )
			{
				std::cerr << e.what() << std::endl;

				std::stringstream sstr;
				sstr << std::endl << "An error occured, processing program options: " << std::endl
					<< e.what() << std::endl;

				std::cout << options.help();

				throw std::runtime_error( sstr.str().c_str() );
			}

			return true;
		}
	};
}

StartupConfig cfg;

bool quit = false;
bool cleaned = false;

bool uiActive = true;

//stupid ImGui is stupid. have to use flag.
bool imGuiInitialized = false;


void displayQuad( float left, float bottom, float right, float top )
{
	std::vector<GLfloat> vertices( 8 );

	vertices[0] = left;
	vertices[1] = bottom;

	vertices[2] = right;
	vertices[3] = bottom;

	vertices[4] = right;
	vertices[5] = top;

	vertices[6] = left;
	vertices[7] = top;

	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2, GL_FLOAT, 0, &vertices[0] );

	glDrawArrays( GL_TRIANGLE_FAN, 0, 8 );

	// deactivate vertex arrays after drawing
	glDisableClientState( GL_VERTEX_ARRAY );
}

void displayGraph( const std::vector<float> &values, unsigned int maxValues )//, float bottom, float top )
{
	if( values.size() )
	{
		std::vector<GLfloat> vertices( values.size() * 2 );

		for( int i = 0; i < values.size(); i++ )
		{
			vertices[i * 2 + 0] = (float) i / ( maxValues - 1.0f );
			vertices[i * 2 + 1] = values[i];
		}

		glEnableClientState( GL_VERTEX_ARRAY );
		glVertexPointer( 2, GL_FLOAT, 0, &vertices[0] );

		glDrawArrays( GL_LINE_STRIP, 0, (GLsizei)values.size() );
		//glDrawArrays( GL_POINTS, 0, values.size() );

		// deactivate vertex arrays after drawing
		glDisableClientState( GL_VERTEX_ARRAY );
	}
}

void motion( int x, int y )
{
#ifdef USE_GLUT
	if( uiActive )
		ImGui_ImplFreeGLUT_MotionFunc( x, y );
#endif

	trackball.onMotion( x, y );
}

void passiveMotion( int x, int y )
{
#ifdef USE_GLUT
	if( uiActive )
		ImGui_ImplFreeGLUT_MotionFunc( x, y );
#endif

	trackball.onPassiveMotion( x, y );
}

void mouse( int button, int state, int x, int y )
{
#ifdef USE_GLUT
	if( uiActive )
		ImGui_ImplFreeGLUT_MouseFunc( button, state, x, y );
#endif

	trackball.onMouse( button, state, x, y );
}

void mouseWheel( int wheel, int direction, int x, int y )
{
#ifdef USE_GLUT
	if( uiActive )
		ImGui_ImplFreeGLUT_MouseWheelFunc( wheel, direction, x, y );
#endif

	trackball.onMouseWheel( wheel, direction, x, y );
}

#ifdef USE_GLUT
void keyDown( unsigned char key, int x, int y )
#else
void keyDown( int key )
#endif
{
	if( trackball.onKeyDown( key, x, y ) )
		return;

#ifdef USE_GLUT
	if( uiActive )
		ImGui_ImplFreeGLUT_KeyboardFunc( key, x, y );

	switch( key )
	{
	case ' ':
		//uiActive = !uiActive;
		//std::cout << ( uiActive ? "activated" : "deactivated" ) << " UI" << std::endl;
		break;
	case 13:
		stitchProgress = 0;
		break;
	case 27:
		quit = true;
		break;
	}
#else
	switch( key )
	{
	case SDLK_SPACE:
		if( !cfg.playbackMode )
		{
			//TODO: fix recording (after adding COM input)
			/*
			if( isRecording )
				stopRecording();
			else
				startRecording();
			*/
		}
		break;
	case SDLK_PLUS:
	case SDLK_KP_PLUS:
		break;
	case SDLK_MINUS:
	case SDLK_KP_MINUS:
		break;
	case SDLK_ESCAPE:
		quit = true;
		break;
	}
#endif
}

void keyUp( unsigned char key, int x, int y )
{
	if( trackball.onKeyUp( key, x, y ) )
		return;

#ifdef USE_GLUT
	if( uiActive )
		ImGui_ImplFreeGLUT_KeyboardUpFunc( key, x, y );
#endif
}

void specialDown( int key, int x, int y )
{
	if( trackball.onSpecialDown( key, x, y ) )
		return;

#ifdef USE_GLUT
	if( uiActive )
		ImGui_ImplFreeGLUT_SpecialFunc( key, x, y );

	switch( key )
	{
	case GLUT_KEY_F2:
		if( pattern )
		{
			if( pattern->save() )
				std::cout << "successfully saved pattern to disc" << std::endl;
			else
				std::cerr << "failed to save pattern to disc" << std::endl;
		}
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
	if( trackball.onSpecialUp( key, x, y ) )
		return;

#ifdef USE_GLUT
	if( uiActive )
		ImGui_ImplFreeGLUT_SpecialUpFunc( key, x, y );
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

	//TODO: update your shit
}

void display()
{
#ifdef USE_GLUT

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

			for( int i = 0; i < grids.size(); i++ )
			{
				glLineWidth( grids.size() - i );
				grids[i]->draw();
			}

			//glScalef( 0.1, 0.1, 0.1 );
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
							std::vector<glm::vec3> verts( stitchProgress );
							std::copy( pattern->getTrace().getStitches().begin(), pattern->getTrace().getStitches().begin() + stitchProgress, verts.begin() );

							glColor3f( 1, 0, 0 );
							glLineWidth( 2.0f );

							glBegin( GL_LINE_STRIP );
							{
								for( auto &v : verts )
									glVertex3fv( glm::value_ptr( v ) );
							}
							glEnd();

							glPointSize( 3.0f );

							glBegin( GL_POINTS );
							{
								for( auto &v : verts )
									glVertex3fv( glm::value_ptr( v ) );
							}
							glEnd();

							glPointSize( 5.0f );
							glColor3f( 1, 1, 1 );

							glBegin( GL_POINTS );
							glVertex3fv( glm::value_ptr( verts.back() ) );
							glEnd();
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

	if( uiActive )
	{
		ImGui_ImplOpenGL2_NewFrame();

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2( (float) windowWidth, (float) windowHeight );
		if( io.DeltaTime < std::numeric_limits<float>::epsilon() )
			io.DeltaTime = (float) dt;
		ImGui::NewFrame();

		//TODO: draw GUI here

		ImGui::Render();

		ImGui_ImplOpenGL2_RenderDrawData( ImGui::GetDrawData() );
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

				glColor4fv( glm::value_ptr( Pattern::Color ) );

				int x0 = 10;
				int y0 = 15;


				sprintf( tempStr, "%s [%s]", pattern->getName().c_str(), pattern->getSizeString().c_str() );
				printText( tempStr, x0, y0 );
				y0 += 15;

				printText( "trace:", x0, y0 );
				y0 += 15;

				sprintf( tempStr, "  vert:   %d", pattern->getTrace().getVertexCount() );
				printText( tempStr, x0, y0 );
				y0 += 15;

				sprintf( tempStr, "  stitches: %d", pattern->getTrace().getStitchCount() );
				printText( tempStr, x0, y0 );
				y0 += 15;

				sprintf( tempStr, "  runlength: %.3f", pattern->getTrace().getRunLength() );
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
	std::cout << "moved to: " << x << ", " << y << std::endl;
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

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void) io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplFreeGLUT_Init();
	ImGui_ImplOpenGL2_Init();

	//stupid ImGui is stupid. have to use flag so it won't crash at shutdown when we did not make it until here.
	imGuiInitialized = true;


	moved( windowX, windowY );
	sizeChanged( windowWidth, windowHeight );

	for( int i = 0; i < grids.size(); i++ )
		grids[i] = VertexBuffer::createGrid( glm::vec2( 10.0f ), ( 10 * pow( 2, ( i ) ) ) - 1, glm::vec3( 0.2f, 0.3f, 0.5f ) * 0.5f );
}

void cleanup()
{
	if( cleaned )
		return;
	cleaned = true;

	safeDelete( pattern );

	for( auto &it : grids )
		safeDelete( it );

#ifdef USE_GLUT

	//stupid ImGui is stupid. have to use flag to avoid potential crash.
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
		
		if( StartupConfig::readPA( argc, argv, cfg ) )
		{
			initGL( argc, argv );

			//pattern = new SpiralCircle( 0.018f, 0.0005f, 0.004f, 0.002f, 0.003f );
			//pattern->build();
			//pattern->save();
			//safeDelete( pattern );

			pattern = new BoustrophedonCircle( 0.018f, 0.0005f, 0.005f, 0.0001f );
			pattern->build();
			//pattern->save();
			//safeDelete( pattern );

			//pattern = new BoustrophedonQuadOrtho( 0.004f, 0.0005f, 0.005f, 0.00001 );
			//pattern->build();
			//pattern->save();
			//safeDelete( pattern );

			//pattern = new BoustrophedonQuadDiag( 0.036f, 0.002f, 0.005f, 0.00001 );
			//pattern->build();
			//pattern->save();
			//safeDelete( pattern );

			//pattern = new BoustrophedonQuadDouble( 0.036f, 0.002f, 1, 0.00001 );
			////pattern = new BoustrophedonQuadDouble( 0.036f, 0.001f, 2, 0.00001 );
			//pattern->build();
			//pattern->save();
			//safeDelete( pattern );

			/*float w[] = { 0.006f, 0.012f, 0.018f };
			float dist[] = { 0.0005f, 0.001f, 0.002f };
			for( int i = 0; i < 3; i++ )
			{
				pattern = new BoustrophedonQuadOrtho( w[i], dist[1], 0.005f, 0.00001 );
				pattern->build();
				pattern->save();
				safeDelete( pattern );
			}
			for( int i = 0; i < 3; i++ )
			{
				pattern = new BoustrophedonQuadOrtho( w[1], dist[i], 0.005f, 0.00001 );
				pattern->build();
				pattern->save();
				safeDelete( pattern );
			}

			return EXIT_SUCCESS;*/

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
