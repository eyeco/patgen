#pragma once

#include <pattern.h>

namespace TextileUX
{
	class BoustrophedonCircle : public Pattern
	{
	private:
		float _diameter;
		float _dist;
		float _rMax;

		virtual void clear();

		virtual void updateSizeString();

	public:
		class PatternParams : public PatternParamsBase
		{
		public:
			float _diameter;

			PatternParams() :
				PatternParamsBase(),
				_diameter( 10 )
			{}

			virtual bool drawUI();
		};


		BoustrophedonCircle();
		virtual ~BoustrophedonCircle();

		virtual bool build( const PatternParamsBase* params );
		virtual std::string getFullName() const;
	};

	class SpiralCircle : public Pattern
	{
	private:
		float _diameter;
		float _dist;
		float _innerDiameter;

		float _innerJumpSize;

		glm::vec3 _first;
		glm::vec3 _last;

		virtual void clear();

		virtual void updateSizeString();

	public:
		class PatternParams : public PatternParamsBase
		{
		public:
			float _diameter;
			float _innerDiameter;
			float _innerJumpSize;

			PatternParams() :
				PatternParamsBase(),
				_diameter( 10 ),
				_innerDiameter( 0 ),
				_innerJumpSize( 0.1 )
			{}

			virtual bool drawUI();
		};

		SpiralCircle();
		virtual ~SpiralCircle();

		virtual bool build( const PatternParamsBase* params );
		virtual std::string getFullName() const;
	};

	class BoustrophedonQuadOrtho : public Pattern
	{
	private:
		float _width;
		float _dist;

		virtual void clear();

		virtual void updateSizeString();

	public:
		class PatternParams : public PatternParamsBase
		{
		public:
			float _width;

			PatternParams() :
				PatternParamsBase(),
				_width( 10 )
			{}

			virtual bool drawUI();
		};

		BoustrophedonQuadOrtho();
		virtual ~BoustrophedonQuadOrtho();

		virtual bool build( const PatternParamsBase* params );
		virtual std::string getFullName() const;
	};

	class BoustrophedonQuadDiag : public Pattern
	{
	private:
		float _width;
		float _dist;

		virtual void clear();

		virtual void updateSizeString();

	public:
		class PatternParams : public PatternParamsBase
		{
		public:
			float _width;

			PatternParams() :
				PatternParamsBase(),
				_width( 10 )
			{}

			virtual bool drawUI();
		};

		BoustrophedonQuadDiag();
		virtual ~BoustrophedonQuadDiag();

		virtual bool build( const PatternParamsBase* params );
		virtual std::string getFullName() const;
	};

	class BoustrophedonQuadDouble : public Pattern
	{
	private:
		float _width;
		float _dist;

		virtual void clear();

		virtual void updateSizeString();

	public:
		class PatternParams : public PatternParamsBase
		{
		public:
			float _width;
			int _jumpMult;

			PatternParams() :
				PatternParamsBase(),
				_width( 10 ),
				_jumpMult( 1 )
			{
				_jumpSize = _dist * _jumpMult;
			}

			virtual bool drawUI();
		};

		explicit BoustrophedonQuadDouble();
		virtual ~BoustrophedonQuadDouble();

		virtual bool build( const PatternParamsBase* params );
		virtual std::string getFullName() const;
	};
}