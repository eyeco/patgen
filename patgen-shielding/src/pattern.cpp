#include <pattern.h>

#include <common.h>

#include <list>
#include <sstream>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#define MIN_JUMP_FACTOR		0.20f
#define CONNECTOR_LENGTH	0.01f

namespace TextileUX
{
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


	Trace::Trace( const glm::vec4 &color ) :
		_color( color ),
		_jumpSize( 0 ),
		_minJumpSize( 0 ),
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
	{
		clear();
	}

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

	void Pattern::rotate90CW()
	{
		for( glm::vec3 &v : _trace.getVerts() )
			v = glm::vec3( v.y, -v.x, 0 );
	}

	void Pattern::rotate90CCW()
	{
		for( glm::vec3 &v : _trace.getVerts() )
			v = glm::vec3( -v.y, v.x, 0 );
	}

	void Pattern::rotate180()
	{
		for( glm::vec3 &v : _trace.getVerts() )
			v = glm::vec3( -v.x, -v.y, 0 );
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
			std::cerr << "ERROR: unit not set" << std::endl;
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


	bool BoustrophedonCircle::PatternParams::drawUI()
	{
		if( ImGui::SliderFloat( "diameter", &_diameter, 1.0f, 100.0f ) )
			_invalidated = true;

		return PatternParamsBase::drawUI();
	}

	BoustrophedonCircle::BoustrophedonCircle() :
		Pattern( "boustrophedon-circle" ),
		_diameter( 0.0f ),
		_dist( 0.0f ),
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

	void BoustrophedonCircle::clear()
	{
		Pattern::clear();
	}

	bool BoustrophedonCircle::build( const PatternParamsBase *params )
	{
		clear();

		const BoustrophedonCircle::PatternParams *p = dynamic_cast<const BoustrophedonCircle::PatternParams*>( params );
		if( !p )
			return false;

		_diameter = p->_diameter;
		_dist = p->_dist;

		std::list<glm::vec3> temp;

		float r = _diameter / 2.0f;
		float jump = p->_jumpSize;

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

		//TODO: add final arc to reach p(0,-r)

		_rMax = 0;
		for( auto &p : temp )
		{
			float d2 = p.x * p.x + p.y * p.y;
			if( d2 > _rMax )
				_rMax = d2;
		}
		_rMax = sqrt( _rMax );
		
		temp.push_front( glm::vec3( 0, r + CONNECTOR_LENGTH, 0 ) );

		for( auto &p : temp )
			_trace.insertBack( p );

		rotate180();

		return Pattern::build( params );
	}

	std::string BoustrophedonCircle::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-D%.03f-d%.04f-j%.03f", getName().c_str(), _diameter, _dist, _trace.getJumpSize() );

		return std::string( tempStr );
	}





	bool SpiralCircle::PatternParams::drawUI()
	{
		if( ImGui::SliderFloat( "diameter", &_diameter, 1.0f, 100.0f ) )
			_invalidated = true;
		if( ImGui::SliderFloat( "inner diameter", &_innerDiameter, 0.0f, 50.0f ) )
			_invalidated = true;
		if( ImGui::SliderFloat( "inner jump size", &_innerJumpSize, 0.1f, 100.0f ) )
			_invalidated = true;

		if( _innerDiameter > _diameter )
			_innerDiameter = _diameter;
		if( _innerJumpSize > _jumpSize )
			_innerJumpSize = _jumpSize;

		return PatternParamsBase::drawUI();
	}

	SpiralCircle::SpiralCircle() :
		Pattern( "spiral-circle" ),
		_diameter( 0.0f ),
		_dist( 0.0f ),
		_innerDiameter( 0.0f ),
		_innerJumpSize( 0.0f )
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

	void SpiralCircle::clear()
	{
		Pattern::clear();
	}

	bool SpiralCircle::build( const PatternParamsBase *params )
	{
		clear();

		const SpiralCircle::PatternParams *p = dynamic_cast<const SpiralCircle::PatternParams*>( params );
		if( !p )
			return false;

		_diameter = p->_diameter;
		_dist = p->_dist;
		_innerDiameter = p->_innerDiameter;
		_innerJumpSize = p->_innerJumpSize;

		if( _innerDiameter > _diameter ) 
			return false;

		std::list<glm::vec3> temp;

		//not a perfect spiral, but good enough...

		float r0 = _diameter / 2.0f;
		float r1 = _innerDiameter / 2.0f;
		float j0 = p->_jumpSize;
		float j1 = _innerJumpSize;
		float jump = 0;

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

			float p = ( r - r1 ) / ( r0 - r1 );	//progress [0 1], moving from outer to inner radius
			jump = j1 * ( 1 - p ) + j0 * p;		//linear interpolation of jump size based on progress

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

		rotate90CW();

		return Pattern::build( params );
	}

	std::string SpiralCircle::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-D%.03f-d%.04f-ID%.03f-j[%.03f-%.03f]", getName().c_str(), _diameter, _dist, _innerDiameter, _trace.getJumpSize(), _innerJumpSize );

		return std::string( tempStr );
	}





	bool BoustrophedonQuadOrtho::PatternParams::drawUI()
	{
		if( ImGui::SliderFloat( "width", &_width, 1.0f, 100.0f ) )
			_invalidated = true;

		return PatternParamsBase::drawUI();
	}

	BoustrophedonQuadOrtho::BoustrophedonQuadOrtho() :
		Pattern( "boustrophedon-quad-ortho" ),
		_width( 0.0f ),
		_dist( 0.0f )
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

	void BoustrophedonQuadOrtho::clear()
	{
		Pattern::clear();
	}

	bool BoustrophedonQuadOrtho::build( const PatternParamsBase *params )
	{
		clear();

		const BoustrophedonQuadOrtho::PatternParams *p = dynamic_cast<const BoustrophedonQuadOrtho::PatternParams*>( params );
		if( !p )
			return false;

		_width = p->_width;
		_dist = p->_dist;

		std::vector<glm::vec3> temp;

		float halfWidth = _width * 0.5f;

		float x = -halfWidth;
		float y = halfWidth;

		int dir = 1;

		int lines = round( _width / _dist ) + 1;

		for( int i = 0; i < lines; i++ )
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

		return Pattern::build( params );
	}

	std::string BoustrophedonQuadOrtho::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-W%.03f-d%.04f-j%.03f", getName().c_str(), _width, _dist, _trace.getJumpSize() );

		return std::string( tempStr );
	}




	bool BoustrophedonQuadDiag::PatternParams::drawUI()
	{
		if( ImGui::SliderFloat( "width", &_width, 1.0f, 100.0f ) )
			_invalidated = true;

		return PatternParamsBase::drawUI();
	}

	BoustrophedonQuadDiag::BoustrophedonQuadDiag() :
		Pattern( "boustrophedon-quad-diag" ),
		_width( 0.0f ),
		_dist( 0.0f )
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

	void BoustrophedonQuadDiag::clear()
	{
		Pattern::clear();
	}

	bool BoustrophedonQuadDiag::build( const PatternParamsBase *params )
	{
		clear();

		const BoustrophedonQuadDiag::PatternParams *p = dynamic_cast<const BoustrophedonQuadDiag::PatternParams*>( params );
		if( !p )
			return false;

		_width = p->_width;
		_dist = p->_dist;

		std::vector<glm::vec3> temp;

		//float jump = p->_jumpSize;

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

		return Pattern::build( params );
	}

	std::string BoustrophedonQuadDiag::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-W%.03f-d%.04f-j%.03f", getName().c_str(), _width, _dist, _trace.getJumpSize() );

		return std::string( tempStr );
	}





	bool BoustrophedonQuadDouble::PatternParams::drawUI()
	{
		if( ImGui::SliderFloat( "width", &_width, 1.0f, 100.0f ) )
			_invalidated = true;

		if( ImGui::SliderInt( "jump multiplier", &_jumpMult, 1, 10 ) )
		{
			_invalidated = true;
			_jumpSize = _jumpMult * _dist;
		}

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

	BoustrophedonQuadDouble::BoustrophedonQuadDouble() :
		Pattern( "boustrophedon-quad-double" ),
		_width( 0.0f ),
		_dist( 0.0f )
	{}

	BoustrophedonQuadDouble::~BoustrophedonQuadDouble()
	{}

	void BoustrophedonQuadDouble::updateSizeString()
	{
		auto &stitches = _trace.getStitches();

		std::stringstream sstr;
		sstr << _width << " x " << _width;

		_sizeString = sstr.str();
	}

	void BoustrophedonQuadDouble::clear()
	{
		Pattern::clear();
	}

	bool BoustrophedonQuadDouble::build( const PatternParamsBase *params )
	{
		clear();

		const BoustrophedonQuadDouble::PatternParams *p = dynamic_cast<const BoustrophedonQuadDouble::PatternParams*>( params );
		if( !p )
			return false;

		_width = p->_width;
		_dist = p->_dist;

		std::vector<glm::vec3> temp;

		float halfWidth = _width * 0.5f;

		//float x = -halfWidth;
		float y = halfWidth - _dist * 0.5f;

		int dir = 1;

		int lines = round( _width / _dist );

		for( int i = 0; i < lines; i++ )
		{
			float x1 = halfWidth * dir;
			float x0 = -x1;

			temp.push_back( glm::vec3( x0, y, 0 ) );
			temp.push_back( glm::vec3( x1, y, 0 ) );

			dir *= -1;

			y -= _dist;
		}

		int xDir = dir;
		float x = -( halfWidth - _dist * 0.5f ) * xDir;
		//y = -halfWidth;

		dir = 1;

		for( int i = 0; i < lines; i++ )
		{
			float y1 = halfWidth * dir;
			float y0 = -y1;

			temp.push_back( glm::vec3( x, y0, 0 ) );
			temp.push_back( glm::vec3( x, y1, 0 ) );

			dir *= -1;

			x += _dist * xDir;
		}

		temp.push_back( glm::vec3( x - _dist * xDir, -( halfWidth + CONNECTOR_LENGTH ) * dir, 0 ) );

		for( auto &p : temp )
			_trace.insertBack( p );

		return Pattern::build( params );
	}

	std::string BoustrophedonQuadDouble::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-W%.03f-d%.04f-j%.03f", getName().c_str(), _width, _dist, _trace.getJumpSize() );

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