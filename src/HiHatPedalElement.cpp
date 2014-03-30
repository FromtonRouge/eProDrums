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

#include "HiHatPedalElement.h"

HiHatPedalElement::HiHatPedalElement():
	Pad(HIHAT_PEDAL, NOTE_HIHAT_PEDAL),
	_isBlue(false),
	_isHalfOpen(false),
	_footCancelTimeLimit(0),
	_halfOpenEnteringTime(0),
	_blueStateEnteringTime(0),
	_previousControlPos(0),
	_currentControlPos(0),
	_currentDeltaPos(0),
	_currentControlSpeed(0),
	_previousControlSpeed(0),
	_currentControlAcceleration(0),
	_currentJerk(0),
	_blueStateChangeReason(INITIAL_STATE),
	_posOnCloseBegin(127),
	_posOnOpenBegin(0),
	isBlueDetectionByAccent(new Property<bool>(false)),
	isBlueDetectionByPosition(new Property<bool>(false)),
	isBlueDetectionBySpeed(new Property<bool>(false)),
	isBlueDetectionByAcceleration(new Property<bool>(false)),
	isHalfOpenModeEnabled(new Property<bool>(false)),
	isFootCancel(new Property<bool>(false)),
	isFootCancelAfterPedalHit(new Property<bool>(false)),
	isCancelHitWhileOpen(new Property<bool>(false)),
	isBlueAccentOverride(new Property<bool>(false)),
	isBowAlwaysYellow(new Property<bool>(false)),
	controlPosThreshold(new Property<int>(127)),
	controlPosDelayTime(new Property<int>(0)),
	openSpeed(new Property<int>(330)),
	closeSpeed(new Property<int>(-200)),
	openAcceleration(new Property<int>(0)),
	closeAcceleration(new Property<int>(0)),
	openPositionDelta(new Property<int>(0)),
	closePositionDelta(new Property<int>(0)),
	footCancelClosingSpeed(new Property<int>(-1500)),
	footCancelPos(new Property<int>(90)),
	footCancelPosDiff(new Property<int>(5)),
	footCancelMaskTime(new Property<int>(30)),
	footCancelVelocity(new Property<int>(42)),
	footCancelAfterPedalHitMaskTime(new Property<int>(20)),
	footCancelAfterPedalHitVelocity(new Property<int>(25)),
	cancelOpenHitThreshold(new Property<int>(0)),
	cancelOpenHitVelocity(new Property<int>(0)),
	securityPosition(new Property<int>(0)),
	halfOpenMaximumPosition(new Property<int>(65)),
	halfOpenActivationTime(new Property<int>(50))
{
	QPolygonF points;
	points.push_back(QPointF(0, 127));
	points.push_back(QPointF(45, 127));
	points.push_back(QPointF(45, 90));
	points.push_back(QPointF(48, 80));
	points.push_back(QPointF(60, 80));
	points.push_back(QPointF(127, 80));
	funcBlueAccent.reset(new Property<QPolygonF>(points));
}

HiHatPedalElement::HiHatPedalElement(const HiHatPedalElement& rOther):Pad(rOther)
{
	Mutex::scoped_lock lock(_mutex);
	this->operator=(rOther);
}

HiHatPedalElement& HiHatPedalElement::operator=(const HiHatPedalElement& rOther)
{
	Mutex::scoped_lock lock(_mutex);
	if (this!=&rOther)
	{
		// Base class
		this->Pad::operator=(rOther);

		_isBlue = rOther._isBlue;
		_isHalfOpen = rOther._isHalfOpen;
		_footCancelTimeLimit = rOther._footCancelTimeLimit;
		_halfOpenEnteringTime = rOther._halfOpenEnteringTime;
		_blueStateEnteringTime = rOther._blueStateEnteringTime;
		_previousControlPos = rOther._previousControlPos;
		_currentControlPos = rOther._currentControlPos;
		_currentDeltaPos = rOther._currentDeltaPos;
		_currentControlSpeed = rOther._currentControlSpeed;
		_previousControlSpeed = rOther._previousControlSpeed;
		_currentControlAcceleration = rOther._currentControlAcceleration;
		_currentJerk = rOther._currentJerk;
		_posOnCloseBegin = rOther._posOnCloseBegin; 
		_posOnOpenBegin = rOther._posOnOpenBegin; 

		isBlueDetectionByAccent = rOther.isBlueDetectionByAccent;
		isBlueDetectionByPosition = rOther.isBlueDetectionByPosition;
		isBlueDetectionBySpeed = rOther.isBlueDetectionBySpeed;
		isBlueDetectionByAcceleration = rOther.isBlueDetectionByAcceleration;
		isHalfOpenModeEnabled = rOther.isHalfOpenModeEnabled;
		isFootCancel = rOther.isFootCancel;
		isFootCancelAfterPedalHit = rOther.isFootCancelAfterPedalHit;
		isCancelHitWhileOpen = rOther.isCancelHitWhileOpen;
		isBlueAccentOverride = rOther.isBlueAccentOverride;
		isBowAlwaysYellow = rOther.isBowAlwaysYellow;
		controlPosThreshold = rOther.controlPosThreshold;
		controlPosDelayTime = rOther.controlPosDelayTime;
		openSpeed = rOther.openSpeed;
		closeSpeed = rOther.closeSpeed;
		openAcceleration = rOther.openAcceleration;
		closeAcceleration = rOther.closeAcceleration;
		openPositionDelta = rOther.openPositionDelta;
		closePositionDelta = rOther.closePositionDelta;
		footCancelClosingSpeed = rOther.footCancelClosingSpeed;
		footCancelPos = rOther.footCancelPos;
		footCancelPosDiff = rOther.footCancelPosDiff;
		footCancelMaskTime = rOther.footCancelMaskTime;
		footCancelVelocity = rOther.footCancelVelocity;
		footCancelAfterPedalHitMaskTime = rOther.footCancelAfterPedalHitMaskTime;
		footCancelAfterPedalHitVelocity = rOther.footCancelAfterPedalHitVelocity;
		cancelOpenHitThreshold = rOther.cancelOpenHitThreshold;
		cancelOpenHitVelocity = rOther.cancelOpenHitVelocity;
		securityPosition = rOther.securityPosition;
		halfOpenMaximumPosition = rOther.halfOpenMaximumPosition;
		halfOpenActivationTime = rOther.halfOpenActivationTime;
		_blueStateChangeReason = rOther._blueStateChangeReason;
		funcBlueAccent = rOther.funcBlueAccent;
	}
	return *this;
}

HiHatPedalElement::~HiHatPedalElement()
{
}

bool HiHatPedalElement::isBlue() const
{
	Mutex::scoped_lock lock(_mutex);
	return _isBlue;
}

void HiHatPedalElement::setBlue(bool state, BlueStateChangeReason reason)
{
	Mutex::scoped_lock lock(_mutex);
	_isBlue = state;
	_blueStateChangeReason = reason;
}

bool HiHatPedalElement::isHalfOpen() const
{
	Mutex::scoped_lock lock(_mutex);
	return _isHalfOpen;
}

void HiHatPedalElement::setHalfOpen(bool state)
{
	Mutex::scoped_lock lock(_mutex);
	_isHalfOpen = state;
}

int HiHatPedalElement::getFootCancelTimeLimit() const
{
	Mutex::scoped_lock lock(_mutex);
	return _footCancelTimeLimit;
}

void HiHatPedalElement::setFootCancelTimeLimit(int value)
{
	Mutex::scoped_lock lock(_mutex);
	_footCancelTimeLimit = value;
}

int HiHatPedalElement::getHalfOpenEnteringTime() const
{
	Mutex::scoped_lock lock(_mutex);
	return _halfOpenEnteringTime;
}

void HiHatPedalElement::setHalfOpenEnteringTime(int value)
{
	Mutex::scoped_lock lock(_mutex);
	_halfOpenEnteringTime = value;
}

int HiHatPedalElement::getBlueStateEnteringTime() const
{
	Mutex::scoped_lock lock(_mutex);
	return _blueStateEnteringTime;
}

void HiHatPedalElement::setBlueStateEnteringTime(int value)
{
	Mutex::scoped_lock lock(_mutex);
	_blueStateEnteringTime = value;
}

float HiHatPedalElement::getCurrentControlSpeed() const
{
	Mutex::scoped_lock lock(_mutex);
	return _currentControlSpeed;
}

HiHatPedalElement::MovingState HiHatPedalElement::setCurrentControlSpeed(float value)
{
	Mutex::scoped_lock lock(_mutex);
	MovingState movingState = MS_NO_CHANGE;
	_currentControlSpeed = value;
	if (_currentControlSpeed <= 0.f)
	{
		if (_previousControlSpeed > 0.f)
		{
			_posOnCloseBegin = _previousControlPos;
			movingState = MS_START_CLOSE;
		}
	}
	else
	{
		if (_previousControlSpeed <= 0.f)
		{
			_posOnOpenBegin = _previousControlPos;
			movingState = MS_START_OPEN;
		}
	}
	_previousControlSpeed = _currentControlSpeed;
	_previousControlPos = _currentControlPos;

	return movingState;
}

int HiHatPedalElement::getCurrentControlPos() const
{
	Mutex::scoped_lock lock(_mutex);
	return _currentControlPos;
}

void HiHatPedalElement::setCurrentControlPos(int value)
{
	Mutex::scoped_lock lock(_mutex);
	_currentControlPos = value;
}

int HiHatPedalElement::getCurrentDeltaPos() const
{
	Mutex::scoped_lock lock(_mutex);
	return _currentDeltaPos;
}

void HiHatPedalElement::setCurrentDeltaPos(int value)
{
	Mutex::scoped_lock lock(_mutex);
	_currentDeltaPos = value;
}

float HiHatPedalElement::getCurrentControlAcceleration() const
{
	Mutex::scoped_lock lock(_mutex);
	return _currentControlAcceleration;
}

void HiHatPedalElement::setCurrentControlAcceleration(float value)
{
	Mutex::scoped_lock lock(_mutex);
	_currentControlAcceleration = value;
}

float HiHatPedalElement::getCurrentJerk() const
{
	Mutex::scoped_lock lock(_mutex);
	return _currentJerk;
}

void HiHatPedalElement::setCurrentJerk(float value)
{
	Mutex::scoped_lock lock(_mutex);
	_currentJerk = value;
}

HiHatPedalElement::BlueStateChangeReason HiHatPedalElement::getBlueStateChangeReason() const
{
	Mutex::scoped_lock lock(_mutex);
	return _blueStateChangeReason;
}

void HiHatPedalElement::setBlueStateChangeReason(BlueStateChangeReason reason)
{
	Mutex::scoped_lock lock(_mutex);
	_blueStateChangeReason = reason;
}

int HiHatPedalElement::getPositionOnCloseBegin() const
{
	Mutex::scoped_lock lock(_mutex);
	return _posOnCloseBegin;
}

int HiHatPedalElement::getPositionOnOpenBegin() const
{
	Mutex::scoped_lock lock(_mutex);
	return _posOnOpenBegin;
}

/* TODO
void HiHatPedalElement::setBlueDetectionByPosition(const Property::Value& state)
{
	Mutex::scoped_lock lock(_mutex);
	_isBlueDetectionByPosition = state;
	if (!boost::get<bool>(_isBlueDetectionByPosition))
	{
		// Reset the blue state
		setBlue(false, POSITION_THRESHOLD);
	}
}
*/
