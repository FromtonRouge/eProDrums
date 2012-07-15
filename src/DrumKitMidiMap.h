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
#include <boost/serialization/vector.hpp>

#include <vector>

/**
 * Drumkit midi mapping between a pad and several midi notes.
 */
class DrumKitMidiMap
{
public:
	typedef std::vector<Pad::MidiDescription> Description;

public:
	DrumKitMidiMap():_hiHatControlCC(4)
	{
		_description.push_back(Pad::MidiDescription(Pad::SNARE));
		_description.push_back(Pad::MidiDescription(Pad::HIHAT));
		_description.push_back(Pad::MidiDescription(Pad::HIHAT_PEDAL));
		_description.push_back(Pad::MidiDescription(Pad::TOM1));
		_description.push_back(Pad::MidiDescription(Pad::TOM2));
		_description.push_back(Pad::MidiDescription(Pad::TOM3));
		_description.push_back(Pad::MidiDescription(Pad::CRASH1));
		_description.push_back(Pad::MidiDescription(Pad::CRASH2));
		_description.push_back(Pad::MidiDescription(Pad::CRASH3));
		_description.push_back(Pad::MidiDescription(Pad::RIDE));
		_description.push_back(Pad::MidiDescription(Pad::BASS_DRUM));
	}

	virtual ~DrumKitMidiMap() {}

	const Description& getDescription() const {return _description;}
	Description& getDescription() {return _description;}

	unsigned short getHiHatControlCC() const {return _hiHatControlCC;}
	void setHiHatControlCC(unsigned short note) {_hiHatControlCC = note;}

private:
	unsigned short _hiHatControlCC;
	Description    _description;

private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int fileVersion)
	{
		ar  & BOOST_SERIALIZATION_NVP(_hiHatControlCC);
		ar  & BOOST_SERIALIZATION_NVP(_description);
	}
};

BOOST_CLASS_VERSION(DrumKitMidiMap, 0)
