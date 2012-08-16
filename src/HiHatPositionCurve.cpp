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
	_hiHatState(HHS_CLOSED),
	_previousHiHatState(HHS_CLOSED),
	_isFootCancelActivated(true),
	_isHiHatStatesActivated(true),
	_isFootCancelLayersShown(true),
	_isHiHatLayersShown(true)
{
	setMarkerInformationOutlineColor(QColor(Qt::red));
	setStyle(EProPlotCurve::Lines);
	QColor qHiHatControl(245, 255, 166);
	setSymbol(new QwtSymbol(QwtSymbol::Ellipse, qHiHatControl, qHiHatControl, QSize(2,2)));

    QColor colorMask(Qt::magenta);
	colorMask.setAlpha(150);
	_pLayerFootCanceMaskTime = new QwtPlotHistogram("Foot Cancel Mask Time");
	_pLayerFootCanceMaskTime->setPen(QPen(QColor(Qt::magenta)));
	_pLayerFootCanceMaskTime->setBrush(colorMask);
    _pLayerFootCanceMaskTime->setStyle(QwtPlotHistogram::Outline);
	_pLayerFootCanceMaskTime->setData(new EProPlotIntervalData());
	_pLayerFootCanceMaskTime->setItemAttribute(Legend, false);
	_pLayerFootCanceMaskTime->attach(pPlot);

	_pLayerStateSecured = new QwtPlotIntervalCurve("Hi Hat secured");
	QColor secured(255, 255, 150);
	secured.setAlpha(170);
	_pLayerStateSecured->setBrush(secured);
    _pLayerStateSecured->setStyle(QwtPlotIntervalCurve::Tube);
	_pLayerStateSecured->setData(new EProPlotIntervalData());
	_pLayerStateSecured->setItemAttribute(Legend, false);
	_pLayerStateSecured->attach(pPlot);

	_pLayerStateClosed = new QwtPlotIntervalCurve("Hi Hat closed");
	QColor yellow(255, 255, 150);
	yellow.setAlpha(150);
	_pLayerStateClosed->setBrush(yellow);
    _pLayerStateClosed->setStyle(QwtPlotIntervalCurve::Tube);
	_pLayerStateClosed->setData(new EProPlotIntervalData());
	_pLayerStateClosed->setItemAttribute(Legend, false);
	_pLayerStateClosed->attach(pPlot);
	
	_pLayerStateHalfOpen = new QwtPlotIntervalCurve("Hi Hat half open");
	QColor green(150, 255, 150);
	green.setAlpha(150);
	_pLayerStateHalfOpen->setBrush(green);
    _pLayerStateHalfOpen->setStyle(QwtPlotIntervalCurve::Tube);
	_pLayerStateHalfOpen->setData(new EProPlotIntervalData());
	_pLayerStateHalfOpen->setItemAttribute(Legend, false);
	_pLayerStateHalfOpen->attach(pPlot);
	
	_pLayerStateOpen = new QwtPlotIntervalCurve("Hi Hat open");
    QColor blue(100, 150, 255);
	blue.setAlpha(150);
	_pLayerStateOpen->setBrush(blue);
    _pLayerStateOpen->setStyle(QwtPlotIntervalCurve::Tube);
	_pLayerStateOpen->setData(new EProPlotIntervalData());
	_pLayerStateOpen->setItemAttribute(Legend, false);
	_pLayerStateOpen->attach(pPlot);

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

void HiHatPositionCurve::updateHiHatLayers()
{
	bool bHiHatStates = _isHiHatLayersShown&&_isHiHatStatesActivated&&isVisible();
	_pLayerStateSecured ->setVisible(bHiHatStates);
	_pLayerStateClosed->setVisible(bHiHatStates);
	_pLayerStateHalfOpen->setVisible(bHiHatStates);
	_pLayerStateOpen->setVisible(bHiHatStates);
}

void HiHatPositionCurve::updateFootCancelLayers()
{
	bool bFootCancelInfo = _isFootCancelLayersShown&&_isFootCancelActivated&&isVisible();
	_pLayerOpenInfo->setVisible(bFootCancelInfo);
	_pLayerCloseInfo->setVisible(bFootCancelInfo);
	_pLayerFootCanceMaskTime->setVisible(bFootCancelInfo);
}

void HiHatPositionCurve::setVisible(bool state)
{
	EProPlotCurve::setVisible(state);
	updateHiHatLayers();
	updateFootCancelLayers();
}

void HiHatPositionCurve::clear()
{
	static_cast<EProPlotIntervalData*>(_pLayerFootCanceMaskTime->data())->clear();
	static_cast<EProPlotIntervalData*>(_pLayerStateSecured ->data())->clear();
	static_cast<EProPlotIntervalData*>(_pLayerStateClosed->data())->clear();
	static_cast<EProPlotIntervalData*>(_pLayerStateHalfOpen->data())->clear();
	static_cast<EProPlotIntervalData*>(_pLayerStateOpen->data())->clear();

	_pLayerOpenInfo->clear();
	_pLayerCloseInfo->clear();
	EProPlotCurve::clear();
}

void HiHatPositionCurve::add(const QPointF& point, const boost::any& userData)
{
	EProPlotIntervalData* pDataSecured = static_cast<EProPlotIntervalData*>(_pLayerStateSecured ->data());
	EProPlotIntervalData* pDataClosed = static_cast<EProPlotIntervalData*>(_pLayerStateClosed->data());
	EProPlotIntervalData* pDataHalfOpen = static_cast<EProPlotIntervalData*>(_pLayerStateHalfOpen->data());
	EProPlotIntervalData* pDataOpen = static_cast<EProPlotIntervalData*>(_pLayerStateOpen->data());

	EProPlotIntervalData* pDataPrevious = NULL;
	EProPlotIntervalData* pDataCurrent = NULL;
	switch (_previousHiHatState)
	{
	case HHS_SECURED: pDataPrevious = pDataSecured; break;
	case HHS_CLOSED: pDataPrevious = pDataClosed; break;
	case HHS_HALF_OPEN: pDataPrevious = pDataHalfOpen; break;
	case HHS_OPEN: pDataPrevious = pDataOpen; break;
	}

	switch (_hiHatState)
	{
	case HHS_SECURED: pDataCurrent = pDataSecured; break;
	case HHS_CLOSED: pDataCurrent = pDataClosed; break;
	case HHS_HALF_OPEN: pDataCurrent = pDataHalfOpen; break;
	case HHS_OPEN: pDataCurrent = pDataOpen; break;
	}

	if (pDataCurrent != pDataPrevious)
	{
		pDataPrevious->add(QwtIntervalSample(point.x(), 0, point.y()));
		pDataPrevious->add(QwtIntervalSample(point.x(), 0, 0));
		pDataCurrent->add(QwtIntervalSample(point.x(), 0, 0));
	}

	pDataCurrent->add(QwtIntervalSample(point.x(), 0, point.y()));

	if (pDataCurrent != pDataPrevious)
	{
		pDataPrevious->add(QwtIntervalSample(point.x(), 0, 0));
	}

	_previousHiHatState = _hiHatState;

	EProPlotCurve::add(point, userData);
}

void HiHatPositionCurve::addFootCancelInfo(int startTime, int maskLength, int velocity)
{
	EProPlotIntervalData* pDataFootCancel = static_cast<EProPlotIntervalData*>(_pLayerFootCanceMaskTime->data());
	pDataFootCancel->add(QwtIntervalSample(velocity, startTime, startTime+maskLength));
}

void HiHatPositionCurve::setFootCancelMaskTime(int maskLength)
{
	EProPlotIntervalData* pDataFootCancel = static_cast<EProPlotIntervalData*>(_pLayerFootCanceMaskTime->data());
	pDataFootCancel->increaseAllIntervallsOnTheRight(maskLength);
	plot()->replot();
}

void HiHatPositionCurve::setFootCancelMaskVelocity(int velocity)
{
	EProPlotIntervalData* pDataFootCancel = static_cast<EProPlotIntervalData*>(_pLayerFootCanceMaskTime->data());
	pDataFootCancel->changeAllValues(velocity);
	plot()->replot();
}

void HiHatPositionCurve::activateHiHatStates(bool state)
{
	_isHiHatStatesActivated = state;
	updateHiHatLayers();
	plot()->replot();
}

void HiHatPositionCurve::activateFootCancel(bool state)
{
	_isFootCancelActivated = state;
	updateFootCancelLayers();
	plot()->replot();
}

void HiHatPositionCurve::showHiHatLayers(bool state)
{
	_isHiHatLayersShown = state;
	updateHiHatLayers();
	plot()->replot();
}

void HiHatPositionCurve::showFootCancelLayers(bool state)
{
	_isFootCancelLayersShown = state;
	updateFootCancelLayers();
	plot()->replot();
}
