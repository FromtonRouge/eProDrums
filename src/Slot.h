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

#include "Pad.h"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/bitset.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp> 

#include <boost/filesystem.hpp>
#include <boost/bind.hpp>

#include <algorithm>
#include <vector>
#include <string>
#include <bitset>

class DrumKitMidiMap;

/**
 * Slot configuration.
 * Note: copyable and assignable
 * but you have to do connections manually
 * \see connectMutex()
 */
class Slot
{
public:
	typedef boost::shared_ptr<Slot> Ptr;
	typedef std::vector<Ptr> List;

	typedef std::bitset<5> MutableCrashSettings;
	static MutableCrashSettings CRASH_CRASH;
	static MutableCrashSettings CRASH_RIDE;
	static MutableCrashSettings CRASH_SNARE;
	static MutableCrashSettings CRASH_TOM2;
	static MutableCrashSettings CRASH_TOM3;

public:

	Slot(): _cymbalSimHitWindow(0) {}
	Slot(const Slot& rOther);
	Slot& operator=(const Slot& rOther);

	virtual ~Slot() {}

	const std::string& getName() const {return _szSlotName;}
	void setName(const std::string& szName) {_szSlotName = szName;}
	const Pad::List& getPads() const {return _pads;}
	Pad::List& getPads() {return _pads;}

	void onDrumKitLoaded(DrumKitMidiMap*, const boost::filesystem::path&);

	bool isMutableCrash(const MutableCrashSettings& bit) const {return (_mutableCrashSettings & bit) == bit; }
	void setMutableCrash(const MutableCrashSettings& bit, bool state) { _mutableCrashSettings = state?_mutableCrashSettings|bit:_mutableCrashSettings^bit; }

	int getCymbalSimHitWindow() const {return _cymbalSimHitWindow;}
	void setCymbalSimHitWindow(int simHit) {_cymbalSimHitWindow = simHit;}

	void connectMutex(const Pad::SignalLockMutex::slot_function_type& funcLock, const Pad::SignalUnlockMutex::slot_function_type& funcUnlock)
   	{
		std::for_each(_pads.begin(), _pads.end(), boost::bind(&Pad::connectMutex, _1, funcLock, funcUnlock));
	}

private:
	std::string  _szSlotName;
	Pad::List    _pads;
    int          _cymbalSimHitWindow;

	/**
	 * 0 = crash-crash
	 * 1 = crash-ride
	 * 2 = crash-snare
	 * 3 = crash-tom2
	 * 4 = crash-tom3
	 */
	MutableCrashSettings _mutableCrashSettings;

private:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive & ar, const unsigned int fileVersion)
	{
		ar & BOOST_SERIALIZATION_NVP(_szSlotName);
		ar & BOOST_SERIALIZATION_NVP(_pads);
		ar & BOOST_SERIALIZATION_NVP(_cymbalSimHitWindow);
		ar & BOOST_SERIALIZATION_NVP(_mutableCrashSettings);
	}
};

BOOST_CLASS_VERSION(Slot, 0)
