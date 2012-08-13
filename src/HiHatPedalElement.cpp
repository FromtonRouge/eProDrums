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
	_previousControlPos(0),
	_currentControlPos(0),
	_currentControlSpeed(0),
	_previousControlSpeed(0),
	_currentControlAcceleration(0),
	_posOnCloseBegin(127),
	_posOnOpenBegin(0),
	_bControlPosActivated(false),
	_bControlSpeedActivated(true),
	_controlPosThreshold(127),
	_accelOpenMax(0),
	_speedOpen(0),
	_speedOff(0),
	_bFootCancelStrategy1Activated(true),
	_footCancelAccelLimit(-8000),
	_footCancelClosingSpeed(0),
	_footCancelPos(0),
	_footCancelPosDiff(0),
	_footCancelMaskTime(0),
	_footCancelVelocity(0),
	_footCancelTimeLimit(0),
	_bCancelOpenHitActivated(false),
	_cancelOpenHitThreshold(0),
	_cancelOpenHitVelocity(0),
	_posThresholdClose(0),
	_posThresholdOpen(127),
	_securityPosition(127)
{
}

HiHatPedalElement::~HiHatPedalElement()
{
}

bool HiHatPedalElement::isControlPosActivated() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<bool>(_bControlPosActivated);
}

void HiHatPedalElement::setControlPosActivation(const Parameter::Value& state)
{
	Mutex::scoped_lock lock(_mutex);
	_bControlPosActivated = state;
}

bool HiHatPedalElement::isControlSpeedActivated() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<bool>(_bControlSpeedActivated);
}

void HiHatPedalElement::setControlSpeedActivation(const Parameter::Value& state)
{
	Mutex::scoped_lock lock(_mutex);
	_bControlSpeedActivated = state;
}

int HiHatPedalElement::getOpenAccelMax() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_accelOpenMax);
}

void HiHatPedalElement::setOpenAccelMax(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_accelOpenMax = value;
}

int HiHatPedalElement::getControlSpeedOn() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_speedOpen);
}

void HiHatPedalElement::setControlSpeedOn(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_speedOpen = value;
}

int HiHatPedalElement::getControlSpeedOff() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_speedOff);
}

void HiHatPedalElement::setControlSpeedOff(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_speedOff = value;
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

int HiHatPedalElement::getFootCancelAccelLimit() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_footCancelAccelLimit);
}

void HiHatPedalElement::setFootCancelAccelLimit(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_footCancelAccelLimit = value;
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
}

bool	HiHatPedalElement::isFootCancelStrategy1Activated() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<bool>(_bFootCancelStrategy1Activated);
}

void	HiHatPedalElement::setFootCancelStrategy1Activation(const Parameter::Value& state)
{
	Mutex::scoped_lock lock(_mutex);
	_bFootCancelStrategy1Activated = state;
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

bool HiHatPedalElement::isCancelOpenHitActivated() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<bool>(_bCancelOpenHitActivated);
}

void HiHatPedalElement::setCancelOpenHit(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_bCancelOpenHitActivated = value;
}

int HiHatPedalElement::getClosePositionThresold() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_posThresholdClose);
}

void HiHatPedalElement::setClosePositionThresold(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_posThresholdClose = value;
}

int HiHatPedalElement::getOpenPositionThresold() const
{
	Mutex::scoped_lock lock(_mutex);
	return boost::get<int>(_posThresholdOpen);
}

void HiHatPedalElement::setOpenPositionThresold(const Parameter::Value& value)
{
	Mutex::scoped_lock lock(_mutex);
	_posThresholdOpen = value;
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
