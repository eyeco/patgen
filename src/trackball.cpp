/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of patgen
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#include <trackball.h>

#include <glm/gtx/transform.hpp>

namespace TextileUX
{
	Trackball::Trackball() :
		_first( true ),
		_oldPos( 0 ),
		_buttonMask( 0 ),
		_m( 1 )
	{}

	Trackball::~Trackball()
	{}

	void Trackball::onMouse( int button, int state, int x, int y )
	{
		if( state )
			_buttonMask &= ~( 0x01 << button );
		else
			_buttonMask |= ( 0x01 << button );

		motion( glm::ivec2( x, y ) );
	}

	void Trackball::onMotion( int x, int y )
	{
		motion( glm::ivec2( x, y ) );
	}

	void Trackball::onPassiveMotion( int x, int y )
	{
		motion( glm::ivec2( x, y ) );
	}

	void Trackball::onMouseWheel( int wheel, int direction, int x, int y )
	{
		mouseWheelMotion( direction, glm::ivec2( x, y ) );
	}

	bool Trackball::keyDown( unsigned char key, int x, int y )
	{
		switch( key )
		{
		case 'r':
			reset();
			return true;
		}

		return false;
	}

	bool Trackball::keyUp( unsigned char key, int x, int y )
	{
		return false;
	}

	bool Trackball::specialDown( int key, int x, int y )
	{
		return false;
	}

	bool Trackball::specialUp( int key, int x, int y )
	{
		return false;
	}




	Trackball2D::Trackball2D( bool lockRotation ) :
		Trackball(),
		_relativeMode( false ),
		_trans( 0 ),
		_motionSpeed( 0.001f ),
		_rotation( 0 ),
		_rotationSpeed( 0.01f ),
		_scaleExp( 0 ),
		_scaleSpeed( 0.01f ),
		_wheelSpeed( 0.1f ),
		_scaleBase( 2.0f ),
		_lockRotation( lockRotation )
	{}

	Trackball2D::Trackball2D( const glm::vec3 &t, float r, float se, bool lockRotation ) :
		Trackball(),
		_relativeMode( false ),
		_trans( t ),
		_motionSpeed( 0.001f ),
		_rotation( r ),
		_rotationSpeed( 0.01f ),
		_scaleExp( se ),
		_scaleSpeed( 0.01f ),
		_wheelSpeed( 0.1f ),
		_scaleBase( 2.0f ),
		_defaultTrans( t ),
		_defaultRotation( r ),
		_defaultScaleExp( se ),
		_lockRotation( lockRotation )
	{
		reset();
	}

	Trackball2D::~Trackball2D()
	{}

	void Trackball2D::reset()
	{
		_trans = _defaultTrans;
		_rotation = _defaultRotation;
		_scaleExp = _defaultScaleExp;

		_m = glm::translate( _trans )
			* glm::rotate( _rotation, unitZ() )
			* glm::scale( glm::vec3( max( 0.000001f, pow( _scaleBase, _scaleExp ) ) ) );
	}

	void Trackball2D::motion( const glm::ivec2 &pos )
	{
		glm::ivec2 d( _first ? glm::ivec2( 0 ) : pos - _oldPos );

		if( _buttonMask & ( 0x01 << 0 ) && !_lockRotation ) //left
		{
			if( _relativeMode )
				_m = glm::rotate( -d.x * _rotationSpeed, unitZ() ) * _m;
			else
				_rotation += -d.x * _rotationSpeed;
		}
		if( _buttonMask & ( 0x01 << 1 ) ) //middle
		{
			if( _relativeMode )
				_m = glm::scale( glm::vec3( max( 0.000001f, pow( _scaleBase, -d.y * _scaleSpeed ) ) ) ) * _m;
			else
				_scaleExp += -d.y * _scaleSpeed;
		}
		if( _buttonMask & ( 0x01 << 2 ) ) //right
		{
			if( _relativeMode )
				_m = glm::translate( glm::vec3( d.x, -d.y, 0 ) * _motionSpeed ) * _m;
			else
				_trans += glm::vec3( d.x, -d.y, 0 ) * _motionSpeed;
		}

		if( _buttonMask )
			update();

		_oldPos = pos;
		_first = false;
	}

	void Trackball2D::mouseWheelMotion( int direction, const glm::ivec2 &pos )
	{
		if( _relativeMode )
			_m = glm::scale( glm::vec3( max( 0.000001f, pow( _scaleBase, direction * _wheelSpeed ) ) ) ) * _m;
		else
			_scaleExp += direction * _wheelSpeed;

		update();
		
		motion( pos );
	}

	void Trackball2D::update()
	{
		if( !_relativeMode )
		{
			_m = glm::translate( _trans )
				* glm::rotate( _rotation, unitZ() )
				* glm::scale( glm::vec3( max( 0.000001f, pow( _scaleBase, _scaleExp ) ) ) )
				;
		}
	}
}
