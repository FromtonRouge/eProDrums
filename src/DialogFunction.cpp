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

#include <qwt_plot_grid.h>
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
	boost::format fmtMsg("x=%.1f, y=%.1f");
	const QPointF& point = invTransform(pointInPixel);
	return (fmtMsg%point.x()%point.y()).str().c_str();
}

DialogFunction::DialogFunction(const LinearFunction::List& functions, QWidget* pParent):QDialog(pParent),
	_pFunctionItemModel(new FunctionItemModel(functions)),
	_pFunctionItemDelegate(new FunctionItemDelegate())
{
	setupUi(this);

	connect(_pFunctionItemModel.get(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(onModelChanged()));

	_pPlot = new QwtPlot(this);
    _pPlot->setAxisScale(QwtPlot::xBottom, 0, 127, 30);
    _pPlot->setAxisScale(QwtPlot::yLeft, 0, 127, 30);
    _pPlot->setAxisMaxMinor(QwtPlot::yLeft, 2);
    _pPlot->setAxisMaxMinor(QwtPlot::xBottom, 2);
    _pPlot->setCanvasBackground(QColor(Qt::black));
    _pPlot->setAxisTitle(QwtPlot::xBottom, "Hi-hat position");
    _pPlot->setAxisTitle(QwtPlot::yLeft, "Velocity");

	gridLayout->addWidget(_pPlot, 0, 0);

    QwtPlotGrid* pGrid = new QwtPlotGrid;
    pGrid->setMajPen(QPen(Qt::gray, 0, Qt::DotLine));
    pGrid->attach(_pPlot);

	_pPlotPicker = new FunctionPlotPicker(_pPlot->canvas());
    _pPlotPicker->setRubberBandPen(QColor(Qt::white));
    _pPlotPicker->setTrackerPen(QColor(Qt::white));
	_pPlotPicker->setTrackerMode(FunctionPlotPicker::AlwaysOn);

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

	for (size_t i=0;i<functions.size();++i)
	{
		const LinearFunction& f = functions[i];

		boost::shared_ptr<QwtPlotIntervalCurve> pCurve(new QwtPlotIntervalCurve(f.getName().c_str()));
		pCurve->setStyle(QwtPlotIntervalCurve::Tube);
		QwtIntervalSeriesData* pData = new QwtIntervalSeriesData();

		QVector<QwtIntervalSample> samples;
		samples.push_back(QwtIntervalSample(f.getX1(), 0, f.getY1()));
		samples.push_back(QwtIntervalSample(f.getX2(), 0, f.getY2()));
		pData->setSamples(samples);

		QColor color(200, 50*i, 50*i+100);
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
