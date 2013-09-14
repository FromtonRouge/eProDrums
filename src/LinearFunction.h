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

#include <QtCore/QPointF>
#include <QtGui/QPolygonF>

#include <string>

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

	static bool apply(const QPolygonF& curveSamples, float x, float& y);

	LinearFunction(const QPointF& point1, const QPointF& point2)
		: _point1(point1)
		, _point2(point2)
		, _a(0), _b(0)
	{
		updateAandB();
	}

	/**
	 * \return y=a*x+b
	 */
	float operator()(float x) const { return _a * x + _b; }

private:
	void updateAandB()
	{
		_a= (_point2.y()-_point1.y())/(_point2.x()-_point1.x());
		_b = _point1.y() - _a*_point1.x();
	}

private:
	QPointF	_point1;
	QPointF	_point2;
	float _a, _b;
};
