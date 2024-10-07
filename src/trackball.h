/*---------------------------------------------------------------------------------------------
* Copyright (C) 2024 eyeco https://github.com/eyeco https://www.rolandaigner.com
* This file is part of patgen
*
* Licensed under the GPL3 License. See LICENSE file in the package root for license information.
*
* You should have received a copy of the GNU General Public License
* along with this code. If not, see < http://www.gnu.org/licenses/>.
*--------------------------------------------------------------------------------------------*/


#pragma once

#include <common.h>

#include <glm/glm.hpp>

namespace TextileUX
{
	class Trackball
	{
	protected:
		bool _first;
		glm::ivec2 _oldPos;
		unsigned int _buttonMask;

		glm::mat4 _m;

		virtual void update() = 0;
		virtual void motion( const glm::ivec2 &pos ) {}
		virtual void mouseWheelMotion( int direction, const glm::ivec2 &pos ) {}

		virtual bool keyDown( unsigned char key, int x, int y );
		virtual bool keyUp( unsigned char key, int x, int y );
		virtual bool specialDown( int key, int x, int y );
		virtual bool specialUp( int key, int x, int y );

	public:
		Trackball();
		~Trackball();

		virtual void reset() = 0;

		void onMouse( int button, int state, int x, int y );
		void onMouseWheel( int wheel, int direction, int x, int y );

		void onMotion( int x, int y );
		void onPassiveMotion( int x, int y );

		bool onKeyDown( unsigned char key, int x, int y ) { return keyDown( key, x, y ); }
		bool onKeyUp( unsigned char key, int x, int y ) { return keyUp( key, x, y ); }
		bool onSpecialDown( int key, int x, int y ) { return specialDown( key, x, y ); }
		bool onSpecialUp( int key, int x, int y ) { return specialUp( key, x, y ); }

		const glm::mat4 &mat() const { return _m; }
	};

	class Trackball2D : public Trackball
	{
	private:
		bool _relativeMode;

		glm::vec3 _trans;
		float _motionSpeed;

		float _rotation;
		float _rotationSpeed;

		float _scaleExp;
		float _scaleSpeed;
		float _wheelSpeed;
		float _scaleBase;

		glm::vec3 _defaultTrans;
		float _defaultRotation;
		float _defaultScaleExp;

		bool _lockRotation;

		virtual void update();
		virtual void motion( const glm::ivec2 &pos );
		virtual void mouseWheelMotion( int direction, const glm::ivec2 &pos );

	public:
		explicit Trackball2D( bool lockRotation = true );
		Trackball2D( const glm::vec3 &t, float r, float s, bool lockRotation = true );
		~Trackball2D();

		virtual void reset();

		const glm::vec3& getTranslation() const { return _trans; }
		float getRotation() const { return _rotation; }
		float getScaleExp() const { return _scaleExp; }
	};
}