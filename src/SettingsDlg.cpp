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

#include "SettingsDlg.h"
#include "DrumKitItemModel.h"
#include "DrumKitItemDelegate.h"
#include "AddMidiNote.h"
#include "Settings.h"

#include <QtGui/QFileDialog>

#include <windows.h>

#include <boost/bind.hpp>

namespace fs = boost::filesystem;

SettingsDlg::SettingsDlg(Settings* pSettings, QWidget* pParent):QDialog(pParent),
	_pSettings(pSettings),
	_pDrumKitItemModel(new DrumKitItemModel()),
	_pDrumKitItemDelegate(new DrumKitItemDelegate())
{
	setupUi(this);

	// Setting item index data
	for (int i=0;i<treeWidget->topLevelItemCount();++i)
	{
		QTreeWidgetItem* p =treeWidget->topLevelItem(i);
		p->setData(0, Qt::UserRole, i);
	}
	
	// boost connections
	_connectionToSettings = _pSettings->connectDrumKitMidiMapLoaded(boost::bind(&SettingsDlg::onDrumKitLoaded, this, _1, _2));

	// Qt connections
	connect(this, SIGNAL(midiNoteOn(int, int)), _pDrumKitItemDelegate.get(), SIGNAL(midiNoteOn(int, int)));

	// Building drum kit view
	tableViewDrumKit->setModel(_pDrumKitItemModel.get());
	tableViewDrumKit->setItemDelegate(_pDrumKitItemDelegate.get());
	tableViewDrumKit->horizontalHeader()->setStretchLastSection(true);
	tableViewDrumKit->verticalHeader()->hide();

	// Set the default stacked widget
	treeWidget->topLevelItem(0)->setSelected(true);
	on_stackedWidget_currentChanged(SETTING_DRUM_KIT);

	// Load the drumkit
	_pSettings->loadDrumKitMidiMap(_pSettings->getDrumKitConfigPath());
}

SettingsDlg::~SettingsDlg()
{
	_connectionToSettings.disconnect();
}

void SettingsDlg::on_treeWidget_itemSelectionChanged()
{
	// Select the correct stacked widget index
    const QList<QTreeWidgetItem*>& selected = treeWidget->selectedItems();
    if (!selected.empty())
    {
		QTreeWidgetItem* pItem = selected[0];
		const QVariant& variant = pItem->data(0, Qt::UserRole);
		stackedWidget->setCurrentIndex(variant.toInt());
	}
}

void SettingsDlg::on_stackedWidget_currentChanged(int index)
{
	switch (index)
	{
	case SETTING_DRUM_KIT:
		{
			break;
		}
	case SETTING_CURVE:
		{
			spinBoxRefreshPeriod->setValue(25);
			spinBoxCurveWindowLength->setValue(5);
			break;
		}
	default:
		{
			break;
		}
	}
}

void SettingsDlg::on_pushButtonSetupDrumKit_clicked(bool)
{
	AddMidiNote dlg;
	dlg.showPrevNextButtons();
	connect(this, SIGNAL(midiNoteOn(int, int)), &dlg, SLOT(onMidiNoteOn(int, int)));

	DrumKitMidiMap* pDrumKit = _pSettings->getDrumKitMidiMap();
	DrumKitMidiMap::Description& rDescription = pDrumKit->getDescription();
	for (size_t i=0;i<rDescription.size();)
	{
		Pad::MidiDescription& padDescription = rDescription[i];

		// Clear the dialog first and set pad info
		dlg.clear();
		dlg.setWindowTitle(padDescription.getTypeLabel().c_str());
		dlg.setNotes(padDescription.midiNotes);

		if (dlg.exec())
		{
			// Get the result
			padDescription.midiNotes = dlg.getNotes();

			// Save the result in the data model
			QVariant variant;
			variant.setValue(padDescription);
			_pDrumKitItemModel->setData(_pDrumKitItemModel->index(i, 1), variant);

			// Go to the next or previous pad
			int state = dlg.getPrevNextState();
			if (state>=0)
			{
				++i;
			}
			else if (state<0)
			{
				// Prev
				if (i>0)
				{
					--i;
				}
			}
		}
		else
		{
			// Cancel
			break;
		}
	}
}

void SettingsDlg::on_pushButtonOpen_clicked(bool)
{
	const QString& filePath = QFileDialog::getOpenFileName(this, "Open drum kit mapping", "./", "Drum Kit Mapping (*.kit)");
	if (filePath.isNull())
	{
		return;
	}

	_pSettings->loadDrumKitMidiMap(fs::path(filePath.toStdString()));
}

void SettingsDlg::on_pushButtonSave_clicked(bool)
{
	const QString& filePath = QFileDialog::getSaveFileName(this, "Save drum kit mapping", "./", "Drum Kit Mapping (*.kit)");
    if (filePath.isNull())
	{
		return;
	}

	_pSettings->saveDrumKitMidiMap(fs::path(filePath.toStdString()));
	lineEditPath->setText(filePath);
}

void SettingsDlg::onDrumKitLoaded(DrumKitMidiMap* pDrumKit, const fs::path& pathConfig)
{
	spinBoxCC->setValue(pDrumKit->getHiHatControlCC());
	_pDrumKitItemModel->setDrumKit(pDrumKit);

	// Open persistent editor
	for (int i=0; i<_pDrumKitItemModel->rowCount(); ++i)
	{
		tableViewDrumKit->openPersistentEditor(_pDrumKitItemModel->index(i, 1));
	}

	lineEditPath->setText(pathConfig.generic_string().c_str());
}

void SettingsDlg::on_spinBoxCC_valueChanged(int value)
{
	_pSettings->getDrumKitMidiMap()->setHiHatControlCC(value);
}

void SettingsDlg::accept()
{
	if (fs::exists(_pSettings->getDrumKitConfigPath()))
	{
		// Be sure to save the drum map before leaving
		_pSettings->saveDrumKitMidiMap(_pSettings->getDrumKitConfigPath());
		QDialog::accept();
	}
	else
	{
		// Ask the user to save the drum map
		on_pushButtonSave_clicked();

		// Drum map saved properly ?
		if (fs::exists(_pSettings->getDrumKitConfigPath()))
		{
			QDialog::accept();
		}
	}
}
