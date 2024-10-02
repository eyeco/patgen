#include <patternsTiled.h>

#include <sstream>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

namespace TextileUX
{
	bool TiledPatternParams::drawUI()
	{
		if( ImGui::SliderInt( "tiles X", &_tilesX, 1, 15 ) )
			_invalidated = true;
		if( ImGui::SliderInt( "tiles Y", &_tilesY, 1, 15 ) )
			_invalidated = true;
		if( ImGui::SliderFloat( "tile dist", &_tileDist, 0.1f, 10.0f ) )
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
		return TiledPattern::save();
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
		_tileDist( 0.0f )
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
		_tileDist = p->_tileDist;

		int tilesX = p->_tilesX;
		int tilesY = p->_tilesY;

		if( _windings < 1 || tilesX < 1 || tilesY < 1 )
			return false;

		std::vector<glm::vec3> temp;

		float sqrt2 = sqrt( 2 );

		float diamondDiagonal = _windings * _dist * 2;
		//float diamondSize = diamondDiagonal / sqrt2;

		float cellDiagonal = _tileDist * sqrt2 + diamondDiagonal;
		//float cellSize = cellDiagonal / sqrt2;

		//float cellPadding = _tileDist * 0.5f;
		float offset = _tileDist * 0.5f * sqrt2;;

		float x0 = -( tilesX - 1 ) * cellDiagonal * 0.5f;
		float y0 = ( tilesY - 1 ) * cellDiagonal * 0.5f + cellDiagonal;

		temp.push_back( glm::vec3( x0, y0, 0 ) );

		for( int i = 0; i <= tilesY; i++ )
		{
			float progress = 0.0f;

			for( int j = 0; j < _windings; j++ )
			{
				if( progress <= diamondDiagonal * 0.5f )
					temp.push_back( glm::vec3( x0 + progress, y0 - offset - progress, 0 ) );
				else
					temp.push_back( glm::vec3( x0 + ( diamondDiagonal - progress ), y0 - offset - progress, 0 ) );

				progress += _dist;

				if( progress <= diamondDiagonal * 0.5f )
					temp.push_back( glm::vec3( x0 + progress, y0 - offset - progress, 0 ) );
				else
					temp.push_back( glm::vec3( x0 + ( diamondDiagonal - progress ), y0 - offset - progress, 0 ) );

				if( progress <= diamondDiagonal * 0.5f )
					temp.push_back( glm::vec3( x0 - progress, y0 - offset - progress, 0 ) );
				else
					temp.push_back( glm::vec3( x0 - ( diamondDiagonal - progress ), y0 - offset - progress, 0 ) );

				progress += _dist;

				if( progress <= diamondDiagonal * 0.5f )
					temp.push_back( glm::vec3( x0 - progress, y0 - offset - progress, 0 ) );
				else
					temp.push_back( glm::vec3( x0 - ( diamondDiagonal - progress ), y0 - offset - progress, 0 ) );
			}

			y0 -= cellDiagonal;
		}

		temp.push_back( glm::vec3( x0, y0, 0 ) );

		for( auto& p : temp )
			_trace.insertBack( p );

		if( tilesX != tilesY )
		{
			temp.clear();

			x0 = -( tilesY - 1 ) * cellDiagonal * 0.5f;
			y0 = ( tilesX - 1 ) * cellDiagonal * 0.5f + cellDiagonal;

			for( int i = 0; i <= tilesX; i++ )
			{
				float progress = 0.0f;

				for( int j = 0; j < _windings; j++ )
				{
					if( progress <= diamondDiagonal * 0.5f )
						temp.push_back( glm::vec3( x0 + progress, y0 - offset - progress, 0 ) );
					else
						temp.push_back( glm::vec3( x0 + ( diamondDiagonal - progress ), y0 - offset - progress, 0 ) );

					progress += _dist;

					if( progress <= diamondDiagonal * 0.5f )
						temp.push_back( glm::vec3( x0 + progress, y0 - offset - progress, 0 ) );
					else
						temp.push_back( glm::vec3( x0 + ( diamondDiagonal - progress ), y0 - offset - progress, 0 ) );

					if( progress <= diamondDiagonal * 0.5f )
						temp.push_back( glm::vec3( x0 - progress, y0 - offset - progress, 0 ) );
					else
						temp.push_back( glm::vec3( x0 - ( diamondDiagonal - progress ), y0 - offset - progress, 0 ) );

					progress += _dist;

					if( progress <= diamondDiagonal * 0.5f )
						temp.push_back( glm::vec3( x0 - progress, y0 - offset - progress, 0 ) );
					else
						temp.push_back( glm::vec3( x0 - ( diamondDiagonal - progress ), y0 - offset - progress, 0 ) );
				}

				y0 -= cellDiagonal;
			}
		}

		for( auto& p : temp )
			_trace2.insertBack( glm::vec3( -p.y, p.x, 0 ) );

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

		sprintf( tempStr, "%s-d%.01f-j[u%.01f-l%.01f]", getName().c_str(), _dist, _trace2.getJumpSize(), _trace.getJumpSize() );

		return std::string( tempStr );
	}
}