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

#include <QtCore/QSettings>

#include <boost/filesystem.hpp>
#include <boost/signals2.hpp>

#include <string>

/**
 * Contains and manage all settings.
 */
class Settings
{
public:
	typedef boost::signals2::signal<void (DrumKitMidiMap*, const boost::filesystem::path&)> OnKitDefined;
	typedef boost::signals2::signal<void (int)> OnRedrawPeriodChanged;
	typedef boost::signals2::signal<void (int)> OnCurveWindowLengthChanged;

public:
	Settings();
	virtual ~Settings();

public:
	boost::filesystem::path getUserSettingsFile() const;
	void setUserSettingsFile(const std::string& szPadConfigFile);

	boost::filesystem::path getDrumKitConfigPath() const;
	void setDrumKitConfigFile(const std::string& szDrumKitConfigFile);

public:
	std::string getMidiIn() const;
	void setMidiIn(const std::string& szMidiIn);

	std::string getMidiOut() const;
	void setMidiOut(const std::string& szMidiOut);

	int getLastSelectedSlotIndex() const;
	void setLastSelectedSlotIndex(int index);

	DrumKitMidiMap* getDrumKitMidiMap() {return &_drumKitMidiMap;}
	void setDrumKitMidiMap(const DrumKitMidiMap& drumKitMidiMap) {_drumKitMidiMap = drumKitMidiMap;}

	int getRedrawPeriod() const;
	void setRedrawPeriod(int periodInMs);

	int getCurveWindowLength() const;
	void setCurveWindowLength(int value);

public:
	boost::signals2::connection connectDrumKitMidiMapLoaded(const OnKitDefined::slot_function_type& func) {return _onKitDefined.connect(func);}
	boost::signals2::connection connectRedrawPeriodChanged(const OnRedrawPeriodChanged::slot_function_type& func) {return _onRedrawPeriodChanged.connect(func);}
	boost::signals2::connection connectCurveWindowLengthChanged(const OnCurveWindowLengthChanged::slot_function_type& func) {return _onCurveWindowLengthChanged.connect(func);}

	void saveDrumKitMidiMap(const boost::filesystem::path& pathConfig);
	void loadDrumKitMidiMap(const boost::filesystem::path& pathConfig);
	void reloadDrumKitMidiMap();

private:
	OnKitDefined				_onKitDefined;
	OnRedrawPeriodChanged		_onRedrawPeriodChanged;
	OnCurveWindowLengthChanged	_onCurveWindowLengthChanged;
	QSettings					_qSettings;				///< Core settings.
	DrumKitMidiMap				_drumKitMidiMap;			///< DrumKitMidiMap midi settings.
};
