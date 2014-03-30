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
#include <QtCore/QObject>

#include <vector>

/**
 * Drumkit midi mapping between a pad and several midi notes.
 */
class DrumKitMidiMap
{
public:

	struct Description
	{
		typedef std::vector<Description> List;
		typedef std::vector<Pad::MidiDescription> Pads;
		std::string	name;
		Pads		pads;

		Description(const std::string& szName = std::string("default")):name(szName)
		{
			/**
			 * From MadCatz midi pro manual.
			 *		38, 31, 34, 37, 39, 40 Snare Red Pad
			 *		48, 50 Hi-tom Yellow Pad
			 *		45, 47 Low-tom Blue Pad
			 *		41, 43 Floor-tom Green Pad
			 *		22, 26, 42, 46, 54 Hi-Hat Cymbal Yellow Cymbal
			 *		51, 53, 56, 59 Ride Cymbal Blue Cymbal
			 *		49, 52, 55, 57 Crash Cymbal Green Cymbal
			 *		33, 35, 36 Kick Pedal Kick Pedal
			 *		44 Hi-Hat Pedal Hi-Hat Pedal
			 *		CC#4 (Foot controller) Hi-Hat Pedal Position Allows Hi-Hat pedal to be held closed
			 */
			pads.push_back(Pad::MidiDescription(Pad::SNARE, 38, Pad::getDefaultColor(Pad::SNARE)));
			pads.push_back(Pad::MidiDescription(Pad::HIHAT, 22, Pad::getDefaultColor(Pad::HIHAT)));
			pads.push_back(Pad::MidiDescription(Pad::HIHAT_PEDAL, 44, Pad::getDefaultColor(Pad::HIHAT_PEDAL)));
			pads.push_back(Pad::MidiDescription(Pad::TOM1, 48, Pad::getDefaultColor(Pad::TOM1)));
			pads.push_back(Pad::MidiDescription(Pad::TOM2, 45, Pad::getDefaultColor(Pad::TOM2)));
			pads.push_back(Pad::MidiDescription(Pad::TOM3, 41, Pad::getDefaultColor(Pad::TOM3)));
			pads.push_back(Pad::MidiDescription(Pad::CRASH1, 49, Pad::getDefaultColor(Pad::CRASH1)));
			pads.push_back(Pad::MidiDescription(Pad::CRASH2, 52, Pad::getDefaultColor(Pad::CRASH2)));
			pads.push_back(Pad::MidiDescription(Pad::CRASH3, 26, Pad::getDefaultColor(Pad::CRASH3)));
			pads.push_back(Pad::MidiDescription(Pad::RIDE, 51, Pad::getDefaultColor(Pad::RIDE)));
			pads.push_back(Pad::MidiDescription(Pad::BASS_DRUM, 33, Pad::getDefaultColor(Pad::BASS_DRUM)));
		}

	private:
		friend class boost::serialization::access;
		template<class Archive> void serialize(Archive & ar, const unsigned int)
		{
			ar  & BOOST_SERIALIZATION_NVP(name);
			ar  & BOOST_SERIALIZATION_NVP(pads);
		}
	};

public:
	DrumKitMidiMap():
		_hiHatControlCC(4),
		_selectedId(0)
   	{
		Description rightHanded(QObject::tr("Right Handed").toStdString());
		_descriptions.push_back(rightHanded);

		Description leftHanded(QObject::tr("Left Handed").toStdString());
		leftHanded.pads.clear();
		leftHanded.pads.push_back(Pad::MidiDescription(Pad::SNARE, 41, Pad::getDefaultColor(Pad::TOM3)));
		leftHanded.pads.push_back(Pad::MidiDescription(Pad::HIHAT, 51, Pad::getDefaultColor(Pad::RIDE)));
		leftHanded.pads.push_back(Pad::MidiDescription(Pad::HIHAT_PEDAL, 44, Pad::getDefaultColor(Pad::HIHAT_PEDAL)));
		leftHanded.pads.push_back(Pad::MidiDescription(Pad::TOM1, 45, Pad::getDefaultColor(Pad::TOM2)));
		leftHanded.pads.push_back(Pad::MidiDescription(Pad::TOM2, 48, Pad::getDefaultColor(Pad::TOM1)));
		leftHanded.pads.push_back(Pad::MidiDescription(Pad::TOM3, 38, Pad::getDefaultColor(Pad::SNARE)));
		leftHanded.pads.push_back(Pad::MidiDescription(Pad::CRASH1, 49, Pad::getDefaultColor(Pad::SNARE)));
		leftHanded.pads.push_back(Pad::MidiDescription(Pad::CRASH2, 52, Pad::getDefaultColor(Pad::SNARE)));
		leftHanded.pads.push_back(Pad::MidiDescription(Pad::CRASH3, 53, Pad::getDefaultColor(Pad::RIDE)));
		leftHanded.pads.push_back(Pad::MidiDescription(Pad::RIDE, 22, Pad::getDefaultColor(Pad::HIHAT)));
		leftHanded.pads.push_back(Pad::MidiDescription(Pad::BASS_DRUM, 33, Pad::getDefaultColor(Pad::BASS_DRUM)));
		_descriptions.push_back(leftHanded);

		Description custom(QObject::tr("Custom").toStdString());
		_descriptions.push_back(custom);
   	}

	virtual ~DrumKitMidiMap() {}

	const Description& getDescription() const {return _descriptions[_selectedId];}
	Description& getDescription() {return _descriptions[_selectedId];}

	unsigned short getHiHatControlCC() const {return _hiHatControlCC;}
	void setHiHatControlCC(unsigned short note) {_hiHatControlCC = note;}
	void select(size_t i) {_selectedId = i;}
	size_t getSelectedId() const {return _selectedId;}

private:
	unsigned short		_hiHatControlCC;
	Description::List   _descriptions;
	size_t				_selectedId;

private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int)
	{
		ar  & BOOST_SERIALIZATION_NVP(_hiHatControlCC);
		ar  & BOOST_SERIALIZATION_NVP(_descriptions);
		ar  & BOOST_SERIALIZATION_NVP(_selectedId);
	}
};

BOOST_CLASS_VERSION(DrumKitMidiMap, 0)
BOOST_CLASS_VERSION(DrumKitMidiMap::Description, 0)
