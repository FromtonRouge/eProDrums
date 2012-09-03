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

struct LinearFunction
{
	typedef std::vector<LinearFunction> List;

	LinearFunction():_x1(0), _x2(30), _y1(50), _y2(50), _a(0), _b(0) {updateAandB();}

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

	const std::string& getName() const {return _name;}
	void setName(const std::string& szName) {_name = szName;}

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
	std::string _name;
	float _x1, _y1;
	float _x2, _y2;
	float _a, _b;

private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int)
	{
		ar  & BOOST_SERIALIZATION_NVP(_name);
		ar  & BOOST_SERIALIZATION_NVP(_x1);
		ar  & BOOST_SERIALIZATION_NVP(_y1);
		ar  & BOOST_SERIALIZATION_NVP(_x2);
		ar  & BOOST_SERIALIZATION_NVP(_y2);
		ar  & BOOST_SERIALIZATION_NVP(_a);
		ar  & BOOST_SERIALIZATION_NVP(_b);
	}
};

BOOST_CLASS_VERSION(LinearFunction, 0)
