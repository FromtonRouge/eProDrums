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

#include <QtCore/QObject>
#include "Pad.h"
#include "Slot.h"

#include "MidiMessage.h"
#include "MidiDevice.h"

#include <boost/filesystem.hpp>

class DrumKitMidiMap;

class MidiEngine : public QObject
{
	Q_OBJECT

signals:
	void signalMidiIn(const MidiMessage&);
	void signalMidiOut(const MidiMessage&);
	void signalHiHatStartMoving(int movingState, int pos, int timestamp);
	void signalFootCancelStarted(int timestamp, int maskLength, int velocity);
	void signalHiHatState(int state);
	void signalAverageLatency(double);
	void signalStarted();
	void signalStopped();

public:
	MidiEngine();
	virtual ~MidiEngine();

	const MidiDevice::List& getMidiInDevices() const {return _midiInDevices;}
	const MidiDevice::List& getMidiOutDevices() const {return _midiOutDevices;}

	void processMidi(PtTimestamp timestamp);
	void transform(MidiMessage& currentMsg);
	void stressTest();

	void onDrumKitLoaded(DrumKitMidiMap* pDrumKit, const boost::filesystem::path&);

public slots:
	void onSlotChanged(const Slot::Ptr&);
	void onInputBufferChanged(int);
	void start(int midiInId, int midiOutId);
	void stop();

private:
    void sendMidiMessage(MidiMessage& midiMessage, bool bForce = false);
	bool classify(MidiMessage& midiMessage);
	void computeMessage(MidiMessage& currentMsg, MidiMessage::DictHistory& lastMsgSent);
    MidiMessage* getNextMessage(const boost::shared_ptr<Pad>& pElement, int msgType = 9);
	void sendMidiMessages(MidiMessage::List& midiMessages, bool bForce);
	void clearAverageLatency();
	void popInputBuffer(PtTimestamp t);

private:
	PmStream*	_pMidiIn;
	PmStream*	_pMidiOut;

	int			_sumLatency;
	size_t		_countLatency;
	int			_inputBufferLength;
	MidiMessage::List	_inputBufferedMessages;
	MidiMessage			_lastHiHatMsgControl;
	MidiMessage			_currentHiHatMsgControl;
	MidiMessage::DictHistory	_lastMsgSent;
	int							_hiHatControlCC;
	Slot::Ptr					_pCurrentSlot;
	MidiDevice::List			_midiInDevices;
	MidiDevice::List			_midiOutDevices;
};
