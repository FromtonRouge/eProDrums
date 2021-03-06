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

#include "MidiDevicesWidget.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>

MidiDevicesWidget::MidiDevicesWidget(QWidget* pParent):QWidget(pParent)
{
	QHBoxLayout* pLayout = new QHBoxLayout;
	pLayout->setContentsMargins(0,0,0,0);
	
	_pComboBoxMidiIn = new QComboBox();
	_pComboBoxMidiIn->setMaximumWidth(100);
	_pComboBoxMidiIn->setToolTip(tr("Midi in"));
	pLayout->addWidget(_pComboBoxMidiIn);

	_pComboBoxMidiOut = new QComboBox();
	_pComboBoxMidiOut->setMaximumWidth(100);
	_pComboBoxMidiOut->setToolTip(tr("Midi out"));
	pLayout->addWidget(_pComboBoxMidiOut);

	_pPushButtonStart = new QPushButton(QIcon(":/icons/control.png"), tr("Start"), this);
	_pPushButtonStart->setToolTip(tr("Start the Midi engine"));
	connect(_pPushButtonStart, SIGNAL(clicked(bool)), this, SLOT(onMidiStart()));
	pLayout->addWidget(_pPushButtonStart);
	_pPushButtonStop = new QPushButton(QIcon(":/icons/control-stop-square.png"), tr("Stop"), this);
	_pPushButtonStop->setToolTip(tr("Stop the Midi engine"));
	connect(_pPushButtonStop, SIGNAL(clicked(bool)), this, SLOT(onMidiStop()));
	pLayout->addWidget(_pPushButtonStop);

	_pPushButtonStart->setEnabled(false);
	_pPushButtonStop->setEnabled(false);

	setLayout(pLayout);
}

MidiDevicesWidget::~MidiDevicesWidget()
{
}

void MidiDevicesWidget::onMidiStart()
{
	int midiInId = getMidiInId();
	int midiOutId = getMidiOutId();
	if (midiInId>=0 && midiOutId>=0)
	{
		_pComboBoxMidiIn->setEnabled(false);
		_pComboBoxMidiOut->setEnabled(false);
		_pPushButtonStart->setEnabled(false);
		_pPushButtonStop->setEnabled(true);
		emit signalStart(midiInId, midiOutId);
	}
	else
	{
		QMessageBox::critical(this, tr("Invalid devices"), tr("Invalid devices"));
	}
}

void MidiDevicesWidget::onMidiStop()
{
	_pComboBoxMidiIn->setEnabled(true);
	_pComboBoxMidiOut->setEnabled(true);
	_pPushButtonStart->setEnabled(true);
	_pPushButtonStop->setEnabled(false);
	emit signalStop();
}

bool MidiDevicesWidget::setMidiIn(const std::string& szMidiIn)
{
	int index = _pComboBoxMidiIn->findText(szMidiIn.c_str());
	if (index>=0)
	{
		_pComboBoxMidiIn->setCurrentIndex(index);

		if (_pComboBoxMidiOut->currentIndex()>=0)
		{
			_pPushButtonStart->setEnabled(true);
			_pPushButtonStop->setEnabled(false);
		}
		return true;
	}
	return false;
}

bool MidiDevicesWidget::setMidiOut(const std::string& szMidiOut)
{
	int index = _pComboBoxMidiOut->findText(szMidiOut.c_str());
	if (index>=0)
	{
		_pComboBoxMidiOut->setCurrentIndex(index);
		if (_pComboBoxMidiIn->currentIndex()>=0)
		{
			_pPushButtonStart->setEnabled(true);
			_pPushButtonStop->setEnabled(false);
		}
		return true;
	}
	return false;
}

int MidiDevicesWidget::getMidiInId() const
{
	int index = _pComboBoxMidiIn->currentIndex();
	if (index>=0)
	{
		return _pComboBoxMidiIn->itemData(index).toInt();
	}
	return -1;
}

int MidiDevicesWidget::getMidiOutId() const
{
	int index = _pComboBoxMidiOut->currentIndex();
	if (index>=0)
	{
		return _pComboBoxMidiOut->itemData(index).toInt();
	}
	return -1;
}

std::string MidiDevicesWidget::getMidiInString() const
{
	return _pComboBoxMidiIn->currentText().toStdString();
}

std::string MidiDevicesWidget::getMidiOutString() const
{
	return _pComboBoxMidiOut->currentText().toStdString();
}

void MidiDevicesWidget::setMidiInDevices(const MidiDevice::List& devices)
{
	_pComboBoxMidiIn->blockSignals(true);
	MidiDevice::List::const_iterator it = devices.begin();
	while (it!=devices.end())
	{
		const MidiDevice& device = *(it++);
		_pComboBoxMidiIn->addItem(device.szName.c_str(), device.id);
	}
	_pComboBoxMidiIn->blockSignals(false);

	if (_pComboBoxMidiIn->count() && _pComboBoxMidiOut->count())
	{
		_pPushButtonStart->setEnabled(true);
		_pPushButtonStop->setEnabled(false);
	}
}

void MidiDevicesWidget::setMidiOutDevices(const MidiDevice::List& devices)
{
	_pComboBoxMidiOut->blockSignals(true);
	MidiDevice::List::const_iterator it = devices.begin();
	while (it!=devices.end())
	{
		const MidiDevice& device = *(it++);
		_pComboBoxMidiOut->addItem(device.szName.c_str(), device.id);
	}
	_pComboBoxMidiOut->blockSignals(false);

	if (_pComboBoxMidiIn->count() && _pComboBoxMidiOut->count())
	{
		_pPushButtonStart->setEnabled(true);
		_pPushButtonStop->setEnabled(false);
	}
}
