/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of patgen
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#pragma once

#include <common.h>

#include <map>
#include <string>
#include <sstream>
#include <fstream>

namespace patgen
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

	template<> std::string IniFile::Section::convert<std::string>( const std::string& s ) const;
	template<> bool IniFile::Section::convert<bool>( const std::string& s ) const;
	template<> glm::vec3 IniFile::Section::get( const std::string& name ) const;
	template<> bool IniFile::Section::tryGet<glm::vec3>( const std::string& name, glm::vec3& m ) const;
	template<> void IniFile::Section::set<glm::vec3>( const std::string& name, const glm::vec3& m );
}
