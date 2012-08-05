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
#include <map>

std::string Pad::MidiDescription::getTypeLabel() const
{
	std::map<int, std::string> dictElementName;
	dictElementName[Pad::SNARE] = "Snare";
	dictElementName[Pad::HIHAT] = "Hi-hat";
	dictElementName[Pad::HIHAT_PEDAL] = "Hi-hat pedal";
	dictElementName[Pad::TOM1] = "Tom 1";
	dictElementName[Pad::TOM2] = "Tom 2";
	dictElementName[Pad::TOM3] = "Tom 3";
	dictElementName[Pad::CRASH1] = "Mutable crash";
	dictElementName[Pad::CRASH2] = "Green Crash";
	dictElementName[Pad::CRASH3] = "Yellow Crash";
	dictElementName[Pad::RIDE] = "Ride";
	dictElementName[Pad::BASS_DRUM] = "Bass Drum";
	std::string szLabel("Unknown");
	std::map<int, std::string>::iterator it = dictElementName.find(type);
	if (it!=dictElementName.end())
	{
		szLabel = it->second;
	}
	return szLabel;
}

Pad::Pad(
                    Type type,
                    int defaultMidiNote,
                    int ghostVelocityLimit,
                    double flamVelocityFactor,
                    int flamTimeWindow1,
                    int flamTimeWindow2,
					int flamCancelDuringRoll
					):
	_type(type),
	_typeFlam(type),
    _defaultOutputNote(defaultMidiNote),
    _ghostVelocityLimit(ghostVelocityLimit),
    _flamVelocityFactor(flamVelocityFactor),
    _flamTimeWindow1(flamTimeWindow1),
    _flamTimeWindow2(flamTimeWindow2),
	_flamCancelDuringRoll(flamCancelDuringRoll)
{
}

Pad::~Pad()
{
}

Pad::Pad(const Pad& rOther)
{
	lock();
	this->operator=(rOther);
	unlock();
}

Pad& Pad::operator=(const Pad& rOther)
{
	lock();
	if (this!=&rOther)
	{
		_midiNotes = rOther._midiNotes;
		_type = rOther._type;
		_typeFlam = rOther._typeFlam;
		_defaultOutputNote = rOther._defaultOutputNote;
		_ghostVelocityLimit = rOther._ghostVelocityLimit;
		_flamVelocityFactor = rOther._flamVelocityFactor;
		_flamTimeWindow1 = rOther._flamTimeWindow1;
		_flamTimeWindow2 = rOther._flamTimeWindow2;
		_flamCancelDuringRoll = rOther._flamCancelDuringRoll;
	}
	unlock();
	return *this;
}

void Pad::setMidiNotes(const MidiNotes& notes)
{
	lock();
	_midiNotes = notes;
	unlock();
}

Pad::MidiDescription Pad::getMidiDescription() const
{
	lock();
	Pad::MidiDescription result(_type);
	result.midiNotes = _midiNotes;
	unlock();

	return result;
}

bool Pad::isA(int note) const
{
	lock();
	bool bResult = _midiNotes.find(note)!=_midiNotes.end();
	unlock();

	return bResult;
}

void Pad::setTypeFlam(Type type)
{
	lock();
    _typeFlam = type;
	unlock();
}

Pad::Type Pad::getTypeFlam() const
{
	lock();
	Pad::Type result(_typeFlam);
	unlock();

    return result;
}

int Pad::getFlamCancelDuringRoll() const
{
	lock();
	int result(_flamCancelDuringRoll);
	unlock();

	return result;
}

void Pad::setFlamCancelDuringRoll(int value)
{
	lock();
	_flamCancelDuringRoll = value;
	unlock();
}

int Pad::getGhostVelocityLimit() const
{
	lock();
	int result(_ghostVelocityLimit);
	unlock();

    return result;
}

void Pad::setGhostVelocityLimit(int velocity)
{
	lock();
    _ghostVelocityLimit = velocity;
	unlock();
} 

double Pad::getFlamVelocityFactor() const
{
	lock();
	double result(_flamVelocityFactor);
	unlock();

    return result;
}

void Pad::setFlamVelocityFactor(double value)
{
	lock();
    _flamVelocityFactor = value;
	unlock();
}

int Pad::getFlamTimeWindow1() const
{
	lock();
	int result(_flamTimeWindow1);
	unlock();

    return result;
}

void Pad::setFlamTimeWindow1(int value)
{
	lock();
    _flamTimeWindow1 = value;
	unlock();
}

int Pad::getFlamTimeWindow2() const
{
	lock();
	int result(_flamTimeWindow2);
	unlock();

    return result;
}

void Pad::setFlamTimeWindow2(int value)
{
	lock();
    _flamTimeWindow2 = value;
	unlock();
}

MidiMessage::List Pad::applyFlamAndGhost(const List& drumKit, const MidiMessage::DictHistory& lastMsgSent, MidiMessage* pCurrent, MidiMessage* pNext)
{
	MidiMessage::List messageToSend;
	//bool bLogsFlams = _pConfig->isLogs() && _pConfig->isLog(UserSettings::LOG_FLAMS);
	//bool bLogsGhost = _pConfig->isLogs() && _pConfig->isLog(UserSettings::LOG_GHOST_NOTES);

	boost::format fmtFlamTw1("Flam in [FTW1] : t1stHit=%d, t2ndHit=%d, tDiff=%d, [FTW1]=%d");
	boost::format fmtFlamTw2("Flam in [FTW2] : t1stHit=%d, t2ndHit=%d, tDiff=%d, [FTW2]=%d, vel1stHit=%d, vel2ndHit=%d");
	boost::format fmtFlamTw1Ghost("Flam in [FTW1] after a ghost note : t1stHit=%d, t2ndHit=%d, tDiff=%d, [FTW1]=%d, vel1stHit=%d, threshold=%d");
	boost::format fmtFlamTw2Ghost("Flam in [FTW2] after a ghost note : t1stHit=%d, t2ndHit=%d, tDiff=%d, [FTW2]=%d, vel1stHit=%d, vel2ndHit=%d");
	boost::format fmtGhostNote("Ghost note detected : velCurrent=%d <= [THRESHOLD]=%d");

	// Hits history for this element
	const MidiMessage::History& history = lastMsgSent[_type];

	const boost::shared_ptr<Pad>& pFlamElement = drumKit[_typeFlam];

	// In buffer case, pNext != NULL, NULL otherwise
	if (pCurrent->getValue() <= getGhostVelocityLimit())
	{
		// Here we have a ghost note
		// TODO: Cancel ghost note if another tom is hit at the same time
		// TODO: Roll detection to cancel flam
		if (pNext && pCurrent->isInTimeWindow(*pNext, getFlamTimeWindow1()))
		{
			if (history.empty() || isFlamAllowed(history[0], *pCurrent))
			{
				{
					int t1stHit = pCurrent->getTimestamp();
					int t2ndHit = pNext->getTimestamp();
					int tDiff = std::abs(pNext->getTimestamp()-pCurrent->getTimestamp());
					int vel1stHit = pCurrent->getValue();
					logFlams(fmtFlamTw1Ghost % t1stHit % t2ndHit % tDiff % getFlamTimeWindow1() % vel1stHit % getGhostVelocityLimit());
				}

				pNext->changeOutputNote(pFlamElement->getDefaultOutputNote());
			}
		}
		else if (pNext && pCurrent->isInTimeWindow(*pNext, getFlamTimeWindow2()) &&
				pNext->getValue() >= int(pCurrent->getValue()*getFlamVelocityFactor()))
		{
			if (history.empty() || isFlamAllowed(history[0], *pCurrent))
			{
				{
					int t1stHit = pCurrent->getTimestamp();
					int t2ndHit = pNext->getTimestamp();
					int tDiff = std::abs(pNext->getTimestamp()-pCurrent->getTimestamp());
					int vel1stHit = pCurrent->getValue();
					int vel2ndHit = pNext->getValue();
					logFlams(fmtFlamTw2Ghost % t1stHit % t2ndHit % tDiff % getFlamTimeWindow2() % vel1stHit % vel2ndHit);
				}

				pNext->changeOutputNote(pFlamElement->getDefaultOutputNote());
			}
		}
		else
		{
			{
				logGhost(fmtGhostNote % pCurrent->getValue() % getGhostVelocityLimit());
			}

			pCurrent->ignore(MidiMessage::IGNORED_BECAUSE_GHOST);
		}
	}
	else if (pNext && pCurrent->isInTimeWindow(*pNext, getFlamTimeWindow1()))
	{
		if (history.empty() || isFlamAllowed(history[0], *pCurrent))
		{
			{
				int t1stHit = pCurrent->getTimestamp();
				int t2ndHit = pNext->getTimestamp();
				int tDiff = std::abs(pNext->getTimestamp()-pCurrent->getTimestamp());
				logFlams(fmtFlamTw1 % t1stHit % t2ndHit % tDiff % getFlamTimeWindow1());
			}

			pNext->changeOutputNote(pFlamElement->getDefaultOutputNote());
		}
	}
	else if (pNext && pCurrent->isInTimeWindow(*pNext, getFlamTimeWindow2()) &&
			pNext->getValue() >= int(pCurrent->getValue()*getFlamVelocityFactor()))
	{
		if (history.empty() || isFlamAllowed(history[0], *pCurrent))
		{
			{
				int t1stHit = pCurrent->getTimestamp();
				int t2ndHit = pNext->getTimestamp();
				int tDiff = std::abs(pNext->getTimestamp()-pCurrent->getTimestamp());
				int vel1stHit = pCurrent->getValue();
				int vel2ndHit = pNext->getValue();
				logFlams(fmtFlamTw2 % t1stHit % t2ndHit % tDiff % getFlamTimeWindow2() % vel1stHit % vel2ndHit);
			}

			pNext->changeOutputNote(pFlamElement->getDefaultOutputNote());
		}
	}
	else if (!history.empty())
	{
		// No buffer code
		// Get the last hit
		const MidiMessage& rLast = history.front();

		if ( !rLast.isAlreadyModified() && rLast.isInTimeWindow(*pCurrent, getFlamTimeWindow1()))
		{
			if (history.size()<2 || isFlamAllowed(history[1], history[0]))
			{
				{
					int t1stHit = rLast.getTimestamp();
					int t2ndHit = pCurrent->getTimestamp();
					int tDiff = std::abs(pCurrent->getTimestamp()-rLast.getTimestamp());
					logFlams(fmtFlamTw1 % t1stHit % t2ndHit % tDiff % getFlamTimeWindow1());
				}

				pCurrent->changeOutputNote(pFlamElement->getDefaultOutputNote());

				if (rLast.getIgnoreReason()==MidiMessage::IGNORED_BECAUSE_GHOST)
				{
					// If the previous hit was a ghost note, we send it for the flam here
					messageToSend.push_back(rLast);
				}
			}
		}
		else if ( !rLast.isAlreadyModified() && rLast.isInTimeWindow(*pCurrent, getFlamTimeWindow2()) &&
				pCurrent->getValue() >= int(rLast.getValue()*getFlamVelocityFactor()))
		{
			if (history.size()<2 || isFlamAllowed(history[1], history[0]))
			{
				{
					int t1stHit = rLast.getTimestamp();
					int t2ndHit = pCurrent->getTimestamp();
					int tDiff = std::abs(pCurrent->getTimestamp()-rLast.getTimestamp());
					int vel1stHit = rLast.getValue();
					int vel2ndHit = pCurrent->getValue();
					logFlams(fmtFlamTw2 % t1stHit % t2ndHit % tDiff % getFlamTimeWindow2() % vel1stHit % vel2ndHit);
				}

				pCurrent->changeOutputNote(pFlamElement->getDefaultOutputNote());

				if (rLast.getIgnoreReason()==MidiMessage::IGNORED_BECAUSE_GHOST)
				{
					// If the previous hit was a ghost note, we send it for the flam here
					messageToSend.push_back(rLast);
				}
			}
		}
	}

	return messageToSend;
}

std::string Pad::getName() const
{
	typedef std::map<int, std::string> DictNames;
	DictNames dictElementName;
	dictElementName[Pad::NOTE_SNARE] = "Snare";
	dictElementName[Pad::NOTE_HIHAT] = "Hi-Hat";
	dictElementName[Pad::NOTE_HIHAT_PEDAL] = "HH Ped";
	dictElementName[Pad::NOTE_TOM1] = "Tom1";
	dictElementName[Pad::NOTE_TOM2] = "Tom2";
	dictElementName[Pad::NOTE_TOM3] = "Tom3";
	dictElementName[Pad::NOTE_CRASH1] = "M.Crash";
	dictElementName[Pad::NOTE_CRASH2] = "G.Crash";
	dictElementName[Pad::NOTE_CRASH3] = "Y.Crash";
	dictElementName[Pad::NOTE_RIDE] = "Ride";
	dictElementName[Pad::NOTE_BASS_DRUM] = "Bass Dr";
	std::string szElementName("Unknown");
	DictNames::iterator it = dictElementName.find(getDefaultOutputNote());
	if (it!=dictElementName.end())
	{
		szElementName = it->second;
	}
	return szElementName;
}

bool Pad::isFlamAllowed(const MidiMessage& beforeFlamHit, const MidiMessage& flamHit) const
{
	return !beforeFlamHit.isInTimeWindow(flamHit, getFlamCancelDuringRoll());
}
