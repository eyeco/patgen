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
}