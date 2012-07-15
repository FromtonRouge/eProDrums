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

#include "HiHatPedalCurve.h"
#include "EProPlotIntervalData.h"
#include "qwt_plot_histogram.h"
#include "qwt_plot.h"

HiHatPedalCurve::HiHatPedalCurve(QwtPlot* pPlot):
	EProPlotCurve("Hi Hat Pedal", QColor(Qt::white), 2, pPlot),
	_bShowFootCancelStragegy2Info(true)
{
	setStyle(EProPlotCurve::Sticks);

    QColor colorMask(Qt::magenta);
	colorMask.setAlpha(150);
	_pLayerFootCancelMaskTime2 = new QwtPlotHistogram("Foot Cancel Mask Time 2");
	_pLayerFootCancelMaskTime2->setPen(QPen(QColor(Qt::magenta)));
	_pLayerFootCancelMaskTime2->setBrush(colorMask);
    _pLayerFootCancelMaskTime2->setStyle(QwtPlotHistogram::Outline);
	_pLayerFootCancelMaskTime2->setData(new EProPlotIntervalData());
	_pLayerFootCancelMaskTime2->setItemAttribute(Legend, false);
	_pLayerFootCancelMaskTime2->attach(pPlot);

	_pLayerFootCancelMaskTime3 = new QwtPlotHistogram("Foot Cancel Mask Time 3");
	_pLayerFootCancelMaskTime3->setPen(QPen(QColor(Qt::magenta)));
	_pLayerFootCancelMaskTime3->setBrush(colorMask);
    _pLayerFootCancelMaskTime3->setStyle(QwtPlotHistogram::Outline);
	_pLayerFootCancelMaskTime3->setData(new EProPlotIntervalData());
	_pLayerFootCancelMaskTime3->setItemAttribute(Legend, false);
	_pLayerFootCancelMaskTime3->attach(pPlot);
}

HiHatPedalCurve::~HiHatPedalCurve()
{
}

void HiHatPedalCurve::setVisible(bool state)
{
	_pLayerFootCancelMaskTime2->setVisible(state&&_bShowFootCancelStragegy2Info);
	_pLayerFootCancelMaskTime3->setVisible(state&&_bShowFootCancelStragegy2Info);
	EProPlotCurve::setVisible(state);
}

void HiHatPedalCurve::clear()
{
	static_cast<EProPlotIntervalData*>(_pLayerFootCancelMaskTime2->data())->clear();
	static_cast<EProPlotIntervalData*>(_pLayerFootCancelMaskTime3->data())->clear();
	EProPlotCurve::clear();
}

void HiHatPedalCurve::add(const QPointF& point, int mt2, int vel2, int mt3, int vel3)
{
	int startTime = point.x();
	EProPlotIntervalData* pDataMt2 = static_cast<EProPlotIntervalData*>(_pLayerFootCancelMaskTime2->data());
	EProPlotIntervalData* pDataMt3 = static_cast<EProPlotIntervalData*>(_pLayerFootCancelMaskTime3->data());
	pDataMt2->add(QwtIntervalSample(vel2, startTime-mt2, startTime));
	pDataMt3->add(QwtIntervalSample(vel3, startTime, startTime+mt3));
	EProPlotCurve::add(point);
}

void HiHatPedalCurve::changeFootCancelStrategy2MaskTime2(int maskLength)
{
	EProPlotIntervalData* pDataFootCancel = static_cast<EProPlotIntervalData*>(_pLayerFootCancelMaskTime2->data());
	pDataFootCancel->increaseAllIntervallsOnTheLeft(maskLength);
	plot()->replot();
}

void HiHatPedalCurve::changeFootCancelStrategy2Velocity2(int value)
{
	EProPlotIntervalData* pDataFootCancel = static_cast<EProPlotIntervalData*>(_pLayerFootCancelMaskTime2->data());
	pDataFootCancel->changeAllValues(value);
	plot()->replot();
}

void HiHatPedalCurve::changeFootCancelStrategy2MaskTime3(int maskLength)
{
	EProPlotIntervalData* pDataFootCancel = static_cast<EProPlotIntervalData*>(_pLayerFootCancelMaskTime3->data());
	pDataFootCancel->increaseAllIntervallsOnTheRight(maskLength);
	plot()->replot();
}

void HiHatPedalCurve::changeFootCancelStrategy2Velocity3(int value)
{
	EProPlotIntervalData* pDataFootCancel = static_cast<EProPlotIntervalData*>(_pLayerFootCancelMaskTime3->data());
	pDataFootCancel->changeAllValues(value);
	plot()->replot();
}

void HiHatPedalCurve::showFootCancelStrategy2Info(bool state)
{
	_bShowFootCancelStragegy2Info = state;
	_pLayerFootCancelMaskTime2->setVisible(state);
	_pLayerFootCancelMaskTime3->setVisible(state);
	plot()->replot();
}

