#pragma once

#include <patternsDouble.h>

namespace TextileUX
{
	class TiledPatternParams : public PatternParamsBase
	{
	public:
		int _tilesX;
		int _tilesY;
		float _tileDist;

		TiledPatternParams() :
			PatternParamsBase(),
			_tilesX( 2 ),
			_tilesY( 2 ),
			_tileDist( 1 )
		{}
		virtual ~TiledPatternParams() {}

		virtual bool drawUI();
	};
	/*
	class TiledPattern : public DoublePattern
	{
	protected:

		//virtual void clear();

		//virtual bool validate();

		virtual bool findShortcuts();

	public:
		TiledPattern( const std::string& name );
		virtual ~TiledPattern();

		virtual bool correct() { return true; }

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
			friend class DiamondZigZagTiled;

		private:
			int _windings;

		public:
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
	/*
	class DiamondSpiralDouble : public TiledPattern
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

	class AntennaDouble : public TiledPattern
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