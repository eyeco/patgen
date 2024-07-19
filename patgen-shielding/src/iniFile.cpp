#include <iniFile.h>

#include <common.h>

#include <iostream>
#include <experimental/filesystem>

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
		using namespace std::experimental::filesystem::v1;

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
}