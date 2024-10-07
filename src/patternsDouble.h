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

#include <pattern.h>

namespace TextileUX
{
	class DoublePattern : public Pattern
	{
	protected:
		float _width;

		Trace _trace2;

		std::vector<glm::vec3> _shortcuts;

		virtual void clear();

		virtual bool validate();

		virtual bool findShortcuts();

	public:
		static glm::vec4 Color2;

		DoublePattern( const std::string& name );
		virtual ~DoublePattern();

		virtual bool correct();

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
		public:
			int _teeth;

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
		public:
			int _windings;

			PatternParams() :
				PatternParamsBase(),
				_windings( 5 )
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
		public:
			int _turns;

			PatternParams() :
				PatternParamsBase(),
				_turns( 5 )
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
		public:
			int _turns;

			PatternParams() :
				PatternParamsBase(),
				_turns( 5 )
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
		public:
			int _order;

			PatternParams() :
				PatternParamsBase(),
				_order( 3 )
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
		public:
			int _order;

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