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

#include "Parameter.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QSlider>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QApplication>

class QCheckBox;
class QSpinBox;
class QDoubleSpinBox;
class QLineEdit;
class QComboBox;
class QPushButton;
class QHBoxLayout;

/**
 * Slider that forward wheel events to the parent.
 * (we don't want to modify the value of the slider
 * when the user want to scroll with the wheel)
 */
class Slider : public QSlider
{
	Q_OBJECT

public:
	Slider(Qt::Orientation orientation, QWidget* pParent=NULL):QSlider(orientation, pParent) {}

protected:
	virtual void wheelEvent(QWheelEvent* event) {QApplication::instance()->notify(parent(), event);}
};

/**
 * Parameter Editor used in ParamItemDelegate.
 */
class ParamItemEditor : public QWidget
{
	Q_OBJECT

signals:
	void editFinished(QWidget*);

public:
	ParamItemEditor(QWidget* pParent=NULL);
	virtual ~ParamItemEditor();
	void setData(const Parameter& data);
	const Parameter& getData() const {return _data;}
	bool hasDataChanged() const {return _bDataChanged;}

private slots:
	void onCheckBoxValueChanged(int);
	void onSliderValueChanged(int);
	void onSpinBoxValueChanged(int);
	void onDoubleSliderValueChanged(int);
	void onDoubleSpinBoxValueChanged(double);
	void onLineEditValueChanged(const QString&);
	void onComboBoxIndexChanged(int);
	void onPushButtonClicked();

private:
	bool				_bDataChanged;
	Parameter			_data;

	QHBoxLayout*		_pLayout;

	QCheckBox*			_pCheckBox;
	QSpinBox*			_pSpinBox;
	QDoubleSpinBox*		_pDoubleSpinBox;
	Slider*				_pSlider;
	Slider*				_pDoubleSlider;
	QLineEdit*			_pLineEdit;
	QComboBox*			_pComboBox;
	QPushButton*		_pPushButton;
};
