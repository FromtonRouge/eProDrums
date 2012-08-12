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
#include <boost/thread/recursive_mutex.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>

#include <algorithm>
#include <vector>
#include <string>
#include <bitset>

class DrumKitMidiMap;

/**
 * Slot configuration.
 * Note: Thread safe
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

private:
	typedef boost::recursive_mutex Mutex;

public:

	Slot(): _cymbalSimHitWindow(0) {}
	Slot(const Slot& rOther);
	Slot& operator=(const Slot& rOther);

	virtual ~Slot() {}

	const std::string& getName() const;
	void setName(const std::string& szName);
	const Pad::List& getPads() const;
	Pad::List& getPads();
	void onDrumKitLoaded(DrumKitMidiMap*, const boost::filesystem::path&);
	bool isMutableCrash(const MutableCrashSettings& bit) const;
	void setMutableCrash(const MutableCrashSettings& bit, const Parameter::Value& state);
	int getCymbalSimHitWindow() const;
	void setCymbalSimHitWindow(const Parameter::Value& simHit);

private:
	mutable Mutex	_mutex;
	std::string  	_szSlotName;
	Pad::List    		_pads;

	Parameter::Value   	_cymbalSimHitWindow;

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
    template<class Archive> void serialize(Archive & ar, const unsigned int)
	{
		ar & BOOST_SERIALIZATION_NVP(_szSlotName);
		ar & BOOST_SERIALIZATION_NVP(_pads);
		ar & BOOST_SERIALIZATION_NVP(_cymbalSimHitWindow);
		ar & BOOST_SERIALIZATION_NVP(_mutableCrashSettings);
	}
};

BOOST_CLASS_VERSION(Slot, 0)
