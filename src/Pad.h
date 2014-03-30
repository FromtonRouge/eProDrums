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

#include "MidiMessage.h"
#include "DrumNote.h"
#include "Property.h"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/string.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <QtCore/QString>
#include <vector>

/**
 * Custom serialization for some Qt types
 */
namespace boost
{
	namespace serialization
	{
		template<class Archive> void serialize(Archive& ar, QPointF& point, const unsigned int)
		{
			ar & boost::serialization::make_nvp("x", point.rx());
			ar & boost::serialization::make_nvp("y", point.ry());
		}

		template<class Archive> void serialize(Archive& ar, QPolygonF& points, const unsigned int version)
		{
			boost::serialization::split_free(ar, points, version);
		}

		template<class Archive> void load(Archive& ar, QPolygonF& points, const unsigned int)
		{
			int size = 0;
			ar >> boost::serialization::make_nvp("size", size);
			points.resize(size);
			for (int i=0; i<points.size();++i)
			{
				QPointF& rPoint = points[i];
				ar >> boost::serialization::make_nvp("point", rPoint);
			}
		}

		template<class Archive> void save(Archive& ar, const QPolygonF& points, const unsigned int)
		{
			int size = points.size();
			ar << boost::serialization::make_nvp("size", size);
			for (int i=0; i<points.size();++i)
			{
				ar << boost::serialization::make_nvp("point", points[i]);
			}
		}

		template<class Archive> void serialize(Archive& ar, QString& sz, const unsigned int version)
		{
			boost::serialization::split_free(ar, sz, version);
		}

		template<class Archive> void load(Archive& ar, QString& sz, const unsigned int)
		{
			std::string szStd;
			ar >> boost::serialization::make_nvp("string", szStd);
			sz = szStd.c_str();
		}

		template<class Archive> void save(Archive& ar, const QString& sz, const unsigned int)
		{
			const std::string& szStd = sz.toStdString();
			ar << boost::serialization::make_nvp("string", szStd);
		}
	}
}

BOOST_CLASS_VERSION(QPointF, 0)
BOOST_CLASS_VERSION(QPolygonF, 0)
BOOST_CLASS_VERSION(QString, 0)

/**
 * Note: Thread safe
 */
class Pad
{
public:
	typedef boost::shared_ptr<Pad> Ptr;
	typedef std::vector<Ptr> List;

	enum Type
	{
		UNKNOWN=-1,
		SNARE=0,
		HIHAT,
		HIHAT_PEDAL,
		TOM1,
		TOM2,
		TOM3,
		CRASH1, // Left green crash
		CRASH2, // Right green crash
		CRASH3, // Yellow crash
		RIDE,
		BASS_DRUM,
		TYPE_COUNT
	};

	enum DefaultOutputNote
	{
		NOTE_SNARE = 38,
		NOTE_HIHAT = 22,
		NOTE_HIHAT_PEDAL = 44,
		NOTE_TOM1 = 48,
		NOTE_TOM2 = 45,
		NOTE_TOM3 = 41,
		NOTE_CRASH1 = 49,
		NOTE_CRASH2 = 52,
		NOTE_CRASH3 = 26,
		NOTE_RIDE = 51,
		NOTE_BASS_DRUM = 33
	};

	/**
	 * Pad description, contains the pad name and list of mapped midi notes.
	 */
	struct MidiDescription
	{
		MidiDescription(Type type = SNARE, int outputNote=0, const QString& color = QString()):
			type(type),
		   	color(color),
		   	outputNote(outputNote)
	   	{}

		Type			type;
		QString			color;
		DrumNotes		inputNotes;
		int				outputNote;

	private:
		friend class boost::serialization::access;
		template<class Archive> void serialize(Archive & ar, const unsigned int)
		{
			ar  & BOOST_SERIALIZATION_NVP(type);
			ar  & BOOST_SERIALIZATION_NVP(color);
			ar  & BOOST_SERIALIZATION_NVP(outputNote);
			ar  & BOOST_SERIALIZATION_NVP(inputNotes);
		}
	};

protected:
	typedef boost::recursive_mutex Mutex;

public:
	static std::map<int, QString>	DICT_NAMES;
	static std::map<int, QString>	DICT_COLORS;

public:
	static QString getName(Type type);
	static QString getDefaultColor(Type type);

public:
	Pad(Type type = SNARE, int defaultMidiNote=0);
	Pad(const Pad& rOther);
	Pad& operator=(const Pad& rOther);

	virtual ~Pad();

public:
	void		setInputNotes(const DrumNotes& notes);
	bool		isA(int midiNote) const;
	bool		isA(int midiNote, DrumNote::HitZone hitZone) const;
	QString		getName() const;

	/**
	 * Compute flams, ghosts on current and next midi message.
	 * @return midi message to send if any
	 */
	void applyFlamAndGhost(const List& drumKit, const MidiMessage::DictHistory& lastMsgSent, MidiMessage* pCurrent, MidiMessage* pNext, MidiMessage::List& rResult);

private:
	bool isFlamAllowed(const MidiMessage& beforeFlamHit, const MidiMessage& flamHit) const;

protected:
	mutable Mutex	_mutex;

private:
	DrumNotes		_drumNotes;

public:
	Property<Type>::Ptr			type;
	Property<QString>::Ptr		color;				///! Not saved
	Property<int>::Ptr			defaultOutputNote;
	Property<int>::Ptr			typeFlam;
	Property<int>::Ptr			ghostVelocityLimit;
	Property<bool>::Ptr			isFlamActivated;
	Property<QPolygonF>::Ptr	funcFlams;
	Property<int>::Ptr			flamCancelDuringRoll;

private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int)
	{
		// Basic Pad
		ar  & BOOST_SERIALIZATION_NVP(type);
		ar  & BOOST_SERIALIZATION_NVP(defaultOutputNote);
		ar  & BOOST_SERIALIZATION_NVP(typeFlam);
		ar  & BOOST_SERIALIZATION_NVP(ghostVelocityLimit);
		ar  & BOOST_SERIALIZATION_NVP(isFlamActivated);
		ar  & BOOST_SERIALIZATION_NVP(funcFlams);
		ar  & BOOST_SERIALIZATION_NVP(flamCancelDuringRoll);
	}
};

BOOST_CLASS_VERSION(Pad, 0)
BOOST_CLASS_VERSION(Pad::MidiDescription, 0)
