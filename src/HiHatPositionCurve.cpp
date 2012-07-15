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

#include "HiHatPositionCurve.h"
#include "EProPlotIntervalData.h"
#include "qwt_plot_intervalcurve.h"
#include "qwt_plot_histogram.h"
#include "qwt_plot.h"

HiHatPositionCurve::HiHatPositionCurve(QwtPlot* pPlot):
	EProPlotCurve("Hi Hat Position", QColor(245, 255, 166), 1, pPlot),
	_blueState(false),
	_previousBlueState(false),
	_bShowFootCancelStragegy1Info(true),
	_bShowBlueState(true)
{
	setMarkerInformationOutlineColor(QColor(Qt::red));
	setStyle(EProPlotCurve::Lines);
	QColor qHiHatControl(245, 255, 166);
	setSymbol(new QwtSymbol(QwtSymbol::Ellipse, qHiHatControl, qHiHatControl, QSize(2,2)));

    QColor colorMask(Qt::magenta);
	colorMask.setAlpha(150);
	_pLayerFootCancelMaskTime1 = new QwtPlotHistogram("Foot Cancel Mask Time 1");
	_pLayerFootCancelMaskTime1->setPen(QPen(QColor(Qt::magenta)));
	_pLayerFootCancelMaskTime1->setBrush(colorMask);
    _pLayerFootCancelMaskTime1->setStyle(QwtPlotHistogram::Outline);
	_pLayerFootCancelMaskTime1->setData(new EProPlotIntervalData());
	_pLayerFootCancelMaskTime1->setItemAttribute(Legend, false);
	_pLayerFootCancelMaskTime1->attach(pPlot);

	_pLayerBlueOnInfo = new QwtPlotIntervalCurve("Hi Hat Blue On");
    QColor blue(100, 150, 255);
	blue.setAlpha(150);
	_pLayerBlueOnInfo->setBrush(blue);
    _pLayerBlueOnInfo->setStyle(QwtPlotIntervalCurve::Tube);
	_pLayerBlueOnInfo->setData(new EProPlotIntervalData());
	_pLayerBlueOnInfo->setItemAttribute(Legend, false);
	_pLayerBlueOnInfo->attach(pPlot);

	_pLayerBlueOffInfo = new QwtPlotIntervalCurve("Hi Hat Blue Off");
	QColor yellow(255, 255, 150);
	yellow.setAlpha(150);
	_pLayerBlueOffInfo->setBrush(yellow);
    _pLayerBlueOffInfo->setStyle(QwtPlotIntervalCurve::Tube);
	_pLayerBlueOffInfo->setData(new EProPlotIntervalData());
	_pLayerBlueOffInfo->setItemAttribute(Legend, false);
	_pLayerBlueOffInfo->attach(pPlot);
	
	_pLayerOpenInfo = new EProPlotCurve("Hi Hat Open Start", QColor(Qt::white), 1, pPlot);
	_pLayerOpenInfo->setStyle(EProPlotCurve::NoCurve);
	_pLayerOpenInfo->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QColor(Qt::green), QColor(Qt::white), QSize(5,5)));
	_pLayerOpenInfo->setItemAttribute(Legend, false);
	_pLayerCloseInfo = new EProPlotCurve("Hi Hat Close Start", QColor(Qt::white), 1, pPlot);
	_pLayerCloseInfo->setStyle(EProPlotCurve::NoCurve);
	_pLayerCloseInfo->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QColor(Qt::red), QColor(Qt::white), QSize(5,5)));
	_pLayerCloseInfo->setItemAttribute(Legend, false);
}

HiHatPositionCurve::~HiHatPositionCurve()
{
}

void HiHatPositionCurve::addOpenInfo(const QPointF& point)
{
	_pLayerOpenInfo->add(point);
}

void HiHatPositionCurve::addCloseInfo(const QPointF& point)
{
	_pLayerCloseInfo->add(point);
}

void HiHatPositionCurve::setVisible(bool state)
{
	_pLayerBlueOnInfo->setVisible(state&&_bShowBlueState);
	_pLayerBlueOffInfo->setVisible(state&&_bShowBlueState);

	_pLayerOpenInfo->setVisible(state&&_bShowFootCancelStragegy1Info);
	_pLayerCloseInfo->setVisible(state&&_bShowFootCancelStragegy1Info);
	_pLayerFootCancelMaskTime1->setVisible(state&&_bShowFootCancelStragegy1Info);

	EProPlotCurve::setVisible(state);
}

void HiHatPositionCurve::clear()
{
	static_cast<EProPlotIntervalData*>(_pLayerFootCancelMaskTime1->data())->clear();
	static_cast<EProPlotIntervalData*>(_pLayerBlueOnInfo->data())->clear();
	static_cast<EProPlotIntervalData*>(_pLayerBlueOffInfo->data())->clear();

	_pLayerOpenInfo->clear();
	_pLayerCloseInfo->clear();
	EProPlotCurve::clear();
}

void HiHatPositionCurve::add(const QPointF& point, const boost::any& userData)
{
	EProPlotIntervalData* pDataBlueOn = static_cast<EProPlotIntervalData*>(_pLayerBlueOnInfo->data());
	EProPlotIntervalData* pDataBlueOff = static_cast<EProPlotIntervalData*>(_pLayerBlueOffInfo->data());

	if (_blueState)
	{
		// Previous state was yellow
		if (!_previousBlueState)
		{
			pDataBlueOff->add(QwtIntervalSample(point.x(), 0, point.y()));
			pDataBlueOn->add(QwtIntervalSample(point.x(), 0, 0));
		}
		pDataBlueOn->add(QwtIntervalSample(point.x(), 0, point.y()));
		pDataBlueOff->add(QwtIntervalSample(point.x(), 0, 0));
	}
	else
	{
		// Previous state was blue
		if (_previousBlueState)
		{
			pDataBlueOff->add(QwtIntervalSample(point.x(), 0, 0));
			pDataBlueOn->add(QwtIntervalSample(point.x(), 0, point.y()));
		}
		pDataBlueOff->add(QwtIntervalSample(point.x(), 0, point.y()));
		pDataBlueOn->add(QwtIntervalSample(point.x(), 0, 0));
	}

	_previousBlueState = _blueState;

	EProPlotCurve::add(point, userData);
}

void HiHatPositionCurve::addFootCancelStrategy1Info(int startTime, int maskLength, int velocity)
{
	EProPlotIntervalData* pDataFootCancel = static_cast<EProPlotIntervalData*>(_pLayerFootCancelMaskTime1->data());
	pDataFootCancel->add(QwtIntervalSample(velocity, startTime, startTime+maskLength));
}

void HiHatPositionCurve::changeFootCancelStrategy1MaskLength(int maskLength)
{
	EProPlotIntervalData* pDataFootCancel = static_cast<EProPlotIntervalData*>(_pLayerFootCancelMaskTime1->data());
	pDataFootCancel->increaseAllIntervallsOnTheRight(maskLength);
	plot()->replot();
}

void HiHatPositionCurve::changeFootCancelStrategy1MaskVelocity(int velocity)
{
	EProPlotIntervalData* pDataFootCancel = static_cast<EProPlotIntervalData*>(_pLayerFootCancelMaskTime1->data());
	pDataFootCancel->changeAllValues(velocity);
	plot()->replot();
}

void HiHatPositionCurve::showBlueState(bool state)
{
	_bShowBlueState = state;
	_pLayerBlueOnInfo->setVisible(state&&isVisible());
	_pLayerBlueOffInfo->setVisible(state&&isVisible());
	plot()->replot();
}

void HiHatPositionCurve::showFootCancelStrategy1Info(bool state)
{
	_bShowFootCancelStragegy1Info = state;
	_pLayerOpenInfo->setVisible(state&&isVisible());
	_pLayerCloseInfo->setVisible(state&&isVisible());
	_pLayerFootCancelMaskTime1->setVisible(state&&isVisible());
	plot()->replot();
}
