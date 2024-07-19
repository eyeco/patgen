#include <pattern.h>

#include <common.h>

#include <list>
#include <sstream>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>


#define MIN_JUMP_FACTOR		0.20f
#define CONNECTOR_LENGTH	0.05f

namespace TextileUX
{
	Trace::Trace( float jumpSize, const glm::vec4 &color ) :
		_color( color ),
		_jumpSize( jumpSize ),
		_runLength( 0.0f ),
		_vbPath( "path" ),
		_vbStitches( "stitches" )
	{}

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

		//for all stitches that are too close remove the earlier one
		// minimum jump size is 25% of targeted jump size
		float mj2 = _jumpSize * MIN_JUMP_FACTOR * _jumpSize * MIN_JUMP_FACTOR;
		std::vector<glm::vec3> temp;
		for( int i = 0; i < _stitches.size() - 1; i++ )
		{
			auto d = _stitches[i] - _stitches[i + 1];
			if( d.x * d.x + d.y * d.y > mj2 )
				temp.push_back( _stitches[i] );
		}
		temp.push_back( _stitches.back() );
		_stitches = temp;

		return true;
	}

	bool Trace::rebuild()
	{
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
			_vbPath.build( &verts[0], &indices[0], verts.size(), GL_LINE_STRIP, GL_UNSIGNED_SHORT, false ) &
			_vbStitches.build( &verts[0], &indices[0], verts.size(), GL_POINTS, GL_UNSIGNED_SHORT, false ) );
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



	float Pattern::Epsilon = 0.00001;
	float Pattern::DistanceTolerance = 0.001f;

	glm::vec4 Pattern::Color = glm::vec4( 1, 1, 0, 1 );


	Pattern::Pattern( const std::string &name, float jumpSize ) :
		_name( name ),
		_trace( jumpSize, Color )
	{}

	Pattern::~Pattern()
	{
		clear();
	}

	void Pattern::clear()
	{
		_trace.clear();

		_sizeString = "";
	}

	bool Pattern::rebuild()
	{
		if( !_trace.rebuild() )
			return false;

		updateSizeString();

		return validate();
	}

	bool Pattern::validate()
	{
		auto &stitches = _trace.getStitches();

		if( !stitches.size() )
			return true;

		float minDist = ( _trace.getJumpSize() * MIN_JUMP_FACTOR );
		float minDist2 = minDist * minDist;

		for( int i = 0; i < stitches.size() - 1; i++ )
		{
			glm::vec3 d( stitches[i + 1] - stitches[i] );
			float l2 = d.x * d.x + d.y * d.y;

			if( l2 < minDist2 )
				std::cerr << "WARNING: stitches #" << i << " and #" << i + 1 << " are quite close: " << sqrt( l2 ) << std::endl;
		}

		return true;
	}

	void Pattern::translate( const glm::vec3 &t )
	{
		for( glm::vec3 &v : _trace.getVerts() )
			v += t;
	}

	void Pattern::rotate( float rad )
	{
		float cosA = cos( rad );
		float sinA = sin( rad );

		for( glm::vec3 &v : _trace.getVerts() )
			v = glm::vec3( cosA * v.x - sinA * v.y, sinA * v.x + cosA * v.y, 0 );
	}

	void Pattern::rotateCW()
	{
		float cosA = 0;
		float sinA = -1;

		for( glm::vec3 &v : _trace.getVerts() )
			v = glm::vec3( v.y, -v.x, 0 );
	}

	void Pattern::rotateCCW()
	{
		for( glm::vec3 &v : _trace.getVerts() )
			v = glm::vec3( -v.y, v.x, 0 );
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
		char tempStr[128];
		sprintf( tempStr, "%s.svg", getFullName().c_str() );

		FILE *fp = fopen( tempStr, "w" );
		if( !fp )
			return false;

		float scale = 100.0f;
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




	BoustrophedonCircle::BoustrophedonCircle( float diameter, float dist, float jumpSize ) :
		Pattern( "boustrophedon-circle", jumpSize ),
		_diameter( diameter ),
		_dist( dist ),
		_rMax( 0.0f )
	{}

	BoustrophedonCircle::~BoustrophedonCircle()
	{}

	void BoustrophedonCircle::updateSizeString()
	{
		auto &stitches = _trace.getStitches();

		std::stringstream sstr;
		sstr << _rMax;

		_sizeString = sstr.str();
	}

	bool BoustrophedonCircle::build()
	{
		clear();

		std::vector<glm::vec3> temp;

		float r = _diameter / 2.0f;
		float jump = _trace.getJumpSize();

		float s = 0;
		int dir  = 1;

		//float x = 0;
		float y = 0;

		while( s < _diameter )
		{
			y = r - s;

			float h = abs( y );

			float x1 = sqrt( abs( r * r - h * h ) ) * dir;
			float x0 = -x1;

			temp.push_back( glm::vec3( x0, y, 0 ) );
			temp.push_back( glm::vec3( x1, y, 0 ) );

			//x = x0;

			/*
			while( true )
			{
				if( dir < 0 && x < x1 )
					break;
				if( dir > 0 && x > x1 )
					break;

				temp.push_back( glm::vec3( x, y, 0 ) );
				x += dir * jump;
			}

			//remove last if too close to the edge, before inserting terminating stitch at (x1,y)
			if( temp.size() && abs( temp.back().x - x1 ) < jump * MIN_JUMP_FACTOR )
				temp.pop_back();
			//add line-terminating stitch to have it exactly at the edge
			temp.push_back( glm::vec3( x1, y, 0 ) );
			*/

			s += _dist;

			if( s < _diameter )
			{
				float a0 = asin( y / r );			//alpha to current edge
				float a1 = asin( ( r - s ) / r );	//alpha to next edge
				float da = abs( a1 - a0 );			//alpha of arc leading to next hor. trace part
				float ds = da * r;					//arc length

				if( ds > jump )						//have to add jumps along circumference
				{
					float aJump = jump / r;

					float a = a0 - aJump;
					while( a > a1 )
					{
						float xx = cos( a ) * r * dir;
						float yy = sin( a ) * r;
						temp.push_back( glm::vec3( xx, yy, 0 ) );

						a -= aJump;
					}
				}
			}

			dir *= -1;
		}

		_rMax = 0;
		for( auto &p : temp )
		{
			float d2 = p.x * p.x + p.y * p.y;
			if( d2 > _rMax )
				_rMax = d2;
		}
		_rMax = sqrt( _rMax );
		
		temp.push_back( glm::vec3( 0, -( r + CONNECTOR_LENGTH ), 0 ) );

		for( auto &p : temp )
			_trace.insertBack( p );

		return rebuild();
	}

	std::string BoustrophedonCircle::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-D%.03f-d%.03f-j%.03f", getName().c_str(), _diameter, _dist, _trace.getJumpSize() );

		return std::string( tempStr );
	}





	SpiralCircle::SpiralCircle( float diameter, float dist, float jumpSize, float innerDiameter ) :
		Pattern( "spiral-circle", jumpSize ),
		_diameter( diameter ),
		_dist( dist ),
		_innerDiameter( innerDiameter )
	{}
	
	SpiralCircle::~SpiralCircle()
	{}

	void SpiralCircle::updateSizeString()
	{
		std::stringstream sstr;
		sstr << 
			glm::length( _first ) <<
			" -> " <<
			glm::length( _last );

		_sizeString = sstr.str();
	}

	bool SpiralCircle::build()
	{
		if( _innerDiameter > _diameter ) 
			return false;

		clear();

		std::list<glm::vec3> temp;


		//not a perfect spiral, but good enough...

		float r0 = _diameter / 2.0f;
		float r1 = _innerDiameter / 2.0f;
		float jump = _trace.getJumpSize();

		float a = 0;
		float r = r0;

		float x = 0;
		float y = 0;

		while( r > r1 )
		{
			x = r * cos(a);
			y = r * sin(a);

			temp.push_back( glm::vec3( x, y, 0 ) );

			float d = hypot( x, y );

			// approximate increment angle alpha for wanted segment length s with circle equation
			float b = 2.0f * asin( jump / ( 2.0f * d ) );

			a -= b;
			r -= _dist * ( b / ( 2.0f * pi() ) );
		}

		if( temp.size() )
		{
			_first = temp.front();
			_last = temp.back();
		}
		else
		{
			_first = glm::vec3();
			_last = glm::vec3();
		}

		temp.push_front( glm::vec3( r0 + CONNECTOR_LENGTH, 0, 0 ) );

		for( auto &p : temp )
			_trace.insertBack( p );

		rotateCW();

		return rebuild();
	}

	std::string SpiralCircle::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-D%.03f-d%.03f-ID%.03f-j%.03f", getName().c_str(), _diameter, _dist, _innerDiameter, _trace.getJumpSize() );

		return std::string( tempStr );
	}





	BoustrophedonQuadOrtho::BoustrophedonQuadOrtho( float width, float dist, float jumpSize ) :
		Pattern( "boustrophedon-quad-ortho", jumpSize ),
		_width( width ),
		_dist( dist )
	{}

	BoustrophedonQuadOrtho::~BoustrophedonQuadOrtho()
	{}

	void BoustrophedonQuadOrtho::updateSizeString()
	{
		auto &stitches = _trace.getStitches();

		std::stringstream sstr;
		sstr << _width << " x " << _width;

		_sizeString = sstr.str();
	}

	bool BoustrophedonQuadOrtho::build()
	{
		clear();

		std::vector<glm::vec3> temp;

		float halfWidth = _width * 0.5f;

		float x = -halfWidth;
		float y = halfWidth;

		int dir = 1;

		while( y > -halfWidth )
		{
			float x1 = halfWidth * dir;
			float x0 = -x1;

			temp.push_back( glm::vec3( x0, y, 0 ) );
			temp.push_back( glm::vec3( x1, y, 0 ) );

			dir *= -1;

			y -= _dist;
		}

		temp.push_back( glm::vec3( -dir * ( halfWidth + CONNECTOR_LENGTH ), y + _dist, 0 ) );

		for( auto &p : temp )
			_trace.insertBack( p );

		return rebuild();
	}

	std::string BoustrophedonQuadOrtho::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-W%.03f-d%.03f-j%.03f", getName().c_str(), _width, _dist, _trace.getJumpSize() );

		return std::string( tempStr );
	}




	BoustrophedonQuadDiag::BoustrophedonQuadDiag( float width, float dist, float jumpSize ) :
		Pattern( "boustrophedon-quad-diag", jumpSize ),
		_width( width ),
		_dist( dist )
	{}

	BoustrophedonQuadDiag::~BoustrophedonQuadDiag()
	{}

	void BoustrophedonQuadDiag::updateSizeString()
	{
		auto &stitches = _trace.getStitches();

		std::stringstream sstr;
		sstr << _width << " x " << _width;

		_sizeString = sstr.str();
	}

	bool BoustrophedonQuadDiag::build()
	{
		clear();

		std::vector<glm::vec3> temp;

		//float jump = _trace.getJumpSize();

		float diagonal = _width * sqrt( 2 );
		float halfDiagonal = diagonal / 2;

		int dir = 1;

		//float x = 0;
		float y = halfDiagonal;

		while( y > -halfDiagonal )
		{
			float x1 = ( halfDiagonal - abs( y ) ) * dir;
			float x0 = -x1;

			temp.push_back( glm::vec3( x0, y, 0 ) );
			temp.push_back( glm::vec3( x1, y, 0 ) );

			if( y > 0 && y - _dist < 0 )				//crossed the corner, add explicit corner stitch
				temp.push_back( glm::vec3( halfDiagonal * dir, 0, 0 ) );

			y -= _dist;
			dir *= -1;
		}

		temp.push_back( glm::vec3( 0, -halfDiagonal, 0 ) );
		temp.push_back( glm::vec3( 0, -( halfDiagonal + CONNECTOR_LENGTH ), 0 ) );
		
		for( auto &p : temp )
			_trace.insertBack( p );

		rotate( toRad( 45 ) );

		return rebuild();
	}

	std::string BoustrophedonQuadDiag::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-W%.03f-d%.03f-j%.03f", getName().c_str(), _width, _dist, _trace.getJumpSize() );

		return std::string( tempStr );
	}


	/*
	//--- taken from Wikipedia ---
	void rot( int n, int *x, int *y, int rx, int ry ) {
		if( ry == 0 )
		{
			if( rx == 1 )
			{
				*x = n - 1 - *x;
				*y = n - 1 - *y;
			}

			int t = *x;
			*x = *y;
			*y = t;
		}
	}

	void d2xy( int n, int d, int *x, int *y )
	{
		int rx, ry, s, t = d;
		*x = *y = 0;
		for( s = 1; s < n; s *= 2 )
		{
			rx = 1 & ( t / 2 );
			ry = 1 & ( t ^ rx );
			rot( s, x, y, rx, ry );
			*x += s * rx;
			*y += s * ry;
			t /= 4;
		}
	}
	//-----------------------
	*/
}