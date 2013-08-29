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

#include "qwt_plot_curve.h"
#include <QtCore/QVector>
#include <QtCore/QPointF>

#include "qwt_symbol.h"

#include <boost/unordered_map.hpp>
#include <boost/any.hpp>

#include <string>
#include <vector>

class QwtPlot;
class EProPlotMarker;
class EProPlotData;

class EProPlotCurve : public QwtPlotCurve
{
public:
	typedef std::vector<EProPlotMarker*> Markers;
	typedef boost::unordered_map<unsigned int, EProPlotCurve*> Dict;

public:
    EProPlotCurve(const std::string& szName, const QColor& color, int width, QwtPlot* pPlot);
    virtual ~EProPlotCurve();

	/**
	 * Clear all data.
	 */
	virtual void clear();

	/**
	 * Add a point to the curve with an optional user data.
	 */
	virtual void add(const QPointF& point, const boost::any& userData = boost::any());

	EProPlotMarker* createMarker(const QPointF& point, QwtSymbol::Style style, const QColor& inColor, const QColor& outColor, const QSize& size);
	EProPlotMarker* getMarker() const {return _pMarkerInformation;}

	double x(int index) const {return sample(index).x();}
	double y(int index) const {return sample(index).y();}


	const boost::any& getUserData(int i) const;

	void showMarkers(bool state = true);
	int updateMarkers(const QPoint& pos);

	void setMarkerInformationLabel(const std::string& szLabel);
	void setMarkerInformationOutlineColor(const QColor& qColorOutline);

	virtual void setVisible(bool state);
	void setColor(const QColor& color);

private:
	EProPlotMarker* _pMarkerInformation;
	Markers _markers;
};