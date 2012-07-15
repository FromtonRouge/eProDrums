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
    _beforeHitMaskTime(0),
    _beforeHitMaskVelocity(0),
	_afterHitMaskTime(0),
	_afterHitMaskVelocity(0),
	_bControlPosActivated(false),
	_bControlSpeedActivated(true),
	_controlPosThreshold(127),
	_accelOpenMax(0),
	_speedOpen(0),
	_speedOff(0),
	_bFootCancelStrategy1Activated(true),
	_bFootCancelStrategy2Activated(false),
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
	_posThresholdOpen(127)
{
}

HiHatPedalElement::~HiHatPedalElement()
{
}

bool HiHatPedalElement::isControlPosActivated() const
{
	lock();
	bool result(_bControlPosActivated);
	unlock();

	return result;
}

void HiHatPedalElement::setControlPosActivation(bool state)
{
	lock();
	_bControlPosActivated = state;
	unlock();
}

bool HiHatPedalElement::isControlSpeedActivated() const
{
	lock();
	bool result(_bControlSpeedActivated);
	unlock();

	return result;
}

void HiHatPedalElement::setControlSpeedActivation(bool state)
{
	lock();
	_bControlSpeedActivated = state;
	unlock();
}

int HiHatPedalElement::getOpenAccelMax() const
{
	lock();
	int result(_accelOpenMax);
	unlock();

	return result;
}

void HiHatPedalElement::setOpenAccelMax(int value)
{
	lock();
	_accelOpenMax = value;
	unlock();
}

int HiHatPedalElement::getControlSpeedOn() const
{
	lock();
	int result(_speedOpen);
	unlock();

	return result;
}

void HiHatPedalElement::setControlSpeedOn(int value)
{
	lock();
	_speedOpen = value;
	unlock();
}

int HiHatPedalElement::getControlSpeedOff() const
{
	lock();
	int result(_speedOff);
	unlock();

	return result;
}

void HiHatPedalElement::setControlSpeedOff(int value)
{
	lock();
	_speedOff = value;
	unlock();
}

int HiHatPedalElement::getControlPosThreshold() const
{
	lock();
	int result(_controlPosThreshold);
	unlock();

    return result;
}

void HiHatPedalElement::setControlPosThreshold(int value)
{
	lock();
    _controlPosThreshold = value;
	unlock();
}

int HiHatPedalElement::getCurrentControlPos() const
{
	lock();
	int result(_currentControlPos);
	unlock();

    return result;
}

void HiHatPedalElement::setCurrentControlPos(int value)
{
	lock();
    _currentControlPos = value;
	unlock();
}

float HiHatPedalElement::getCurrentControlSpeed() const
{
	lock();
	float result(_currentControlSpeed);
	unlock();

    return result;
}

int HiHatPedalElement::getPositionOnCloseBegin() const
{
	lock();
	int result(_posOnCloseBegin);
	unlock();

	return result;
}

int HiHatPedalElement::getPositionOnOpenBegin() const
{
	lock();
	int result(_posOnOpenBegin);
	unlock();

	return result;
}

HiHatPedalElement::MovingState HiHatPedalElement::setCurrentControlSpeed(float value)
{
	lock();
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
	unlock();

	return movingState;
}

float HiHatPedalElement::getCurrentControlAcceleration() const
{
	lock();
	float result(_currentControlAcceleration);
	unlock();

	return result;
}

void HiHatPedalElement::setCurrentControlAcceleration(float value)
{
	lock();
	_currentControlAcceleration = value;
	unlock();
}


int HiHatPedalElement::getBeforeHitMaskTime() const
{
	lock();
	int result(_beforeHitMaskTime);
	unlock();

    return result;
}

void HiHatPedalElement::setBeforeHitMaskTime(int value)
{
	lock();
    _beforeHitMaskTime = value;
	unlock();
}

int HiHatPedalElement::getBeforeHitMaskVelocity() const
{
	lock();
	int result(_beforeHitMaskVelocity);
	unlock();

    return result;
}

void HiHatPedalElement::setBeforeHitMaskVelocity(int value)
{
	lock();
    _beforeHitMaskVelocity = value;
	unlock();
}

int HiHatPedalElement::getAfterHitMaskTime() const
{
	lock();
	int result(_afterHitMaskTime);
	unlock();

	return result;
}

void HiHatPedalElement::setAfterHitMaskTime(int value)
{
	lock();
	_afterHitMaskTime = value;
	unlock();
}

int HiHatPedalElement::getAfterHitMaskVelocity() const
{
	lock();
	int result(_afterHitMaskVelocity);
	unlock();

	return result;
}

void HiHatPedalElement::setAfterHitMaskVelocity(int value)
{
	lock();
	_afterHitMaskVelocity = value;
	unlock();
}

int HiHatPedalElement::getFootCancelAccelLimit() const
{
	lock();
	int result(_footCancelAccelLimit);
	unlock();

	return result;
}

void HiHatPedalElement::setFootCancelAccelLimit(int value)
{
	lock();
	_footCancelAccelLimit = value;
	unlock();
}

int HiHatPedalElement::getFootCancelClosingSpeed() const
{
	lock();
	int result(_footCancelClosingSpeed);
	unlock();

	return result;
}

void HiHatPedalElement::setFootCancelClosingSpeed(int value)
{
	lock();
	_footCancelClosingSpeed = value;
	unlock();
}

int HiHatPedalElement::getFootCancelPosDiff() const
{
	lock();
	int result(_footCancelPosDiff);
	unlock();

	return result;
}

void HiHatPedalElement::setFootCancelPosDiff(int value)
{
	lock();
	_footCancelPosDiff = value;
	unlock();
}

int HiHatPedalElement::getFootCancelPos() const
{
	lock();
	int result(_footCancelPos);
	unlock();

	return result;
}

void HiHatPedalElement::setFootCancelPos(int value)
{
	lock();
	_footCancelPos = value;
	unlock();
}

int HiHatPedalElement::getFootCancelMaskTime() const
{
	lock();
	int result(_footCancelMaskTime);
	unlock();

	return result;
}

void HiHatPedalElement::setFootCancelMaskTime(int value)
{
	lock();
	_footCancelMaskTime = value;
	unlock();
}

int HiHatPedalElement::getFootCancelVelocity() const
{
	lock();
	int result(_footCancelVelocity);
	unlock();

	return result;
}

void HiHatPedalElement::setFootCancelVelocity(int value)
{
	lock();
	_footCancelVelocity = value;
	unlock();
}

bool	HiHatPedalElement::isFootCancelStrategy1Activated() const
{
	lock();
	bool	result(_bFootCancelStrategy1Activated);
	unlock();

	return result;
}

void	HiHatPedalElement::setFootCancelStrategy1Activation(bool state)
{
	lock();
	_bFootCancelStrategy1Activated = state;
	unlock();
}

bool	HiHatPedalElement::isFootCancelStrategy2Activated() const
{
	lock();
	bool	result(_bFootCancelStrategy2Activated);
	unlock();

	return result;
}

void	HiHatPedalElement::setFootCancelStrategy2Activation(bool state)
{
	lock();
	_bFootCancelStrategy2Activated = state;
	unlock();
}

int HiHatPedalElement::getFootCancelTimeLimit() const
{
	lock();
	int result(_footCancelTimeLimit);
	unlock();

	return result;
}

void HiHatPedalElement::setFootCancelTimeLimit(int value)
{
	lock();
	_footCancelTimeLimit = value;
	unlock();
}

int HiHatPedalElement::getCancelOpenHitThreshold() const
{
	lock();
	int result(_cancelOpenHitThreshold);
	unlock();

	return result;
}

int HiHatPedalElement::getCancelOpenHitVelocity() const
{
	lock();
	int result(_cancelOpenHitVelocity);
	unlock();

	return result;
}

void HiHatPedalElement::setCancelOpenHitThreshold(int value)
{
	lock();
	_cancelOpenHitThreshold = value;
	unlock();
}

void HiHatPedalElement::setCancelOpenHitVelocity(int value)
{
	lock();
	_cancelOpenHitVelocity = value;
	unlock();
}

bool HiHatPedalElement::isCancelOpenHitActivated() const
{
	lock();
	bool result(_bCancelOpenHitActivated);
	unlock();

	return result;
}

void HiHatPedalElement::setCancelOpenHit(bool value)
{
	lock();
	_bCancelOpenHitActivated = value;
	unlock();
}

int HiHatPedalElement::getClosePositionThresold() const
{
	lock();
	int result(_posThresholdClose);
	unlock();

	return result;
}

void HiHatPedalElement::setClosePositionThresold(int value)
{
	lock();
	_posThresholdClose = value;
	unlock();
}

int HiHatPedalElement::getOpenPositionThresold() const
{
	lock();
	int result(_posThresholdOpen);
	unlock();

	return result;
}

void HiHatPedalElement::setOpenPositionThresold(int value)
{
	lock();
	_posThresholdOpen = value;
	unlock();
}
