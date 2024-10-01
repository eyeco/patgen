#pragma once

#include <pattern.h>

namespace TextileUX
{
	class DoublePattern : public Pattern
	{
	protected:
		Trace _trace2;

		std::vector<glm::vec3> _shortcuts;

		virtual void clear();

		virtual bool validate();

	public:
		static glm::vec4 Color2;

		DoublePattern( const std::string& name );
		virtual ~DoublePattern();

		bool correct();

		virtual void draw();
		virtual bool save();

		float getTotalRunLength() const;
		size_t getTotalStitchCount() const;

		const Trace& getTrace2() const { return _trace2; }
		Trace& getTrace2() { return _trace2; }

		virtual void translate( const glm::vec3& t );
		virtual void rotate( float rad );

		virtual void rotate90CW();
		virtual void rotate90CCW();
		virtual void rotate180();
	};

	class IDEDouble : public DoublePattern
	{
	private:
		int _teeth;
		float _dist;

		virtual void updateSizeString();

	public:
		class PatternParams : public PatternParamsBase
		{
			friend class IDEDouble;

		private:
			int _teeth;
			float _dist;

		public:
			PatternParams() :
				PatternParamsBase(),
				_teeth( 5 ),
				_dist( 1 )
			{}

			virtual bool drawUI();

			float getLength() const {	return ( ( _teeth - 2 ) * _dist );	}
		};

		IDEDouble();
		virtual ~IDEDouble();

		virtual bool build( const PatternParamsBase* params );
		virtual std::string getFullName() const;
	};

	/*
	class BoustrophedonDouble : public DoublePattern
	{
	private:
		int _windings;
		float _dist;

	public:
		BoustrophedonDouble();
		virtual ~BoustrophedonDouble();

		virtual bool build( const PatternParamsBase *params );
		virtual std::string getFullName() const;
	};

	class MeanderDouble : public DoublePattern
	{
	private:
		int _turns;
		float _dist;

		bool _diagonalConnectors;
		float _cellDist;

	public:
		MeanderDouble();
		virtual ~MeanderDouble();

		virtual bool build( const PatternParamsBase *params );
		virtual std::string getFullName() const;
	};

	class SpiralDouble : public DoublePattern
	{
	private:
		int _turns;
		float _dist;
		bool _linkable;

	public:
		SpiralDouble();
		virtual ~SpiralDouble();

		virtual bool build( const PatternParamsBase *params );
		virtual std::string getFullName() const;
	};

	class HilbertDouble : public DoublePattern
	{
	private:
		unsigned int _order;
		float _dist;
		bool _linkable;

	public:
		HilbertDouble();
		virtual ~HilbertDouble();

		virtual bool build( const PatternParamsBase *params );
		virtual std::string getFullName() const;
	};

	class PeanoDoule : public DoublePattern
	{
	private:
		unsigned int _order;
		float _dist;
		bool _linkable;

		void recurse( int x, int y, int o, bool hFlip, bool vFlip, std::vector<glm::vec3>& verts );

	public:
		PeanoDoule();
		virtual ~PeanoDoule();

		virtual bool build( const PatternParamsBase *params );
		virtual std::string getFullName() const;
	};

	class DiamondZigZagDouble : public DoublePattern
	{
	private:
		unsigned int _windings;
		float _dist;

		float _cellDist;

	public:
		DiamondZigZagDouble();
		~DiamondZigZagDouble();

		virtual bool build( const PatternParamsBase *params );
		virtual std::string getFullName() const;
	};

	class DiamondSpiralDouble : public DoublePattern
	{
	private:
		unsigned int _turns;
		float _dist;

		float _cellDist;

	public:
		DiamondSpiralDouble();
		~DiamondSpiralDouble();

		virtual bool build( const PatternParamsBase *params );
		virtual std::string getFullName() const;
	};

	class AntennaDouble : public DoublePattern
	{
	private:
		unsigned int _order;
		float _dist;

		float _cellDist;

	public:
		AntennaDouble();
		~AntennaDouble();

		virtual bool build( const PatternParamsBase *params );
		virtual std::string getFullName() const;
	};
	*/
}