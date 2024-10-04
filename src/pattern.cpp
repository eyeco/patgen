#include <pattern.h>

#include <common.h>

#include <sstream>
#include <iostream>

#define MIN_JUMP_FACTOR		0.20f

namespace TextileUX
{
	Trace::Trace( const glm::vec4 &color ) :
		_color( color ),
		_jumpSize( 0 ),
		_minJumpSize( 0 ),
		_runLength( 0.0f ),
		_vbPath( "path" ),
		_vbStitches( "stitches" )
	{}

	Trace::Trace( const Trace& t ) :
		_color( t._color ),
		_verts( t._verts ),
		_stitches( t._stitches ),
		_jumpSize( t._jumpSize ),
		_minJumpSize( t._minJumpSize ),
		_runLength( t._runLength ),
		_vbPath( "path" ),
		_vbStitches( "stitches" )
	{
		rebuildVBOs();
	}

	void Trace::clear()
	{
		_verts.clear();
		_stitches.clear();

		_runLength = 0.0f;

		_vbPath.destroy();
		_vbStitches.destroy();
	}

	bool Trace::resample()
	{
		if( !_verts.size() || _jumpSize < Pattern::Epsilon )
			return false;

		_stitches.clear();

		if( _verts.size() > 1 )
		{
			for( int i = 0; i < _verts.size() - 1; i++ )
			{
				glm::vec3 v = _verts[i];
				_stitches.push_back( v );

				glm::vec3 d = _verts[i + 1] - v;
				float l = glm::length( d );

				glm::vec3 jump = glm::normalize( d ) * _jumpSize;
				
				l -= _jumpSize;
				while( l > Pattern::Epsilon )
				{
					v += jump;
					_stitches.push_back( v );

					l -= _jumpSize;
				}
			}

			_stitches.push_back( _verts.back() );
		}
		else
			_stitches = _verts;

		/*
		//for all stitches that are too close remove the earlier one
		// minimum jump size is 25% of targeted jump size
		float mj2 = _minJumpSize * _minJumpSize;
		std::vector<glm::vec3> temp;
		for( int i = 0; i < _stitches.size() - 1; i++ )
		{
			auto d = _stitches[i] - _stitches[i + 1];
			if( d.x * d.x + d.y * d.y > mj2 )
				temp.push_back( _stitches[i] );
		}
		temp.push_back( _stitches.back() );
		_stitches = temp;
		*/

		return true;
	}

	bool Trace::rebuild( float jumpSize, float minJumpSize )
	{
		_jumpSize = jumpSize;
		_minJumpSize = minJumpSize < 0.00000001 ? jumpSize * MIN_JUMP_FACTOR : minJumpSize;

		if( !resample() )
			return false;

		if( !_stitches.size() )
			return false;

		return rebuildVBOs();
	}

	bool Trace::rebuildVBOs()
	{
		std::vector<VertexBuffer::Vertex> verts;
		for( auto &s : _stitches )
			verts.push_back( VertexBuffer::Vertex( s, _color, glm::vec2() ) );

		std::vector<unsigned short> indices;
		indices.clear();
		for( int i = 0; i < _stitches.size(); i++ )
			indices.push_back( i );

		return(
			_vbPath.build( &verts[0], &indices[0], verts.size(), GL_LINE_STRIP, GL_UNSIGNED_SHORT, false ) &&
			_vbStitches.build( &verts[0], &indices[0], verts.size(), GL_POINTS, GL_UNSIGNED_SHORT, false ) );
	}

	bool Trace::validate()
	{
		if( !_stitches.size() )
			return true;

		float minDist = ( _jumpSize * MIN_JUMP_FACTOR );
		float minDist2 = minDist * minDist;

		for( int i = 0; i < _stitches.size() - 1; i++ )
		{
			glm::vec3 d( _stitches[i + 1] - _stitches[i] );
			float l2 = d.x * d.x + d.y * d.y;

			if( l2 < minDist2 )
				std::cerr << "WARNING: stitches #" << i << " and #" << i + 1 << " are quite close: " << sqrt( l2 ) << std::endl;
		}

		return true;
	}

	void Trace::draw()
	{
		_vbPath.draw();
		_vbStitches.draw();
	}

	void Trace::insertBack( const glm::vec3 &v )
	{
		if( _verts.size() )
			_runLength += glm::distance( v, _verts.back() );
		_verts.push_back( v );
	}

	void Trace::insertFront( const glm::vec3 &v )
	{
		if( _verts.size() )
			_runLength += glm::distance( v, _verts.front() );
		_verts.insert( _verts.begin(), v );
	}

	bool Trace::removeStitch( int index )
	{
		if( index >= _stitches.size() )
			return false;

		_stitches.erase( _stitches.begin() + index );

		return rebuildVBOs();
	}

	bool Trace::removeStitches( std::vector<int> indices )
	{
		if( !indices.size() )
			return true;

		//check for and remove duplicate indices
		std::sort( indices.begin(), indices.end() );
		indices.erase( unique( indices.begin(), indices.end() ), indices.end() );

		for( auto &idx : indices )
		{
			if( idx >= _stitches.size() )
				return false;

			_stitches.erase( _stitches.begin() + idx );
		}

		return rebuildVBOs();
	}

	void Trace::translate( const glm::vec3& t )
	{
		for( glm::vec3& v : _verts )
			v += t;
	}

	void Trace::rotate( float rad )
	{
		float cosA = cos( rad );
		float sinA = sin( rad );

		for( glm::vec3& v : _verts )
			v = glm::vec3( cosA * v.x - sinA * v.y, sinA * v.x + cosA * v.y, 0 );
	}

	void Trace::rotate90CW()
	{
		for( glm::vec3& v : _verts )
			v = glm::vec3( v.y, -v.x, 0 );
	}

	void Trace::rotate90CCW()
	{
		for( glm::vec3& v : _verts )
			v = glm::vec3( -v.y, v.x, 0 );
	}

	void Trace::rotate180()
	{
		for( glm::vec3& v : _verts )
			v = glm::vec3( -v.x, -v.y, 0 );
	}



	bool PatternParamsBase::drawUI()
	{
		if( ImGui::SliderFloat( "jump size", &_jumpSize, 0.1f, 30.0f ) )
			_invalidated = true;
		if( ImGui::SliderFloat( "min jump size", &_minJumpSize, 0.0f, 1.0f ) )
			_invalidated = true;
		if( ImGui::SliderFloat( "trace dist", &_dist, 0.1, 10.0f ) )
			_invalidated = true;

		ScopedImGuiDisable disable( !_invalidated );

		if( ImGui::Button( "rebuild" ) )
		{
			_invalidated = false;
			return true;
		}

		return false;
	}


	float Pattern::Epsilon = 0.00001f;
	float Pattern::DistanceTolerance = 0.001f;

	glm::vec4 Pattern::Color = glm::vec4( 1, 1, 0, 1 );


	Pattern::Pattern( const std::string &name ) :
		_name( name ),
		_trace( Color ),
		_unit( U_COUNT )
	{}

	Pattern::~Pattern()
	{}

	void Pattern::clear()
	{
		_trace.clear();

		_sizeString = "";
	}

	bool Pattern::build( const PatternParamsBase *params )
	{
		if( !params )
			return false;

		if( !_trace.rebuild( params->_jumpSize, params->_minJumpSize ) )
			return false;

		updateSizeString();

		return validate();
	}

	bool Pattern::validate()
	{
		return _trace.validate();
	}

	void Pattern::translate( const glm::vec3 &t )
	{
		_trace.translate( t );
	}

	void Pattern::rotate( float rad )
	{
		_trace.rotate( rad );
	}

	void Pattern::rotate90CW()
	{
		_trace.rotate90CW();
	}

	void Pattern::rotate90CCW()
	{
		_trace.rotate90CCW();
	}

	void Pattern::rotate180()
	{
		_trace.rotate180();
	}

	void Pattern::draw()
	{
		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glLineWidth( 1 );
			glPointSize( 3 );

			_trace.draw();
		}
		glPopAttrib();
	}

	bool Pattern::save()
	{
		float scale = 0.0f;

		switch( _unit )
		{
		case U_M:
			scale = 1000;
			break;
		case U_CM:
			scale = 10;
			break;
		case U_MM:
			scale = 1;
			break;
		default:
			//TODO: implement inch (also set reasonable reference frame)
			std::cerr << "ERROR: unit not implemented" << std::endl;
			return false;
		}

		char tempStr[128];
		sprintf( tempStr, "%s.svg", getFullName().c_str() );

		FILE *fp = fopen( tempStr, "w" );
		if( !fp )
			return false;

		//embroidery frame size is 30x20 cm
		int canvasWidth = 300;
		int canvasHeight = 200;
		float canvasCenterX = canvasWidth / 2.0f;
		float canvasCenterY = canvasHeight / 2.0f;

		fprintf( fp, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n" );
		fprintf( fp, 
			"<svg xmlns=\"http://www.w3.org/2000/svg\"\n"
			"  xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
			"  version=\"1.1\" baseProfile=\"full\"\n"
			"  width=\"%dmm\" height=\"%dmm\"\n"
			"  viewBox=\"%d %d %d %d\">\n"
			"  <title>%s</title>\n"
			"  <desc></desc>\n", canvasWidth, canvasHeight, 0, 0, canvasWidth, canvasHeight, _name.c_str() );

		auto &stitches = _trace.getStitches();
		if( stitches.size() )
		{
			std::stringstream sstr;
			sstr.precision( 5 );
			for( int i = 0; i < stitches.size(); i++ )
			{
				float x = canvasCenterX + stitches[i].x * scale;
				float y = canvasCenterY - stitches[i].y * scale;

				sstr << ( i ? " " : "" ) << x << "," << y;
			}

			fprintf( fp, "  <polyline points=\"%s\" stroke=\"#%02X%02X%02X\" stroke-width=\"1\" fill=\"none\" />\n", sstr.str().c_str(),
				(int) ( clamp01( Color.r ) * 255 ),
				(int) ( clamp01( Color.g ) * 255 ),
				(int) ( clamp01( Color.b ) * 255 )
			);
		}

		fprintf( fp, "</svg>\n" );

		fclose( fp );

		return true;
	}
}