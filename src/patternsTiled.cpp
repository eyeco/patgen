#include <patternsTiled.h>

#include <list>
#include <sstream>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

namespace TextileUX
{
	bool TiledPatternParams::drawUI()
	{
		if( ImGui::SliderInt( "cells X", &_cellsX, 1, 15 ) )
			_invalidated = true;
		if( ImGui::SliderInt( "cells Y", &_cellsY, 1, 15 ) )
			_invalidated = true;
		if( ImGui::SliderFloat( "cell dist", &_cellDist, 0.1f, 10.0f ) )
			_invalidated = true;

		return PatternParamsBase::drawUI();
	}


	TiledPattern::TiledPattern( const std::string& name ) :
		DoublePattern( name )
	{}

	TiledPattern::~TiledPattern()
	{
		swipeTraces();
	}

	void TiledPattern::clear()
	{
		DoublePattern::clear();

		swipeTraces();
	}

	void TiledPattern::swipeTraces()
	{
		//skip first one since it is our prototype (_trace)
		for( int i = 1; i < _traces.size(); i++ )
			safeDelete( _traces[i] );
		_traces.clear();
		_traces.push_back( &_trace );

		//skip first one since it is our prototype (_trace)
		for( int i = 1; i < _traces2.size(); i++ )
			safeDelete( _traces2[i] );
		_traces2.clear();
		_traces2.push_back( &_trace2 );
	}

	bool TiledPattern::findShortcuts()
	{
		_shortcuts.clear();

		return true;
	}

	void TiledPattern::draw()
	{
		glPushAttrib( GL_ALL_ATTRIB_BITS );
		{
			glLineWidth( 1 );
			glPointSize( 3 );

			for( auto it : _traces )
				it->draw();
			for( auto it : _traces2 )
				it->draw();

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

	bool TiledPattern::save()
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

		for( auto it : _traces )
		{
			auto stitches = it->getStitches();
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
		}

		for( auto it : _traces2 )
		{
			auto stitches2 = it->getStitches();
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
		}

		fprintf( fp, "</svg>\n" );

		fclose( fp );

		return true;
	}


	float TiledPattern::getTotalRunLength() const
	{
		float sum = 0;
		for( auto it : _traces )
			sum += it->getRunLength();
		for( auto it : _traces2 )
			sum += it->getRunLength();

		return sum;
	}
	size_t TiledPattern::getTotalStitchCount() const
	{
		size_t sum = 0;
		for( auto it : _traces )
			sum += it->getStitchCount();
		for( auto it : _traces2 )
			sum += it->getStitchCount();

		return sum;
	}

	void TiledPattern::translate( const glm::vec3& t )
	{
		for( auto it : _traces )
			it->translate( t );
		for( auto it : _traces2 )
			it->translate( t );
	}

	void TiledPattern::rotate( float rad )
	{
		for( auto it : _traces )
			it->rotate( rad );
		for( auto it : _traces2 )
			it->rotate( rad );
	}

	void TiledPattern::rotate90CW()
	{
		for( auto it : _traces )
			it->rotate90CW();
		for( auto it : _traces2 )
			it->rotate90CW();
	}

	void TiledPattern::rotate90CCW()
	{
		for( auto it : _traces )
			it->rotate90CCW();
		for( auto it : _traces2 )
			it->rotate90CCW();
	}

	void TiledPattern::rotate180()
	{
		for( auto it : _traces )
			it->rotate180();
		for( auto it : _traces2 )
			it->rotate180();

	}



	bool DiamondZigZagTiled::PatternParams::drawUI()
	{
		if( ImGui::SliderInt( "windings", &_windings, 1, 50 ) )
			_invalidated = true;

		return TiledPatternParams::drawUI();
	}


	DiamondZigZagTiled::DiamondZigZagTiled() :
		TiledPattern( "DiamondZigZag" ),
		_windings( 0 ),
		_dist( 0.0f ),
		_cellDist( 0.0f )
	{}

	DiamondZigZagTiled::~DiamondZigZagTiled()
	{}

	void DiamondZigZagTiled::updateSizeString()
	{
		std::stringstream sstr;
		sstr <<
			"TODO" <<
			" -> " <<
			"TODO";

		_sizeString = sstr.str();
	}

	bool DiamondZigZagTiled::build( const PatternParamsBase* params )
	{
		clear();

		const DiamondZigZagTiled::PatternParams* p = dynamic_cast<const DiamondZigZagTiled::PatternParams*>( params );
		if( !p )
			return false;

		clear();

		_windings = p->_windings;
		_dist = p->_dist;
		_cellDist = p->_cellDist;

		int tilesX = p->_cellsX;
		int tilesY = p->_cellsY;

		if( _windings < 1 || tilesX < 1 || tilesY < 1 )
			return false;


		float sqrt2 = sqrt( 2 );

		float diamondDiagonal = _windings * _dist * 2;

		float cellDiagonal = _cellDist * sqrt2 + diamondDiagonal;

		float offset = _cellDist * 0.5f * sqrt2;

		std::vector<glm::vec3> temp;

		float progress = 0.0f;

		for( int j = 0; j < _windings; j++ )
		{
			if( progress <= diamondDiagonal * 0.5f )
				temp.push_back( glm::vec3( progress, -offset - progress, 0 ) );
			else
				temp.push_back( glm::vec3( diamondDiagonal - progress, -offset - progress, 0 ) );

			progress += _dist;

			if( progress <= diamondDiagonal * 0.5f )
				temp.push_back( glm::vec3( progress, -offset - progress, 0 ) );
			else
				temp.push_back( glm::vec3( diamondDiagonal - progress, -offset - progress, 0 ) );

			if( progress <= diamondDiagonal * 0.5f )
				temp.push_back( glm::vec3( - progress, -offset - progress, 0 ) );
			else
				temp.push_back( glm::vec3( -diamondDiagonal + progress, -offset - progress, 0 ) );

			progress += _dist;

			if( progress <= diamondDiagonal * 0.5f )
				temp.push_back( glm::vec3( - progress, -offset - progress, 0 ) );
			else
				temp.push_back( glm::vec3( -diamondDiagonal + progress, -offset - progress, 0 ) );
		}

		float x0 = -( tilesX - 1 ) * cellDiagonal * 0.5f;
		float y0 = ( tilesY - 1 ) * cellDiagonal * 0.5f + cellDiagonal;

		_trace.insertBack( glm::vec3( x0, y0, 0 ) );
		for( int i = 0; i <= tilesY; i++ )
			for( auto& p : temp )
				_trace.insertBack( glm::vec3( x0, y0 - i * cellDiagonal, 0 ) + p );
		_trace.insertBack( glm::vec3( x0, y0 - ( tilesY + 1 ) * cellDiagonal, 0 ) );

		x0 = -( tilesY - 1 ) * cellDiagonal * 0.5f;
		y0 = ( tilesX - 1 ) * cellDiagonal * 0.5f + cellDiagonal;

		_trace2.insertBack( glm::vec3( -y0, x0, 0 ) );
		for( int i = 0; i <= tilesX; i++ )
			for( auto& p : temp )
				_trace2.insertBack( glm::vec3( -y0 + i * cellDiagonal, x0, 0 ) + glm::vec3( -p.y, p.x, 0 ) );
		_trace2.insertBack( glm::vec3( -y0 + ( tilesX + 1 ) * cellDiagonal, x0, 0 ) );

		if( !DoublePattern::build( params ) )
			return false;

		for( int i = 1; i < tilesX; i++ )
		{
			Trace *t = new Trace( _trace );
			t->translate( glm::vec3( i * cellDiagonal, 0, 0 ) );
			t->rebuild( params->_jumpSize, params->_minJumpSize );

			_traces.push_back( t );
		}
		for( int i = 1; i < tilesY; i++ )
		{
			Trace *t = new Trace( _trace2 );
			t->translate( glm::vec3( 0, i * cellDiagonal, 0 ) );
			t->rebuild( params->_jumpSize, params->_minJumpSize );

			_traces2.push_back( t );
		}

		return true;
	}

	std::string DiamondZigZagTiled::getFullName() const
	{
		char tempStr[128];

		//TODO: add windings and tilesX/Y
		sprintf( tempStr, "%s-d%.01f-j[u%.01f-l%.01f]", getName().c_str(), _dist, _trace2.getJumpSize(), _trace.getJumpSize() );

		return std::string( tempStr );
	}





	bool DiamondSpiralTiled::PatternParams::drawUI()
	{
		if( ImGui::SliderInt( "turns", &_turns, 1, 50 ) )
			_invalidated = true;

		return TiledPatternParams::drawUI();
	}


	DiamondSpiralTiled::DiamondSpiralTiled() :
		TiledPattern( "DiamondSpiral" ),
		_turns( 0 ),
		_dist( 0.0f ),
		_cellDist( 0.0f )
	{}

	DiamondSpiralTiled::~DiamondSpiralTiled()
	{}

	void DiamondSpiralTiled::updateSizeString()
	{
		std::stringstream sstr;
		sstr <<
			"TODO" <<
			" -> " <<
			"TODO";

		_sizeString = sstr.str();
	}

	bool DiamondSpiralTiled::build( const PatternParamsBase* params )
	{
		clear();

		const DiamondSpiralTiled::PatternParams* p = dynamic_cast<const DiamondSpiralTiled::PatternParams*>( params );
		if( !p )
			return false;

		clear();

		_turns = p->_turns;
		_dist = p->_dist;
		_cellDist = p->_cellDist;

		int tilesX = p->_cellsX;
		int tilesY = p->_cellsY;

		if( _turns < 1 || tilesX < 1 || tilesY < 1 )
			return false;

		float sqrt2 = sqrt( 2 );

		float diamondDiagonal = _turns * _dist * 2 * sqrt2;

		float cellDiagonal = diamondDiagonal + _cellDist * sqrt2;

		float offset = _cellDist * 0.5f * sqrt2;;
		float d = _dist * sqrt2;

		std::vector<glm::vec3> temp;

		temp.push_back( glm::vec3( 0, -offset, 0 ) );
		for( int j = 0; j < _turns; j++ )
		{
			temp.push_back( glm::vec3( diamondDiagonal * 0.5 - j * d, -cellDiagonal * 0.5, 0 ) );
			temp.push_back( glm::vec3( 0, -diamondDiagonal + j * d - offset, 0  ) );
			temp.push_back( glm::vec3( -diamondDiagonal * 0.5 + j * d, -cellDiagonal * 0.5, 0 ) );
			temp.push_back( glm::vec3( -d * 0.5, - j * d - d * 0.5 - offset, 0  ) );
		}
		temp.push_back( glm::vec3( 0, -cellDiagonal * 0.5, 0 ) );

		float x0 = -( tilesX - 1 ) * cellDiagonal * 0.5f;
		float y0 = ( tilesY - 1 ) * cellDiagonal * 0.5f + cellDiagonal;

		_trace.insertBack( glm::vec3( x0, y0, 0 ) );
		for( int i = 0; i <= tilesY; i++ )
			for( auto& p : temp )
				_trace.insertBack( glm::vec3( x0, y0 - i * cellDiagonal, 0 ) + p );
		_trace.insertBack( glm::vec3( x0, y0 - ( tilesY + 1 ) * cellDiagonal, 0 ) );

		x0 = -( tilesY - 1 ) * cellDiagonal * 0.5f;
		y0 = ( tilesX - 1 ) * cellDiagonal * 0.5f + cellDiagonal;

		_trace2.insertBack( glm::vec3( -y0, x0, 0 ) );
		for( int i = 0; i <= tilesX; i++ )
			for( auto& p : temp )
				_trace2.insertBack( glm::vec3( -y0 + i * cellDiagonal, x0, 0 ) + glm::vec3( -p.y, p.x, 0 ) );
		_trace2.insertBack( glm::vec3( -y0 + ( tilesX + 1 ) * cellDiagonal, x0, 0 ) );

		if( !DoublePattern::build( params ) )
			return false;

		for( int i = 1; i < tilesX; i++ )
		{
			Trace* t = new Trace( _trace );
			t->translate( glm::vec3( i * cellDiagonal, 0, 0 ) );
			t->rebuild( params->_jumpSize, params->_minJumpSize );

			_traces.push_back( t );
		}
		for( int i = 1; i < tilesY; i++ )
		{
			Trace* t = new Trace( _trace2 );
			t->translate( glm::vec3( 0, i * cellDiagonal, 0 ) );
			t->rebuild( params->_jumpSize, params->_minJumpSize );

			_traces2.push_back( t );
		}

		return true;
	}

	std::string DiamondSpiralTiled::getFullName() const
	{
		char tempStr[128];

		//TODO: add turns and tilesX/Y
		sprintf( tempStr, "%s-d%.01f-j[u%.01f-l%.01f]", getName().c_str(), _dist, _trace2.getJumpSize(), _trace.getJumpSize() );

		return std::string( tempStr );
	}




	bool MeanderTiled::PatternParams::drawUI()
	{
		if( ImGui::SliderInt( "turns", &_turns, 1, 50 ) )
			_invalidated = true;

		return TiledPatternParams::drawUI();
	}


	MeanderTiled::MeanderTiled() :
		TiledPattern( "Meander" ),
		_turns( 0 ),
		_dist( 0.0f ),
		_cellDist( 0.0f )
	{}

	MeanderTiled::~MeanderTiled()
	{}

	void MeanderTiled::updateSizeString()
	{
		std::stringstream sstr;
		sstr <<
			"TODO" <<
			" -> " <<
			"TODO";

		_sizeString = sstr.str();
	}

	bool MeanderTiled::build( const PatternParamsBase* params )
	{
		clear();

		const MeanderTiled::PatternParams* p = dynamic_cast<const MeanderTiled::PatternParams*>( params );
		if( !p )
			return false;

		clear();

		_turns = p->_turns;
		_dist = p->_dist;
		_cellDist = p->_cellDist;

		int tilesX = p->_cellsX;
		int tilesY = p->_cellsY;

		if( _turns < 1 || tilesX < 1 || tilesY < 1 )
			return false;

		int dir = 0; // 0, 1, 2, 3 -> r, d, l, u
		float w = ( _turns + 1 ) * _dist * 2;

		std::list<glm::vec3> temp;

		glm::vec3 v( -w * 0.5f, w * 0.5f - _dist, 0 );
		temp.push_back( v );

		for( int i = 0; i <= _turns; i++, dir = ( dir + 1 ) % 4 )
		{
			float offset = max<int>( 1, ( _turns - i ) ) * 2 * _dist;
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

		//remove first and last as neighbouring cells share one edge
		temp.pop_front();
		temp.pop_back();

		float x0 = -( w - 2 * _dist ) * ( tilesX - 1 ) * 0.5f;
		float y0 = ( w - 2 * _dist ) * ( tilesY - 1 ) * 0.5f;

		for( int i = 0; i < tilesY; i++ )
			for( auto& p : temp )
				_trace.insertBack( glm::vec3( x0, y0 - i * ( w - 2 * _dist ), 0 ) + p );

		for( int i = 0; i < tilesX; i++ )
			for( auto& p : temp )
				_trace2.insertBack( glm::vec3( x0 + i * ( w - 2 * _dist ), y0, 0 ) + glm::vec3( -p.y, p.x, 0 ) );

		if( !DoublePattern::build( params ) )
			return false;

		for( int i = 1; i < tilesX; i++ )
		{
			Trace* t = new Trace( _trace );
			t->translate( glm::vec3( i * ( w - 2 * _dist ), 0, 0 ) );
			t->rebuild( params->_jumpSize, params->_minJumpSize );

			_traces.push_back( t );
		}
		for( int i = 1; i < tilesY; i++ )
		{
			Trace* t = new Trace( _trace2 );
			t->translate( glm::vec3( 0, -i * ( w - 2 * _dist ), 0 ) );
			t->rebuild( params->_jumpSize, params->_minJumpSize );

			_traces2.push_back( t );
		}

		return true;
	}

	std::string MeanderTiled::getFullName() const
	{
		char tempStr[128];

		//TODO: add turns and tilesX/Y
		sprintf( tempStr, "%s-d%.01f-j[u%.01f-l%.01f]", getName().c_str(), _dist, _trace2.getJumpSize(), _trace.getJumpSize() );

		return std::string( tempStr );
	}




	bool AntennaTiled::PatternParams::drawUI()
	{
		if( ImGui::SliderInt( "order", &_order, 1, 20 ) )
			_invalidated = true;

		return TiledPatternParams::drawUI();
	}


	AntennaTiled::AntennaTiled() :
		TiledPattern( "Antenna" ),
		_order( 0 ),
		_dist( 0.0f ),
		_cellDist( 0.0f )
	{}

	AntennaTiled::~AntennaTiled()
	{}

	void AntennaTiled::updateSizeString()
	{
		std::stringstream sstr;
		sstr <<
			"TODO" <<
			" -> " <<
			"TODO";

		_sizeString = sstr.str();
	}

	bool AntennaTiled::build( const PatternParamsBase* params )
	{
		clear();

		const AntennaTiled::PatternParams* p = dynamic_cast<const AntennaTiled::PatternParams*>( params );
		if( !p )
			return false;

		clear();

		_order = p->_order;
		_dist = p->_dist;
		_cellDist = p->_cellDist;

		int tilesX = p->_cellsX;
		int tilesY = p->_cellsY;

		if( _order < 1 || tilesX < 1 || tilesY < 1 )
			return false;


		std::vector<glm::vec3> temp;

		float cellSize = _order * 4 * _dist;
		float tileSize = cellSize + _cellDist;

		//build lower

		temp.clear();

		float x0 = 0;
		float y0 = tileSize / 2 - _cellDist * 0.5f;

		//temp.push_back( glm::vec3( x0, y0, 0 ) );

		//y0 -= _cellDist * 0.5f;
		temp.push_back( glm::vec3( x0, y0, 0 ) );

		//TOP
		//left
		for( int i = 0; i < _order; i++ )
		{
			float x = x0 - ( _order - i ) * _dist * 2;
			float y1 = y0 - ( cellSize * 0.5f - _dist );
			temp.push_back( glm::vec3( x, y0, 0 ) );
			temp.push_back( glm::vec3( x, y1, 0 ) );
			temp.push_back( glm::vec3( x, y0, 0 ) );
		}

		//right
		for( int i = 0; i < _order; i++ )
		{
			float x = x0 + ( _order - i ) * _dist * 2;
			float y1 = y0 - ( cellSize * 0.5f - _dist );
			temp.push_back( glm::vec3( x, y0, 0 ) );
			temp.push_back( glm::vec3( x, y1, 0 ) );
			temp.push_back( glm::vec3( x, y0, 0 ) );
		}
		temp.push_back( glm::vec3( x0, y0, 0 ) );

		y0 -= cellSize;
		temp.push_back( glm::vec3( x0, y0, 0 ) );

		//BOTTOM
		//left
		for( int i = 0; i < _order; i++ )
		{
			float x = x0 - ( _order - i ) * _dist * 2;
			float y1 = y0 + ( cellSize * 0.5f - _dist );
			temp.push_back( glm::vec3( x, y0, 0 ) );
			temp.push_back( glm::vec3( x, y1, 0 ) );
			temp.push_back( glm::vec3( x, y0, 0 ) );
		}

		//right
		for( int i = 0; i < _order; i++ )
		{
			float x = x0 + ( _order - i ) * _dist * 2;
			float y1 = y0 + ( cellSize * 0.5f - _dist );
			temp.push_back( glm::vec3( x, y0, 0 ) );
			temp.push_back( glm::vec3( x, y1, 0 ) );
			temp.push_back( glm::vec3( x, y0, 0 ) );
		}
		temp.push_back( glm::vec3( x0, y0, 0 ) );

		//y0 -= _cellDist * 0.5f;
		//temp.push_back( glm::vec3( x0, y0, 0 ) );

		x0 = -tileSize * ( tilesX - 1 ) * 0.5f;
		y0 = tileSize * ( tilesY - 1 ) * 0.5f;

		_trace.insertBack( glm::vec3( x0, y0 + tileSize / 2, 0 ) );
		for( int i = 0; i < tilesY; i++ )
			for( auto& p : temp )
				_trace.insertBack( p + glm::vec3( x0, y0 - i * tileSize, 0 ) );
		_trace.insertBack( glm::vec3( x0, y0 - tilesY * tileSize + tileSize / 2, 0 ) );

		//build upper

		temp.clear();

		x0 = -tileSize * 0.5f + _cellDist * 0.5f;
		y0 = 0;

		//temp.push_back( glm::vec3( x0, y0, 0 ) );

		//x0 += _cellDist * 0.5f;
		temp.push_back( glm::vec3( x0, y0, 0 ) );

		for( int i = 0; i < _order * 2; i++ )
		{
			x0 += _dist;

			temp.push_back( glm::vec3( x0, y0, 0 ) );
			temp.push_back( glm::vec3( x0, y0 + ( cellSize * 0.5f - _dist ), 0 ) );
			temp.push_back( glm::vec3( x0, y0 - ( cellSize * 0.5f - _dist ), 0 ) );
			temp.push_back( glm::vec3( x0, y0, 0 ) );

			x0 += _dist;
		}
		temp.push_back( glm::vec3( x0, y0, 0 ) );

		//x0 += _cellDist * 0.5f;
		//temp.push_back( glm::vec3( x0, y0, 0 ) );

		x0 = -tileSize * ( tilesX - 1 ) * 0.5f;
		y0 = tileSize * ( tilesY - 1 ) * 0.5f;

		_trace2.insertBack( glm::vec3( x0 - tileSize * 0.5, y0, 0 ) );
		for( int i = 0; i < tilesX; i++ )
			for( auto& p : temp )
				_trace2.insertBack( p + glm::vec3( x0 + i * tileSize, y0, 0 ) );
		_trace2.insertBack( glm::vec3( x0 + tilesX * tileSize - tileSize * 0.5, y0, 0 ) );

		if( !DoublePattern::build( params ) )
			return false;

		for( int i = 1; i < tilesX; i++ )
		{
			Trace* t = new Trace( _trace );
			t->translate( glm::vec3( i * tileSize, 0, 0 ) );
			t->rebuild( params->_jumpSize, params->_minJumpSize );

			_traces.push_back( t );
		}
		for( int i = 1; i < tilesY; i++ )
		{
			Trace* t = new Trace( _trace2 );
			t->translate( glm::vec3( 0, -i * tileSize, 0 ) );
			t->rebuild( params->_jumpSize, params->_minJumpSize );

			_traces2.push_back( t );
		}

		return true;
	}

	std::string AntennaTiled::getFullName() const
	{
		char tempStr[128];

		//TODO: add order and tilesX/Y
		sprintf( tempStr, "%s-d%.01f-j[u%.01f-l%.01f]", getName().c_str(), _dist, _trace2.getJumpSize(), _trace.getJumpSize() );

		return std::string( tempStr );
	}





	bool FlowerTiled::PatternParams::drawUI()
	{
		if( ImGui::SliderInt( "turns", &_turns, 1, 10 ) )
			_invalidated = true;

		return TiledPatternParams::drawUI();
	}


	FlowerTiled::FlowerTiled() :
		TiledPattern( "Flower" ),
		_turns( 0 ),
		_dist( 0.0f ),
		_cellDist( 0.0f )
	{}

	FlowerTiled::~FlowerTiled()
	{}

	void FlowerTiled::updateSizeString()
	{
		std::stringstream sstr;
		sstr <<
			"TODO" <<
			" -> " <<
			"TODO";

		_sizeString = sstr.str();
	}

	bool FlowerTiled::build( const PatternParamsBase* params )
	{
		clear();

		const FlowerTiled::PatternParams* p = dynamic_cast<const FlowerTiled::PatternParams*>( params );
		if( !p )
			return false;

		clear();

		_turns = p->_turns;
		_dist = p->_dist;
		_cellDist = p->_cellDist;

		int tilesX = p->_cellsX;
		int tilesY = p->_cellsY;

		if( _turns < 1 || tilesX < 1 || tilesY < 1 )
			return false;

		float sqrt2 = sqrt( 2 );

		float triHeight = _turns * ( _dist * ( 1 + sqrt2 ) );
		float triWidth = triHeight * sqrt2;

		float cellSize = ( 2 * triHeight + _cellDist ) / sqrt2;
		float tileSize = 2 * ( cellSize + _cellDist );

		std::vector<glm::vec3> temp;

		//lower left segment
		float x0 = _cellDist * 0.5 -tileSize * 0.5f;
		float y0 = -_cellDist * 0.5;

		float x = x0;
		float y = y0;
		float a = triWidth;
		for( int i = 0; i < _turns; i++ )
		{
			temp.push_back( glm::vec3( x, y, 0 ) );
			if( i )
				a -= _dist * sqrt2;
			y -= a;
			temp.push_back( glm::vec3( x, y, 0 ) );
			if( i )
				a -= _dist;
			y += a;
			x += a;
			temp.push_back( glm::vec3( x, y, 0 ) );
			a -= _dist;
			x -= a;
		}
		//temp.push_back( glm::vec3( x, y, 0 ) );	//skip this here, to avoid acute angle
		temp.push_back( glm::vec3( -_cellDist * 0.5f, _cellDist * 0.25, 0 ) );

		//upper left segment
		x0 += cellSize;
		y0 += _cellDist + _cellDist / sqrt2;

		x = x0;
		y = y0;
		a = triWidth;
		for( int i = 0; i < _turns; i++ )
		{
			temp.push_back( glm::vec3( x, y, 0 ) );
			if( i )
				a -= _dist;
			y += a;
			temp.push_back( glm::vec3( x, y, 0 ) );
			if( i )
				a -= _dist * sqrt2;
			x -= a;
			temp.push_back( glm::vec3( x, y, 0 ) );
			a -= _dist;
			x += a;
			y -= a;
		}
		temp.push_back( glm::vec3( x, y, 0 ) );
		temp.push_back( glm::vec3( _cellDist * 0.5, -_cellDist * 0.25f, 0 ) );

		//lower right segment
		x0 += _cellDist;
		y0 -= _cellDist + ( 2 * _cellDist / sqrt2 );

		x = x0;
		y = y0;
		a = triWidth;
		for( int i = 0; i < _turns; i++ )
		{
			temp.push_back( glm::vec3( x, y, 0 ) );
			if( i )
				a -= _dist;
			y -= a;
			temp.push_back( glm::vec3( x, y, 0 ) );
			if( i )
				a -= _dist * sqrt2;
			x += a;
			temp.push_back( glm::vec3( x, y, 0 ) );
			a -= _dist;
			x -= a;
			y += a;
		}
		temp.push_back( glm::vec3( x, y, 0 ) );
		temp.push_back( glm::vec3( _cellDist * 0.25f, _cellDist * 0.5, 0 ) );

		//upper right segment
		x0 += _cellDist / sqrt2;
		y0 += _cellDist + ( _cellDist / sqrt2 );

		x = x0;
		y = y0;
		a = triWidth;
		for( int i = 0; i < _turns; i++ )
		{
			temp.push_back( glm::vec3( x, y, 0 ) );
			if( i )
				a -= _dist;
			x += a;
			temp.push_back( glm::vec3( x, y, 0 ) );
			if( i )
				a -= _dist * sqrt2;
			y += a;
			temp.push_back( glm::vec3( x, y, 0 ) );
			a -= _dist;
			x -= a;
			y -= a;
		}
		temp.push_back( glm::vec3( x, y, 0 ) );
		temp.push_back( glm::vec3( tileSize * 0.5f, 0, 0 ) );


		x0 = -tileSize * ( tilesX - 1 ) * 0.5f;
		y0 = tileSize * ( tilesY - 1 ) * 0.5f;

		_trace.insertBack( glm::vec3( x0, y0 + tileSize / 2, 0 ) );
		for( int i = 0; i < tilesY; i++ )
			for( auto& p : temp )
				_trace.insertBack( glm::vec3( -p.y, -p.x, 0 ) + glm::vec3( x0, y0 - i * tileSize, 0 ) );

		_trace2.insertBack( glm::vec3( x0 - tileSize / 2, y0, 0 ) );
		for( int i = 0; i < tilesX; i++ )
			for( auto& p : temp )
				_trace2.insertBack( p + glm::vec3( x0 + i * tileSize, y0, 0 ) );

		if( !DoublePattern::build( params ) )
			return false;

		for( int i = 1; i < tilesX; i++ )
		{
			Trace* t = new Trace( _trace );
			t->translate( glm::vec3( i * tileSize, 0, 0 ) );
			t->rebuild( params->_jumpSize, params->_minJumpSize );

			_traces.push_back( t );
		}
		for( int i = 1; i < tilesY; i++ )
		{
			Trace* t = new Trace( _trace2 );
			t->translate( glm::vec3( 0, -i * tileSize, 0 ) );
			t->rebuild( params->_jumpSize, params->_minJumpSize );

			_traces2.push_back( t );
		}

		return true;
	}

	std::string FlowerTiled::getFullName() const
	{
		char tempStr[128];

		//TODO: add tilesX/Y
		sprintf( tempStr, "%s-T%d-d%.03f-cd%.03f-j[u%.03f-l%.03f]", getName().c_str(), _turns, _dist, _cellDist, _trace2.getJumpSize(), _trace.getJumpSize() );

		return std::string( tempStr );
	}
}