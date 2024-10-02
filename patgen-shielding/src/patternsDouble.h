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

		bool findShortcuts();

	public:
		static glm::vec4 Color2;

		DoublePattern( const std::string& name );
		virtual ~DoublePattern();

		bool correct();

		virtual bool build( const PatternParamsBase *params );

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

	protected:
		virtual void updateSizeString();

	public:
		class PatternParams : public PatternParamsBase
		{
			friend class IDEDouble;

		private:
			int _teeth;

		public:
			PatternParams() :
				PatternParamsBase(),
				_teeth( 10 )
			{}

			virtual bool drawUI();

			float getLength() const {	return ( ( _teeth - 2 ) * _dist );	}
		};

		IDEDouble();
		virtual ~IDEDouble();

		virtual bool build( const PatternParamsBase* params );
		virtual std::string getFullName() const;
	};

	class BoustrophedonDouble : public DoublePattern
	{
	private:
		int _windings;
		float _dist;

	protected:
		virtual void updateSizeString();

	public:
		class PatternParams : public PatternParamsBase
		{
			friend class BoustrophedonDouble;

		private:
			int _windings;

		public:
			PatternParams() :
				PatternParamsBase(),
				_windings( 10 )
			{}

			virtual bool drawUI();
		};

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

	protected:
		virtual void updateSizeString();

	public:
		class PatternParams : public PatternParamsBase
		{
			friend class MeanderDouble;

		private:
			int _turns;

		public:
			PatternParams() :
				PatternParamsBase(),
				_turns( 10 )
			{}

			virtual bool drawUI();
		};

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

	protected:
		virtual void updateSizeString();

	public:
		class PatternParams : public PatternParamsBase
		{
			friend class SpiralDouble;

		private:
			int _turns;

		public:
			PatternParams() :
				PatternParamsBase(),
				_turns( 10 )
			{}

			virtual bool drawUI();
		};

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


	protected:
		virtual void updateSizeString();

	public:
		class PatternParams : public PatternParamsBase
		{
			friend class HilbertDouble;

		private:
			int _order;

		public:
			PatternParams() :
				PatternParamsBase(),
				_order( 4 )
			{}

			virtual bool drawUI();
		};

	public:
		HilbertDouble();
		virtual ~HilbertDouble();

		virtual bool build( const PatternParamsBase *params );
		virtual std::string getFullName() const;
	};

	class PeanoDouble : public DoublePattern
	{
	private:
		unsigned int _order;
		float _dist;

	protected:
		virtual void updateSizeString();

	public:
		class PatternParams : public PatternParamsBase
		{
			friend class PeanoDouble;

		private:
			int _order;

		public:
			PatternParams() :
				PatternParamsBase(),
				_order( 2 )
			{}

			virtual bool drawUI();
		};

		void recurse( int x, int y, int o, bool hFlip, bool vFlip, std::vector<glm::vec3>& verts );

	public:
		PeanoDouble();
		virtual ~PeanoDouble();

		virtual bool build( const PatternParamsBase *params );
		virtual std::string getFullName() const;
	};
	/*
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