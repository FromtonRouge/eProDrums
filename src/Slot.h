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

#include <QtCore/QMetaType>
#include <QtCore/QString>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp> 
#include <boost/thread/recursive_mutex.hpp>
#include <boost/filesystem.hpp>

#include <algorithm>
#include <vector>
#include <string>

class DrumKitMidiMap;
class ParamItemModel;

/**
 * Slot configuration.
 * Note: Thread safe
 */
class Slot
{
public:
	typedef boost::shared_ptr<Slot> Ptr;
	typedef std::vector<Ptr> List;

private:
	typedef boost::recursive_mutex Mutex;

public:

	Slot();
	Slot(const Slot& rOther);
	Slot& operator=(const Slot& rOther);

	virtual ~Slot() {}

	const QString& getName() const;
	void setName(const QString& szName);
	const Pad::List& getPads() const;
	Pad::List& getPads();
	void onDrumKitLoaded(DrumKitMidiMap*, const boost::filesystem::path&);

private:
	mutable Mutex		_mutex;
	QString		  		_szSlotName;
	Pad::List    		_pads;

public:
	Property<int>::Ptr					cymbalSimHitWindow;
	Property<bool>::Ptr					isChameleonCrashWithCrash;
	Property<bool>::Ptr					isChameleonCrashWithRide;
	Property<bool>::Ptr					isChameleonCrashWithSnare;
	Property<bool>::Ptr					isChameleonCrashWithTom2;
	Property<bool>::Ptr					isChameleonCrashWithTom3;
	boost::shared_ptr<ParamItemModel>	model;

private:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive & ar, const unsigned int)
	{
		ar & BOOST_SERIALIZATION_NVP(_szSlotName);
		ar & BOOST_SERIALIZATION_NVP(_pads);
		ar & BOOST_SERIALIZATION_NVP(cymbalSimHitWindow);
		ar & BOOST_SERIALIZATION_NVP(isChameleonCrashWithCrash);
		ar & BOOST_SERIALIZATION_NVP(isChameleonCrashWithRide);
		ar & BOOST_SERIALIZATION_NVP(isChameleonCrashWithSnare);
		ar & BOOST_SERIALIZATION_NVP(isChameleonCrashWithTom2);
		ar & BOOST_SERIALIZATION_NVP(isChameleonCrashWithTom3);
	}
};

BOOST_CLASS_VERSION(Slot, 0)

Q_DECLARE_METATYPE(Slot)
Q_DECLARE_METATYPE(Slot::Ptr)
