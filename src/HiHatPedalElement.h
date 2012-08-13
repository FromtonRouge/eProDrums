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

#include "Pad.h"

class HiHatPedalElement : public Pad
{
public:
	typedef boost::shared_ptr<HiHatPedalElement> Ptr;
	enum MovingState
	{
		MS_NO_CHANGE,
		MS_START_OPEN,
		MS_START_CLOSE
	};

public:
	HiHatPedalElement();
	virtual ~HiHatPedalElement();

public:
    int getCurrentControlPos() const;
    void setCurrentControlPos(int value);
    float getCurrentControlSpeed() const;
    MovingState setCurrentControlSpeed(float value);
    float getCurrentControlAcceleration() const;
    void setCurrentControlAcceleration(float value);
	bool isControlPosActivated() const;
	void setControlPosActivation(const Parameter::Value& state);
	bool isControlSpeedActivated() const;
	void setControlSpeedActivation(const Parameter::Value& state);
    int getControlPosThreshold() const;
    void setControlPosThreshold(const Parameter::Value& value);
    int getOpenAccelMax() const;
    void setOpenAccelMax(const Parameter::Value& value);
    int getControlSpeedOn() const;
    void setControlSpeedOn(const Parameter::Value& value);
    int getControlSpeedOff() const;
    void setControlSpeedOff(const Parameter::Value& value);
	bool isBlue() const;
	void setBlue(bool state);
	bool isFootCancelStrategy1Activated() const;
	void setFootCancelStrategy1Activation(const Parameter::Value& state);
	int getFootCancelAccelLimit() const;
	void setFootCancelAccelLimit(const Parameter::Value& value);
	int getFootCancelClosingSpeed() const;
	void setFootCancelClosingSpeed(const Parameter::Value& value);
	int getFootCancelPos() const;
	void setFootCancelPos(const Parameter::Value& value);
	int getFootCancelPosDiff() const;
	void setFootCancelPosDiff(const Parameter::Value& value);
	int getFootCancelMaskTime() const;
	void setFootCancelMaskTime(const Parameter::Value& value);
	int getFootCancelVelocity() const;
	void setFootCancelVelocity(const Parameter::Value& value);
	int getFootCancelTimeLimit() const;
	void setFootCancelTimeLimit(int value);
	int getPositionOnCloseBegin() const;
	int getPositionOnOpenBegin() const;
	void setCancelOpenHitThreshold(const Parameter::Value& value);
	void setCancelOpenHitVelocity(const Parameter::Value& value);
	int getCancelOpenHitThreshold() const;
	int getCancelOpenHitVelocity() const;
	bool isCancelOpenHitActivated() const;
	void setCancelOpenHit(const Parameter::Value& value);
	int getClosePositionThresold() const;
	void setClosePositionThresold(const Parameter::Value& value);
	int getOpenPositionThresold() const;
	void setOpenPositionThresold(const Parameter::Value& value);
	int getSecurityPosition() const;
	void setSecurityPosition(const Parameter::Value& value);

private:
	bool	_isBlue;
	int		_footCancelTimeLimit;
    int		_previousControlPos;
    int		_currentControlPos;
	float	_currentControlSpeed;			// in unit/s
	float	_previousControlSpeed;			// in unit/s
	float 	_currentControlAcceleration;	// in unit/s²

	/** Position of the hh control at the very start of a closing movement. */
	int		_posOnCloseBegin; 

	/** Position of the hh control at the very start of a opening movement. */
	int		_posOnOpenBegin; 

	// Archived data
	Parameter::Value	_afterHitMaskVelocity;
	Parameter::Value	_bControlPosActivated;
	Parameter::Value	_bControlSpeedActivated;
	Parameter::Value	_controlPosThreshold;
	Parameter::Value	_accelOpenMax;
	Parameter::Value	_speedOpen;
	Parameter::Value	_speedOff;
	Parameter::Value	_bFootCancelStrategy1Activated;
	Parameter::Value	_footCancelAccelLimit;
	Parameter::Value	_footCancelClosingSpeed;
	Parameter::Value	_footCancelPos;
	Parameter::Value	_footCancelPosDiff;
	Parameter::Value	_footCancelMaskTime;
	Parameter::Value	_footCancelVelocity;
	Parameter::Value	_bCancelOpenHitActivated;
	Parameter::Value	_cancelOpenHitThreshold;
	Parameter::Value	_cancelOpenHitVelocity;
	Parameter::Value	_posThresholdClose;
	Parameter::Value	_posThresholdOpen;
	Parameter::Value	_securityPosition;

private:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive & ar, const unsigned int)
	{
		// Base class serialization
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Pad);

		// Hi Hat control and pedal
		ar  & BOOST_SERIALIZATION_NVP(_afterHitMaskVelocity);
		ar  & BOOST_SERIALIZATION_NVP(_bControlPosActivated);
		ar  & BOOST_SERIALIZATION_NVP(_bControlSpeedActivated);
		ar  & BOOST_SERIALIZATION_NVP(_controlPosThreshold);
		ar  & BOOST_SERIALIZATION_NVP(_accelOpenMax);
		ar  & BOOST_SERIALIZATION_NVP(_speedOpen);
		ar  & BOOST_SERIALIZATION_NVP(_speedOff);
		ar  & BOOST_SERIALIZATION_NVP(_bFootCancelStrategy1Activated);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelAccelLimit);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelClosingSpeed);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelPos);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelPosDiff);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelMaskTime);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelVelocity);
		ar  & BOOST_SERIALIZATION_NVP(_bCancelOpenHitActivated);
		ar  & BOOST_SERIALIZATION_NVP(_cancelOpenHitThreshold);
		ar  & BOOST_SERIALIZATION_NVP(_cancelOpenHitVelocity);
		ar  & BOOST_SERIALIZATION_NVP(_posThresholdClose);
		ar  & BOOST_SERIALIZATION_NVP(_posThresholdOpen);
		ar  & BOOST_SERIALIZATION_NVP(_securityPosition);
	}
};

BOOST_CLASS_VERSION(HiHatPedalElement, 0)
