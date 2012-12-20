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
#include "Slot.h"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/bitset.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/export.hpp> 

#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>

#include <string>
#include <vector>
#include <algorithm>
#include <bitset>

/**
 * Structure containing all parameters.
 */
struct UserSettings
{
	typedef std::bitset<13>	CurveVisibility;
	typedef std::bitset<5>	LogActivation;

	static CurveVisibility CURVE_HIHAT_CONTROL;
	static CurveVisibility CURVE_HIHAT_ACCELERATION;
	static CurveVisibility CURVE_HIHAT_JERK;
	static CurveVisibility CURVE_HIHAT;
	static CurveVisibility CURVE_HIHAT_PEDAL;
	static CurveVisibility CURVE_CRASH;
	static CurveVisibility CURVE_YELLOW_CRASH;
	static CurveVisibility CURVE_RIDE;
	static CurveVisibility CURVE_TOM1;
	static CurveVisibility CURVE_TOM2;
	static CurveVisibility CURVE_TOM3;
	static CurveVisibility CURVE_SNARE;
	static CurveVisibility CURVE_BASS_PEDAL;

	static LogActivation LOG_ACTIVATED;
	static LogActivation LOG_RAW_DATA;
	static LogActivation LOG_FILTERED_DATA;
	static LogActivation LOG_HIHAT_CONTROL;
	static LogActivation LOG_OTHERS;

	/**
	 * Constructor.
	 */
	UserSettings(): bufferLength(0)
	{
		curveVisibility.set();	// set all
		setCurveVisibility(CURVE_HIHAT_ACCELERATION, false);
		setCurveVisibility(CURVE_HIHAT_JERK, false);
   	}

	/**
	 * Destructor.
	 */
	virtual ~UserSettings() { }

	bool operator==(const UserSettings&) const
	{
		// TODO
		return true;
	}

	bool operator!=(const UserSettings& rObj) const {return !((*this)==rObj);}

	bool isCurveVisible(const CurveVisibility& bit) const {return (curveVisibility & bit) == bit; }
	void setCurveVisibility(const CurveVisibility& bit, bool state) { curveVisibility = state?curveVisibility|bit:(curveVisibility|bit)^bit; }

	bool isLogs() const {return isLog(LOG_ACTIVATED);}
	bool isLog(const LogActivation& bit) const {return (logActivation & bit) == bit; }
	void setLog(const LogActivation& bit, bool state) { logActivation = state?logActivation|bit:logActivation^bit; }

public:
	std::string		filePath; ///< Config file path. Do not serialize this member
	LogActivation	logActivation;

    int				bufferLength;
	Slot::List		configSlots;
	CurveVisibility	curveVisibility;

private:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive & ar, const unsigned int)
	{
		ar  & BOOST_SERIALIZATION_NVP(bufferLength);
		ar  & BOOST_SERIALIZATION_NVP(configSlots);
		ar  & BOOST_SERIALIZATION_NVP(curveVisibility);
    }
};

BOOST_CLASS_VERSION(UserSettings, 0)
