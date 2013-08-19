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

#include "DialogFunction.h"
#include "FunctionItemModel.h"
#include "FunctionItemDelegate.h"

#include <qwt_symbol.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_intervalcurve.h>
#include <qwt_series_data.h>
#include <QtGui/QPen>
#include <QtGui/QMenu>
#include <QtGui/QCursor>

#include <boost/format.hpp>

DialogFunction::FunctionPlotPicker::FunctionPlotPicker(QwtPlotCanvas* pCanvas):QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, pCanvas)
{
}

QwtText DialogFunction::FunctionPlotPicker::trackerText(const QPoint& pointInPixel) const
{
	onTrackerPosChanged(invTransform(pointInPixel));
	return QwtText();
}

DialogFunction::DialogFunction(	const LinearFunction::Description::Ptr& pDescription,
		const LinearFunction::List& functions,
		QWidget* pParent):QDialog(pParent),
	_pFunctionItemModel(new FunctionItemModel(pDescription, functions)),
	_pFunctionItemDelegate(new FunctionItemDelegate(pDescription)),
	_pDescription(pDescription)
{
	setupUi(this);

	connect(_pFunctionItemModel.get(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(onModelChanged()));

	_pPlot = new QwtPlot(this);
	_pPlot->setAxisScale(QwtPlot::xBottom, _pDescription->xMin, _pDescription->xMax);
	_pPlot->setAxisScale(QwtPlot::yLeft, _pDescription->yMin, _pDescription->yMax);
	_pPlot->setAxisMaxMinor(QwtPlot::yLeft, 2);
	_pPlot->setAxisMaxMinor(QwtPlot::xBottom, 2);
	_pPlot->setCanvasBackground(QColor(Qt::black));
	_pPlot->setAxisTitle(QwtPlot::xBottom, _pDescription->szLabelX.c_str());
	_pPlot->setAxisTitle(QwtPlot::yLeft, _pDescription->szLabelY.c_str());

	QColor inColor(255, 0, 0);
	QColor outColor(255, 255, 255);
	_pPlotMarker = new QwtPlotMarker;
	_pPlotMarker->setLineStyle(QwtPlotMarker::Cross);
	_pPlotMarker->setLabelAlignment(Qt::AlignLeft|Qt::AlignTop);
	_pPlotMarker->setLinePen(QPen(QColor(Qt::white), 0, Qt::DashDotLine));
	_pPlotMarker->setSymbol(new QwtSymbol(QwtSymbol::Diamond, inColor, outColor, QSize(4,4)));
	_pPlotMarker->attach(_pPlot);
	_pPlotMarker->setVisible(false);

	gridLayout->addWidget(_pPlot, 0, 0);

	QwtPlotGrid* pGrid = new QwtPlotGrid;
	pGrid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
	pGrid->attach(_pPlot);

	_pPlotPicker = new FunctionPlotPicker(static_cast<QwtPlotCanvas*>(_pPlot->canvas()));
	_pPlotPicker->setRubberBandPen(QColor(Qt::white));
	_pPlotPicker->setTrackerPen(QColor(Qt::white));
	_pPlotPicker->setTrackerMode(FunctionPlotPicker::AlwaysOn);
	_pPlotPicker->onTrackerPosChanged.connect(boost::bind(&DialogFunction::onTrackerPosChanged, this, _1));

	tableView->setModel(_pFunctionItemModel.get());
	tableView->setItemDelegate(_pFunctionItemDelegate.get());
	tableView->horizontalHeader()->setStretchLastSection(true);
	tableView->verticalHeader()->hide();

	connect(tableView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onCustomContextMenuRequested(const QPoint&)));
	tableView->setContextMenuPolicy(Qt::CustomContextMenu);

	_pMenu = new QMenu(this);
	QAction* pActionAdd = _pMenu->addAction(tr("Add"));
	connect(pActionAdd, SIGNAL(triggered(bool)), this, SLOT(onActionAddTriggered(bool)));
	QAction* pActionRemove = _pMenu->addAction(tr("Remove"));
	connect(pActionRemove, SIGNAL(triggered(bool)), this, SLOT(onActionRemoveTriggered(bool)));

	// Open persistent editor
	for (int i=0; i<_pFunctionItemModel->rowCount(); ++i)
	{
		tableView->openPersistentEditor(_pFunctionItemModel->index(i, 1));
		tableView->openPersistentEditor(_pFunctionItemModel->index(i, 2));
		tableView->openPersistentEditor(_pFunctionItemModel->index(i, 3));
		tableView->openPersistentEditor(_pFunctionItemModel->index(i, 4));
		tableView->openPersistentEditor(_pFunctionItemModel->index(i, 5));
		tableView->openPersistentEditor(_pFunctionItemModel->index(i, 6));
	}

	tableView->resizeColumnToContents(0);
	onModelChanged();
}

DialogFunction::~DialogFunction()
{
}

void DialogFunction::onCustomContextMenuRequested(const QPoint&)
{
	_pMenu->exec(QCursor::pos());
}

void DialogFunction::onActionAddTriggered(bool)
{
	int row = _pFunctionItemModel->addFunction();
	tableView->openPersistentEditor(_pFunctionItemModel->index(row, 1));
	tableView->openPersistentEditor(_pFunctionItemModel->index(row, 2));
	tableView->openPersistentEditor(_pFunctionItemModel->index(row, 3));
	tableView->openPersistentEditor(_pFunctionItemModel->index(row, 4));
	tableView->openPersistentEditor(_pFunctionItemModel->index(row, 5));
	tableView->openPersistentEditor(_pFunctionItemModel->index(row, 6));
}

void DialogFunction::onActionRemoveTriggered(bool)
{
	QItemSelectionModel* pItemSelectionModel = tableView->selectionModel();
	if (pItemSelectionModel)
	{
		QModelIndexList selected = pItemSelectionModel->selectedIndexes();
		while (!selected.empty())
		{
			_pFunctionItemModel->removeRow(selected[0].row());
			selected = pItemSelectionModel->selectedIndexes();
		}
	}
}

void DialogFunction::onModelChanged()
{
	const LinearFunction::List& functions = _pFunctionItemModel->getFunctions();
	_curves.clear();

	int r = 200;
	int g = 30;
	int b = 30;
	for (size_t i=0;i<functions.size();++i)
	{
		const LinearFunction& f = functions[i];

		boost::shared_ptr<QwtPlotIntervalCurve> pCurve(new QwtPlotIntervalCurve);
		pCurve->setStyle(QwtPlotIntervalCurve::Tube);
		QwtIntervalSeriesData* pData = new QwtIntervalSeriesData();

		QVector<QwtIntervalSample> samples;
		samples.push_back(QwtIntervalSample(f.getX1(), 0, f.getY1()));
		samples.push_back(QwtIntervalSample(f.getX2(), 0, f.getY2()));
		pData->setSamples(samples);

		g += 40;
		b += 40; 
		g = g>255?40:g;
		b = b>255?40:b;
		QColor color(r,g,b);
		color.setAlpha(170);
		pCurve->setBrush(color);
		pCurve->setData(pData);
		pCurve->attach(_pPlot);
		_curves.push_back(pCurve);
	}

	_pPlot->replot();
}

const LinearFunction::List& DialogFunction::getFunctions()
{
	return _pFunctionItemModel->getFunctions();
}

void DialogFunction::onTrackerPosChanged(const QPointF& point)
{
	float x = point.x();
	float y = 0.f;
	if (LinearFunction::apply(_pFunctionItemModel->getFunctions(), x, y))
	{
		if (!_pPlotMarker->isVisible())
		{
			_pPlotMarker->setVisible(true);
		}
		_pPlotMarker->setValue(x, y);

		Qt::Alignment alignment;
		if (y>=(_pDescription->yMax-_pDescription->yMin)/2)
		{
			alignment = Qt::AlignBottom;
		}
		else
		{
			alignment = Qt::AlignTop;
		}

		if (x<=(_pDescription->xMax-_pDescription->xMin)/2)
		{
			alignment |= Qt::AlignRight;
		}
		else
		{
			alignment |= Qt::AlignLeft;
		}

		_pPlotMarker->setLabelAlignment(alignment);

		boost::format fmtMsg("x=%.1f, y=%.1f");
		QwtText label((fmtMsg%x%y).str().c_str());
		_pPlotMarker->setLabel(label);
		_pPlot->replot();
	}
}
