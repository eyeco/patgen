#pragma once

#include <common.h>

namespace TextileUX
{
	class Shader
	{
		friend class Program;

	private:

		//intentionally no implemented
		//prevent from copying via copy constructor -- have to use reference counting to not end up with invalid gl shader IDs
		Shader( const Shader& );

	protected:
		std::string _name;

		bool _valid;

		GLuint _id;
		GLenum _type;

		Shader( const std::string &name, GLenum type );

	public:
		virtual ~Shader();

		bool compileSource( const char *source );
		bool compileSource( const std::string &source );
		bool compileFromFile( const std::string &path );
		void release();

		const std::string &getName() const { return _name; }
	};

	class VertexShader : public Shader
	{
	private:

	protected:

	public:
		explicit VertexShader( const std::string &name );
		virtual ~VertexShader();
	};

	class FragmentShader : public Shader
	{
	private:

	protected:

	public:
		explicit FragmentShader( const std::string &name );
		virtual ~FragmentShader();
	};
}