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
#include "qwt_plot_intervalcurve.h"
#include "qwt_plot_histogram.h"
#include "qwt_plot.h"

HiHatPedalCurve::HiHatPedalCurve(QwtPlot* pPlot): EProPlotCurve("Hi Hat Pedal", QColor(Qt::white), 2, pPlot),
	_maskVelocity(0),
	_maskTime(0),
	_isShowMask(true)
{
	setStyle(EProPlotCurve::Sticks);

    QColor colorMask(Qt::magenta);
	colorMask.setAlpha(150);
	_pLayerFootCancelMask = new QwtPlotHistogram("Foot Cancel Mask Time");
	_pLayerFootCancelMask->setPen(QPen(QColor(Qt::magenta)));
	_pLayerFootCancelMask->setBrush(colorMask);
    _pLayerFootCancelMask->setStyle(QwtPlotHistogram::Outline);
	_pLayerFootCancelMask->setData(new EProPlotIntervalData());
	_pLayerFootCancelMask->setItemAttribute(Legend, false);
	_pLayerFootCancelMask->attach(pPlot);
}

HiHatPedalCurve::~HiHatPedalCurve()
{
}

void HiHatPedalCurve::showMask(bool state)
{
	_isShowMask = state;
	_pLayerFootCancelMask->setVisible(_isShowMask);
	plot()->replot();
}

void HiHatPedalCurve::setVisible(bool state)
{
	_pLayerFootCancelMask->setVisible(state&&_isShowMask);
	EProPlotCurve::setVisible(state);
}

void HiHatPedalCurve::clear()
{
	static_cast<EProPlotIntervalData*>(_pLayerFootCancelMask->data())->clear();
	EProPlotCurve::clear();
}

void HiHatPedalCurve::add(const QPointF& point, const boost::any& userData)
{
	EProPlotIntervalData* pDataFootCancel = static_cast<EProPlotIntervalData*>(_pLayerFootCancelMask->data());
	pDataFootCancel->add(QwtIntervalSample(_maskVelocity, point.x(), point.x() + _maskTime));
	EProPlotCurve::add(point, userData);
}

void HiHatPedalCurve::setFootCancelMaskTime(int maskTime)
{
	_maskTime = maskTime;
	EProPlotIntervalData* pDataFootCancel = static_cast<EProPlotIntervalData*>(_pLayerFootCancelMask->data());
	pDataFootCancel->increaseAllIntervallsOnTheRight(_maskTime);
	plot()->replot();
}

void HiHatPedalCurve::setFootCancelMaskVelocity(int velocity)
{
	_maskVelocity = velocity;
	EProPlotIntervalData* pDataFootCancel = static_cast<EProPlotIntervalData*>(_pLayerFootCancelMask->data());
	pDataFootCancel->changeAllValues(_maskVelocity);
	plot()->replot();
}
