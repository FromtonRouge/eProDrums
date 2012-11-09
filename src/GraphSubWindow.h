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

#include <QtGui/QMdiSubWindow>

#include "EProPlotCurve.h"

class EProPlot;
class EProPlotZoomer;
class HiHatPositionCurve;
class QTimer;
struct UserSettings;

/**
 * GraphSubWindow.
 */
class GraphSubWindow : public QMdiSubWindow
{
	Q_OBJECT

public:
	GraphSubWindow(UserSettings* pUserSettings, QWidget* pParent = NULL);
	virtual ~GraphSubWindow();

	void replot();
	void clearPlots();
	void setCurveVisibility(EProPlotCurve* pCurve, bool state);

	void loadCurveVisibility();
	void saveCurveVisibility();

	void showHiHatLayers(bool state);
	void showFootCancelLayers(bool state);
	void showHiHatPedalMaskLayer(bool state);

	void onCurveWindowLengthChanged(int value);
	void onRedrawPeriodChanged(int value);
	void onFootCancelActivated(bool state);
	void onFootCancelMaskTime(int value);
	void onFootCancelVelocity(int value);
	void onFootCancelAfterPedalHitActivated(bool state);
	void onFootCancelAfterPedalHitMaskTime(int value);
	void onFootCancelAfterPedalHitVelocity(int value);

public Q_SLOTS:
    void onUpdatePlot(int, int, int, int, int, float, float);
	void onLeftMouseClicked(const QPoint&);
	void onRedrawCurves();
	void onHiHatStartMoving(int movingState, int pos, int timestamp);
	void onHiHatState(int state);
	void onFootCancelStarted(int startTime, int maskLength, int velocity);
	void onRectSelection(bool);

private:
	virtual void showEvent(QShowEvent* pEvent);
	virtual void hideEvent(QHideEvent* pEvent);
	virtual void closeEvent(QCloseEvent* pEvent);

private:
	UserSettings*		_pUserSettings;

    EProPlot*			_pPlot;
    EProPlotZoomer*		_pPlotZoomer;

	bool				_bRedrawState;
	QTimer*				_pRedrawTimer;
	int					_redrawPeriod;

    HiHatPositionCurve*	_curveHiHatPosition;
    EProPlotCurve*		_curveHiHatAcceleration;
	EProPlotCurve::Dict	_curves;
};
