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

#include "Settings.h"
#include <QtGui/QMessageBox>
#include <QtGui/QApplication>

#include <boost/format.hpp>

namespace fs = boost::filesystem;

Settings::Settings():
	_qSettings("FromtonRouge", "eProDrums")
{
	// Load the drum kit midi configuration
	loadDrumKitMidiMap(getDrumKitConfigPath());
}

Settings::~Settings()
{
}

fs::path Settings::getUserSettingsFile() const
{
	return fs::path(_qSettings.value("UserSettingsFile", "").toString().toStdString());
}

void Settings::setUserSettingsFile(const std::string& szUserSettingsFile)
{
	_qSettings.setValue("UserSettingsFile", szUserSettingsFile.c_str());
}

fs::path Settings::getDrumKitConfigPath() const
{
	return fs::path(_qSettings.value("DrumKitConfigFile", "").toString().toStdString());
}

void Settings::setDrumKitConfigFile(const std::string& szDrumKitConfigFile)
{
	_qSettings.setValue("DrumKitConfigFile", szDrumKitConfigFile.c_str());
}

std::string Settings::getMidiIn() const
{
	return _qSettings.value("MidiIn").toString().toStdString();
}

void Settings::setMidiIn(const std::string& szMidiIn)
{
	_qSettings.setValue("MidiIn", szMidiIn.c_str());
}

std::string Settings::getMidiOut() const
{
	return _qSettings.value("MidiOut").toString().toStdString();
}

void Settings::setMidiOut(const std::string& szMidiOut)
{
	_qSettings.setValue("MidiOut", szMidiOut.c_str());
}

int Settings::getLastSelectedSlotIndex() const
{
	return _qSettings.value("LastSelectedSlotIndex", 0).toInt();
}

void Settings::setLastSelectedSlotIndex(int index)
{
	_qSettings.setValue("LastSelectedSlotIndex", index);
}

void Settings::saveDrumKitMidiMap(const boost::filesystem::path& pathConfig)
{
	if (pathConfig.empty())
	{
		return;
	}

	try
	{
		std::ofstream ofs(pathConfig.generic_string().c_str());
		boost::archive::xml_oarchive oa(ofs);
		oa << BOOST_SERIALIZATION_NVP(_drumKitMidiMap);
		setDrumKitConfigFile(pathConfig.generic_string());
		_onKitDefined(&_drumKitMidiMap, pathConfig);
	}
	catch (const std::exception& e)
	{
		boost::format fmtMsg("Error while saving drumkit configuration. Reason : %s");
		fmtMsg%e.what();
		QMessageBox::critical(QApplication::activeWindow(),"Error while saving configuration", fmtMsg.str().c_str());

		setDrumKitConfigFile("");
	}
}

void Settings::reloadDrumKitMidiMap()
{
	loadDrumKitMidiMap(getDrumKitConfigPath());
}

void Settings::loadDrumKitMidiMap(const boost::filesystem::path& pathConfig)
{
	if (pathConfig.empty())
	{
		// Default drum kit map
		setDrumKitConfigFile("");
		_onKitDefined(&_drumKitMidiMap, fs::path());
	}
	else if (fs::exists(pathConfig))
	{
		std::ifstream ifs(pathConfig.generic_string().c_str());
		if (ifs.good())
		{
			boost::archive::xml_iarchive ia(ifs);
			try
			{
				ia >> BOOST_SERIALIZATION_NVP(_drumKitMidiMap);
				setDrumKitConfigFile(pathConfig.generic_string());
				_onKitDefined(&_drumKitMidiMap, pathConfig);
			}
			catch (const std::exception& e)
			{
				boost::format fmtMsg("Error while loading drumkit midi mapping. Reason : %s");
				fmtMsg%e.what();
				QMessageBox::critical(QApplication::activeWindow(),"Error while loading configuration", fmtMsg.str().c_str());
				
				// Default drum kit map
				setDrumKitConfigFile("");
				_drumKitMidiMap = DrumKitMidiMap();
				_onKitDefined(&_drumKitMidiMap, fs::path());
			}
		}
		else
		{
			// Default drum kit map
			setDrumKitConfigFile("");
			_onKitDefined(&_drumKitMidiMap, fs::path());
		}
	}
	else
	{
		boost::format fmtMsg("Error while loading drumkit midi mapping. Reason : %s");
		fmtMsg%(boost::format("The file %s does not exist")%pathConfig);
		QMessageBox::critical(QApplication::activeWindow(),"Error while loading configuration", fmtMsg.str().c_str());

		// Default drum kit map
		setDrumKitConfigFile("");
		_onKitDefined(&_drumKitMidiMap, fs::path());
	}
}

int Settings::getRedrawPeriod() const
{
	return _qSettings.value("RedrawPeriod", 25).toInt();
}

void Settings::setRedrawPeriod(int periodInMs)
{
	_qSettings.setValue("RedrawPeriod", periodInMs);
	_onRedrawPeriodChanged(periodInMs);
}

int Settings::getCurveWindowLength() const
{
	return _qSettings.value("CurveWindowLenthInSeconds", 5).toInt();
}

void Settings::setCurveWindowLength(int value)
{
	_qSettings.setValue("CurveWindowLenthInSeconds", value);
	_onCurveWindowLengthChanged(value);
}
