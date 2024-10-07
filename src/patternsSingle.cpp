/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of patgen
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <patternsSingle.h>

#include <common.h>

#include <list>
#include <sstream>

#define CONNECTOR_LENGTH	0.01f

namespace TextileUX
{
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
		std::stringstream sstr;
		sstr << "r: " << _rMax;

		_sizeString = sstr.str();
	}

	void BoustrophedonCircle::clear()
	{
		Pattern::clear();

		_rMax = 0.0f;
	}

	bool BoustrophedonCircle::build( const PatternParamsBase* params )
	{
		clear();

		const BoustrophedonCircle::PatternParams* p = dynamic_cast<const BoustrophedonCircle::PatternParams*>( params );
		if( !p )
			return false;

		_diameter = p->_diameter;
		_dist = p->_dist;

		std::list<glm::vec3> temp;

		float r = _diameter / 2.0f;
		float jump = p->_jumpSize;

		float s = 0;
		int dir = 1;

		float y = 0;

		while( s < _diameter )
		{
			y = r - s;

			float h = abs( y );

			float x1 = sqrt( abs( r * r - h * h ) ) * dir;
			float x0 = -x1;

			temp.push_back( glm::vec3( x0, y, 0 ) );
			temp.push_back( glm::vec3( x1, y, 0 ) );

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
		for( auto& p : temp )
		{
			float d2 = p.x * p.x + p.y * p.y;
			if( d2 > _rMax )
				_rMax = d2;
		}
		_rMax = sqrt( _rMax );

		temp.push_front( glm::vec3( 0, r + CONNECTOR_LENGTH, 0 ) );

		for( auto& p : temp )
			_trace.insertBack( p );

		rotate180();

		return Pattern::build( params );
	}

	std::string BoustrophedonCircle::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-D%.03f-d%.03f-j%.03f", getName().c_str(), _diameter, _dist, _trace.getJumpSize() );

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
		_innerJumpSize( 0.0f ),
		_first( 0 ),
		_last( 0 )
	{}

	SpiralCircle::~SpiralCircle()
	{}

	void SpiralCircle::updateSizeString()
	{
		std::stringstream sstr;
		sstr <<
			"ri: " << glm::length( _first ) <<
			", ro: " << glm::length( _last );

		_sizeString = sstr.str();
	}

	void SpiralCircle::clear()
	{
		Pattern::clear();

		_first = zero();
		_last = zero();
	}

	bool SpiralCircle::build( const PatternParamsBase* params )
	{
		clear();

		const SpiralCircle::PatternParams* p = dynamic_cast<const SpiralCircle::PatternParams*>( params );
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
			x = r * cos( a );
			y = r * sin( a );

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

		for( auto& p : temp )
			_trace.insertBack( p );

		rotate90CW();

		return Pattern::build( params );
	}

	std::string SpiralCircle::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-D%.03f-ID%.03f-d%.03f-j[%.03f-i%.03f]", getName().c_str(), _diameter, _innerDiameter, _dist, _trace.getJumpSize(), _innerJumpSize );

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
		auto& stitches = _trace.getStitches();

		std::stringstream sstr;
		sstr << "w: " << _width;

		_sizeString = sstr.str();
	}

	void BoustrophedonQuadOrtho::clear()
	{
		Pattern::clear();

		_width = 0.0f;
	}

	bool BoustrophedonQuadOrtho::build( const PatternParamsBase* params )
	{
		clear();

		const BoustrophedonQuadOrtho::PatternParams* p = dynamic_cast<const BoustrophedonQuadOrtho::PatternParams*>( params );
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

		for( auto& p : temp )
			_trace.insertBack( p );

		return Pattern::build( params );
	}

	std::string BoustrophedonQuadOrtho::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-W%.03f-d%.03f-j%.03f", getName().c_str(), _width, _dist, _trace.getJumpSize() );

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
		auto& stitches = _trace.getStitches();

		std::stringstream sstr;
		sstr << "w: " << _width;

		_sizeString = sstr.str();
	}

	void BoustrophedonQuadDiag::clear()
	{
		Pattern::clear();

		_width = 0.0f;
	}

	bool BoustrophedonQuadDiag::build( const PatternParamsBase* params )
	{
		clear();

		const BoustrophedonQuadDiag::PatternParams* p = dynamic_cast<const BoustrophedonQuadDiag::PatternParams*>( params );
		if( !p )
			return false;

		_width = p->_width;
		_dist = p->_dist;

		std::vector<glm::vec3> temp;

		float diagonal = _width * sqrt( 2 );
		float halfDiagonal = diagonal / 2;

		int dir = 1;

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

		for( auto& p : temp )
			_trace.insertBack( p );

		rotate( toRad( 45 ) );

		return Pattern::build( params );
	}

	std::string BoustrophedonQuadDiag::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-W%.03f-d%.03f-j%.03f", getName().c_str(), _width, _dist, _trace.getJumpSize() );

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
		if( ImGui::SliderFloat( "trace dist", &_dist, 0.1f, 10.0f ) )
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
		auto& stitches = _trace.getStitches();

		std::stringstream sstr;
		sstr << "w: " << _width;

		_sizeString = sstr.str();
	}

	void BoustrophedonQuadDouble::clear()
	{
		Pattern::clear();

		_width = 0;
	}

	bool BoustrophedonQuadDouble::build( const PatternParamsBase* params )
	{
		clear();

		const BoustrophedonQuadDouble::PatternParams* p = dynamic_cast<const BoustrophedonQuadDouble::PatternParams*>( params );
		if( !p )
			return false;

		_width = p->_width;
		_dist = p->_dist;

		std::vector<glm::vec3> temp;

		float halfWidth = _width * 0.5f;
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

		for( auto& p : temp )
			_trace.insertBack( p );

		return Pattern::build( params );
	}

	std::string BoustrophedonQuadDouble::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-W%.03f-d%.03f-j%.03f", getName().c_str(), _width, _dist, _trace.getJumpSize() );

		return std::string( tempStr );
	}
}