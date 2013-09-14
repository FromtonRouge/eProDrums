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

#include "Pad.h"
#include <boost/assign.hpp>
#include <map>

Parameter::DictEnums Pad::DICT_NAMES = boost::assign::map_list_of
	(SNARE, "Snare")
	(HIHAT, "Hi-hat")
	(HIHAT_PEDAL, "Hi-hat pedal")
	(TOM1, "Tom 1")
	(TOM2, "Tom 2")
	(TOM3, "Tom 3")
	(CRASH1, "Chameleon Crash")
	(CRASH2, "Crash")
	(CRASH3, "Alternate Crash")
	(RIDE, "Ride")
	(BASS_DRUM, "Bass Drum");

std::map<int, std::string> Pad::DICT_COLORS = boost::assign::map_list_of
	(SNARE, "#FF6060")
	(HIHAT, "#FFFD70")
	(HIHAT_PEDAL, "#FFFFFF")
	(TOM1, "#FFFD70")
	(TOM2, "#70A5FF")
	(TOM3, "#86FF70")
	(CRASH1, "#86FF70")
	(CRASH2, "#86FF70")
	(CRASH3, "#FFFD70")
	(RIDE, "#70A5FF")
	(BASS_DRUM, "#FCCB42");

Pad::Pad():
	_color(DICT_COLORS[SNARE]),
	_type(SNARE),
	_typeFlam(SNARE),
	_defaultOutputNote(0),
	_ghostVelocityLimit(0),
	_isFlamActivated(false),
	_flamCancelDuringRoll(100)
{
	QPolygonF points;
	points.push_back(QPointF(0, 1.0));
	points.push_back(QPointF(45, 1.0));
	points.push_back(QPointF(45, 1.15));
	points.push_back(QPointF(80, 1.20));
	_flamFunctions = points;
}

Pad::Pad(Type type, int defaultMidiNote):
	_type(type),
	_typeFlam(type),
	_defaultOutputNote(defaultMidiNote),
	_ghostVelocityLimit(0),
	_isFlamActivated(false),
	_flamCancelDuringRoll(100)
{
	_color = DICT_COLORS[_type];

	QPolygonF points;
	points.push_back(QPointF(0, 1.0));
	points.push_back(QPointF(45, 1.0));
	points.push_back(QPointF(45, 1.15));
	points.push_back(QPointF(80, 1.20));
	_flamFunctions = points;
}

std::string Pad::getName(Type type)
{
	return DICT_NAMES[type];
}

std::string Pad::getDefaultColor(Type type)
{
	return DICT_COLORS[type];
}

Pad::~Pad()
{
}

Pad::Pad(const Pad& rOther)
{
	Mutex::scoped_lock lock(_mutex);
	this->operator=(rOther);
}

Pad& Pad::operator=(const Pad& rOther)
{
	Mutex::scoped_lock lock(_mutex);
	if (this!=&rOther)
	{
		_drumNotes = rOther._drumNotes;
		_color = rOther._color;
		_type = rOther._type;
		_typeFlam = rOther._typeFlam;
		_defaultOutputNote = rOther._defaultOutputNote;
		_ghostVelocityLimit = rOther._ghostVelocityLimit;
		_isFlamActivated = rOther._isFlamActivated;
		_flamFunctions = rOther._flamFunctions;
		_flamCancelDuringRoll = rOther._flamCancelDuringRoll;
	}
	return *this;
}

std::string Pad::getName() const
{
	Mutex::scoped_lock lock(_mutex);
	return DICT_NAMES[_type];
}

void Pad::setInputNotes(const DrumNotes& notes)
{
	Mutex::scoped_lock lock(_mutex);
	_drumNotes = notes;
}

void Pad::setColor(const std::string& color)
{
	Mutex::scoped_lock lock(_mutex);
	_color = color;
}

std::string Pad::getColor() const
{
	Mutex::scoped_lock lock(_mutex);
	return _color;
}

Pad::Type Pad::getType() const
{
	Mutex::scoped_lock lock(_mutex);
	return _type;
}

void Pad::setType(Type type)
{
	Mutex::scoped_lock lock(_mutex);
	_type = type;
}

void Pad::setTypeFlam(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_typeFlam = value;
}

Pad::Type Pad::getTypeFlam() const
{
	Mutex::scoped_lock lock(_mutex);
	return static_cast<Type>(boost::get<int>(_typeFlam));
}

bool Pad::isA(int midiNote) const
{
	Mutex::scoped_lock lock(_mutex);
	return _drumNotes.findMidiNote(midiNote)!=_drumNotes.endMidiNote();
}

bool Pad::isA(int midiNote, DrumNote::HitZone hitZone) const
{
	Mutex::scoped_lock lock(_mutex);
	DrumNotes::IteratorMidiNote it = _drumNotes.findMidiNote(midiNote);
	return it!=_drumNotes.endMidiNote() && (it->hitZone == hitZone);
}

int Pad::getFlamCancelDuringRoll() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_flamCancelDuringRoll);
}

void Pad::setFlamCancelDuringRoll(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_flamCancelDuringRoll = value;
}

void Pad::setDefaultOutputNote(int outputNote)
{
	Mutex::scoped_lock lock(_mutex);
	_defaultOutputNote = outputNote;
}

int Pad::getDefaultOutputNote() const
{
	Mutex::scoped_lock lock(_mutex);
	return _defaultOutputNote;
}

int Pad::getGhostVelocityLimit() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_ghostVelocityLimit);
}

void Pad::setGhostVelocityLimit(const Parameter::Value& velocity)
{
	Mutex::scoped_lock lock(_mutex);
	_ghostVelocityLimit = velocity;
} 

bool Pad::isFlamActivated() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<bool>(_isFlamActivated);
}

void Pad::setFlamActivated(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_isFlamActivated = value;
}

QPolygonF Pad::getFlamFunctions() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<QPolygonF>(_flamFunctions);
}

void Pad::setFlamFunctions(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_flamFunctions = value;
}

void Pad::applyFlamAndGhost(const List& drumKit, const MidiMessage::DictHistory& lastMsgSent, MidiMessage* pCurrent, MidiMessage* pNext, MidiMessage::List& rResult)
{
	Mutex::scoped_lock lock(_mutex);
	rResult.clear();

	// Hits history for this element
	const MidiMessage::History& history = lastMsgSent[_type];

	const boost::shared_ptr<Pad>& pFlamElement = drumKit[getTypeFlam()];

	// Note: In buffer case, pNext != NULL, NULL otherwise
	if (pCurrent->getValue() <= getGhostVelocityLimit())
	{
		bool bDoGhostNoteTest = true;
		if (isFlamActivated())
		{
			// If the next hit is not a ghost note
			if (pNext && pNext->getValue() > getGhostVelocityLimit())
			{
				if (history.empty() || isFlamAllowed(history[0], *pCurrent))
				{
					float y = 0.f;
					int timeDiff = pCurrent->getAbsTimeDiff(*pNext);
					int nextValue = pNext->getValue();
					if (LinearFunction::apply(getFlamFunctions(), timeDiff, y) && (nextValue==127 || nextValue >= int(pCurrent->getValue()*y)))
					{
						pNext->changeNoteTo(pFlamElement.get());
						bDoGhostNoteTest = false;
					}
				}
			}
		}
		
		if (bDoGhostNoteTest)
		{
			pCurrent->ignore(MidiMessage::IGNORED_BECAUSE_GHOST);
		}
	}
	else if (isFlamActivated())
	{
		// If the next hit is not a ghost note
		if (pNext && pNext->getValue() > getGhostVelocityLimit())
		{
			if (history.empty() || isFlamAllowed(history[0], *pCurrent))
			{
				float y = 0.f;
				int timeDiff = pCurrent->getAbsTimeDiff(*pNext);
				int nextValue = pNext->getValue();
				if (LinearFunction::apply(getFlamFunctions(), timeDiff, y) && (nextValue==127 || nextValue >= int(pCurrent->getValue()*y)))
				{
					pNext->changeNoteTo(pFlamElement.get());
				}
			}
		}
		else if (!history.empty())
		{
			// No buffer code
			// Get the last hit
			const MidiMessage& rLast = history.front();
			if (!rLast.isAlreadyModified())
			{
				if (history.size()<2 || isFlamAllowed(history[1], history[0]))
				{
					float y = 0.f;
					int timeDiff = rLast.getAbsTimeDiff(*pCurrent);
					int currentValue = pCurrent->getValue();
					if (LinearFunction::apply(getFlamFunctions(), timeDiff, y) && (currentValue==127 || currentValue >= int(rLast.getValue()*y)))
					{
						pCurrent->changeNoteTo(pFlamElement.get());
						if (rLast.getIgnoreReason()==MidiMessage::IGNORED_BECAUSE_GHOST)
						{
							// If the previous hit was a ghost note, we send it for the flam here
							rResult.push_back(rLast);
						}
					}
				}
			}
		}
	}
}

bool Pad::isFlamAllowed(const MidiMessage& beforeFlamHit, const MidiMessage& flamHit) const
{
	Mutex::scoped_lock lock(_mutex);
	return !beforeFlamHit.isInTimeWindow(flamHit, getFlamCancelDuringRoll());
}
