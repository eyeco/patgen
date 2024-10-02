#include <trackball.h>

#include <glm/gtx/transform.hpp>

//test
#include <iostream>
//---

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
		_scale( 0 ),
		_scaleSpeed( 0.01f ),
		_wheelSpeed( 0.1f ),
		_scalePow( 2.0f ),
		_lockRotation( lockRotation )
	{}

	Trackball2D::Trackball2D( const glm::vec3 &t, float r, float s, bool lockRotation ) :
		Trackball(),
		_relativeMode( false ),
		_trans( t ),
		_motionSpeed( 0.001f ),
		_rotation( r ),
		_rotationSpeed( 0.01f ),
		_scale( s ),
		_scaleSpeed( 0.01f ),
		_wheelSpeed( 0.1f ),
		_scalePow( 2.0f ),
		_defaultTrans( t ),
		_defaultRotation( r ),
		_defaultScale( s ),
		_lockRotation( lockRotation )
	{
		reset();
	}

	Trackball2D::~Trackball2D()
	{}

	void Trackball2D::reset()
	{
		_m = glm::translate( _defaultTrans )
			* glm::rotate( _defaultRotation, unitZ() )
			* glm::scale( glm::vec3( max( 0.000001f, pow( _scalePow, _defaultScale ) ) ) );
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
				_m = glm::scale( glm::vec3( max( 0.000001f, pow( _scalePow, -d.y * _scaleSpeed ) ) ) ) * _m;
			else
				_scale += -d.y * _scaleSpeed;
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
			_m = glm::scale( glm::vec3( max( 0.000001f, pow( _scalePow, direction * _wheelSpeed ) ) ) ) * _m;
		else
			_scale += direction * _wheelSpeed;

		update();
		
		motion( pos );
	}

	void Trackball2D::update()
	{
		if( !_relativeMode )
		{
			_m = glm::translate( _trans )
				* glm::rotate( _rotation, unitZ() )
				* glm::scale( glm::vec3( max( 0.000001f, pow( _scalePow, _scale ) ) ) )
				;
		}
	}




	Trackball3D::Trackball3D() :
		Trackball(),
		_relativeMode( false ),
		_trans( 0 ),
		_motionSpeed( 0.01f ),
		_wheelSpeed( 0.1f ),
		_euler( 0 ),
		_rotationSpeed( 0.01f )
	{}

	Trackball3D::Trackball3D( const glm::vec3 &t, const glm::vec3 &e ) :
		Trackball(),
		_relativeMode( false ),
		_trans( t ),
		_motionSpeed( 0.01f ),
		_wheelSpeed( 0.1f ),
		_euler( e ),
		_rotationSpeed( 0.01f ),
		_defaultTrans( t ),
		_defaultEuler( e )
	{
		reset();
	}

	Trackball3D::~Trackball3D()
	{}

	void Trackball3D::reset()
	{
		_m = glm::translate( _defaultTrans )
			* glm::rotate( _defaultEuler.x, unitX() )
			* glm::rotate( _defaultEuler.y, unitY() )
			* glm::rotate( _defaultEuler.z, unitZ() );
	}

	void Trackball3D::motion( const glm::ivec2 &pos )
	{
		glm::ivec2 d( _first ? glm::ivec2( 0 ) : pos - _oldPos );

		if( _buttonMask & ( 0x01 << 0 ) ) //left
		{
			if( _relativeMode )
				_m = glm::rotate( d.x * _rotationSpeed, unitY() ) * glm::rotate( d.y * _rotationSpeed, unitX() ) * _m;
			else
				_euler += glm::vec3( d.y, d.x, 0 ) * _rotationSpeed;
		}
		if( _buttonMask & ( 0x01 << 1 ) ) //middle
		{
			if( _relativeMode )
				_m = glm::translate( glm::vec3( 0, 0, -d.y ) * _motionSpeed ) * _m;
			else
				_trans += glm::vec3( 0, 0, -d.y ) * _motionSpeed;
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

	void Trackball3D::mouseWheelMotion( int direction, const glm::ivec2 &pos )
	{
		if( _relativeMode )
			_m = glm::translate( glm::vec3( 0, 0, direction ) * _wheelSpeed ) * _m;
		else
			_trans += glm::vec3( 0, 0, direction ) * _wheelSpeed;
		update();

		motion( pos );
	}

	void Trackball3D::update()
	{
		if( !_relativeMode )
		{
			_m = glm::translate( _trans )
				* glm::rotate( _euler.x, unitX() )
				* glm::rotate( _euler.y, unitY() )
				* glm::rotate( _euler.z, unitZ() )
				;
		}
	}
}
