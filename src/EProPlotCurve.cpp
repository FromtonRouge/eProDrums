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

#include "EProPlotCurve.h"
#include "EProPlotMarker.h"
#include "EProPlotData.h"
#include "qwt_plot.h"
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <algorithm>


EProPlotCurve::EProPlotCurve(const std::string& szName, const QColor& color, int width, QwtPlot* pPlot):
    QwtPlotCurve(szName.c_str())
{
	setStyle(Sticks);
	attach(pPlot);

	QPen pen(color);
	pen.setWidth(width);
	setPen(pen);

	_pMarkerInformation = new EProPlotMarker(pPlot, QwtSymbol::Diamond, color);
	setPaintAttribute(ClipPolygons, true);

	setData(new EProPlotData());
}

EProPlotCurve::~EProPlotCurve()
{
}

void EProPlotCurve::clear()
{
	_markers.clear();
	EProPlotData* pData = static_cast<EProPlotData*>(data());
	pData->clear();
}

void EProPlotCurve::add(const QPointF& point, const boost::any& userData)
{
	EProPlotData* pData = static_cast<EProPlotData*>(data());
	pData->add(point, userData);
}

const boost::any& EProPlotCurve::getUserData(int i) const
{
	EProPlotData* pData = static_cast<EProPlotData*>(const_cast<QwtSeriesData<QPointF>*>(data()));
	return pData->sampleUser(i);
}

void EProPlotCurve::showMarkers(bool state)
{
	_pMarkerInformation->setVisible(state);
	std::for_each(_markers.begin(), _markers.end(), boost::bind(&EProPlotMarker::setVisible, boost::lambda::_1, state));
}

int EProPlotCurve::updateMarkers(const QPoint& pos)
{
	bool bVisibleState = isVisible();
	int index = closestPoint(pos);
	if (index>=0)
	{
		const QPointF& point = sample(index);
		_pMarkerInformation->setVisible(isVisible());
		_pMarkerInformation->move(point.x(), point.y());
	}
	else
	{
		bVisibleState = false;
		_pMarkerInformation->setVisible(false);
	}

	std::for_each(_markers.begin(), _markers.end(), boost::bind(&EProPlotMarker::setVisible, boost::lambda::_1, bVisibleState));

	return index;
}

void EProPlotCurve::setMarkerInformationLabel(const std::string& szLabel)
{
	QwtText label(szLabel.c_str());
	label.setColor(QColor(Qt::white));
	_pMarkerInformation->setLabel(label);
}

void EProPlotCurve::setMarkerInformationOutlineColor(const QColor& qColorOutline)
{
	_pMarkerInformation->setOutlineColor(qColorOutline);
}

EProPlotMarker* EProPlotCurve::createMarker(const QPointF& point, QwtSymbol::Style style, const QColor& inColor, const QColor& outColor, const QSize& size)
{
	EProPlotMarker* pNewMarker = new EProPlotMarker(plot(), style, inColor, outColor, size);
	pNewMarker->move(point.x(), point.y());
	_markers.push_back(pNewMarker);
	return pNewMarker;
}

void EProPlotCurve::setVisible(bool state)
{
	showMarkers(state);
	QwtPlotCurve::setVisible(state);
}

void EProPlotCurve::setColor(const QColor& color)
{
	QPen p(pen());
	p.setColor(color);
	setPen(p);

	_pMarkerInformation->setColor(color);
}
