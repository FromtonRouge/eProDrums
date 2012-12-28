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

#include "MidiEngine.h"

#include "HiHatPedalElement.h"
#include "HiHatPositionCurve.h"
#include "DrumKitMidiMap.h"

#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include <QtCore/QMetaType>
#include <iostream>

#include <boost/shared_ptr.hpp>

Q_DECLARE_METATYPE(MidiMessage)

/**
 * Compute current speed, accel and jerk
 */
void computeSpeedAccelAndJerk(const MidiMessage& previous, MidiMessage& rCurrent)
{
	// Default values
	rCurrent.hiHatSpeed = 0;
	rCurrent.hiHatAcceleration = 0;

	const int DEFAULT_NOTE_MSG_CTRL(4);
	if (previous.isControllerMsg() && previous.getOutputNote()==DEFAULT_NOTE_MSG_CTRL)
	{
		float deltaTimeInS = float(rCurrent.getTimestamp()-previous.getTimestamp())/1000;
		int deltaPosition = previous.getValue() - rCurrent.getValue();
		if (deltaTimeInS==0.0f) // it happens and we don't want that (infinite acceleration)
		{
			rCurrent = previous; // Copy the previous
		}
		else if (deltaPosition)
		{
			// Speed computation
			rCurrent.hiHatSpeed = float(deltaPosition)/deltaTimeInS;

			// Acceleration computation
			rCurrent.hiHatAcceleration = (rCurrent.hiHatSpeed-previous.hiHatSpeed)/deltaTimeInS;

			// Jerk computation
			rCurrent.hiHatJerk = (rCurrent.hiHatAcceleration-previous.hiHatAcceleration)/deltaTimeInS;
		}
	}
}

/**
 * readMidi is called every milliseconds.
 */
void readMidi(PtTimestamp timestamp, void* pUserData)
{
	static_cast<MidiEngine*>(pUserData)->processMidi(timestamp);
}

MidiEngine::MidiEngine():
	_pMidiIn(NULL),
	_pMidiOut(NULL),
	_sumLatency(0),
	_countLatency(0),
	_bufferLength(0),
	_lastMsgSent(Pad::TYPE_COUNT),
	_hiHatControlCC(4)
{
	qRegisterMetaType<MidiMessage>();

	Pm_Initialize();
	int nbDevices = Pm_CountDevices();
	for (int deviceId=0; deviceId<nbDevices; ++deviceId)
	{
		const PmDeviceInfo* pDeviceInfo = Pm_GetDeviceInfo(deviceId);
		if (pDeviceInfo)
		{
			if (pDeviceInfo->input)
			{
				_midiInDevices.push_back(MidiDevice(pDeviceInfo->name, deviceId));
			}

			if (pDeviceInfo->output)
			{
				_midiOutDevices.push_back(MidiDevice(pDeviceInfo->name, deviceId));
			}
		}
	}
	Pm_Terminate();
}

MidiEngine::~MidiEngine()
{
	stop();
}

void MidiEngine::start(int midiInId, int midiOutId)
{
	clearAverageLatency();

	Pt_Start(1, &readMidi, this); // start a timer with millisecond accuracy

	Pm_Initialize();

#define INPUT_BUFFER_SIZE 0 // if INPUT_BUFFER_SIZE is 0, PortMidi uses a default value
#define OUTPUT_BUFFER_SIZE 100
#define DRIVER_INFO NULL
#define TIME_PROC NULL
#define TIME_INFO NULL
#define LATENCY 0 // use zero latency because we want output to be immediate

	PmError error = pmNoError;

	// Open Midi in device
	const PmDeviceInfo* pDeviceInInfo = Pm_GetDeviceInfo(midiInId);
	error = Pm_OpenInput(	&_pMidiIn, 
			midiInId, 
			DRIVER_INFO,
			INPUT_BUFFER_SIZE,
			TIME_PROC,
			TIME_INFO);
	if (error!=pmNoError)
	{
		QMessageBox::critical(QApplication::activeWindow(), tr("Midi Error"), tr("Cannot open midi in %1, reason : %2").arg(pDeviceInInfo->name).arg(Pm_GetErrorText(error)));
		return;
	}
	else
	{
		// Ignoring sysex, tick, song position etc...
		Pm_SetFilter(_pMidiIn, PM_FILT_REALTIME | PM_FILT_SYSTEMCOMMON);
	}

	// Open Midi out device
	const PmDeviceInfo* pDeviceOutInfo = Pm_GetDeviceInfo(midiOutId);
	error = Pm_OpenOutput(	&_pMidiOut, 
			midiOutId, 
			DRIVER_INFO,
			OUTPUT_BUFFER_SIZE,
			TIME_PROC,
			TIME_INFO,
			LATENCY);
	if (error!=pmNoError)
	{
		QMessageBox::critical(QApplication::activeWindow(), tr("Midi Error"), tr("Cannot open midi out %1, reason : %2").arg(pDeviceOutInfo->name).arg(Pm_GetErrorText(error)));
	}
	emit signalStarted();
}

void MidiEngine::stop()
{
	Pt_Stop();

	if (_pMidiOut)
	{
		Pm_Close(_pMidiOut);
		_pMidiOut = NULL;
	}

	if (_pMidiIn)
	{
		Pm_Close(_pMidiIn);
		_pMidiIn = NULL;
	}

	Pm_Terminate();
	emit signalStopped();
}

void MidiEngine::sendMidiMessage(MidiMessage& midiMessage, bool bForce)
{
	if (!midiMessage.isIgnored() || bForce)
	{
		const int NOT_USED(0); // no latency configuration
		PmError error = Pm_WriteShort(_pMidiOut, NOT_USED, midiMessage.computeOutputMessage());

		PtTimestamp tWrite = Pt_Time();
		midiMessage.setSentTimestamp(tWrite);
		_currentHiHatMsgControl.setSentTimestamp(tWrite);

		if (error==pmNoError)
		{
			if (!midiMessage.isControllerMsg())
			{
				// Note on message = 9
				// Sending a fake hh control to the plotter
				const int DEFAULT_NOTE_MSG_CTRL(4);
				if (_currentHiHatMsgControl.isControllerMsg() && _currentHiHatMsgControl.getOutputNote() == DEFAULT_NOTE_MSG_CTRL)
				{
					// Update hi-hat curves
					emit signalMidiOut(_currentHiHatMsgControl);
				}
			}

			// Midi out notification
			emit signalMidiOut(midiMessage);

			// Average latency
			_sumLatency += midiMessage.getLatency();
			_countLatency++;
			emit signalAverageLatency(double(_sumLatency)/_countLatency);
		}
		else
		{
			std::cout << (tr("Error can't send midi message: %1 Reason: %2").arg(midiMessage.str().c_str()).arg(Pm_GetErrorText(error))).toStdString() <<  std::endl;
		}
	}
}

void MidiEngine::processMidi(PtTimestamp timestamp)
{
	PmEvent buffer[5];
	MidiMessage::Status status = 0;
	MidiMessage::Data data1 = 0;
	MidiMessage::Data data2 = 0;
	while (Pm_Poll(_pMidiIn))
	{
		int eventsReaded = Pm_Read(_pMidiIn, buffer, 5);
		if (eventsReaded < 0) 
		{
			continue;
		}

		for (int i=0; i<eventsReaded; ++i)
		{
			// Unless there was overflow, we should have a message now
			status = Pm_MessageStatus(buffer[i].message);
			data1 = Pm_MessageData1(buffer[i].message);
			data2 = Pm_MessageData2(buffer[i].message);

			MidiMessage midiMessage(timestamp, status, data1, data2);
			emit signalMidiIn(midiMessage);

			// First filtering, we get only note-on and controller messages
			if (midiMessage.isNoteOnMsg() || midiMessage.isControllerMsg())
			{
				if (midiMessage.isNoteOnMsg() && midiMessage.getValue()==0)
				{
					// Ignore note-on with velocity 0 (it happens...)
				}
				else
				{
					if (classify(midiMessage))
					{
						if (_bufferLength==0)
						{
							// Transform the message now when no buffering
							transform(midiMessage);
						}
						else
						{
							// Add the message in the midi buffer
							_bufferedMessages.push_back(midiMessage);
						}
					}
				}
			}
		}
	}

	// Pop buffer if any
	if (_bufferLength)
	{
		while (!_bufferedMessages.empty())
		{
			MidiMessage& midiMessage = _bufferedMessages.front();

			// Is it time to transform and send buffered messages ?
			if (timestamp - midiMessage.getTimestamp() >= _bufferLength)
			{
				transform(midiMessage);
				_bufferedMessages.pop_front();
			}
			else
			{
				// No need to continue
				break;
			}
		}
	}
}

void MidiEngine::transform(MidiMessage& currentMsg)
{
	if (!_pCurrentSlot.get())
	{
		return;
	}

	Pad::List& pads = _pCurrentSlot->getPads();

	// Hi hat pedal controller msg
	const int DEFAULT_NOTE_MSG_CTRL(4);
	if (currentMsg.isControllerMsg() && currentMsg.getOutputNote() == DEFAULT_NOTE_MSG_CTRL && !currentMsg.isAlreadyModified())
	{
		computeSpeedAccelAndJerk(_lastHiHatMsgControl, currentMsg);

		// Set pos, speed, accel, jerk
		const HiHatPedalElement::Ptr& pHiHatPedal = boost::dynamic_pointer_cast<HiHatPedalElement>(pads[Pad::HIHAT_PEDAL]);
		pHiHatPedal->setCurrentControlPos(127-currentMsg.getValue());
		pHiHatPedal->setCurrentDeltaPos(_lastHiHatMsgControl.getValue()-currentMsg.getValue());
		HiHatPedalElement::MovingState movingState = pHiHatPedal->setCurrentControlSpeed(currentMsg.hiHatSpeed);
		pHiHatPedal->setCurrentControlAcceleration(currentMsg.hiHatAcceleration);
		pHiHatPedal->setCurrentJerk(currentMsg.hiHatJerk);

		// Moving state
		switch (movingState)
		{
		case HiHatPedalElement::MS_START_CLOSE:
			{
				emit signalHiHatStartMoving(movingState, pHiHatPedal->getPositionOnCloseBegin(), _lastHiHatMsgControl.getTimestamp());
				break;
			}
		case HiHatPedalElement::MS_START_OPEN:
			{
				emit signalHiHatStartMoving(movingState, pHiHatPedal->getPositionOnOpenBegin(), _lastHiHatMsgControl.getTimestamp());
				break;
			}
		default:
			{
				break;
			}
		}

		_currentHiHatMsgControl = currentMsg;
	}
	else
	{
		_currentHiHatMsgControl = _lastHiHatMsgControl;
		_currentHiHatMsgControl.setTimestamp(currentMsg.getTimestamp());
		computeSpeedAccelAndJerk(_lastHiHatMsgControl, _currentHiHatMsgControl);
	}

	// Compute midi message
	computeMessage(currentMsg, _lastMsgSent);

	// Send the message
	sendMidiMessage(currentMsg);

	// Saving the sended message
	if (currentMsg.isNoteOnMsg())
	{
		Pad::List::iterator it = pads.begin();
		while (it!=pads.end())
		{
			Pad::List::value_type& p = *(it++);
			if (p->isA(currentMsg.getOriginalNote()))
			{
				MidiMessage::History& rHistory = _lastMsgSent[p->getType()];
				rHistory.push_front(currentMsg);
				// We save the last 5 notes for this element
				rHistory.resize(5);
				break;
			}
		}
		_lastHiHatMsgControl = _currentHiHatMsgControl;
	}
	else if (currentMsg.isControllerMsg() && currentMsg.getOutputNote() == DEFAULT_NOTE_MSG_CTRL)
	{
		_lastHiHatMsgControl = _currentHiHatMsgControl;
	}
}

void MidiEngine::onSlotChanged(const Slot::Ptr& pSlot)
{
	_pCurrentSlot = pSlot;
}

void MidiEngine::onBufferLengthChanged(int value)
{
	clearAverageLatency();
	_bufferLength = value;
	emit signalAverageLatency(_bufferLength);
}

void MidiEngine::clearAverageLatency()
{
	_sumLatency = 0;
	_countLatency = 0;
	emit signalAverageLatency(0);
}

bool MidiEngine::classify(MidiMessage& midiMessage)
{
	if (!_pCurrentSlot.get())
	{
		return false;
	}

	Pad::List& pads = _pCurrentSlot->getPads();

	bool bAccepted = false;
	if (midiMessage.isNoteOnMsg())
	{
		// Filtering, we only process notes defined in the drumkit, we also change the default output note
		Pad::List::const_iterator it = pads.begin();
		while (it!=pads.end())
		{
			const Pad::Ptr& pPad = *(it++);
			if (pPad->isA(midiMessage.getOriginalNote()))
			{
				midiMessage.changeNoteTo(pPad.get(), false);
				midiMessage.padType = pPad->getType();
				bAccepted = true;
				break;
			}
		}
	}
	else if (midiMessage.isControllerMsg() && midiMessage.getOriginalNote() == _hiHatControlCC)
	{
		midiMessage.changeNoteTo(4, false);
		bAccepted = true;
	}

	return bAccepted;
}

void MidiEngine::onDrumKitLoaded(DrumKitMidiMap* pDrumKit, const boost::filesystem::path&)
{
	_hiHatControlCC = pDrumKit->getHiHatControlCC();
}

void MidiEngine::computeMessage(MidiMessage& currentMsg, MidiMessage::DictHistory& lastMsgSent)
{
	if (!_pCurrentSlot.get())
	{
		return;
	}

	Pad::List& pads = _pCurrentSlot->getPads();

	const Pad::Ptr& pElSnare =		pads[Pad::SNARE];
	const Pad::Ptr& pElHihat =		pads[Pad::HIHAT];
	const HiHatPedalElement::Ptr& pElHihatPedal =	boost::dynamic_pointer_cast<HiHatPedalElement>(pads[Pad::HIHAT_PEDAL]);
	const Pad::Ptr& pElTom1 =		pads[Pad::TOM1];
	const Pad::Ptr& pElTom2 =		pads[Pad::TOM2];
	const Pad::Ptr& pElTom3 =		pads[Pad::TOM3];
	const Pad::Ptr& pElCrash1 =		pads[Pad::CRASH1];
	const Pad::Ptr& pElCrash2 =		pads[Pad::CRASH2];
	const Pad::Ptr& pElCrash3 =		pads[Pad::CRASH3];
	const Pad::Ptr& pElRide =		pads[Pad::RIDE];
	const Pad::Ptr& pElBassDrum =	pads[Pad::BASS_DRUM];

	int cymbalsSimHitWindow = _pCurrentSlot->getCymbalSimHitWindow();
	bool mutableCrashWithCrash = _pCurrentSlot->isAutoConvertCrash(Slot::CRASH_CRASH);
	bool mutableCrashWithRide = _pCurrentSlot->isAutoConvertCrash(Slot::CRASH_RIDE);
	bool mutableCrashWithSnare = _pCurrentSlot->isAutoConvertCrash(Slot::CRASH_SNARE);
	bool mutableCrashWithTom2 = _pCurrentSlot->isAutoConvertCrash(Slot::CRASH_TOM2);
	bool mutableCrashWithTom3 = _pCurrentSlot->isAutoConvertCrash(Slot::CRASH_TOM3);
	bool bHasNextMidiMessage = !_bufferedMessages.empty();

	const int DEFAULT_NOTE_MSG_CTRL(4);
	int currentTime = currentMsg.getTimestamp();
	if (currentMsg.isControllerMsg() && currentMsg.getOutputNote() == DEFAULT_NOTE_MSG_CTRL)
	{
		int currentControlPos = pElHihatPedal->getCurrentControlPos();
		bool bSecured = currentControlPos<=pElHihatPedal->getSecurityPosition();
		if (bSecured)
		{
			pElHihatPedal->setBlue(false, HiHatPedalElement::IN_SECURED_ZONE);
		}

		// Half-open mode
		if (pElHihatPedal->isHalfOpenModeEnabled())
		{
			if (bSecured)
			{
				pElHihatPedal->setHalfOpenEnteringTime(0);
				pElHihatPedal->setHalfOpen(false);
			}
			else if (currentControlPos<=pElHihatPedal->getHalfOpenMaximumPosition() && pElHihatPedal->getHalfOpenEnteringTime()==0)
			{
				pElHihatPedal->setHalfOpenEnteringTime(currentTime);
			}
			else if (currentControlPos<=pElHihatPedal->getHalfOpenMaximumPosition() && pElHihatPedal->getHalfOpenEnteringTime()>0)
			{
				int enteringTime = pElHihatPedal->getHalfOpenEnteringTime();
				int activationTime = pElHihatPedal->getHalfOpenActivationTime();
				if (currentTime-enteringTime > activationTime && !pElHihatPedal->isBlue())
				{
					// Still yellow after activationTime, we can switch to half-open mode
					pElHihatPedal->setHalfOpen(true);
				}
			}
			else
			{
				pElHihatPedal->setHalfOpenEnteringTime(0);
			}
		}
		else
		{
			pElHihatPedal->setHalfOpen(false);
		}

		// Blue state by pedal acceleration
		float currentSpeed = pElHihatPedal->getCurrentControlSpeed();
		if (pElHihatPedal->isBlueDetectionByAcceleration())
		{
			if (!pElHihatPedal->isHalfOpen() && !bSecured)
			{
				float currentAccel = pElHihatPedal->getCurrentControlAcceleration();
				float deltaPos = pElHihatPedal->getCurrentDeltaPos();

				// Are we opening the Hi-Hat
				if (currentSpeed > 0)
				{
					if (currentAccel >= pElHihatPedal->getOpenAcceleration() && deltaPos >= pElHihatPedal->getOpenPositionDelta())
					{
						pElHihatPedal->setBlue(true, HiHatPedalElement::OPENING_MOVEMENT);
					}
					else
					{
						// Only change the reason to OPENING_MOVEMENT
						pElHihatPedal->setBlueStateChangeReason(HiHatPedalElement::OPENING_MOVEMENT);
					}
				}
				else if (currentSpeed < 0)
				{
					// Hi-Hat closing
					if (currentAccel <= pElHihatPedal->getCloseAcceleration() && deltaPos <= pElHihatPedal->getClosePositionDelta())
					{
						pElHihatPedal->setBlue(false, HiHatPedalElement::CLOSING_MOVEMENT);
					}
				}
				else
				{
					// current accel == 0 => nothing we keep the last blue state (on or off)
				}
			}
		}

		// Blue state by pedal speed
		if (pElHihatPedal->isBlueDetectionBySpeed())
		{
			if (!pElHihatPedal->isHalfOpen() && !bSecured)
			{
				// Are we opening the Hi-Hat
				if (currentSpeed > 0)
				{
					if (currentSpeed >= pElHihatPedal->getOpenSpeed())
					{
						pElHihatPedal->setBlue(true, HiHatPedalElement::OPENING_MOVEMENT);
					}
					else
					{
						// Only change the reason to OPENING_MOVEMENT
						pElHihatPedal->setBlueStateChangeReason(HiHatPedalElement::OPENING_MOVEMENT);
					}
				}
				else if (currentSpeed < 0)
				{
					// Hi-Hat closing
					if (currentSpeed <= pElHihatPedal->getCloseSpeed())
					{
						// Hi Hat closing and the close speed is reached
						pElHihatPedal->setBlue(false, HiHatPedalElement::CLOSING_MOVEMENT);
					}
				}
				else
				{
					// current speed == 0 => nothing we keep the last blue state (on or off)
				}
			}
		}

		// Blue state by pedal position
		if (!pElHihatPedal->isHalfOpen() && pElHihatPedal->isBlueDetectionByPosition())
		{
			if (currentControlPos > pElHihatPedal->getControlPosThreshold())
			{
				if (pElHihatPedal->getBlueStateEnteringTime()==0)
				{
					pElHihatPedal->setBlueStateEnteringTime(currentTime); }

				if (currentTime-pElHihatPedal->getBlueStateEnteringTime() >= pElHihatPedal->getControlPosDelayTime())
				{
					pElHihatPedal->setBlue(true, HiHatPedalElement::POSITION_THRESHOLD);
				}
			}
			else if (!pElHihatPedal->isBlueDetectionBySpeed())
			{
				pElHihatPedal->setBlueStateEnteringTime(0);
				pElHihatPedal->setBlue(false, HiHatPedalElement::POSITION_THRESHOLD);
			}
		}

		// Send the hi-hat open state to the GUI
		HiHatPositionCurve::HiHatState state = HiHatPositionCurve::HHS_CLOSED;
		if (pElHihatPedal->isBlue())
		{
			state = HiHatPositionCurve::HHS_OPEN;
		}
		else if (pElHihatPedal->isHalfOpen())
		{
			state = HiHatPositionCurve::HHS_HALF_OPEN;
		}
		else if (bSecured)
		{
			state = HiHatPositionCurve::HHS_SECURED;
		}
		else
		{
			state = HiHatPositionCurve::HHS_CLOSED;
		}
		emit signalHiHatState(state);

		if (pElHihatPedal->isFootCancel())
		{
			int posDiff = pElHihatPedal->getPositionOnCloseBegin()-currentControlPos;
			float speed = pElHihatPedal->getCurrentControlSpeed();

			if ( 
					currentTime > pElHihatPedal->getFootCancelTimeLimit() && 
					speed <= pElHihatPedal->getFootCancelClosingSpeed() &&
					currentControlPos <= pElHihatPedal->getFootCancelPos() &&
					posDiff >= pElHihatPedal->getFootCancelPosDiff()
			   )
			{
				int cancelMaskTime = pElHihatPedal->getFootCancelMaskTime();
				pElHihatPedal->setFootCancelTimeLimit(currentTime+cancelMaskTime);

				emit signalFootCancelStarted(currentTime, cancelMaskTime, pElHihatPedal->getFootCancelVelocity());
			}
		}
	}

	if (currentMsg.isNoteOnMsg() && !currentMsg.isAlreadyModified())
	{
		MidiMessage::List messagesToSend;
		switch (currentMsg.padType)
		{
		case Pad::HIHAT_PEDAL:
			{
				if (bHasNextMidiMessage && pElHihatPedal->isFootCancelAfterPedalHit())
				{
					MidiMessage* pNextHiHat = getNextMessage(pElHihat);
					if (	pNextHiHat && currentMsg.isInTimeWindow(*pNextHiHat, pElHihatPedal->getFootCancelAfterPedalHitMaskTime()) &&
							pNextHiHat->getValue() < pElHihatPedal->getFootCancelAfterPedalHitVelocity())
					{
						pNextHiHat->ignore(MidiMessage::IGNORED_BECAUSE_FOOT_CANCEL);
					}
				}
				break;
			}

		case Pad::HIHAT:
			{
				int currentControlPos = pElHihatPedal->getCurrentControlPos();
				if ( pElHihatPedal->isCancelHitWhileOpen()
						&& currentControlPos >= pElHihatPedal->getCancelOpenHitThreshold() 
						&& currentMsg.getValue()<pElHihatPedal->getCancelOpenHitVelocity())
				{
					currentMsg.ignore(MidiMessage::IGNORED_BECAUSE_FOOT_CANCEL);
				}
				else if (currentMsg.getTimestamp() <= pElHihatPedal->getFootCancelTimeLimit() && currentMsg.getValue() < pElHihatPedal->getFootCancelVelocity())
				{
					currentMsg.ignore(MidiMessage::IGNORED_BECAUSE_FOOT_CANCEL);
				}

				if (pElHihatPedal->isBlue())
				{
					if (!pElHihatPedal->isBowAlwaysYellow() || !pElHihat->isA(currentMsg.getOriginalNote(), DrumNote::BOW))
					{
						// Change the yellow hi-hat to blue if the pedal is blue
						currentMsg.changeNoteTo(pElRide.get());
					}
				}
				else if (pElHihatPedal->isBlueDetectionByAccent() && !pElHihatPedal->isHalfOpen())
				{
					// Above security position ?
					if (currentControlPos > pElHihatPedal->getSecurityPosition() || pElHihatPedal->isBlueAccentOverride())
					{
						// Accented notes only detected on hi-hat edge (bow are alwyas yellow)
						if (pElHihat->isA(currentMsg.getOriginalNote(), DrumNote::EDGE))
						{
							// When blue detection by speed is activated 
							// and if the blue state is false because of a closing movement we don't apply linear functions
							if (!pElHihatPedal->isBlueDetectionBySpeed() || pElHihatPedal->getBlueStateChangeReason()!=HiHatPedalElement::CLOSING_MOVEMENT)
							{
								float y = 0.f;
								if (LinearFunction::apply(pElHihatPedal->getBlueAccentFunctions(), currentControlPos, y) && currentMsg.getValue() > y)
								{
									currentMsg.changeNoteTo(pElRide.get());
								}
							}
						}
					}
				}

				if (pElHihatPedal->isFootCancelAfterPedalHit() && !lastMsgSent[Pad::HIHAT_PEDAL].empty())
				{
					const MidiMessage& rLastHiHatPedal = lastMsgSent[Pad::HIHAT_PEDAL].front();
					if (pElHihatPedal->isA(rLastHiHatPedal.getOriginalNote()))
					{
						if (	currentMsg.isInTimeWindow(rLastHiHatPedal, pElHihatPedal->getFootCancelAfterPedalHitMaskTime()) &&
								currentMsg.getValue() < pElHihatPedal->getFootCancelAfterPedalHitVelocity())
						{
							currentMsg.ignore(MidiMessage::IGNORED_BECAUSE_FOOT_CANCEL);
						}
					}
				}

				// Flam and ghost
				if (!currentMsg.isIgnored())
				{
					pElHihat->applyFlamAndGhost(pads, lastMsgSent, &currentMsg, getNextMessage(pElHihat), messagesToSend);
					sendMidiMessages(messagesToSend, true);
				}
				break;
			}

		case Pad::CRASH1:
			{
				// For the crash 2 we only get the previous
				MidiMessage* pPreviousCRASH2 = NULL;
				if (!lastMsgSent[Pad::CRASH2].empty())
				{
					pPreviousCRASH2 = pads[Pad::CRASH2]->isA(lastMsgSent[Pad::CRASH2].front().getOriginalNote())?&(lastMsgSent[Pad::CRASH2].front()):NULL;
				}

				MidiMessage* pNextOrPreviousRIDE = NULL;
				MidiMessage* pNextOrPreviousSNARE = NULL;
				MidiMessage* pNextOrPreviousTOM2 = NULL;
				MidiMessage* pNextOrPreviousTOM3 = NULL;
				if (bHasNextMidiMessage)
				{
					// Get next messages
					pNextOrPreviousRIDE = getNextMessage(pElRide);
					pNextOrPreviousSNARE = getNextMessage(pElSnare);
					pNextOrPreviousTOM2 = getNextMessage(pElTom2);
					pNextOrPreviousTOM3 = getNextMessage(pElTom3);
				}
				else
				{
					// Get last messages
					if (!lastMsgSent[Pad::RIDE].empty())
					{
						pNextOrPreviousRIDE = pads[Pad::RIDE]->isA(lastMsgSent[Pad::RIDE].front().getOriginalNote())?&(lastMsgSent[Pad::RIDE].front()):NULL;
					}
					if (!lastMsgSent[Pad::SNARE].empty())
					{
						pNextOrPreviousSNARE = pads[Pad::SNARE]->isA(lastMsgSent[Pad::SNARE].front().getOriginalNote())?&(lastMsgSent[Pad::SNARE].front()):NULL;
					}
					if (!lastMsgSent[Pad::TOM2].empty())
					{
						pNextOrPreviousTOM2 = pads[Pad::TOM2]->isA(lastMsgSent[Pad::TOM2].front().getOriginalNote())?&(lastMsgSent[Pad::TOM2].front()):NULL;
					}
					if (!lastMsgSent[Pad::TOM3].empty())
					{
						pNextOrPreviousTOM3 = pads[Pad::TOM3]->isA(lastMsgSent[Pad::TOM3].front().getOriginalNote())?&(lastMsgSent[Pad::TOM3].front()):NULL;
					}
				}

				if (mutableCrashWithCrash && pPreviousCRASH2 && !pPreviousCRASH2->isAlreadyModified() && currentMsg.isInTimeWindow(*pPreviousCRASH2, cymbalsSimHitWindow))
				{
					currentMsg.changeNoteTo(pElHihat.get());
				}
				else if (mutableCrashWithRide && pNextOrPreviousRIDE && currentMsg.isInTimeWindow(*pNextOrPreviousRIDE, cymbalsSimHitWindow))
				{
					currentMsg.changeNoteTo(pElHihat.get());
				}
				else if (mutableCrashWithSnare && pNextOrPreviousSNARE && currentMsg.isInTimeWindow(*pNextOrPreviousSNARE, cymbalsSimHitWindow))
				{
					currentMsg.changeNoteTo(pElHihat.get());
				}
				else if (mutableCrashWithTom2 && pNextOrPreviousTOM2 && currentMsg.isInTimeWindow(*pNextOrPreviousTOM2, cymbalsSimHitWindow))
				{
					currentMsg.changeNoteTo(pElHihat.get());
				}
				else if (mutableCrashWithTom3 && pNextOrPreviousTOM3 && currentMsg.isInTimeWindow(*pNextOrPreviousTOM3, cymbalsSimHitWindow))
				{
					currentMsg.changeNoteTo(pElHihat.get());
				}
				else
				{
					// Flam and ghost
					pElCrash1->applyFlamAndGhost(pads, lastMsgSent, &currentMsg, getNextMessage(pElCrash1), messagesToSend);
					sendMidiMessages(messagesToSend, true);
				}
				break;
			}

		case Pad::CRASH3:
			{
				// Flam and ghost
				pElCrash3->applyFlamAndGhost(pads, lastMsgSent, &currentMsg, getNextMessage(pElCrash3), messagesToSend);
				sendMidiMessages(messagesToSend, true);
				break;
			}

		case Pad::CRASH2:
			{
				MidiMessage* pPreviousAutoConvertCrash = NULL;
				if (!lastMsgSent[Pad::CRASH1].empty())
				{
					pPreviousAutoConvertCrash = pads[Pad::CRASH1]->isA(lastMsgSent[Pad::CRASH1].front().getOriginalNote())?&(lastMsgSent[Pad::CRASH1].front()):NULL;
				}

				if (mutableCrashWithCrash && pPreviousAutoConvertCrash && !pPreviousAutoConvertCrash->isAlreadyModified() && currentMsg.isInTimeWindow(*pPreviousAutoConvertCrash, cymbalsSimHitWindow))
				{
					// Previous was a mutable crash, if the mutable was not changed we have to change the CRASH2 to yellow
					currentMsg.changeNoteTo(pElHihat.get());
				}
				else
				{
					// Flam and ghost
					pElCrash2->applyFlamAndGhost(pads, lastMsgSent, &currentMsg, getNextMessage(pElCrash2), messagesToSend);
					sendMidiMessages(messagesToSend, true);
				}
				break;
			}

		case Pad::RIDE:
			{
				MidiMessage* pNextAutoConvertCrash = getNextMessage(pElCrash1);
				if (mutableCrashWithRide && pNextAutoConvertCrash && currentMsg.isInTimeWindow(*pNextAutoConvertCrash, cymbalsSimHitWindow))
				{
					// Yellow Crash
					pNextAutoConvertCrash->changeNoteTo(pElHihat.get());
				}
				else
				{
					// Flam and ghost
					pElRide->applyFlamAndGhost(pads, lastMsgSent, &currentMsg, getNextMessage(pElRide), messagesToSend);
					sendMidiMessages(messagesToSend, true);
				}
				break;
			}

		case Pad::SNARE:
			{
				MidiMessage* pNextAutoConvertCrash = getNextMessage(pElCrash1);
				if (mutableCrashWithSnare && pNextAutoConvertCrash && currentMsg.isInTimeWindow(*pNextAutoConvertCrash, cymbalsSimHitWindow))
				{
					// Yellow Crash
					pNextAutoConvertCrash->changeNoteTo(pElHihat.get());
				}
				else
				{
					// Flam and ghost
					pElSnare->applyFlamAndGhost(pads, lastMsgSent, &currentMsg, getNextMessage(pElSnare), messagesToSend);
					sendMidiMessages(messagesToSend, true);
				}
				break;
			}

		case Pad::TOM1:
			{
				// Flam and ghost
				pElTom1->applyFlamAndGhost(pads, lastMsgSent, &currentMsg, getNextMessage(pElTom1), messagesToSend);
				sendMidiMessages(messagesToSend, true);
				break;
			}

		case Pad::TOM2:
			{
				MidiMessage* pNextAutoConvertCrash = getNextMessage(pElCrash1);
				if (mutableCrashWithTom2 && pNextAutoConvertCrash && currentMsg.isInTimeWindow(*pNextAutoConvertCrash, cymbalsSimHitWindow))
				{
					// Yellow crash
					pNextAutoConvertCrash->changeNoteTo(pElHihat.get());
				}
				else
				{
					// Flam and ghost
					pElTom2->applyFlamAndGhost(pads, lastMsgSent, &currentMsg, getNextMessage(pElTom2), messagesToSend);
					sendMidiMessages(messagesToSend, true);
				}
				break;
			}

		case Pad::TOM3:
			{
				MidiMessage* pNextAutoConvertCrash = getNextMessage(pElCrash1);
				if (mutableCrashWithTom3 && pNextAutoConvertCrash && currentMsg.isInTimeWindow(*pNextAutoConvertCrash, cymbalsSimHitWindow))
				{
					// Yellow crash
					pNextAutoConvertCrash->changeNoteTo(pElHihat.get());
				}
				else
				{
					// Flam and ghost
					pElTom3->applyFlamAndGhost(pads, lastMsgSent, &currentMsg, getNextMessage(pElTom3), messagesToSend);
					sendMidiMessages(messagesToSend, true);
				}
				break;
			}

		case Pad::BASS_DRUM:
			{
				// Flam and ghost
				pElBassDrum->applyFlamAndGhost(pads, lastMsgSent, &currentMsg, getNextMessage(pElBassDrum), messagesToSend);
				sendMidiMessages(messagesToSend, true);
				break;
			}

		default:
			{
				break;
			}
		}
	}
}

MidiMessage* MidiEngine::getNextMessage(const boost::shared_ptr<Pad>& pElement, int msgType)
{
	MidiMessage* pResult = NULL;
	MidiMessage::List::iterator it = _bufferedMessages.begin();
	while (it!=_bufferedMessages.end())
	{
		MidiMessage& r = *(it++);
		if (r.getMsgType() == msgType && pElement->isA(r.getOriginalNote()))
		{
			pResult = &r;
			break;
		}
	}
	return pResult;
}

void MidiEngine::sendMidiMessages(MidiMessage::List& midiMessages, bool bForce)
{
	MidiMessage::List::iterator it = midiMessages.begin();
	while (it!=midiMessages.end())
	{
		MidiMessage& msg = *(it++);
		sendMidiMessage(msg, bForce);
	}
}
