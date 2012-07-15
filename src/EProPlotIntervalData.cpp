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

#include "EProPlotIntervalData.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>

EProPlotIntervalData::EProPlotIntervalData()
{
}

EProPlotIntervalData::~EProPlotIntervalData()
{
}

QRectF EProPlotIntervalData::boundingRect() const
{
	QRectF result;
	return result;
}

void EProPlotIntervalData::add(const QwtIntervalSample& interval)
{
	_data.push_back(interval);
}

void EProPlotIntervalData::clear()
{
	_data.clear();
}

void EProPlotIntervalData::increaseAllIntervallsOnTheLeft(double width)
{
	Data::iterator it = _data.begin();
	while (it!=_data.end())
	{
		QwtIntervalSample& v = *(it++);
		v.interval.setMinValue(v.interval.maxValue()-width);
	}
}

void EProPlotIntervalData::increaseAllIntervallsOnTheRight(double width)
{
	Data::iterator it = _data.begin();
	while (it!=_data.end())
	{
		QwtIntervalSample& v = *(it++);
		v.interval.setMaxValue(v.interval.minValue()+width);
	}
}

void EProPlotIntervalData::changeAllValues(double value)
{
	Data::iterator it = _data.begin();
	while (it!=_data.end())
	{
		QwtIntervalSample& v = *(it++);
		v.value = value;
	}
}
