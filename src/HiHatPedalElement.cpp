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
	_previousControlPos(0),
	_currentControlPos(0),
	_currentControlSpeed(0),
	_previousControlSpeed(0),
	_currentControlAcceleration(0),
	_posOnCloseBegin(127),
	_posOnOpenBegin(0),
	_isBlueDetectionByAccent(false),
	_isBlueDetectionByPosition(false),
	_isBlueDetectionBySpeed(false),
	_isHalfOpenModeEnabled(false),
	_isFootCancel(false),
	_isFootCancelAfterPedalHit(false),
	_isBlueAccentOverride(false),
	_isBowAlwaysYellow(false),
	_isCancelHitWhileOpen(false),
	_controlPosThreshold(127),
	_controlPosDelayTime(0),
	_openSpeed(330),
	_closeSpeed(-200),
	_footCancelClosingSpeed(-1500),
	_footCancelPos(90),
	_footCancelPosDiff(5),
	_footCancelMaskTime(30),
	_footCancelVelocity(42),
	_footCancelAfterPedalHitMaskTime(20),
	_footCancelAfterPedalHitVelocity(25),
	_footCancelTimeLimit(0),
	_halfOpenEnteringTime(0),
	_blueStateEnteringTime(0),
	_cancelOpenHitThreshold(0),
	_cancelOpenHitVelocity(0),
	_securityPosition(0),
	_halfOpenMaximumPosition(65),
	_halfOpenActivationTime(50),
	_blueStateChangeReason(INITIAL_STATE)
{
	LinearFunction::List functions;
	functions.push_back(LinearFunction(0, 45, 127, 127));
	functions.push_back(LinearFunction(45, 48, 90, 80));
	functions.push_back(LinearFunction(48, 60, 80, 80));
	functions.push_back(LinearFunction(60, 127, 80, 80));
	_blueAccentFunctions = functions;
}

HiHatPedalElement::HiHatPedalElement(const HiHatPedalElement& rOther)
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
		_currentControlSpeed = rOther._currentControlSpeed;
		_previousControlSpeed = rOther._previousControlSpeed;
		_currentControlAcceleration = rOther._currentControlAcceleration;
		_posOnCloseBegin = rOther._posOnCloseBegin; 
		_posOnOpenBegin = rOther._posOnOpenBegin; 
		_isBlueDetectionByAccent = rOther._isBlueDetectionByAccent;
		_isBlueDetectionByPosition = rOther._isBlueDetectionByPosition;
		_isBlueDetectionBySpeed = rOther._isBlueDetectionBySpeed;
		_isHalfOpenModeEnabled = rOther._isHalfOpenModeEnabled;
		_isFootCancel = rOther._isFootCancel;
		_isFootCancelAfterPedalHit = rOther._isFootCancelAfterPedalHit;
		_isCancelHitWhileOpen = rOther._isCancelHitWhileOpen;
		_isBlueAccentOverride = rOther._isBlueAccentOverride;
		_isBowAlwaysYellow = rOther._isBowAlwaysYellow;
		_controlPosThreshold = rOther._controlPosThreshold;
		_controlPosDelayTime = rOther._controlPosDelayTime;
		_openSpeed = rOther._openSpeed;
		_closeSpeed = rOther._closeSpeed;
		_footCancelClosingSpeed = rOther._footCancelClosingSpeed;
		_footCancelPos = rOther._footCancelPos;
		_footCancelPosDiff = rOther._footCancelPosDiff;
		_footCancelMaskTime = rOther._footCancelMaskTime;
		_footCancelVelocity = rOther._footCancelVelocity;
		_footCancelAfterPedalHitMaskTime = rOther._footCancelAfterPedalHitMaskTime;
		_footCancelAfterPedalHitVelocity = rOther._footCancelAfterPedalHitVelocity;
		_cancelOpenHitThreshold = rOther._cancelOpenHitThreshold;
		_cancelOpenHitVelocity = rOther._cancelOpenHitVelocity;
		_securityPosition = rOther._securityPosition;
		_halfOpenMaximumPosition = rOther._halfOpenMaximumPosition;
		_halfOpenActivationTime = rOther._halfOpenActivationTime;
		_blueStateChangeReason = rOther._blueStateChangeReason;
		_blueAccentFunctions = rOther._blueAccentFunctions;
	}
	return *this;
}

HiHatPedalElement::~HiHatPedalElement()
{
}

bool HiHatPedalElement::isBlueDetectionByAccent() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<bool>(_isBlueDetectionByAccent);
}

void HiHatPedalElement::setBlueDetectionByAccent(const Parameter::Value& state)
{
	Mutex::scoped_lock lock(_mutex);
	_isBlueDetectionByAccent = state;
}

bool HiHatPedalElement::isBlueDetectionByPosition() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<bool>(_isBlueDetectionByPosition);
}

void HiHatPedalElement::setBlueDetectionByPosition(const Parameter::Value& state)
{
	Mutex::scoped_lock lock(_mutex);
	_isBlueDetectionByPosition = state;
	if (!boost::get<bool>(_isBlueDetectionByPosition))
	{
		// Reset the blue state
		setBlue(false, POSITION_THRESHOLD);
	}
}

bool HiHatPedalElement::isBlueDetectionBySpeed() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<bool>(_isBlueDetectionBySpeed);
}

void HiHatPedalElement::setBlueDetectionBySpeed(const Parameter::Value& state)
{
	Mutex::scoped_lock lock(_mutex);
	_isBlueDetectionBySpeed = state;
}

bool HiHatPedalElement::isHalfOpenModeEnabled() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<bool>(_isHalfOpenModeEnabled);
}

void HiHatPedalElement::setHalfOpenModeEnabled(const Parameter::Value& state)
{
	Mutex::scoped_lock lock(_mutex);
	_isHalfOpenModeEnabled = state;
}

int HiHatPedalElement::getOpenSpeed() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_openSpeed);
}

void HiHatPedalElement::setOpenSpeed(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_openSpeed = value;
}

int HiHatPedalElement::getCloseSpeed() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_closeSpeed);
}

void HiHatPedalElement::setCloseSpeed(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_closeSpeed = value;
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

HiHatPedalElement::BlueStateChangeReason HiHatPedalElement::getBlueStateChangeReason() const
{
	Mutex::scoped_lock lock(_mutex);
	return _blueStateChangeReason;
}

void HiHatPedalElement::setBlueAccentOverride(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_isBlueAccentOverride = value;
}

bool HiHatPedalElement::isBlueAccentOverride() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<bool>(_isBlueAccentOverride);
}

void HiHatPedalElement::setBowAlwaysYellow(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_isBowAlwaysYellow = value;
}

bool HiHatPedalElement::isBowAlwaysYellow() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<bool>(_isBowAlwaysYellow);
}

void HiHatPedalElement::setBlueStateChangeReason(BlueStateChangeReason reason)
{
	Mutex::scoped_lock lock(_mutex);
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

int HiHatPedalElement::getControlPosThreshold() const
{
	Mutex::scoped_lock lock(_mutex);
    return boost::get<int>(_controlPosThreshold);
}

void HiHatPedalElement::setControlPosThreshold(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
    _controlPosThreshold = value;
}

int HiHatPedalElement::getControlPosDelayTime() const
{
	Mutex::scoped_lock lock(_mutex);
    return boost::get<int>(_controlPosDelayTime);
}

void HiHatPedalElement::setControlPosDelayTime(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
    _controlPosDelayTime = value;
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

float HiHatPedalElement::getCurrentControlSpeed() const
{
	Mutex::scoped_lock lock(_mutex);
    return _currentControlSpeed;
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

int HiHatPedalElement::getFootCancelClosingSpeed() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_footCancelClosingSpeed);
}

void HiHatPedalElement::setFootCancelClosingSpeed(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_footCancelClosingSpeed = value;
}

int HiHatPedalElement::getFootCancelPosDiff() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_footCancelPosDiff);
}

void HiHatPedalElement::setFootCancelPosDiff(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_footCancelPosDiff = value;
}

int HiHatPedalElement::getFootCancelPos() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_footCancelPos);
}

void HiHatPedalElement::setFootCancelPos(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_footCancelPos = value;
}

int HiHatPedalElement::getFootCancelMaskTime() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_footCancelMaskTime);
}

void HiHatPedalElement::setFootCancelMaskTime(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_footCancelMaskTime = value;
	if (!_onFootCancelMaskTimeChanged.empty())
	{
		_onFootCancelMaskTimeChanged(boost::get<int>(_footCancelMaskTime));
	}
}

int HiHatPedalElement::getFootCancelVelocity() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_footCancelVelocity);
}

void HiHatPedalElement::setFootCancelVelocity(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_footCancelVelocity = value;
	if (!_onFootCancelVelocityChanged.empty())
	{
		_onFootCancelVelocityChanged(boost::get<int>(_footCancelVelocity));
	}
}

int HiHatPedalElement::getFootCancelAfterPedalHitMaskTime() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_footCancelAfterPedalHitMaskTime);
}

void HiHatPedalElement::setFootCancelAfterPedalHitMaskTime(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_footCancelAfterPedalHitMaskTime = value;
	if (!_onFootCancelAfterPedalHitMaskTimeChanged.empty())
	{
		_onFootCancelAfterPedalHitMaskTimeChanged(boost::get<int>(_footCancelAfterPedalHitMaskTime));
	}
}

int HiHatPedalElement::getFootCancelAfterPedalHitVelocity() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_footCancelAfterPedalHitVelocity);
}

void HiHatPedalElement::setFootCancelAfterPedalHitVelocity(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_footCancelAfterPedalHitVelocity = value;
	if (!_onFootCancelAfterPedalHitVelocityChanged.empty())
	{
		_onFootCancelAfterPedalHitVelocityChanged(boost::get<int>(_footCancelAfterPedalHitVelocity));
	}
}

bool	HiHatPedalElement::isFootCancel() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<bool>(_isFootCancel);
}

void	HiHatPedalElement::setFootCancel(const Parameter::Value& state)
{
	Mutex::scoped_lock lock(_mutex);
	_isFootCancel = state;
	if (!_onFootCancelActivated.empty())
	{
		_onFootCancelActivated(boost::get<bool>(_isFootCancel));
	}
}

bool	HiHatPedalElement::isFootCancelAfterPedalHit() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<bool>(_isFootCancelAfterPedalHit);
}

void	HiHatPedalElement::setFootCancelAfterPedalHit(const Parameter::Value& state)
{
	Mutex::scoped_lock lock(_mutex);
	_isFootCancelAfterPedalHit = state;
	if (!_onFootCancelAfterPedalHitActivated.empty())
	{
		_onFootCancelAfterPedalHitActivated(boost::get<bool>(_isFootCancelAfterPedalHit));
	}
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

int HiHatPedalElement::getCancelOpenHitThreshold() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_cancelOpenHitThreshold);
}

int HiHatPedalElement::getCancelOpenHitVelocity() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_cancelOpenHitVelocity);
}

void HiHatPedalElement::setCancelOpenHitThreshold(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_cancelOpenHitThreshold = value;
}

void HiHatPedalElement::setCancelOpenHitVelocity(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_cancelOpenHitVelocity = value;
}

bool HiHatPedalElement::isCancelHitWhileOpen() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<bool>(_isCancelHitWhileOpen);
}

void HiHatPedalElement::setCancelHitWhileOpen(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_isCancelHitWhileOpen = value;
}

int HiHatPedalElement::getSecurityPosition() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_securityPosition);
}
void HiHatPedalElement::setSecurityPosition(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_securityPosition = value;
}

int HiHatPedalElement::getHalfOpenMaximumPosition() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_halfOpenMaximumPosition);
}

void HiHatPedalElement::setHalfOpenMaximumPosition(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_halfOpenMaximumPosition = value;
}

int HiHatPedalElement::getHalfOpenActivationTime() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_halfOpenActivationTime);
}

void HiHatPedalElement::setHalfOpenActivationTime(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_halfOpenActivationTime = value;
}

LinearFunction::List HiHatPedalElement::getBlueAccentFunctions() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<LinearFunction::List>(_blueAccentFunctions);
}

void HiHatPedalElement::setBlueAccentFunctions(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_blueAccentFunctions = value;
}
