#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include <gl/glew.h>

#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <glm/glm.hpp>

#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#include <GL/freeglut.h>

#include <vld.h>

namespace ImGui
{
	enum ImGuiPlotType
	{
		ImGuiPlotType_Lines,
		ImGuiPlotType_Histogram
	};

	void PlotMultiLines(
		const char* label,
		int num_datas,
		const float * const *values,
		int values_count,
		const char ** names,
		const ImColor* colors,
		float scale_min = FLT_MIN,
		float scale_max = FLT_MAX,
		ImVec2 graph_size = ImVec2( 0, 0 ) );

	void PlotMultiHistograms(
		const char* label,
		int num_hists,
		const float * const *values,
		int values_count,
		const char ** names,
		const ImColor* colors,
		float scale_min = FLT_MIN,
		float scale_max = FLT_MAX,
		ImVec2 graph_size = ImVec2( 0, 0 ) );
}

namespace TextileUX
{
	enum Unit
	{
		U_MM,
		U_CM,
		U_M,

		U_COUNT
	};

	template<typename T>
	inline void safeDelete( T* &ptr )
	{
		if( ptr )
		{
			delete ptr;
			ptr = nullptr;
		}
	}

	template<typename T>
	inline void safeDeleteArray( T* &ptr )
	{
		if( ptr )
		{
			delete [] ptr;
			ptr = nullptr;
		}
	}

	//custom min function since std::min produced slow code in vc++
	// http://randomascii.wordpress.com/2013/11/24/stdmin-causing-three-times-slowdown-on-vc/
	template<typename T>
	inline const T min( const T left, const T right )
	{
		return ( right < left ? right : left );
	}

	//custom max function since std::max produced slow code in vc++
	// http://randomascii.wordpress.com/2013/11/24/stdmin-causing-three-times-slowdown-on-vc/
	template<typename T>
	inline const T max( const T left, const T right )
	{
		return ( left < right ? right : left );
	}

	template<typename T>
	inline T clamp( T value, T minValue, T maxValue )
	{
		return TextileUX::min( TextileUX::max( value, minValue ), maxValue );
	}

	template<typename T>
	inline T clamp01( T value )
	{
		return TextileUX::min( TextileUX::max( value, (T)0 ), (T)1 );
	}

	inline const glm::vec3 &zero()
	{
		static const glm::vec3 z( 0 );
		return z;
	}

	inline const glm::vec3 &one()
	{
		static const glm::vec3 o( 1 );
		return o;
	}

	inline const glm::vec3 &unitX()
	{
		static const glm::vec3 x( 1, 0, 0 );
		return x;
	}

	inline const glm::vec3 &unitY()
	{
		static const glm::vec3 y( 0, 1, 0 );
		return y;
	}

	inline const glm::vec3 &unitZ()
	{
		static const glm::vec3 z( 0, 0, 1 );
		return z;
	}

	double getGlobalTime();

	std::vector<std::string> split( const std::string &s, char delimiter );

	void trim( std::string &s );
	void trimLeft( std::string &s );
	void trimRight( std::string &s );

	void toLower( std::string &s );
	void toUpper( std::string &s );

	bool endsWith( const std::string &s, const std::string &end );

	template<size_t S>
	void makeDateTimeString( char( &str )[S], time_t time, bool fileSafe = false )
	{
		struct tm *timeinfo;
		timeinfo = localtime( &time );

		sprintf_s( str, ( fileSafe ? "%d-%02d-%02d %02d-%02d-%02d" : "%d-%02d-%02d %02d:%02d:%02d" ),
			timeinfo->tm_year + 1900,
			timeinfo->tm_mon + 1,
			timeinfo->tm_mday,
			timeinfo->tm_hour,
			timeinfo->tm_min,
			timeinfo->tm_sec );
	}

	template<size_t S>
	void makeDateTimeString( char( &str )[S], bool fileSafe = false )
	{
		time_t rawtime;

		::time( &rawtime );

		makeDateTimeString( str, rawtime, fileSafe );
	}

	inline std::string	getDateTimeString( bool fileSafe = false )
	{
		char tempStr[32];
		makeDateTimeString( tempStr, fileSafe );
		return std::string( tempStr );
	}

	template<typename T> size_t arraySize( const T& t )
	{
		return sizeof( t ) / sizeof( *t );
	}

	inline unsigned int nextPo2( unsigned int x )
	{
		unsigned int power = 1;
		while( power < x && power )
			power <<= 1;

		if( !power )
			throw std::runtime_error( "nextpo2 overflow" );

		return power;
	}

	inline ImVec2 glm2im( const glm::ivec2 &v )
	{
		return ImVec2( (float)v.x, (float) v.y );
	}

	inline ImVec4 glm2im( const glm::ivec4 &v )
	{
		return ImVec4( (float)v.x, (float)v.y, (float)v.z, (float)v.w );
	}

	inline ImVec2 glm2im( const glm::vec2 &v )
	{
		return ImVec2( v.x, v.y );
	}

	inline ImVec4 glm2im( const glm::vec4 &v )
	{
		return ImVec4( v.x, v.y, v.z, v.w );
	}

	inline glm::vec2 im2glm( const ImVec2 &v )
	{
		return glm::vec2( v.x, v.y );
	}

	inline glm::vec4 im2glm( const ImVec4 &v )
	{
		return glm::vec4( v.x, v.y, v.z, v.w );
	}

	inline long double pi()
	{
		return 3.141592653589793238462643383279502884L;
	}

	inline long double pi2()
	{
		return 2 * 3.141592653589793238462643383279502884L;
	}

	inline double toRad( double deg )
	{
		static double s = pi2() / 360.0;
		return deg * s;
	}

	inline double toDeg( double rad )
	{
		static double s = 360.0 / pi2();
		return rad * s;
	}

	bool checkForGLError();

	class ScopedGlAttribs
	{
	public:
		ScopedGlAttribs( GLbitfield bits )
		{
			glPushAttrib( bits );
		}

		~ScopedGlAttribs()
		{
			glPopAttrib();
		}
	};

	class ScopedImGuiDisable
	{
	private:
		bool _disabled;

	public:
		explicit ScopedImGuiDisable( bool disable = true ) :
			_disabled( false )
		{
			if( disable )
				this->disable();
		}

		~ScopedImGuiDisable()
		{
			enable();
		}

		void disable()
		{
			if( !_disabled )
			{
				ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
				ImGui::PushStyleVar( ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f );

				_disabled = true;
			}
		}

		void enable()
		{
			if( _disabled )
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();

				_disabled = false;
			}
		}
	};

	class Line
	{
	private:
		glm::vec3 _p0;
		glm::vec3 _dir;

		Line( const glm::vec3 &p0, const glm::vec3 &dir ) :
			_p0( p0 ),
			_dir( dir )
		{}

	public:
		float normaldist( const glm::vec3 &p ) const
		{
			return glm::length( normaldistVec( p ) );
		}

		glm::vec3 normaldistVec( const glm::vec3 &p ) const
		{
			glm::vec3 d( _p0 - p );
			return ( glm::dot( d, _dir ) * _dir ) - d;
		}

		static Line fromDir( const glm::vec3 &p0, const glm::vec3 &dir )
		{
			return Line( p0, glm::normalize( dir ) );
		}

		static Line fromPoints( const glm::vec3 &p0, const glm::vec3 &p1 )
		{
			return Line( p0, glm::normalize( p1 - p0 ) );
		}
	};

	class Path
	{
	private:
		glm::vec3 _p0;
		glm::vec3 _p1;
		glm::vec3 _dir;

		float _l;

	public:
		Path( const glm::vec3 &p0, const glm::vec3 &p1 ) :
			_p0( p0 ),
			_p1( p1 ),
			_dir( glm::normalize( p1 - p0 ) ),
			_l( glm::length( p1 - p0 ) )
		{}

		bool hittest( const glm::vec3 &p, float tolerance = 0.00001f )
		{
			float tolerance2 = tolerance * tolerance;

			glm::vec3 d0( p - _p0 );
			glm::vec3 d1( p - _p1 );

			if( glm::dot( d0, d0 ) < tolerance2 || glm::dot( d1, d1 ) < tolerance2 )
				return true;

			float u = glm::dot( d0, _dir );
			if( u < 0 || u > _l )
				return false;

			glm::vec3 n( d0 - ( glm::dot( d0, _dir ) * _dir ) );
			return ( glm::dot( n, n ) < tolerance2 );
		}

		const glm::vec3 &getP0() const { return  _p0; }
		const glm::vec3 &getP1() const { return  _p1; }
		const glm::vec3 &getDir() const { return  _dir; }
	};

#ifdef _WIN32
	std::string formatWinError( DWORD err );
	std::string formatLastWinError();

	void disbleQuickEditMode();
#endif

	void printText( const char *string, float x, float y, void *font = GLUT_BITMAP_HELVETICA_12 );
}

std::ostream &operator << ( std::ostream &ostr, const glm::vec2 &v );
std::ostream &operator << ( std::ostream &ostr, const glm::vec3 &v );
std::ostream &operator << ( std::ostream &ostr, const glm::vec4 &v );
