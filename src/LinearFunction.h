// ============================================================
// 
// This file is a part of the eProDrums rock band project
// 
// Copyright (C) 2012 by Vissale Neang <fromtonrouge at gmail dot com>
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License or (at your option) version 3 or any later version
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// ============================================================ 

#pragma once

#include <vector>
#include <string>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/shared_ptr.hpp>

struct LinearFunction
{
	struct Description
	{
		typedef boost::shared_ptr<Description> Ptr;
		Description():
			szLabelX("X"),
		   	szLabelY("Y"),
		   	xMin(0), xMax(127),
		   	yMin(0), yMax(127),
		   	x1Default(0), x2Default(30),
		   	y1Default(50), y2Default(50),
			xStep(1), yStep(1),
			aStep(0.01f), bStep(1),
			aDecimals(2)
	   	{}

		std::string szLabelX;
		std::string szLabelY;
		float xMin, xMax;
		float yMin, yMax;
		float x1Default, x2Default;
		float y1Default, y2Default;
		float xStep, yStep;
		float aStep, bStep;
		int aDecimals;
	};

	typedef std::vector<LinearFunction> List;

	static bool apply(const LinearFunction::List& functions, float x, float& y);

	LinearFunction(float x1=0.f, float x2=30.f, float y1=50.f, float y2=50.f):
		_x1(x1), _x2(x2),
	   	_y1(y1), _y2(y2),
	   	_a(0), _b(0)
   	{
		updateAandB();
	}

	/**
	 * \return true if x is between _x1 and _x2
	 */
	bool canApply(float x) const
	{
		return x>=_x1 && x<=_x2;
	}

	/**
	 * \return y=a*x+b
	 */
	float operator()(float x) const
	{
		return _a * x + _b;
	}

	void setPoints(float x1, float y1, float x2, float y2)
	{
		_x1 = x1;
		_y1 = y1;
		_x2 = x2;
		_y2 = y2;
		updateAandB();
	}

	void setA(float a)
	{
		_a = a;
		_y1 = _a*_x1+_b;
		_y2 = _a*_x2+_b;
	}

	void setB(float b)
	{
		_b = b;
		_y1 = _a*_x1+_b;
		_y2 = _a*_x2+_b;
	}

	float getX1() const {return _x1;}
	float getX2() const {return _x2;}
	float getY1() const {return _y1;}
	float getY2() const {return _y2;}

	void setX1(float value) {_x1 = value; updateAandB();}
	void setX2(float value) {_x2 = value; updateAandB();}
	void setY1(float value) {_y1 = value; updateAandB();}
	void setY2(float value) {_y2 = value; updateAandB();}

	float getA() const {return _a;}
	float getB() const {return _b;}

private:
	void updateAandB()
	{
		_a= (_y2-_y1)/(_x2-_x1);
		_b = _y1 - _a*_x1;
	}

private:
	float _x1, _y1;
	float _x2, _y2;
	float _a, _b;

private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int)
	{
		ar  & BOOST_SERIALIZATION_NVP(_x1);
		ar  & BOOST_SERIALIZATION_NVP(_y1);
		ar  & BOOST_SERIALIZATION_NVP(_x2);
		ar  & BOOST_SERIALIZATION_NVP(_y2);
		ar  & BOOST_SERIALIZATION_NVP(_a);
		ar  & BOOST_SERIALIZATION_NVP(_b);
	}
};

BOOST_CLASS_VERSION(LinearFunction, 0)
