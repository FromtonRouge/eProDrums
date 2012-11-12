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

#include "GraphSubWindow.h"

#include "DrumKitMidiMap.h"
#include "EProPlot.h"
#include "HiHatPositionCurve.h"
#include "EProPlotZoomer.h"
#include "EProPlotMarker.h"
#include "HiHatPedalCurve.h"
#include "UserSettings.h"

#include "HiHatPedalElement.h"

#include <qwt_symbol.h>
#include <qwt_scale_div.h>
#include <qwt_legend_item.h>

#include <QtGui/QMdiArea>
#include <QtGui/QCloseEvent>
#include <QtCore/QTimer>

#include <boost/format.hpp>
#include <iostream>

GraphSubWindow::GraphSubWindow(UserSettings* pUserSettings, QWidget* pParent):QMdiSubWindow(pParent),
	_pUserSettings(pUserSettings),
	_bRedrawState(true),
	_redrawPeriod(25)
{
	setAttribute(Qt::WA_DeleteOnClose);

	// Timer used to redraw plots
	_pRedrawTimer = new QTimer(this);
	connect(_pRedrawTimer, SIGNAL(timeout()), this, SLOT(onRedrawCurves()));

    _pPlot = new EProPlot(this);
	setWidget(_pPlot);

    // Plot zoomer
    _pPlotZoomer = new EProPlotZoomer(_pPlot->canvas());
	connect(_pPlotZoomer, SIGNAL(inRectSelection(bool)), this, SLOT(onRectSelection(bool)));
	connect(_pPlotZoomer, SIGNAL(leftMouseClicked(const QPoint&)), this, SLOT(onLeftMouseClicked(const QPoint&)));
	_pPlotZoomer->setEnabled(true);

    // Curve CC#4
	QColor qHiHatAccel(245, 0, 150);
	_curveHiHatPosition = new HiHatPositionCurve(_pPlot);
	_curveHiHatAcceleration = new EProPlotCurve("Hi Hat Acceleration", qHiHatAccel, 1, _pPlot);
	_curveHiHatAcceleration->setMarkerInformationOutlineColor(QColor(Qt::white));
	_curveHiHatAcceleration->getMarker()->setLabelAlignment(Qt::AlignRight|Qt::AlignBottom);
	_curveHiHatAcceleration->setStyle(EProPlotCurve::Lines);
	_curveHiHatAcceleration->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, qHiHatAccel, qHiHatAccel, QSize(2,2)));

	_curves[Pad::SNARE] = new EProPlotCurve("Snare", QColor(Pad::getDefaultColor(Pad::SNARE).c_str()), 2, _pPlot);
	_curves[Pad::HIHAT] = new EProPlotCurve("Hi Hat", QColor(Pad::getDefaultColor(Pad::HIHAT).c_str()), 2, _pPlot);
	_curves[Pad::HIHAT_PEDAL] = new HiHatPedalCurve(_pPlot);
	_curves[Pad::TOM1] = new EProPlotCurve("Tom 1", QColor(Pad::getDefaultColor(Pad::TOM1).c_str()), 2, _pPlot);
	_curves[Pad::TOM2] = new EProPlotCurve("Tom 2", QColor(Pad::getDefaultColor(Pad::TOM2).c_str()), 2, _pPlot);
	_curves[Pad::TOM3] = new EProPlotCurve("Tom 3", QColor(Pad::getDefaultColor(Pad::TOM3).c_str()), 2, _pPlot);
	_curves[Pad::CRASH1] = new EProPlotCurve("Green Crash", QColor(Pad::getDefaultColor(Pad::CRASH1).c_str()), 2, _pPlot);
	_curves[Pad::CRASH2] = _curves[Pad::CRASH1];
	_curves[Pad::CRASH3] = new EProPlotCurve("Yellow Crash", QColor(Pad::getDefaultColor(Pad::CRASH3).c_str()), 2, _pPlot);
	_curves[Pad::RIDE] = new EProPlotCurve("Ride", QColor(Pad::getDefaultColor(Pad::RIDE).c_str()), 2, _pPlot);
	_curves[Pad::BASS_DRUM] = new EProPlotCurve("Bass Drum", QColor(Pad::getDefaultColor(Pad::BASS_DRUM).c_str()), 2, _pPlot);

    _pPlot->showAll();

	// By default we hide the hh accel
	setCurveVisibility(_curveHiHatAcceleration, _pUserSettings->isCurveVisible(UserSettings::CURVE_HIHAT_ACCELERATION));
}

GraphSubWindow::~GraphSubWindow()
{
}

void GraphSubWindow::clearPlots()
{
	_pPlotZoomer->moveWindow(0, 5*1000);

	_curveHiHatPosition->showMarkers(false);
	_curveHiHatAcceleration->showMarkers(false);

	_pPlot->clear();
}

void GraphSubWindow::setCurveVisibility(EProPlotCurve* pCurve, bool state)
{
	QwtLegend* pLegend = _pPlot->legend();
	QwtLegendItem* pLegendItem = dynamic_cast<QwtLegendItem*>(pLegend->find(pCurve));
	pCurve->setVisible(state);
	pLegendItem->setChecked(state);
}

void GraphSubWindow::replot()
{
	_pPlot->replot();
}

void GraphSubWindow::onCurveWindowLengthChanged(int value)
{
	QwtScaleDiv* pScaleDiv = _pPlot->axisScaleDiv(QwtPlot::xBottom);
	int timeWindowInMs = value*1000;
	int maxValue = pScaleDiv->interval().maxValue();
	_pPlotZoomer->moveWindow(maxValue-timeWindowInMs,timeWindowInMs, true, false);
}

void GraphSubWindow::onUpdatePlot(const MidiMessage& midiMessage)
{
	int msgType = midiMessage.getMsgType();
	int msgVelocity = midiMessage.getValue();
	int timestamp = midiMessage.getTimestamp();
   	float hiHatControlSpeed = midiMessage.hiHatSpeed;
	float hiHatAcceleration = midiMessage.hiHatAcceleration;

	int plotTimeWindow = 5*1000;
	if (timestamp>plotTimeWindow)
	{
		_pPlotZoomer->moveWindow(timestamp-plotTimeWindow, plotTimeWindow);
	}
	else
	{
		_pPlotZoomer->moveWindow(0, plotTimeWindow);
	}

    // CC#4
    if (msgType == 11)
    {
		int hiHatLevel(127-msgVelocity);
		boost::any userData(std::make_pair(hiHatControlSpeed, hiHatAcceleration));
		_curveHiHatPosition->add(QPointF(timestamp, hiHatLevel), userData);
		_curveHiHatAcceleration->add(QPointF(timestamp, hiHatAcceleration*(127.f/HiHatPedalElement::MAX_ALLOWED_ACCELERATION)));
		if (_pUserSettings->isLogs() && _pUserSettings->isLog(UserSettings::LOG_HIHAT_CONTROL))
		{
			std::cout << (boost::format("%d (unit), %.3f (unit/s), %.3f (unit/ss)")%hiHatLevel%hiHatControlSpeed%hiHatAcceleration) << std::endl;
		}
    }
    else if (msgType == 9)
    {
		EProPlotCurve::Dict::iterator it = _curves.find(midiMessage.padType);
		if (it!=_curves.end())
		{
			it->second->add(QPointF(timestamp, msgVelocity));
		}
    }

	// Redraw order, plots are redrawn after an idle time
	if (_bRedrawState)
	{
		_pRedrawTimer->start(_redrawPeriod);
	}
}

void GraphSubWindow::onLeftMouseClicked(const QPoint& pos)
{
	int index = _curveHiHatPosition->updateMarkers(pos);
	if (index>=0)
	{
		int position(_curveHiHatPosition->y(index));
		std::pair<float, float> speedAndAccel = boost::any_cast< std::pair<float, float> >(_curveHiHatPosition->getUserData(index));

		boost::format fmtControl("%d (unit), %.3f (unit/s), %.3f (unit/s²)");
		_curveHiHatPosition->setMarkerInformationLabel((fmtControl%position%speedAndAccel.first%speedAndAccel.second).str());
	}

	index = _curveHiHatAcceleration->updateMarkers(pos);
	if (index>=0)
	{
		float accel = _curveHiHatAcceleration->y(index)*float(HiHatPedalElement::MAX_ALLOWED_ACCELERATION)/127;
		
		boost::format fmtAccel("%.3f (unit/s²)");
		_curveHiHatAcceleration->setMarkerInformationLabel((fmtAccel%accel).str());
	}

	EProPlotCurve::Dict::iterator it = _curves.begin();
	while (it!=_curves.end())
	{
		EProPlotCurve::Dict::value_type& v = *(it++);
		EProPlotCurve* pCurve = v.second;
		index = pCurve->updateMarkers(pos);
		if (index>=0)
		{
			pCurve->setMarkerInformationLabel((boost::format("%d")%pCurve->y(index)).str());
		}
	}

	replot();
}

void GraphSubWindow::showEvent(QShowEvent* pEvent)
{
	_pRedrawTimer->start(_redrawPeriod);
	_bRedrawState = true;
	QWidget::showEvent(pEvent);
}

void GraphSubWindow::hideEvent(QHideEvent* pEvent)
{
	_bRedrawState = false;
	QWidget::hideEvent(pEvent);
}

void GraphSubWindow::onRedrawPeriodChanged(int value)
{
	_redrawPeriod = value;
}

void GraphSubWindow::onRedrawCurves()
{
	_pRedrawTimer->stop();
	replot();
}

void GraphSubWindow::loadCurveVisibility()
{
	setCurveVisibility(_curveHiHatPosition, _pUserSettings->isCurveVisible(UserSettings::CURVE_HIHAT_CONTROL));
	setCurveVisibility(_curveHiHatAcceleration, _pUserSettings->isCurveVisible(UserSettings::CURVE_HIHAT_ACCELERATION));
	setCurveVisibility(_curves[Pad::HIHAT], _pUserSettings->isCurveVisible(UserSettings::CURVE_HIHAT));
	setCurveVisibility(_curves[Pad::HIHAT_PEDAL], _pUserSettings->isCurveVisible(UserSettings::CURVE_HIHAT_PEDAL));
	setCurveVisibility(_curves[Pad::CRASH1], _pUserSettings->isCurveVisible(UserSettings::CURVE_CRASH));
	setCurveVisibility(_curves[Pad::CRASH3], _pUserSettings->isCurveVisible(UserSettings::CURVE_YELLOW_CRASH));
	setCurveVisibility(_curves[Pad::RIDE], _pUserSettings->isCurveVisible(UserSettings::CURVE_RIDE));
	setCurveVisibility(_curves[Pad::TOM1], _pUserSettings->isCurveVisible(UserSettings::CURVE_TOM1));
	setCurveVisibility(_curves[Pad::TOM2], _pUserSettings->isCurveVisible(UserSettings::CURVE_TOM2));
	setCurveVisibility(_curves[Pad::TOM3], _pUserSettings->isCurveVisible(UserSettings::CURVE_TOM3));
	setCurveVisibility(_curves[Pad::SNARE], _pUserSettings->isCurveVisible(UserSettings::CURVE_SNARE));
	setCurveVisibility(_curves[Pad::BASS_DRUM], _pUserSettings->isCurveVisible(UserSettings::CURVE_BASS_PEDAL));
}

void GraphSubWindow::saveCurveVisibility()
{
	_pUserSettings->setCurveVisibility(UserSettings::CURVE_HIHAT_CONTROL, _curveHiHatPosition->isVisible());
	_pUserSettings->setCurveVisibility(UserSettings::CURVE_HIHAT_ACCELERATION, _curveHiHatAcceleration->isVisible());
	_pUserSettings->setCurveVisibility(UserSettings::CURVE_HIHAT, _curves[Pad::HIHAT]->isVisible());
	_pUserSettings->setCurveVisibility(UserSettings::CURVE_HIHAT_PEDAL, _curves[Pad::HIHAT_PEDAL]->isVisible());
	_pUserSettings->setCurveVisibility(UserSettings::CURVE_CRASH, _curves[Pad::CRASH1]->isVisible());
	_pUserSettings->setCurveVisibility(UserSettings::CURVE_YELLOW_CRASH, _curves[Pad::CRASH3]->isVisible());
	_pUserSettings->setCurveVisibility(UserSettings::CURVE_RIDE, _curves[Pad::RIDE]->isVisible());
	_pUserSettings->setCurveVisibility(UserSettings::CURVE_TOM1, _curves[Pad::TOM1]->isVisible());
	_pUserSettings->setCurveVisibility(UserSettings::CURVE_TOM2, _curves[Pad::TOM2]->isVisible());
	_pUserSettings->setCurveVisibility(UserSettings::CURVE_TOM3, _curves[Pad::TOM3]->isVisible());
	_pUserSettings->setCurveVisibility(UserSettings::CURVE_SNARE, _curves[Pad::SNARE]->isVisible());
	_pUserSettings->setCurveVisibility(UserSettings::CURVE_BASS_PEDAL, _curves[Pad::BASS_DRUM]->isVisible());

}

void GraphSubWindow::onFootCancelAfterPedalHitActivated(bool state)
{
	HiHatPedalCurve* pPedalCurve = dynamic_cast<HiHatPedalCurve*>(_curves[Pad::HIHAT_PEDAL]);
	pPedalCurve->activateMask(state);
}

void GraphSubWindow::onFootCancelAfterPedalHitMaskTime(int value)
{
	HiHatPedalCurve* pPedalCurve = dynamic_cast<HiHatPedalCurve*>(_curves[Pad::HIHAT_PEDAL]);
	pPedalCurve->setFootCancelMaskTime(value);
}

void GraphSubWindow::onFootCancelAfterPedalHitVelocity(int value)
{
	HiHatPedalCurve* pPedalCurve = dynamic_cast<HiHatPedalCurve*>(_curves[Pad::HIHAT_PEDAL]);
	pPedalCurve->setFootCancelMaskVelocity(value);
}

void GraphSubWindow::onFootCancelActivated(bool state)
{
	_curveHiHatPosition->activateFootCancel(state);
}

void GraphSubWindow::onFootCancelMaskTime(int value)
{
	_curveHiHatPosition->setFootCancelMaskTime(value);
}

void GraphSubWindow::onFootCancelVelocity(int value)
{
	_curveHiHatPosition->setFootCancelMaskVelocity(value);
}

void GraphSubWindow::onHiHatStartMoving(int movingState, int pos, int timestamp)
{
	switch (movingState)
	{
	case HiHatPedalElement::MS_START_OPEN:
		{
			_curveHiHatPosition->addOpenInfo(QPointF(timestamp, pos));
			break;
		}
	case HiHatPedalElement::MS_START_CLOSE:
		{
			_curveHiHatPosition->addCloseInfo(QPointF(timestamp, pos));
			break;
		}
	default:
		{
			break;
		}
	}
}

void GraphSubWindow::onHiHatState(int state)
{
	HiHatPositionCurve::HiHatState hhState= static_cast<HiHatPositionCurve::HiHatState>(state);
	_curveHiHatPosition->setHiHatState(hhState);
}

void GraphSubWindow::onFootCancelStarted(int startTime, int maskLength, int velocity)
{
	_curveHiHatPosition->addFootCancelInfo(startTime, maskLength, velocity);
}

void GraphSubWindow::showHiHatLayers(bool state)
{
	_curveHiHatPosition->showHiHatLayers(state);
}

void GraphSubWindow::showFootCancelLayers(bool state)
{
	_curveHiHatPosition->showFootCancelLayers(state);
}

void GraphSubWindow::showHiHatPedalMaskLayer(bool state)
{
	dynamic_cast<HiHatPedalCurve*>(_curves[Pad::HIHAT_PEDAL])->showMaskLayer(state);
}

void GraphSubWindow::onRectSelection(bool bState)
{
	if (bState)
	{
		//statusBar()->showMessage("[Ctrl + Left Click] to zoom");
	}
	else
	{
		//statusBar()->showMessage("");
	}
}

void GraphSubWindow::closeEvent(QCloseEvent* pEvent)
{
	if (mdiArea()->subWindowList().size()==1)
	{
		pEvent->ignore();
	}
	else
	{
		pEvent->accept();
	}
}

void GraphSubWindow::onDrumKitLoaded(DrumKitMidiMap* pDrumKit, const boost::filesystem::path&)
{
	const DrumKitMidiMap::Description::Pads& pads = pDrumKit->getDescription().pads;
	DrumKitMidiMap::Description::Pads::const_iterator it = pads.begin();
	while (it!=pads.end())
	{
		const Pad::MidiDescription& padDesc = *(it++);
		EProPlotCurve::Dict::const_iterator itCurve = _curves.find(padDesc.type);
		if (itCurve!=_curves.end())
		{
			itCurve->second->setColor(QColor(padDesc.color.c_str()));
			itCurve->second->setTitle(Pad::getName(padDesc.type).c_str());
		}
	}

	DrumKitMidiMap::Description::Pads::const_iterator itClosedHiHat = std::find_if(pads.begin(), pads.end(), boost::bind(&Pad::MidiDescription::type, _1)==Pad::HIHAT);
	if (itClosedHiHat!=pads.end())
	{
		_curveHiHatPosition->setClosedColor(QColor(itClosedHiHat->color.c_str()));
	}

	DrumKitMidiMap::Description::Pads::const_iterator itOpenHiHat = std::find_if(pads.begin(), pads.end(), boost::bind(&Pad::MidiDescription::type, _1)==Pad::RIDE);
	if (itOpenHiHat!=pads.end())
	{
		_curveHiHatPosition->setOpenedColor(QColor(itOpenHiHat->color.c_str()));
	}
}
