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

#include <qwt_series_data.h>

#include <QtCore/QPointF>

#include <boost/any.hpp>

#include <vector>

class EProPlotData : public QwtSeriesData<QPointF>
{
public:
	typedef std::vector<QPointF> Data;
	typedef std::vector<boost::any> UserData;

public:
	EProPlotData();
	virtual ~EProPlotData();

	void add(const QPointF& point, const boost::any& userData = boost::any());
	void clear();
	const boost::any& sampleUser(size_t i) const {return _userData[i];}

	virtual size_t size() const {return _data.size();}
	virtual QPointF sample(size_t i) const {return _data[i];}
	virtual QRectF boundingRect() const;

private:
	Data _data;
	UserData _userData;
	double _yMin;
	double _yMax;
};
