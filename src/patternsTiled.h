#pragma once

#include <patternsDouble.h>

namespace TextileUX
{
	class TiledPatternParams : public PatternParamsBase
	{
	public:
		int _cellsX;
		int _cellsY;
		float _cellDist;

		TiledPatternParams() :
			PatternParamsBase(),
			_cellsX( 2 ),
			_cellsY( 2 ),
			_cellDist( 1 )
		{}
		virtual ~TiledPatternParams() {}

		virtual bool drawUI();
	};

	class TiledPattern : public DoublePattern
	{
	protected:

		virtual void clear();

		std::vector<Trace*> _traces;	//all lower traces
		std::vector<Trace*> _traces2;	//all upper traces

		virtual bool findShortcuts();

		void swipeTraces();

	public:
		TiledPattern( const std::string& name );
		virtual ~TiledPattern();

		virtual bool correct() { return true; }

		virtual void draw();
		virtual bool save();

		float getTotalRunLength() const;
		size_t getTotalStitchCount() const;

		virtual void translate( const glm::vec3& t );
		virtual void rotate( float rad );

		virtual void rotate90CW();
		virtual void rotate90CCW();
		virtual void rotate180();
	};

	class DiamondZigZagTiled : public TiledPattern
	{
	private:
		unsigned int _windings;
		float _dist;

		float _cellDist;

	protected:
		virtual void updateSizeString();

	public:
		class PatternParams : public TiledPatternParams
		{
		public:
			int _windings;

			PatternParams() :
				TiledPatternParams(),
				_windings( 10 )
			{}

			virtual bool drawUI();
		};

	public:
		DiamondZigZagTiled();
		~DiamondZigZagTiled();

		virtual bool build( const PatternParamsBase *params );
		virtual std::string getFullName() const;
	};

	class DiamondSpiralTiled : public TiledPattern
	{
	private:
		unsigned int _turns;
		float _dist;

		float _cellDist;

	protected:
		virtual void updateSizeString();

	public:
		class PatternParams : public TiledPatternParams
		{
		public:
			int _turns;

			PatternParams() :
				TiledPatternParams(),
				_turns( 10 )
			{}

			virtual bool drawUI();
		};

	public:
		DiamondSpiralTiled();
		~DiamondSpiralTiled();

		virtual bool build( const PatternParamsBase *params );
		virtual std::string getFullName() const;
	};

	class MeanderTiled : public TiledPattern
	{
	private:
		unsigned int _turns;
		float _dist;

		float _cellDist;

	protected:
		virtual void updateSizeString();

	public:
		class PatternParams : public TiledPatternParams
		{
		public:
			int _turns;

			PatternParams() :
				TiledPatternParams(),
				_turns( 4 )
			{}

			virtual bool drawUI();
		};

	public:
		MeanderTiled();
		~MeanderTiled();

		virtual bool build( const PatternParamsBase* params );
		virtual std::string getFullName() const;
	};
	
	class AntennaTiled : public TiledPattern
	{
	private:
		unsigned int _order;
		float _dist;

		float _cellDist;

	protected:
		virtual void updateSizeString();

	public:
		class PatternParams : public TiledPatternParams
		{
		public:
			int _order;

			PatternParams() :
				TiledPatternParams(),
				_order( 5 )
			{}

			virtual bool drawUI();
		};

	public:
		AntennaTiled();
		~AntennaTiled();

		virtual bool build( const PatternParamsBase *params );
		virtual std::string getFullName() const;
	};

	class FlowerTiled : public TiledPattern
	{
	private:
		unsigned int _turns;
		float _dist;

		float _cellDist;

	protected:
		virtual void updateSizeString();

	public:
		class PatternParams : public TiledPatternParams
		{
		public:
			int _turns;

			PatternParams() :
				TiledPatternParams(),
				_turns( 4 )
			{}

			virtual bool drawUI();
		};

	public:
		FlowerTiled();
		~FlowerTiled();

		virtual bool build( const PatternParamsBase* params );
		virtual std::string getFullName() const;
	};
}