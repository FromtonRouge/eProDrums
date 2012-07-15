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

    int getCurrentControlPos() const;
    void setCurrentControlPos(int value);
    float getCurrentControlSpeed() const;
    MovingState setCurrentControlSpeed(float value);
    float getCurrentControlAcceleration() const;
    void setCurrentControlAcceleration(float value);

	int getBeforeHitMaskTime() const;
	void setBeforeHitMaskTime(int value);
	int getBeforeHitMaskVelocity() const;
	void setBeforeHitMaskVelocity(int value);
	int getAfterHitMaskTime() const;
	void setAfterHitMaskTime(int value);
	int getAfterHitMaskVelocity() const;
	void setAfterHitMaskVelocity(int value);

	bool isControlPosActivated() const;
	void setControlPosActivation(bool state);
	bool isControlSpeedActivated() const;
	void setControlSpeedActivation(bool state);

    int getControlPosThreshold() const;
    void setControlPosThreshold(int value);

    int getOpenAccelMax() const;
    void setOpenAccelMax(int value);
    int getControlSpeedOn() const;
    void setControlSpeedOn(int value);
    int getControlSpeedOff() const;
    void setControlSpeedOff(int value);

	bool isBlue() const {return _isBlue;}
	void setBlue(bool state) {_isBlue = state;}

	bool isFootCancelStrategy1Activated() const;
	void setFootCancelStrategy1Activation(bool state);
	bool isFootCancelStrategy2Activated() const;
	void setFootCancelStrategy2Activation(bool state);

	int getFootCancelAccelLimit() const;
	void setFootCancelAccelLimit(int value);
	int getFootCancelClosingSpeed() const;
	void setFootCancelClosingSpeed(int value);
	int getFootCancelPos() const;
	void setFootCancelPos(int value);
	int getFootCancelPosDiff() const;
	void setFootCancelPosDiff(int value);
	int getFootCancelMaskTime() const;
	void setFootCancelMaskTime(int value);
	int getFootCancelVelocity() const;
	void setFootCancelVelocity(int value);

	int getFootCancelTimeLimit() const;
	void setFootCancelTimeLimit(int value);

	int getPositionOnCloseBegin() const;
	int getPositionOnOpenBegin() const;

	void setCancelOpenHitThreshold(int value);
	void setCancelOpenHitVelocity(int value);
	int getCancelOpenHitThreshold() const;
	int getCancelOpenHitVelocity() const;

	bool isCancelOpenHitActivated() const;
	void setCancelOpenHit(bool value);

	int getClosePositionThresold() const;
	void setClosePositionThresold(int value);

	int getOpenPositionThresold() const;
	void setOpenPositionThresold(int value);

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
    int		_beforeHitMaskTime;
    int		_beforeHitMaskVelocity;
	int		_afterHitMaskTime;
	int		_afterHitMaskVelocity;
	bool	_bControlPosActivated;
	bool	_bControlSpeedActivated;
    int		_controlPosThreshold;
    int		_accelOpenMax;
    int		_speedOpen;
    int		_speedOff;
	bool	_bFootCancelStrategy1Activated;
	bool	_bFootCancelStrategy2Activated;
	int		_footCancelAccelLimit;
	int		_footCancelClosingSpeed;
	int		_footCancelPos;
	int		_footCancelPosDiff;
	int		_footCancelMaskTime;
	int		_footCancelVelocity;
	bool	_bCancelOpenHitActivated;
	int		_cancelOpenHitThreshold;
	int		_cancelOpenHitVelocity;
    int		_posThresholdClose;
    int		_posThresholdOpen;

private:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive & ar, const unsigned int fileVersion)
	{
		// Base class serialization
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Pad);

		// Hi Hat control and pedal
		ar  & BOOST_SERIALIZATION_NVP(_beforeHitMaskTime);
		ar  & BOOST_SERIALIZATION_NVP(_beforeHitMaskVelocity);
		ar  & BOOST_SERIALIZATION_NVP(_afterHitMaskTime);
		ar  & BOOST_SERIALIZATION_NVP(_afterHitMaskVelocity);
		ar  & BOOST_SERIALIZATION_NVP(_bControlPosActivated);
		ar  & BOOST_SERIALIZATION_NVP(_bControlSpeedActivated);
		ar  & BOOST_SERIALIZATION_NVP(_controlPosThreshold);
		ar  & BOOST_SERIALIZATION_NVP(_accelOpenMax);
		ar  & BOOST_SERIALIZATION_NVP(_speedOpen);
		ar  & BOOST_SERIALIZATION_NVP(_speedOff);
		ar  & BOOST_SERIALIZATION_NVP(_bFootCancelStrategy1Activated);
		ar  & BOOST_SERIALIZATION_NVP(_bFootCancelStrategy2Activated);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelAccelLimit);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelClosingSpeed);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelPos);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelPosDiff);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelMaskTime);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelVelocity);

		if (fileVersion>=1)
		{
			ar  & BOOST_SERIALIZATION_NVP(_bCancelOpenHitActivated);
			ar  & BOOST_SERIALIZATION_NVP(_cancelOpenHitThreshold);
			ar  & BOOST_SERIALIZATION_NVP(_cancelOpenHitVelocity);
		}

		if (fileVersion>=2)
		{
			ar  & BOOST_SERIALIZATION_NVP(_posThresholdClose);
			ar  & BOOST_SERIALIZATION_NVP(_posThresholdOpen);
		}
	}
};

BOOST_CLASS_VERSION(HiHatPedalElement, 2)
