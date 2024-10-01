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
			friend class BoustrophedonCircle;

		private:
			float _diameter;

		public:
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
			friend class SpiralCircle;

		private:
			float _diameter;
			float _innerDiameter;
			float _innerJumpSize;

		public:
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
			friend class BoustrophedonQuadOrtho;

		private:
			float _width;

		public:
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
			friend class BoustrophedonQuadDiag;

		private:
			float _width;

		public:
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
			friend class BoustrophedonQuadDouble;

		private:
			float _width;
			int _jumpMult;

		public:
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