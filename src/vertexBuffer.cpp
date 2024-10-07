/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of patgen
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <vertexBuffer.h>

#include <program.h>

#include <iostream>

namespace TextileUX
{
	VertexBuffer::VertexBuffer( const std::string &name ) :
		_name( name ),
		_valid( false ),
		_vboID( ~0x00 ),
		_iboID( ~0x00 ),
		_vertexCount( 0 ),
		_vertexElementsMask( VE_POSITION | VE_COLOR | VE_UV ),
		_primitiveType( GL_NONE ),
		_indexDataType( GL_NONE ),
		_usage( GL_NONE )
	{}

	VertexBuffer::~VertexBuffer()
	{
		this->destroy();
	}

	bool VertexBuffer::build( const Vertex *vertices, unsigned short *indices, size_t size, GLenum primitiveType, GLenum indexDataType, bool dynamic )
	{
		if( _valid )
			destroy();

		if( !size )
		{
			std::cerr << "<error> vertex count must not be 0" << std::endl;
			return false;
		}

		_vertexCount = size;
		_primitiveType = primitiveType;
		_indexDataType = indexDataType;
		_usage = ( dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW );

		glGenBuffers( 1, &_vboID );
		glBindBuffer( GL_ARRAY_BUFFER, _vboID );
		glBufferData( GL_ARRAY_BUFFER, size * sizeof( Vertex ), vertices, _usage );

		glGenBuffers( 1, &_iboID );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, _iboID );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, size * sizeof( unsigned short ), indices, _usage );

		// bind with 0, so, switch back to normal pointer operation
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

		_valid = true;

		return true;
	}

	void VertexBuffer::destroy()
	{
		if( _valid )
		{
			glDeleteBuffers( 1, &_vboID );
			_vboID = ~0x00;

			glDeleteBuffers( 1, &_iboID );
			_iboID = ~0x00;

			_valid = false;
		}
	}

	bool VertexBuffer::activate( Program *program )
	{
		if( _valid )
		{
			glBindBuffer( GL_ARRAY_BUFFER, _vboID );
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, _iboID );

			int stride = VertexBuffer::getVertexStride();
			int offsetPosition = VertexBuffer::getPositionOffset();
			int offsetColor = VertexBuffer::getColorOffset();
			int offsetUV = VertexBuffer::getUVOffset();

			if( program )
			{
				GLint loc = -1;

				if( _vertexElementsMask & VE_POSITION )
				{
					loc = program->getAttribute( "inPosition" );
					if( loc >= 0 )
					{
						glEnableVertexAttribArray( loc );
						glVertexAttribPointer( loc, 3, GL_FLOAT, GL_FALSE, stride, (void*) ( ( char* )nullptr + offsetPosition ) );
					}
				}

				if( _vertexElementsMask & VE_COLOR )
				{
					loc = program->getAttribute( "inColor" );
					if( loc >= 0 )
					{
						glEnableVertexAttribArray( loc );
						glVertexAttribPointer( loc, 4, GL_FLOAT, GL_FALSE, stride, (void*) ( ( char* )nullptr + offsetColor ) );
					}
				}

				if( _vertexElementsMask & VE_UV )
				{
					loc = program->getAttribute( "inUV" );
					if( loc >= 0 )
					{
						glEnableVertexAttribArray( loc );
						glVertexAttribPointer( loc, 2, GL_FLOAT, GL_FALSE, stride, (void*) ( ( char* )nullptr + offsetUV ) );
					}
				}
			}
			else
			{
				if( _vertexElementsMask & VE_POSITION )
				{
					glEnableClientState( GL_VERTEX_ARRAY );
					glVertexPointer( 3, GL_FLOAT, stride, (void*) ( ( char* )nullptr + offsetPosition ) );
				}
				if( _vertexElementsMask & VE_NORMAL )
				{
					//TODO
					std::cerr << "<error> normals not supported yet" << std::endl;
				}
				if( _vertexElementsMask & VE_COLOR )
				{
					glEnableClientState( GL_COLOR_ARRAY );
					glColorPointer( 4, GL_FLOAT, stride, (void*) ( ( char* )nullptr + offsetColor ) );
				}
				if( _vertexElementsMask & VE_UV )
				{
					glEnableClientState( GL_TEXTURE_COORD_ARRAY );
					glTexCoordPointer( 2, GL_FLOAT, stride, (void*) ( ( char* )nullptr + offsetUV ) );
				}
			}
		}

		return _valid;
	}

	bool VertexBuffer::deactivate( Program *program )
	{
		if( _valid )
		{
			if( program )
			{
				GLint loc = -1;

				if( _vertexElementsMask & VE_POSITION )
				{
					loc = program->getAttribute( "inPosition" );
					if( loc >= 0 )
						glDisableVertexAttribArray( loc );
				}

				if( _vertexElementsMask & VE_NORMAL )
				{
					//TODO
					std::cerr << "<error> normals not supported yet" << std::endl;
				}

				if( _vertexElementsMask & VE_COLOR )
				{
					loc = program->getAttribute( "inColor" );
					if( loc >= 0 )
						glDisableVertexAttribArray( loc );
				}

				if( _vertexElementsMask & VE_UV )
				{
					loc = program->getAttribute( "inUV" );
					if( loc >= 0 )
						glDisableVertexAttribArray( loc );
				}
			}
			else
			{
				if( _vertexElementsMask & VE_POSITION )
					glDisableClientState( GL_VERTEX_ARRAY );
				if( _vertexElementsMask & VE_NORMAL )
				{
					//TODO
					std::cerr << "<error> normals not supported yet" << std::endl;
				}
				if( _vertexElementsMask & VE_COLOR )
					glDisableClientState( GL_COLOR_ARRAY );
				if( _vertexElementsMask & VE_UV )
					glDisableClientState( GL_TEXTURE_COORD_ARRAY );
			}

			// bind with 0, so, switch back to normal pointer operation
			glBindBuffer( GL_ARRAY_BUFFER, 0 );
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
		}

		return _valid;
	}

	void VertexBuffer::draw( Program *program )
	{
		if( _valid )
		{
			activate( program );

			glDrawElements( _primitiveType, _vertexCount, _indexDataType, nullptr );

			deactivate( program );
		}
	}



	VertexBuffer *VertexBuffer::createAxis( float scale )
	{
		const int vertCount = 6;

		Vertex vertices[vertCount];
		vertices[0] = Vertex( glm::vec3( 0, 0, 0 ), glm::vec4( 1, 0, 0, 1 ), glm::vec2( 0, 0 ) );
		vertices[1] = Vertex( glm::vec3( scale, 0, 0 ), glm::vec4( 1, 0, 0, 1 ), glm::vec2( 0, 0 ) );
		vertices[2] = Vertex( glm::vec3( 0, 0, 0 ), glm::vec4( 0, 1, 0, 1 ), glm::vec2( 0, 0 ) );
		vertices[3] = Vertex( glm::vec3( 0, scale, 0 ), glm::vec4( 0, 1, 0, 1 ), glm::vec2( 0, 0 ) );
		vertices[4] = Vertex( glm::vec3( 0, 0, 0 ), glm::vec4( 0, 0, 1, 1 ), glm::vec2( 0, 0 ) );
		vertices[5] = Vertex( glm::vec3( 0, 0, scale ), glm::vec4( 0, 0, 1, 1 ), glm::vec2( 0, 0 ) );

		unsigned short indices[vertCount];
		for( int i = 0; i < vertCount; i++ )
			indices[i] = i;

		VertexBuffer *vb = new VertexBuffer( "axis" );
		vb->build( vertices, indices, vertCount, GL_LINES, GL_UNSIGNED_SHORT, false );

		return vb;
	}

	VertexBuffer *VertexBuffer::createLocator( const glm::vec3 &color, float scale )
	{
		const int vertCount = 6;

		glm::vec4 c( color.rgb, 1 );

		Vertex vertices[vertCount];
		vertices[0] = Vertex( glm::vec3( -scale, 0, 0 ), c, glm::vec2( 0, 0 ) );
		vertices[1] = Vertex( glm::vec3( scale, 0, 0 ), c, glm::vec2( 0, 0 ) );
		vertices[2] = Vertex( glm::vec3( 0, -scale, 0 ), c, glm::vec2( 0, 0 ) );
		vertices[3] = Vertex( glm::vec3( 0, scale, 0 ), c, glm::vec2( 0, 0 ) );
		vertices[4] = Vertex( glm::vec3( 0, 0, -scale ), c, glm::vec2( 0, 0 ) );
		vertices[5] = Vertex( glm::vec3( 0, 0, scale ), c, glm::vec2( 0, 0 ) );

		unsigned short indices[vertCount];
		for( int i = 0; i < vertCount; i++ )
			indices[i] = i;

		VertexBuffer *vb = new VertexBuffer( "locator" );
		vb->build( vertices, indices, vertCount, GL_LINES, GL_UNSIGNED_SHORT, false );

		return vb;
	}

	VertexBuffer *VertexBuffer::createQuad( const glm::vec2 &size, const glm::vec3 &center )
	{
		const int vertCount = 4;

		Vertex vertices[vertCount];
		vertices[0] = Vertex( center + glm::vec3( -size.x * 0.5f, -size.y * 0.5f, 0 ), glm::vec4( 1 ), glm::vec2( 0, 1 ) );
		vertices[1] = Vertex( center + glm::vec3( size.x * 0.5f, -size.y * 0.5f, 0 ), glm::vec4( 1 ), glm::vec2( 1, 1 ) );
		vertices[2] = Vertex( center + glm::vec3( size.x * 0.5f, size.y * 0.5f, 0 ), glm::vec4( 1 ), glm::vec2( 1, 0 ) );
		vertices[3] = Vertex( center + glm::vec3( -size.x * 0.5f, size.y * 0.5f, 0 ), glm::vec4( 1 ), glm::vec2( 0, 0 ) );

		unsigned short indices[vertCount];
		for( int i = 0; i < vertCount; i++ )
			indices[i] = i;

		VertexBuffer *vb = new VertexBuffer( "quad" );
		vb->build( vertices, indices, vertCount, GL_QUADS, GL_UNSIGNED_SHORT, false );

		return vb;
	}

	VertexBuffer *VertexBuffer::createGrid( const glm::vec2 &size, unsigned int subdivisions, const glm::vec3 &color )
	{
		const int vertCount = ( subdivisions + 2 ) * 4;

		glm::vec4 c( color.rgb, 1.0f );

		std::vector<Vertex> vertices( vertCount );
		for( int i = 0; i <= subdivisions + 1; i++ )
		{
			float t = i / ( subdivisions + 1.0f ) - 0.5f;

			//hor line
			vertices[i * 4 + 0] = Vertex( glm::vec3( size.x * 0.5f, size.y * t, 0.0f ), c, glm::vec2( 0, 0 ) );
			vertices[i * 4 + 1] = Vertex( glm::vec3( -size.x * 0.5f, size.y * t, 0.0f ), c, glm::vec2( 0, 0 ) );

			//ver line
			vertices[i * 4 + 2] = Vertex( glm::vec3( size.x * t, -size.y * 0.5f, 0.0f ), c, glm::vec2( 0, 0 ) );
			vertices[i * 4 + 3] = Vertex( glm::vec3( size.x * t, size.y * 0.5f, 0.0f ), c, glm::vec2( 0, 0 ) );
		}

		std::vector<unsigned short> indices( vertCount );
		for( int i = 0; i < vertCount; i++ )
			indices[i] = i;

		VertexBuffer *vb = new VertexBuffer( "grid" );
		vb->build( &vertices[0], &indices[0], vertCount, GL_LINES, GL_UNSIGNED_SHORT, false );

		return vb;
	}
}