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

#include "LinearFunction.h"

#include "ui_DialogFunction.h"
#include <qwt_plot.h>
#include <qwt_plot_picker.h>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#include <vector>

class FunctionItemModel;
class FunctionItemDelegate;
class QwtPlotIntervalCurve;
class QwtPlotCanvas;
class QwtPlotMarker;
class QMenu;

class DialogFunction : public QDialog, private Ui::DialogFunction
{
	Q_OBJECT
private:
	struct FunctionPlotPicker : public QwtPlotPicker
	{
		typedef boost::signals2::signal<void (const QPointF&)> OnTrackerPosChanged;
		OnTrackerPosChanged onTrackerPosChanged;

		FunctionPlotPicker(QwtPlotCanvas*);

	protected:
		virtual QwtText trackerText(const QPoint &) const;
	};

public:
	DialogFunction(	const LinearFunction::Description::Ptr& pDescription,
					const LinearFunction::List& functions, QWidget* pParent=NULL);
	virtual ~DialogFunction();

	const LinearFunction::List& getFunctions();

private slots:
	void onModelChanged();
	void onCustomContextMenuRequested(const QPoint&);
	void onActionAddTriggered(bool checked=false);
	void onActionRemoveTriggered(bool checked=false);

protected:
	void onTrackerPosChanged(const QPointF&);

private:
	QwtPlot*			_pPlot;
	QwtPlotMarker*		_pPlotMarker;
	FunctionPlotPicker*	_pPlotPicker;
	QMenu*				_pMenu;
	boost::scoped_ptr<FunctionItemModel>			_pFunctionItemModel;
	boost::scoped_ptr<FunctionItemDelegate>			_pFunctionItemDelegate;
	std::vector< boost::shared_ptr<QwtPlotIntervalCurve> >	_curves;
	LinearFunction::List	_functions;
	LinearFunction::Description::Ptr	_pDescription;
};
