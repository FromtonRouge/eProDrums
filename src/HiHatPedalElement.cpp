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
	_isBlueDetectionByPosition(false),
	_isBlueDetectionBySpeed(false),
	_controlPosThreshold(127),
	_openSpeed(330),
	_closeSpeed(-200),
	_isFootSplashCancel(false),
	_footCancelClosingSpeed(-1500),
	_footCancelPos(90),
	_footCancelPosDiff(5),
	_footCancelMaskTime(30),
	_footCancelVelocity(35),
	_footCancelTimeLimit(0),
	_halfOpenEnteringTime(0),
	_isCancelHitWhileOpen(false),
	_cancelOpenHitThreshold(0),
	_cancelOpenHitVelocity(0),
	_securityPosition(0),
	_securityOpenPosition(127),
	_halfOpenMaximumPosition(0),
	_halfOpenActivationTime(0)
{
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
		_previousControlPos = rOther._previousControlPos;
		_currentControlPos = rOther._currentControlPos;
		_currentControlSpeed = rOther._currentControlSpeed;
		_previousControlSpeed = rOther._previousControlSpeed;
		_currentControlAcceleration = rOther._currentControlAcceleration;
		_posOnCloseBegin = rOther._posOnCloseBegin; 
		_posOnOpenBegin = rOther._posOnOpenBegin; 
		_isBlueDetectionByPosition = rOther._isBlueDetectionByPosition;
		_isBlueDetectionBySpeed = rOther._isBlueDetectionBySpeed;
		_controlPosThreshold = rOther._controlPosThreshold;
		_openSpeed = rOther._openSpeed;
		_closeSpeed = rOther._closeSpeed;
		_isFootSplashCancel = rOther._isFootSplashCancel;
		_footCancelClosingSpeed = rOther._footCancelClosingSpeed;
		_footCancelPos = rOther._footCancelPos;
		_footCancelPosDiff = rOther._footCancelPosDiff;
		_footCancelMaskTime = rOther._footCancelMaskTime;
		_footCancelVelocity = rOther._footCancelVelocity;
		_isCancelHitWhileOpen = rOther._isCancelHitWhileOpen;
		_cancelOpenHitThreshold = rOther._cancelOpenHitThreshold;
		_cancelOpenHitVelocity = rOther._cancelOpenHitVelocity;
		_securityPosition = rOther._securityPosition;
		_securityOpenPosition = rOther._securityOpenPosition;
		_halfOpenMaximumPosition = rOther._halfOpenMaximumPosition;
		_halfOpenActivationTime = rOther._halfOpenActivationTime;
	}
	return *this;
}

HiHatPedalElement::~HiHatPedalElement()
{
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

void HiHatPedalElement::setBlue(bool state)
{
	Mutex::scoped_lock lock(_mutex);
	_isBlue = state;
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

bool	HiHatPedalElement::isFootSplashCancel() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<bool>(_isFootSplashCancel);
}

void	HiHatPedalElement::setFootSplashCancel(const Parameter::Value& state)
{
	Mutex::scoped_lock lock(_mutex);
	_isFootSplashCancel = state;
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

int HiHatPedalElement::getSecurityOpenPosition() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_securityOpenPosition);
}
void HiHatPedalElement::setSecurityOpenPosition(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_securityOpenPosition = value;
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
