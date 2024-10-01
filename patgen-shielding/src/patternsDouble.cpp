#include <patternsDouble.h>

#include <sstream>
#include <iostream>

namespace TextileUX
{
	glm::vec4 DoublePattern::Color2 = glm::vec4( 0, 1, 1, 1 );


	DoublePattern::DoublePattern( const std::string& name ) : 
		Pattern( name ),
		_trace2( Color2 )
	{}

	DoublePattern::~DoublePattern()
	{}

	void DoublePattern::clear()
	{
		Pattern::clear();

		_trace2.clear();
		_shortcuts.clear();
	}

	bool DoublePattern::validate()
	{
		return Pattern::validate() && _trace2.validate();
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

		if( !_trace.removeStitches( rem ) )
		{
			std::cerr << "<error> removing shorting stitches failed" << std::endl;
			return false;
		}

		std::cout << "successfully removed " << ( temp - upper.size() ) << " stitch(es) to prevent from shorting -- validate should now pass" << std::endl;

		return validate();
	}

	void DoublePattern::draw()
	{
		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glLineWidth( 1 );
			glPointSize( 3 );

			_trace.draw();
			_trace2.draw();
		}
		glPopAttrib();
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
			std::cerr << "ERROR: unit not set" << std::endl;
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

		auto& stitches = _trace.getStitches();
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

		stitches = _trace2.getStitches();
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
		if( ImGui::SliderFloat( "dist", &_dist, 1.0f, 100.0f ) )
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
			"TODO" <<
			" -> " <<
			"TODO";

		_sizeString = sstr.str();
	}

	bool IDEDouble::build( const PatternParamsBase* params )
	{
		clear();

		const IDEDouble::PatternParams* p = dynamic_cast<const IDEDouble::PatternParams*>( params );
		if( !p )
			return false;

		if( p->_teeth < 2 || p->_dist < 0 || p->getLength() <= 0 )
			return false;

		clear();

		_teeth = p->_teeth;
		_dist = p->_dist;
		float length = p->getLength();

		float w = ( _teeth - 1 ) * _dist;
		float h = _dist + length;

		int i = 0;
		_trace2.insertBack( glm::vec3( -w * 0.5f, h * 0.5f, 0 ) );
		for( i = 0; i < _teeth; i++ )
		{
			if( i % 2 )
			{
				_trace2.insertBack( glm::vec3( i * _dist - w * 0.5f, h * 0.5f, 0 ) );
				_trace2.insertBack( glm::vec3( i * _dist - w * 0.5f, h * 0.5f - length, 0 ) );
				_trace2.insertBack( glm::vec3( i * _dist - w * 0.5f, h * 0.5f, 0 ) );
			}
			else
			{
				_trace.insertBack( glm::vec3( i * _dist - w * 0.5f, -h * 0.5f, 0 ) );
				_trace.insertBack( glm::vec3( i * _dist - w * 0.5f, length - h * 0.5f, 0 ) );
				_trace.insertBack( glm::vec3( i * _dist - w * 0.5f, -h * 0.5f, 0 ) );
			}
		}
		if( i % 2 )
			_trace2.insertBack( glm::vec3( ( i - 1 ) * _dist - w * 0.5f, h * 0.5f, 0 ) );
		else
			_trace.insertBack( glm::vec3( ( i - 1 ) * _dist - w * 0.5f, -h * 0.5f, 0 ) );


		if( !_trace2.rebuild( params->_jumpSize, params->_minJumpSize ) )
			return false;

		return Pattern::build( params );
	}

	std::string IDEDouble::getFullName() const
	{
		char tempStr[128];

		sprintf( tempStr, "%s-d%.01f-t%d-j[u%.01f-l%.01f]", getName().c_str(), _dist, _teeth, _trace2.getJumpSize(), _trace.getJumpSize() );

		return std::string( tempStr );
	}
}