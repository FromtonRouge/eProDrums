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

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>

/**
 * A midi note with a hit zone information.
 */
struct DrumNote
{
	enum HitZone
	{
		NONE=0,
		HEAD,
		RIM,
		BOW,
		EDGE,
		BELL
	};

	explicit DrumNote(int midiNote=0, HitZone zone = NONE):midiNote(midiNote), hitZone(zone) {}

	int		midiNote;
	HitZone	hitZone;

private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int)
	{
		ar  & BOOST_SERIALIZATION_NVP(midiNote);
		ar  & BOOST_SERIALIZATION_NVP(hitZone);
	}
};

class DrumNoteItemModel;

/**
 * DrumNote container
 */
struct DrumNotes
{
	friend class DrumNoteItemModel;

	typedef boost::multi_index_container<DrumNote, boost::multi_index::indexed_by< 
		boost::multi_index::random_access<>,
		boost::multi_index::hashed_non_unique< BOOST_MULTI_INDEX_MEMBER(DrumNote, DrumNote::HitZone, hitZone) > ,
		boost::multi_index::hashed_unique< BOOST_MULTI_INDEX_MEMBER(DrumNote, int, midiNote) > > > Container;

	typedef Container::nth_index<2>::type::iterator IteratorMidiNote;

	size_t size() const {return _container.size();}
	const DrumNote& at(size_t i) const {return _container[i];}

	void insert(const DrumNote& drumNote)
	{
		_container.push_back(drumNote);
	}

	void eraseMidiNote(int midiNote)
	{
		_container.get<2>().erase(midiNote);
	}

	void clearMidiNote()
	{
		_container.get<2>().clear();
	}

	IteratorMidiNote beginMidiNote() const
	{
		return _container.get<2>().begin();
	}

	IteratorMidiNote findMidiNote(int midiNote) const
	{
		return _container.get<2>().find(midiNote);
	}

	IteratorMidiNote endMidiNote() const
	{
		return _container.get<2>().end();
	}

private:
	Container _container;

private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int)
	{
		ar  & BOOST_SERIALIZATION_NVP(_container);
	}
};

BOOST_CLASS_VERSION(DrumNote, 0)
BOOST_CLASS_VERSION(DrumNotes, 0)
