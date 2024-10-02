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

		bool validate();

		void clear();
		bool rebuild( float jumpSize, float minJumpSize );

		float getJumpSize() const { return _jumpSize; }

		float getRunLength() const { return _runLength; }
		size_t getVertexCount() const { return _verts.size(); }
		size_t getStitchCount() const { return _stitches.size(); }

		void insertBack( const glm::vec3 &v );
		void insertFront( const glm::vec3 &v );

		bool removeStitch( int index );
		bool removeStitches( std::vector<int> indices );

		std::vector<glm::vec3> &getVerts() { return _verts; }
		const std::vector<glm::vec3> &getVerts() const { return _verts; }

		std::vector<glm::vec3> &getStitches() { return _stitches; }
		const std::vector<glm::vec3> &getStitches() const { return _stitches; }

		void translate( const glm::vec3& t );
		void rotate( float rad );

		void rotate90CW();
		void rotate90CCW();
		void rotate180();
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

		virtual bool validate();

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

		virtual void draw();
		virtual bool save();

		const std::string &getSizeString() const { return _sizeString; }

		const Trace &getTrace() const { return _trace; }
		Trace &getTrace() { return _trace; }

		const std::string &getName() const { return _name; }

		virtual void translate( const glm::vec3 &t );
		virtual void rotate( float rad );

		virtual void rotate90CW();
		virtual void rotate90CCW();
		virtual void rotate180();
	};
}