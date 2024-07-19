#pragma once

#include <common.h>

#include <map>
#include <string>
#include <sstream>
#include <fstream>

namespace TextileUX
{
	class IniFile
	{
	public:
		class Section
		{
			friend class IniFile;

		private:
			std::string name;
			std::map<std::string, std::string> entries;

			void write( std::fstream &file );

			template<typename T>
			T convert( const std::string &s ) const
			{
				std::stringstream sstr( s );

				T value = 0;
				sstr >> value;

				return value;
			}

			template<>
			std::string convert<std::string>( const std::string &s ) const
			{
				return s;
			}

			template<>
			bool convert<bool>( const std::string &s ) const
			{
				std::string str( s );
				trim( str );
				
				if( !_stricmp( s.c_str(), "false" ) || !_stricmp( s.c_str(), "0" ) || !_stricmp( s.c_str(), "low" ) )
					return false;
				else if( !_stricmp( s.c_str(), "true" ) || !_stricmp( s.c_str(), "1" ) || !_stricmp( s.c_str(), "high" ) )
					return true;

				throw std::runtime_error( "could not parse to bool value" );
			}

		public:
			explicit Section( const std::string &name ) :
				name( name )
			{}

			template<typename T>
			T get( const std::string &name ) const
			{
				auto it = entries.find( name );
				if( it == entries.end() )
					return 0;

				return convert<T>( it->second );
			}

			template<typename T>
			bool tryGet( const std::string &name, T &t ) const
			{
				auto it = entries.find( name );
				if( it == entries.end() )
					return false;

				t = convert<T>( it->second );
				return true;
			}

			template<typename T>
			void set( const std::string &name, const T &t )
			{
				std::stringstream sstr;
				sstr << t;

				entries[name] = sstr.str();
			}
		};

	private:
		std::string filename;
		std::map<std::string, IniFile::Section*> sections;

	public:
		explicit IniFile( const std::string &filename );
		~IniFile();

		bool read();
		bool write();

		IniFile::Section *get( const std::string &name )
		{
			auto it = sections.find( name );
			if( it == sections.end() )
				it = sections.insert( std::make_pair( name, new IniFile::Section( name ) ) ).first;
			return it->second;
		}

		IniFile::Section *tryGet( const std::string &name ) const
		{
			auto it = sections.find( name );
			if( it == sections.end() )
				return nullptr;
			return it->second;
		}

		IniFile::Section &operator[]( const std::string &name )
		{
			return *this->get( name );
		}
	};
}