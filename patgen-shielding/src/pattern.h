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
		Trace( float jumpSize, float minJumpSize, const glm::vec4 &color );

		void draw();

		void clear();
		bool rebuild();

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

	class Pattern
	{
	protected:
		std::string _name;

		Trace _trace;

		std::string _sizeString;

		void clear();
		bool rebuild();

		bool validate();

		virtual void updateSizeString() = 0;

	public:
		static float Epsilon;
		static float DistanceTolerance;

		static glm::vec4 Color;

		explicit Pattern( const std::string &name, float jumpSize, float minJumpSize = 0 );
		virtual ~Pattern();

		virtual bool build() = 0;
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

		virtual void updateSizeString();

	public:
		BoustrophedonCircle( float diameter, float dist, float jumpSize, float minJumpSize = 0 );
		virtual ~BoustrophedonCircle();

		virtual bool build();
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

		virtual void updateSizeString();

	public:
		SpiralCircle( float diameter, float dist, float outerJumpSize, float innerJumpSize, float innerDiameter = 0, float minJumpSize = 0 );
		virtual ~SpiralCircle();

		virtual bool build();
		virtual std::string getFullName() const;
	};

	class BoustrophedonQuadOrtho : public Pattern
	{
	private:
		float _width;
		float _dist;

		virtual void updateSizeString();

	public:
		BoustrophedonQuadOrtho( float width, float dist, float jumpSize, float minJumpSize = 0 );
		virtual ~BoustrophedonQuadOrtho();

		virtual bool build();
		virtual std::string getFullName() const;
	};

	class BoustrophedonQuadDiag : public Pattern
	{
	private:
		float _width;
		float _dist;

		virtual void updateSizeString();

	public:
		BoustrophedonQuadDiag( float width, float dist, float jumpSize, float minJumpSize = 0 );
		virtual ~BoustrophedonQuadDiag();

		virtual bool build();
		virtual std::string getFullName() const;
	};

	class BoustrophedonQuadDouble : public Pattern
	{
	private:
		float _width;
		float _dist;

		virtual void updateSizeString();

	public:
		BoustrophedonQuadDouble( float width, float dist, int jumpMult, float minJumpSize = 0 );
		virtual ~BoustrophedonQuadDouble();

		virtual bool build();
		virtual std::string getFullName() const;
	};
}