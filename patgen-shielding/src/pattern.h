#pragma once

#include <vector>
#include <vertexBuffer.h>

namespace TextileUX
{
	class Trace
	{
	private:
		glm::vec4 _color;

		std::vector<glm::vec3> _verts;		//minimum required stitches (i.e. trail edges)
		std::vector<glm::vec3> _stitches;	//actual stitches, resulting from resampling, e.g. due to maximum jump size supported by machine

		float _jumpSize;
		float _minJumpSize;

		float _runLength;

		VertexBuffer _vbPath;
		VertexBuffer _vbStitches;

		bool resample();
		bool rebuildVBOs();

	public:
		explicit Trace( const glm::vec4 &color );

		void draw();

		void clear();
		bool rebuild( float jumpSize, float minJumpSize );

		float getJumpSize() const { return _jumpSize; }

		float getRunLength() const { return _runLength; }
		unsigned int getVertexCount() const { return _verts.size(); }
		unsigned int getStitchCount() const { return _stitches.size(); }

		void insertBack( const glm::vec3 &v );
		void insertFront( const glm::vec3 &v );

		bool removeStitch( int index );
		bool removeStitches( std::vector<int> indices );

		std::vector<glm::vec3> &getVerts() { return _verts; }
		const std::vector<glm::vec3> &getVerts() const { return _verts; }

		std::vector<glm::vec3> &getStitches() { return _stitches; }
		const std::vector<glm::vec3> &getStitches() const { return _stitches; }
	};

	class PatternParamsBase
	{
	protected:
		bool _invalidated;

	public:
		float _jumpSize;
		float _minJumpSize;
		float _dist;

		PatternParamsBase() :
			_invalidated( false ),
			_jumpSize( 1 ),
			_minJumpSize( 0 ),
			_dist( 1 )
		{}
		virtual ~PatternParamsBase() {}

		virtual bool drawUI();
	};

	class Pattern
	{
	protected:
		std::string _name;

		Trace _trace;

		std::string _sizeString;

		Unit _unit;

		virtual void clear();

		bool validate();

		virtual void updateSizeString() = 0;

	public:
		static float Epsilon;
		static float DistanceTolerance;

		static glm::vec4 Color;

		Pattern( const std::string &name );
		virtual ~Pattern();

		void setUnit( Unit unit) { _unit = unit; }
		Unit getUnit() const { return _unit; }

		virtual bool build( const PatternParamsBase *params );
		virtual std::string getFullName() const = 0;

		void draw();
		bool save();

		float getTotalRunLength() const { return _trace.getRunLength(); }
		unsigned int getTotalStitchCount() const { return _trace.getStitchCount(); }

		const std::string &getSizeString() const { return _sizeString; }

		const Trace &getTrace() const { return _trace; }
		Trace &getLowerTrace() { return _trace; }

		const std::string &getName() const { return _name; }

		void translate( const glm::vec3 &t );
		void rotate( float rad );

		void rotate90CW();
		void rotate90CCW();
		void rotate180();
	};

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

		virtual bool build( const PatternParamsBase *params );
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

		virtual bool build( const PatternParamsBase *params );
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

		virtual bool build( const PatternParamsBase *params );
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

		virtual bool build( const PatternParamsBase *params );
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

		virtual bool build( const PatternParamsBase *params );
		virtual std::string getFullName() const;
	};
}