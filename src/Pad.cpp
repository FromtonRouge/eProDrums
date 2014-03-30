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
#include "LinearFunction.h"
#include <boost/assign.hpp>
#include <map>

std::map<int, QString> Pad::DICT_NAMES = boost::assign::map_list_of
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

std::map<int, QString> Pad::DICT_COLORS = boost::assign::map_list_of
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

Pad::Pad(Type t, int defaultMidiNote)
	: type(new Property<Type>(t))
	, color(new Property<QString>(DICT_COLORS[t]))
	, defaultOutputNote(new Property<int>(defaultMidiNote))
	, typeFlam(new Property<int>(t))
	, ghostVelocityLimit(new Property<int>(0))
	, isFlamActivated(new Property<bool>(false))
	, flamCancelDuringRoll(new Property<int>(100))
{

	QPolygonF points;
	points.push_back(QPointF(0, 1.0));
	points.push_back(QPointF(45, 1.0));
	points.push_back(QPointF(45, 1.15));
	points.push_back(QPointF(80, 1.20));
	funcFlams.reset(new Property<QPolygonF>(points));
}

Pad::Pad(const Pad& rOther)
{
	this->operator=(rOther);
}

Pad& Pad::operator=(const Pad& rOther)
{
	Mutex::scoped_lock lock(_mutex);
	if (this!=&rOther)
	{
		_drumNotes = rOther._drumNotes;

		type = rOther.type;
		color = rOther.color;
		defaultOutputNote = rOther.defaultOutputNote;
		typeFlam = rOther.typeFlam;
		ghostVelocityLimit = rOther.ghostVelocityLimit;
		isFlamActivated = rOther.isFlamActivated;
		funcFlams = rOther.funcFlams;
		flamCancelDuringRoll = rOther.flamCancelDuringRoll;
	}
	return *this;
}

QString Pad::getName(Type t)
{
	return DICT_NAMES[t];
}

QString Pad::getDefaultColor(Type t)
{
	return DICT_COLORS[t];
}

Pad::~Pad()
{
}

QString Pad::getName() const
{
	Mutex::scoped_lock lock(_mutex);
	return DICT_NAMES[type->get()];
}

void Pad::setInputNotes(const DrumNotes& notes)
{
	Mutex::scoped_lock lock(_mutex);
	_drumNotes = notes;
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

void Pad::applyFlamAndGhost(const List& drumKit, const MidiMessage::DictHistory& lastMsgSent, MidiMessage* pCurrent, MidiMessage* pNext, MidiMessage::List& rResult)
{
	Mutex::scoped_lock lock(_mutex);

	rResult.clear();

	// Hits history for this element
	const MidiMessage::History& history = lastMsgSent[type->get()];

	const boost::shared_ptr<Pad>& pFlamElement = drumKit[typeFlam->get()];

	// Note: In buffer case, pNext != NULL, NULL otherwise
	if (pCurrent->getValue() <= ghostVelocityLimit->get())
	{
		bool bDoGhostNoteTest = true;
		if (isFlamActivated->get())
		{
			// If the next hit is not a ghost note
			if (pNext && pNext->getValue() > ghostVelocityLimit->get())
			{
				if (history.empty() || isFlamAllowed(history[0], *pCurrent))
				{
					float y = 0.f;
					int timeDiff = pCurrent->getAbsTimeDiff(*pNext);
					int nextValue = pNext->getValue();
					if (LinearFunction::apply(funcFlams->get(), timeDiff, y) && (nextValue==127 || nextValue >= int(pCurrent->getValue()*y)))
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
	else if (isFlamActivated->get())
	{
		// If the next hit is not a ghost note
		if (pNext && pNext->getValue() > ghostVelocityLimit->get())
		{
			if (history.empty() || isFlamAllowed(history[0], *pCurrent))
			{
				float y = 0.f;
				int timeDiff = pCurrent->getAbsTimeDiff(*pNext);
				int nextValue = pNext->getValue();
				if (LinearFunction::apply(funcFlams->get(), timeDiff, y) && (nextValue==127 || nextValue >= int(pCurrent->getValue()*y)))
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
					if (LinearFunction::apply(funcFlams->get(), timeDiff, y) && (currentValue==127 || currentValue >= int(rLast.getValue()*y)))
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
	return !beforeFlamHit.isInTimeWindow(flamHit, flamCancelDuringRoll->get());
}
