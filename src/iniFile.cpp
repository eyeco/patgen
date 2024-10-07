/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of patgen
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <iniFile.h>

#include <common.h>

#include <iostream>
#include <filesystem>

#include <glm/gtc/type_ptr.hpp>

namespace TextileUX
{
	void IniFile::Section::write( std::fstream &file )
	{
		file << "[" << name << "]" << std::endl;

		for( auto it = entries.begin(); it != entries.end(); ++it )
			file << it->first << "=" << it->second << std::endl;

		file << std::endl;
	}

	IniFile::IniFile( const std::string &filename ) :
		filename( filename )
	{}

	IniFile::~IniFile()
	{
		for( auto it = sections.begin(); it != sections.end(); ++it )
			safeDelete( it->second );
		sections.clear();
	}

	bool IniFile::read()
	{
		using namespace std::filesystem;

		if( !exists( this->filename ) )
		{
			//file not yet created -- not an error though
			std::cerr << "<warning> file " << filename << " does not exists (yet?)" << std::endl;
			return true;
		}

		std::fstream file( this->filename, std::ios::in );
		if( !file.is_open() )
		{
			std::cerr << "<error> could not open file " << filename << " for reading" << std::endl;
			return false;
		}

		int cntr = 0;
		std::string line;
		Section *currentSection = nullptr;
		for( std::string line; std::getline( file, line ); cntr++ )
		{
			trim( line );

			if( line.empty() )
				continue;

			if( line.front() == '[' )
			{
				if( line.back() != ']' || line.size() < 3 )
				{
					std::cerr << "<error> parsing error in file " << filename << " line #" << cntr << std::endl;
					return false;
				}

				std::string name( line.begin() + 1, line.end() - 1 );
				currentSection = new Section( name );
				sections.insert( std::make_pair( name, currentSection ) );

				continue;
			}
			
			if( !currentSection )
			{
				std::cerr << "<error> parsing error in file " << filename << " line #" << cntr << " (no section specified)" << std::endl;
				return false;
			}

			std::vector<std::string> subs = split( line, '=' );
			if( subs.size() != 2 )
			{
				std::cerr << "<error> parsing error in file " << filename << " line #" << cntr << std::endl;
				return false;
			}

			std::string &key = subs[0];
			std::string &value = subs[1];

			currentSection->set( key, value );
		}

		return true;
	}

	bool IniFile::write()
	{
		std::fstream file( this->filename, std::ios::out | std::ios::trunc );
		if( !file.is_open() )
		{
			std::cerr << "<error> could not open file " << filename << " for writing" << std::endl;
			return false;
		}

		for( auto it = sections.begin(); it != sections.end(); ++it )
			it->second->write( file );

		return true;
	}

	template<>
	std::string IniFile::Section::convert<std::string>( const std::string& s ) const
	{
		return s;
	}

	template<>
	bool IniFile::Section::convert<bool>( const std::string& s ) const
	{
		std::string str( s );
		trim( str );

		if( !_stricmp( s.c_str(), "false" ) || !_stricmp( s.c_str(), "0" ) || !_stricmp( s.c_str(), "low" ) )
			return false;
		else if( !_stricmp( s.c_str(), "true" ) || !_stricmp( s.c_str(), "1" ) || !_stricmp( s.c_str(), "high" ) )
			return true;

		throw std::runtime_error( "could not parse to bool value" );
	}

	template<>
	glm::vec3 IniFile::Section::get( const std::string& name ) const
	{
		glm::vec3 v;

		auto it = entries.find( name );
		if( it == entries.end() )
			return v;

		std::string str( it->second );
		trim( str );

		/*
		size_t pos1 = str.find_first_of('[');
		size_t pos2 = str.find_last_of(']', pos1);

		if( pos1 == std::string::npos || pos2 == std::string::npos )
			return v;

		str = str.substr( pos1 + 1, pos2 - pos1 - 1 );
		*/

		std::vector<std::string> values = split( str, ',' );
		if( values.size() < 3 )
			return v;

		float* f = glm::value_ptr( v );
		for( int i = 0; i < 3; i++ )
			*( f++ ) = atof( values[i].c_str() );

		return v;
	}

	template<>
	bool IniFile::Section::tryGet<glm::vec3>( const std::string& name, glm::vec3& v ) const
	{
		auto it = entries.find( name );
		if( it == entries.end() )
			return false;

		std::string str( it->second );
		trim( str );

		/*
		size_t pos1 = str.find_first_of( '[' );
		size_t pos2 = str.find_last_of( ']', pos1 );

		if( pos1 == std::string::npos || pos2 == std::string::npos )
			return false;

		str = str.substr( pos1 + 1, pos2 - pos1 - 1 );
		*/

		std::vector<std::string> values = split( str, ',' );
		if( values.size() < 3 )
			return false;

		float* f = glm::value_ptr( v );
		for( int i = 0; i < 3; i++ )
			*( f++ ) = atof( values[i].c_str() );

		return true;
	}

	template<>
	void IniFile::Section::set<glm::vec3>( const std::string& name, const glm::vec3& v )
	{
		const float* f = glm::value_ptr( v );
		std::stringstream sstr;
		for( int i = 0; i < 3; i++ )
			sstr << *( f++ ) << ",";

		entries[name] = sstr.str();
	}
}