#include <patternsDouble.h>

#include <sstream>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

//--- taken from Wikipedia ---
void rot( int n, int* x, int* y, int rx, int ry ) {
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

void d2xy( int n, int d, int* x, int* y )
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

namespace TextileUX
{
	glm::vec4 DoublePattern::Color2 = glm::vec4( 0, 1, 1, 1 );


	DoublePattern::DoublePattern( const std::string& name ) : 
		Pattern( name ),
		_width( 0.0f ),
		_trace2( Color2 )
	{}

	DoublePattern::~DoublePattern()
	{}

	void DoublePattern::clear()
	{
		Pattern::clear();

		_width = 0.0f;

		_trace2.clear();
		_shortcuts.clear();
	}

	bool DoublePattern::validate()
	{
		return( _trace2.validate() && Pattern::validate() );
	}

	bool DoublePattern::correct()
	{
		auto& lower = _trace.getStitches();
		auto& upper = _trace2.getStitches();

		if( !lower.size() || !upper.size() )
			return true;

		std::vector<int> rem;
		for( int j = 0; j < upper.size(); j++ )
		{
			auto& u = upper[j];

			for( int i = 0; i < lower.size() - 1; i++ )
			{
				Path p = Path( lower[i], lower[i + 1] );
				if( p.hittest( u, DistanceTolerance ) )
				{
					rem.push_back( j );
					break;
				}
			}
		}

		size_t temp = upper.size();

		if( !_trace2.removeStitches( rem ) )
		{
			std::cerr << "<error> removing shorting stitches failed" << std::endl;
			return false;
		}

		std::cout << "successfully removed " << ( temp - upper.size() ) << " stitch(es) to prevent from shorting" << std::endl;

		bool ret = findShortcuts();
		ret &= validate();

		return ret;
	}

	void DoublePattern::draw()
	{
		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glLineWidth( 1 );
			glPointSize( 3 );

			_trace.draw();
			_trace2.draw();

			if( _shortcuts.size() )
			{
				glDisable( GL_TEXTURE_2D );
				glDisable( GL_LIGHTING );
				glEnable( GL_COLOR_MATERIAL );

				glLineWidth( 2 );

				for( auto& s : _shortcuts )
				{
					glColor3f( 1, 0, 0 );
					glBegin( GL_LINES );
					{
						glVertex3fv( glm::value_ptr( s + glm::vec3( -0.25f, -0.25f, 0 ) ) );
						glVertex3fv( glm::value_ptr( s + glm::vec3( 0.25f, 0.25f, 0 ) ) );
						glVertex3fv( glm::value_ptr( s + glm::vec3( -0.25f, 0.25f, 0 ) ) );
						glVertex3fv( glm::value_ptr( s + glm::vec3( 0.25f, -0.25f, 0 ) ) );
					}
					glEnd();
				}
			}
		}
		glPopAttrib();
	}

	bool DoublePattern::build( const PatternParamsBase* params )
	{
		_shortcuts.clear();

		if( !params )
			return false;

		if( !_trace2.rebuild( params->_jumpSize, params->_minJumpSize ) )
			return false;

		if( !Pattern::build( params ) )
			return false;

		findShortcuts();

		return true;
	}

	bool DoublePattern::findShortcuts()
	{
		_shortcuts.clear();

		auto& lower = _trace.getStitches();
		auto& upper = _trace2.getStitches();

		for( auto& u : upper )
		{
			for( int i = 0; i < lower.size() - 1; i++ )
			{
				Path p = Path( lower[i], lower[i + 1] );
				if( p.hittest( u, DistanceTolerance ) )
				{
					_shortcuts.push_back( u );
					std::cerr << "shortcut in pattern " << _name << " at " << u << std::endl;
				}
			}
		}

		return ( _shortcuts.size() == 0 );
	}

	bool DoublePattern::save()
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

		FILE* fp = fopen( tempStr, "w" );
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

		auto stitches = _trace.getStitches();
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

		auto stitches2 = _trace2.getStitches();
		if( stitches2.size() )
		{
			std::stringstream sstr;
			sstr.precision( 5 );
			for( int i = 0; i < stitches2.size(); i++ )
			{
				float x = canvasCenterX + stitches2[i].x * scale;
				float y = canvasCenterY - stitches2[i].y * scale;

				sstr << ( i ? " " : "" ) << x << "," << y;
			}

			fprintf( fp, "  <polyline points=\"%s\" stroke=\"#%02X%02X%02X\" stroke-width=\"1\" fill=\"none\" />\n", sstr.str().c_str(),
				(int) ( clamp01( Color2.r ) * 255 ),
				(int) ( clamp01( Color2.g ) * 255 ),
				(int) ( clamp01( Color2.b ) * 255 )
			);
		}

		fprintf( fp, "</svg>\n" );

		fclose( fp );

		return true;
	}

	float DoublePattern::getTotalRunLength() const
	{
		return _trace.getRunLength() + _trace2.getRunLength();
	}
	size_t DoublePattern::getTotalStitchCount() const
	{
		return _trace.getStitchCount() + _trace2.getStitchCount();
	}

	void DoublePattern::translate( const glm::vec3& t )
	{
		Pattern::translate( t );

		_trace2.translate( t );
	}

	void DoublePattern::rotate( float rad )
	{
		Pattern::rotate( rad );

		_trace2.rotate( rad );
	}

	void DoublePattern::rotate90CW()
	{
		Pattern::rotate90CW();

		_trace2.rotate90CW();
	}

	void DoublePattern::rotate90CCW()
	{
		Pattern::rotate90CCW();

		_trace2.rotate90CCW();
	}

	void DoublePattern::rotate180()
	{
		Pattern::rotate180();

		_trace2.rotate180();
	}




	bool IDEDouble::PatternParams::drawUI()
	{
		if( ImGui::SliderInt( "teeth", &_teeth, 1, 50) )
			_invalidated = true;

		return PatternParamsBase::drawUI();
	}


	IDEDouble::IDEDouble() :
		DoublePattern( "IDE" ),
		_teeth( 0 ),
		_dist( 0.0f )
	{}

	IDEDouble::~IDEDouble()
	{}

	void IDEDouble::updateSizeString()
	{
		std::stringstream sstr;
		sstr <<
			"w: " << _width;

		_sizeString = sstr.str();
	}

	bool IDEDouble::build( const PatternParamsBase* params )
	{
		clear();

		const IDEDouble::PatternParams* p = dynamic_cast<const IDEDouble::PatternParams*>( params );
		if( !p )
			return false;

		_teeth = p->_teeth;
		_dist = p->_dist;
		float length = p->getLength();

		if( _teeth < 2 || _dist < 0 || length <= 0 )
			return false;

		_width = ( _teeth - 1 ) * _dist;
		float h = _dist + length;

		int i = 0;
		_trace2.insertBack( glm::vec3( -_width * 0.5f, h * 0.5f, 0 ) );
		for( i = 0; i < _teeth; i++ )
		{
			if( i % 2 )
			{
				_trace2.insertBack( glm::vec3( i * _dist - _width * 0.5f, h * 0.5f, 0 ) );
				_trace2.insertBack( glm::vec3( i * _dist - _width * 0.5f, h * 0.5f - length, 0 ) );
				_trace2.insertBack( glm::vec3( i * _dist - _width * 0.5f, h * 0.5f, 0 ) );
			}
			else
			{
				_trace.insertBack( glm::vec3( i * _dist - _width * 0.5f, -h * 0.5f, 0 ) );
				_trace.insertBack( glm::vec3( i * _dist - _width * 0.5f, length - h * 0.5f, 0 ) );
				_trace.insertBack( glm::vec3( i * _dist - _width * 0.5f, -h * 0.5f, 0 ) );
			}
		}
		if( i % 2 )
			_trace2.insertBack( glm::vec3( ( i - 1 ) * _dist - _width * 0.5f, h * 0.5f, 0 ) );
		else
			_trace.insertBack( glm::vec3( ( i - 1 ) * _dist - _width * 0.5f, -h * 0.5f, 0 ) );

		return DoublePattern::build( params );
	}

	std::string IDEDouble::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-T%d-d%.03f-j%.03f", getName().c_str(), _teeth, _dist, _trace.getJumpSize() );

		return std::string( tempStr );
	}



	bool BoustrophedonDouble::PatternParams::drawUI()
	{
		if( ImGui::SliderInt( "windings", &_windings, 1, 50 ) )
			_invalidated = true;

		return PatternParamsBase::drawUI();
	}


	BoustrophedonDouble::BoustrophedonDouble() :
		DoublePattern( "Boustrophedon" ),
		_windings( 0 ),
		_dist( 0.0f )
	{}

	BoustrophedonDouble::~BoustrophedonDouble()
	{}

	void BoustrophedonDouble::updateSizeString()
	{
		std::stringstream sstr;
		sstr <<
			"w: " << _width;

		_sizeString = sstr.str();
	}

	bool BoustrophedonDouble::build( const PatternParamsBase* params )
	{
		clear();

		const BoustrophedonDouble::PatternParams* p = dynamic_cast<const BoustrophedonDouble::PatternParams*>( params );
		if( !p )
			return false;

		_windings = p->_windings;
		_dist = p->_dist;

		if( _windings < 1 || _dist <= 0.0f )
			return false;

		_width = 2 * _dist * _windings;

		_trace.insertBack( glm::vec3( -_width * 0.5f, -_width * 0.5f, 0 ) );
		_trace2.insertBack( glm::vec3( _width * 0.5f, -_width * 0.5f, 0 ) );
		for( int i = 0; i < _windings; i++ )
		{
			_trace.insertBack( glm::vec3( -_width * 0.5f + _dist * ( 2 * i + 0.5f ), -_width * 0.5f, 0 ) );
			_trace.insertBack( glm::vec3( -_width * 0.5f + _dist * ( 2 * i + 0.5f ), _width * 0.5f, 0 ) );
			_trace.insertBack( glm::vec3( -_width * 0.5f + _dist * ( 2 * i + 1.5f ), _width * 0.5f, 0 ) );
			_trace.insertBack( glm::vec3( -_width * 0.5f + _dist * ( 2 * i + 1.5f ), -_width * 0.5f, 0 ) );

			_trace2.insertBack( glm::vec3( _width * 0.5f, -_width * 0.5f + _dist * ( 2 * i + 0.5f ), 0 ) );
			_trace2.insertBack( glm::vec3( -_width * 0.5f, -_width * 0.5f + _dist * ( 2 * i + 0.5f ), 0 ) );
			_trace2.insertBack( glm::vec3( -_width * 0.5f, -_width * 0.5f + _dist * ( 2 * i + 1.5f ), 0 ) );
			_trace2.insertBack( glm::vec3( _width * 0.5f, -_width * 0.5f + _dist * ( 2 * i + 1.5f ), 0 ) );
		}
		_trace.insertBack( glm::vec3( _width * 0.5f, -_width * 0.5f, 0 ) );
		_trace2.insertBack( glm::vec3( _width * 0.5f, _width * 0.5f, 0 ) );

		return DoublePattern::build( params );
	}

	std::string BoustrophedonDouble::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-W%d-d%.03f-j%.03f", getName().c_str(), _windings, _dist, _trace.getJumpSize() );

		return std::string( tempStr );
	}



	bool MeanderDouble::PatternParams::drawUI()
	{
		if( ImGui::SliderInt( "turns", &_turns, 1, 50 ) )
			_invalidated = true;

		return PatternParamsBase::drawUI();
	}


	MeanderDouble::MeanderDouble() :
		DoublePattern( "Meander" ),
		_turns( 0 ),
		_dist( 0.0f )
	{}

	MeanderDouble::~MeanderDouble()
	{}

	void MeanderDouble::updateSizeString()
	{
		std::stringstream sstr;
		sstr <<
			"w: " << _width;

		_sizeString = sstr.str();
	}

	bool MeanderDouble::build( const PatternParamsBase* params )
	{
		clear();

		const MeanderDouble::PatternParams* p = dynamic_cast<const MeanderDouble::PatternParams*>( params );
		if( !p )
			return false;

		_turns = p->_turns;
		_dist = p->_dist;

		if( _turns < 1 )
			return false;

		int dir = 0; // 0, 1, 2, 3 -> r, d, l, u
		_width = _turns * _dist * 2;

		std::vector<glm::vec3> temp;

		glm::vec3 v( -( _width + 2 * _dist ) * 0.5f, _width * 0.5f, 0 );
		temp.push_back( v );

		for( int i = 0; i <= _turns; i++, dir = ( dir + 1 ) % 4 )
		{
			float offset = max( 1, ( _turns - i ) ) * 2 * _dist;
			switch( dir )
			{
			case 0:
				v.x += offset;
				break;
			case 1:
				v.y -= offset;
				break;
			case 2:
				v.x -= offset;
				break;
			case 3:
				v.y += offset;
				break;
			}
			temp.push_back( v );
		}

		dir = ( dir + 2 ) % 4;

		for( int i = 0; i < _turns; i++, dir = ( dir + 3 ) % 4 )
		{
			float offset = ( i + 1 ) * 2 * _dist;
			switch( dir )
			{
			case 0:
				v.x += offset;
				break;
			case 1:
				v.y -= offset;
				break;
			case 2:
				v.x -= offset;
				break;
			case 3:
				v.y += offset;
				break;
			}
			temp.push_back( v );
		}

		temp.front().x += _dist;
		temp.back().x -= _dist;

		for( auto& p : temp )
		{
			_trace.insertFront( p );
			_trace2.insertFront( glm::vec3( -p.y, p.x, 0 ) );
		}

		return DoublePattern::build( params );
	}

	std::string MeanderDouble::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-T%d-d%.03f-j%.03f", getName().c_str(), _turns, _dist, _trace.getJumpSize() );

		return std::string( tempStr );
	}





	bool SpiralDouble::PatternParams::drawUI()
	{
		if( ImGui::SliderInt( "turns", &_turns, 1, 50 ) )
			_invalidated = true;

		return PatternParamsBase::drawUI();
	}


	SpiralDouble::SpiralDouble() :
		DoublePattern( "Spiral" ),
		_turns( 0 ),
		_dist( 0.0f )
	{}

	SpiralDouble::~SpiralDouble()
	{}

	void SpiralDouble::updateSizeString()
	{
		std::stringstream sstr;
		sstr <<
			"w: " << _width;

		_sizeString = sstr.str();
	}

	bool SpiralDouble::build( const PatternParamsBase* params )
	{
		clear();

		const SpiralDouble::PatternParams* p = dynamic_cast<const SpiralDouble::PatternParams*>( params );
		if( !p )
			return false;

		_turns = p->_turns;
		_dist = p->_dist;

		if( _turns < 1 )
			return false;

		int dir = 0; // 0, 1, 2, 3 -> r, d, l, u
		_width = _turns * _dist * 2;

		std::vector<glm::vec3> temp;

		glm::vec3 v( -_width * 0.5f, _width * 0.5f, 0 );
		temp.push_back( v );

		for( int i = 0; i <= _turns * 2; i++, dir = ( dir + 1 ) % 4 )
		{
			float offset = ( 2 * _turns - std::max( 0, ( ( std::max( 0, i + 1 ) / 2 ) * 2 - 1 ) ) ) * _dist;
			switch( dir )
			{
			case 0:
				v.x += offset;
				break;
			case 1:
				v.y -= offset;
				break;
			case 2:
				v.x -= offset;
				break;
			case 3:
				v.y += offset;
				break;
			}
			temp.push_back( v );
		}

		temp.back().x += _dist * 0.5f * ( _turns % 2 ? 1 : -1 );

		for( auto& p : temp )
		{
			_trace.insertBack( p );
			_trace2.insertBack( glm::vec3( -p.x, -p.y, 0 ) );
		}

		return DoublePattern::build( params );
	}

	std::string SpiralDouble::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-T%d-d%.03f-j%.03f", getName().c_str(), _turns, _dist, _trace.getJumpSize() );

		return std::string( tempStr );
	}




	bool HilbertDouble::PatternParams::drawUI()
	{
		if( ImGui::SliderInt( "order", &_order, 1, 6 ) )
			_invalidated = true;

		return PatternParamsBase::drawUI();
	}


	HilbertDouble::HilbertDouble() :
		DoublePattern( "Hilbert" ),
		_order( 0 ),
		_dist( 0.0f )
	{}

	HilbertDouble::~HilbertDouble()
	{}

	void HilbertDouble::updateSizeString()
	{
		std::stringstream sstr;
		sstr <<
			"w: " << _width;

		_sizeString = sstr.str();
	}

	bool HilbertDouble::build( const PatternParamsBase* params )
	{
		clear();

		const HilbertDouble::PatternParams* p = dynamic_cast<const HilbertDouble::PatternParams*>( params );
		if( !p )
			return false;

		_order = p->_order;
		_dist = p->_dist;

		if( _order < 1 )
			return false;

		int w = pow( 2, _order );
		int cells = w * w;

		_width = w * _dist * 2;

		glm::vec3 c(
			_width * 0.5f,
			_width * 0.5f,
			0 );

		std::vector<glm::vec3> temp;

		int x = 0;
		int y = 0;
		for( int i = 0; i < cells; i++ )
		{
			d2xy( w, i, &x, &y );

			glm::vec3 v( glm::vec3( ( x + 0.5f ) * _dist * 2, ( y + 0.5f ) * _dist * 2, 0 ) - c );

			if( temp.size() > 1 )
			{
				//if p[n-2], p[n-1] and p[n] are collinear, remove p[n-1]
				if( abs( glm::dot( glm::normalize( temp[temp.size() - 1] - temp[temp.size() - 2] ), glm::normalize( v - temp[temp.size() - 1] ) ) - 1.0 ) < Epsilon )
					temp.pop_back();
			}

			temp.push_back( v );
		}

		std::vector<glm::vec3> u( temp );

		//move first stitch to perimeter
		glm::vec3 back( glm::normalize( u[0] - u[1] ) );
		u[0] += back * _dist;

		glm::vec3 prev;

		std::vector<glm::vec3> lo;
		for( int i = 0; i < temp.size(); i++ )
			lo.push_back( temp[i] );

		prev = lo[0];
		for( int i = 0; i < lo.size() - 1; i++ )
		{
			glm::vec3 fwd = glm::normalize( lo[i + 1] - prev );
			prev = lo[i + 1];

			glm::vec3 right = glm::cross( fwd, glm::vec3( 0, 0, 1 ) );
			glm::vec3 offset = right * ( _dist ) * -1.0f;

			lo[i] += offset;
			lo[i + 1] += offset;
		}
		lo[0] -= glm::normalize( lo[1] - lo[0] ) * _dist;
		if( lo.size() > 1 )
			lo[lo.size() - 1] += glm::normalize( lo[lo.size() - 1] - lo[lo.size() - 2] ) * _dist;

		std::vector<glm::vec3> lu;
		for( int i = temp.size() - 1; i >= 0; i-- )
			lu.push_back( temp[i] );

		prev = lu[0];
		for( int i = 0; i < lu.size() - 1; i++ )
		{
			glm::vec3 fwd = glm::normalize( lu[i + 1] - prev );
			prev = lu[i + 1];

			glm::vec3 right = glm::cross( fwd, glm::vec3( 0, 0, 1 ) );
			glm::vec3 offset = right * ( _dist ) * -1.0f;

			lu[i] += offset;
			lu[i + 1] += offset;
		}
		lu[0] -= glm::normalize( lu[1] - lu[0] ) * _dist;
		if( lu.size() > 1 )
			lu[lu.size() - 1] += glm::normalize( lu[lu.size() - 1] - lu[lu.size() - 2] ) * _dist;

		temp.clear();

		for( auto& p : lo )
		{
			if( temp.size() && glm::distance( temp.back(), p ) < Epsilon )
				continue;
			temp.push_back( p );
		}
		for( auto& p : lu )
		{
			if( temp.size() && glm::distance( temp.back(), p ) < Epsilon )
				continue;
			temp.push_back( p );
		}

		//fill curve electrode
		for( auto& p : u )
			_trace2.insertBack( p );

		//fill tree electrode
		for( auto& p : temp )
			_trace.insertBack( p );

		return DoublePattern::build( params );
	}

	std::string HilbertDouble::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-O%d-d%.03f-j%.03f", getName().c_str(), _order, _dist, _trace.getJumpSize() );

		return std::string( tempStr );
	}




	bool PeanoDouble::PatternParams::drawUI()
	{
		if( ImGui::SliderInt( "order", &_order, 1, 4 ) )
			_invalidated = true;

		return PatternParamsBase::drawUI();
	}


	PeanoDouble::PeanoDouble() :
		DoublePattern( "Peano" ),
		_order( 0 ),
		_dist( 0.0f )
	{}

	PeanoDouble::~PeanoDouble()
	{}

	void PeanoDouble::updateSizeString()
	{
		std::stringstream sstr;
		sstr <<
			"w: " << _width;

		_sizeString = sstr.str();
	}

	void PeanoDouble::recurse( int x, int y, int o, bool hFlip, bool vFlip, std::vector<glm::vec3>& verts )
	{
		if( o == 0 )
		{
			glm::vec3 v( glm::vec3( x, y, 0 ) );

			if( verts.size() > 1 )
			{

				//if p[n-2], p[n-1] and p[n] are collinear, remove p[n-1]
				if( abs( glm::dot( glm::normalize( verts[verts.size() - 1] - verts[verts.size() - 2] ), glm::normalize( v - verts[verts.size() - 1] ) ) - 1.0 ) < Epsilon )
					verts.pop_back();
			}

			verts.push_back( v );
			return;
		}

		o--;

		int dx = pow( 3, o );
		int dy = pow( 3, o );

		recurse( x + ( hFlip ? 2 - 0 : 0 ) * dx, y + ( vFlip ? 2 - 0 : 0 ) * dy, o, hFlip, vFlip, verts );
		recurse( x + ( hFlip ? 2 - 0 : 0 ) * dx, y + ( vFlip ? 2 - 1 : 1 ) * dy, o, !hFlip, vFlip, verts );
		recurse( x + ( hFlip ? 2 - 0 : 0 ) * dx, y + ( vFlip ? 2 - 2 : 2 ) * dy, o, hFlip, vFlip, verts );
		recurse( x + ( hFlip ? 2 - 1 : 1 ) * dx, y + ( vFlip ? 2 - 2 : 2 ) * dy, o, hFlip, !vFlip, verts );
		recurse( x + ( hFlip ? 2 - 1 : 1 ) * dx, y + ( vFlip ? 2 - 1 : 1 ) * dy, o, !hFlip, !vFlip, verts );
		recurse( x + ( hFlip ? 2 - 1 : 1 ) * dx, y + ( vFlip ? 2 - 0 : 0 ) * dy, o, hFlip, !vFlip, verts );
		recurse( x + ( hFlip ? 2 - 2 : 2 ) * dx, y + ( vFlip ? 2 - 0 : 0 ) * dy, o, hFlip, vFlip, verts );
		recurse( x + ( hFlip ? 2 - 2 : 2 ) * dx, y + ( vFlip ? 2 - 1 : 1 ) * dy, o, !hFlip, vFlip, verts );
		recurse( x + ( hFlip ? 2 - 2 : 2 ) * dx, y + ( vFlip ? 2 - 2 : 2 ) * dy, o, hFlip, vFlip, verts );
	}

	bool PeanoDouble::build( const PatternParamsBase* params )
	{
		clear();

		const PeanoDouble::PatternParams* p = dynamic_cast<const PeanoDouble::PatternParams*>( params );
		if( !p )
			return false;

		_order = p->_order;
		_dist = p->_dist;

		if( _order < 1 )
			return false;

		std::vector<glm::vec3> temp;
		recurse( 0, 0, _order, false, false, temp );

		int w = pow( 3, _order );

		_width = w * _dist * 2.0f;

		glm::vec3 c(
			_width * 0.5f - _dist,
			_width * 0.5f - _dist,
			0 );

		for( auto& p : temp )
			p = p * _dist * 2.0f - c;

		std::vector<glm::vec3> u( temp );

		//move first stitch to perimeter
		glm::vec3 back( glm::normalize( u[0] - u[1] ) );
		u[0] += back * _dist;

		glm::vec3 prev;

		std::vector<glm::vec3> lo;
		for( int i = 0; i < temp.size(); i++ )
			lo.push_back( temp[i] );

		prev = lo[0];
		for( int i = 0; i < lo.size() - 1; i++ )
		{
			glm::vec3 fwd = glm::normalize( lo[i + 1] - prev );
			prev = lo[i + 1];

			glm::vec3 right = glm::cross( fwd, glm::vec3( 0, 0, 1 ) );
			glm::vec3 offset = right * ( _dist ) * -1.0f;

			lo[i] += offset;
			lo[i + 1] += offset;
		}
		lo[0] -= glm::normalize( lo[1] - lo[0] ) * _dist;
		if( lo.size() > 1 )
			lo[lo.size() - 1] += glm::normalize( lo[lo.size() - 1] - lo[lo.size() - 2] ) * _dist;

		std::vector<glm::vec3> lu;
		for( int i = temp.size() - 1; i >= 0; i-- )
			lu.push_back( temp[i] );

		prev = lu[0];
		for( int i = 0; i < lu.size() - 1; i++ )
		{
			glm::vec3 fwd = glm::normalize( lu[i + 1] - prev );
			prev = lu[i + 1];

			glm::vec3 right = glm::cross( fwd, glm::vec3( 0, 0, 1 ) );
			glm::vec3 offset = right * ( _dist ) * -1.0f;

			lu[i] += offset;
			lu[i + 1] += offset;
		}
		lu[0] -= glm::normalize( lu[1] - lu[0] ) * _dist;
		if( lu.size() > 1 )
			lu[lu.size() - 1] += glm::normalize( lu[lu.size() - 1] - lu[lu.size() - 2] ) * _dist;

		temp.clear();

		for( auto& p : lo )
		{
			if( temp.size() && glm::distance( temp.back(), p ) < Epsilon )
				continue;
			temp.push_back( p );
		}
		for( auto& p : lu )
		{
			if( temp.size() && glm::distance( temp.back(), p ) < Epsilon )
				continue;
			temp.push_back( p );
		}

		for( auto& p : u )
			_trace2.insertBack( p );

		//fill tree electrode
		for( auto& p : temp )
			_trace.insertBack( p );

		return DoublePattern::build( params );
	}

	std::string PeanoDouble::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-O%d-d%.03f-j%.03f", getName().c_str(), _order, _dist, _trace.getJumpSize() );

		return std::string( tempStr );
	}
}