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

#include "DrumKitMidiMap.h"

#include "ui_SettingsDlg.h"

#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>

#include <vector>

// Forward declarations
class DrumKitItemModel;
class DrumKitItemDelegate;
class Settings;
class MidiMessage;

/**
 * Settings dialog.
 */
class SettingsDlg : public QDialog, private Ui::SettingsDlg
{
	Q_OBJECT

public:
	enum SettingIndex
	{
		SETTING_DRUM_KIT,
		SETTING_CURVE
	};

signals:
	void onMidiIn(const MidiMessage&);

public:
	SettingsDlg(Settings* pSettings, QWidget* pParent=NULL);
	virtual ~SettingsDlg();

	virtual void accept();

private Q_SLOTS:

	void on_treeWidget_itemSelectionChanged();
	void on_stackedWidget_currentChanged(int);
	void on_pushButtonSetupDrumKit_clicked(bool checked=false);
	void on_pushButtonOpen_clicked(bool checked=false);
	void on_pushButtonSave_clicked(bool checked=false);
	void on_spinBoxRefreshPeriod_valueChanged(int value);
	void on_spinBoxCurveWindowLength_valueChanged(int value);
	void on_radioButtonRightHanded_toggled(bool checked);
	void on_radioButtonLeftHanded_toggled(bool checked);
	void on_radioButtonCustom_toggled(bool checked);

private:

	void onDrumKitLoaded(DrumKitMidiMap* pDrumKit, const boost::filesystem::path& pathConfig = boost::filesystem::path());

private:
	Settings*								_pSettings;
	boost::scoped_ptr<DrumKitItemModel>		_pDrumKitItemModel;
	boost::scoped_ptr<DrumKitItemDelegate>	_pDrumKitItemDelegate;
	boost::signals2::connection				_connectionToSettings;
};
