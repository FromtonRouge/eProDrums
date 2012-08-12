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
#include "Parameter.h"

#include <QtGui/QColor>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#include <boost/format.hpp>
#include <boost/thread/recursive_mutex.hpp>

/**
 * Note: Thread safe
 */
class Pad : protected boost::noncopyable
{
public:
	typedef boost::shared_ptr<Pad> Ptr;
	typedef std::vector<Ptr> List;

	typedef boost::signals2::signal<void (const boost::format&)> SignalLog;

	enum Type
	{
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
		MidiDescription(Type type = SNARE):type(type) {}
		Type           type;
		MidiNotes midiNotes;

		std::string getTypeLabel() const;

	private:
		friend class boost::serialization::access;
		template<class Archive> void serialize(Archive & ar, const unsigned int)
		{
			ar  & BOOST_SERIALIZATION_NVP(type);
			ar  & BOOST_SERIALIZATION_NVP(midiNotes);
		}
	};

protected:
	typedef boost::recursive_mutex Mutex;

public:
	Pad():
		_defaultOutputNote(0),
		_ghostVelocityLimit(0),
		_flamVelocityFactor(0),
		_flamTimeWindow1(0),
		_flamTimeWindow2(0),
		_flamCancelDuringRoll(0)
	{}

	Pad(	Type type,
			int defaultMidiNote,
			int ghostVelocityLimit = 0,
			float flamVelocityFactor = 0,
			int flamTimeWindow1 = 0,
			int flamTimeWindow2 = 0,
			int flamCancelDuringRoll = 0);

	Pad(const Pad& rOther);
	Pad& operator=(const Pad& rOther);

	virtual ~Pad();

public:
	void connectLogs(const SignalLog::slot_function_type& funcFlams, const SignalLog::slot_function_type& funcGhost) {_signalLogFlams.connect(funcFlams); _signalLogGhost.connect(funcGhost);}
	MidiDescription getMidiDescription() const;
	void setMidiNotes(const MidiNotes& notes);
	Type getType() const;
	void setType(Type type);
	void setTypeFlam(Type type);
	Type getTypeFlam() const;
	std::string getName() const;
	bool isA(int note) const;
	int getDefaultOutputNote() const;
	int getGhostVelocityLimit() const;
	void setGhostVelocityLimit(const Parameter::Value& velocity);
	float getFlamVelocityFactor() const;
	void setFlamVelocityFactor(float value);
	int getFlamTimeWindow1() const;
	void setFlamTimeWindow1(int value);
	int getFlamTimeWindow2() const;
	void setFlamTimeWindow2(int value);
	int getFlamCancelDuringRoll() const;
	void setFlamCancelDuringRoll(int value);
	QColor getColor() const {return QColor(_color.c_str());}
	void setColor(const QColor& color) {_color = color.name().toStdString();}

	/**
	 * Compute flams, ghosts on current and next midi message.
	 * @return midi message to send if any
	 */
	MidiMessage::List applyFlamAndGhost(const List& drumKit, const MidiMessage::DictHistory& lastMsgSent, MidiMessage* pCurrent, MidiMessage* pNext);

protected:
	void logFlams(const boost::format& fmt) const {_signalLogFlams(fmt);}
	void logGhost(const boost::format& fmt) const {_signalLogGhost(fmt);}

private:
	bool isFlamAllowed(const MidiMessage& beforeFlamHit, const MidiMessage& flamHit) const;

protected:
	mutable Mutex		_mutex;

private:
	SignalLog			_signalLogFlams;
	SignalLog			_signalLogGhost;

	MidiNotes			_midiNotes;

	// Archived data
	std::string			_color;
	Type				_type;
	Type				_typeFlam;
	int					_defaultOutputNote;

	// Settings
	Parameter::Value	_ghostVelocityLimit;
	Parameter::Value	_flamVelocityFactor;
	Parameter::Value	_flamTimeWindow1;
	Parameter::Value	_flamTimeWindow2;
	Parameter::Value	_flamCancelDuringRoll;

private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int)
	{
		// Basic Pad
		ar  & BOOST_SERIALIZATION_NVP(_color);
		ar  & BOOST_SERIALIZATION_NVP(_type);
		ar  & BOOST_SERIALIZATION_NVP(_typeFlam);
		ar  & BOOST_SERIALIZATION_NVP(_defaultOutputNote);
		ar  & BOOST_SERIALIZATION_NVP(_ghostVelocityLimit);
		ar  & BOOST_SERIALIZATION_NVP(_flamVelocityFactor);
		ar  & BOOST_SERIALIZATION_NVP(_flamTimeWindow1);
		ar  & BOOST_SERIALIZATION_NVP(_flamTimeWindow2);
		ar  & BOOST_SERIALIZATION_NVP(_flamCancelDuringRoll);
	}
};

BOOST_CLASS_VERSION(Pad, 0)
BOOST_CLASS_VERSION(Pad::MidiDescription, 0)
