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

class QwtPlotCurve;

class DialogFunction : public QDialog, private Ui::DialogFunction
{
	Q_OBJECT

public:
	DialogFunction(	const LinearFunction::Description::Ptr& pDescription,
					const QPolygonF& curveSamples,
					QWidget* pParent=NULL);
	virtual ~DialogFunction();

	QPolygonF getCurveSamples() const;

	virtual bool eventFilter(QObject* watched, QEvent* event);

private slots:
	void onInsertPoint();
	void onRemovePoint();

protected:
	void updatePoint(const QPointF& point, QPolygonF& rSamples, int indexPoint);
	void updateBranch(const QPointF& point, QPolygonF& rSamples, int indexPoint1, int indexPoint2);

private:
	boost::scoped_ptr<QwtPlotCurve>		_pCurve;
	LinearFunction::Description::Ptr	_pDescription;
	int									_indexSelectedPoint;
	int									_relativeIndex;
	QPoint								_lastPosition;
};
