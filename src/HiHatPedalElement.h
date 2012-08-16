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
#include <boost/signals2.hpp>

class HiHatPedalElement : public Pad
{
public:
	static const int MAX_ALLOWED_ACCELERATION = 2500000;
	static const int MIN_FOOT_SPEED = -8000;

	typedef boost::signals2::signal<void (int)> OnFootCancelMaskTimeChanged;
	typedef boost::signals2::signal<void (int)> OnFootCancelVelocityChanged;
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
	HiHatPedalElement(const HiHatPedalElement&);
	HiHatPedalElement& operator=(const HiHatPedalElement& rOther);

public:
    int getCurrentControlPos() const;
    void setCurrentControlPos(int value);
    float getCurrentControlSpeed() const;
    MovingState setCurrentControlSpeed(float value);
    float getCurrentControlAcceleration() const;
    void setCurrentControlAcceleration(float value);
	bool isBlueDetectionByPosition() const;
	void setBlueDetectionByPosition(const Parameter::Value& state);
	bool isBlueDetectionBySpeed() const;
	void setBlueDetectionBySpeed(const Parameter::Value& state);
    int getControlPosThreshold() const;
    void setControlPosThreshold(const Parameter::Value& value);
    int getOpenSpeed() const;
    void setOpenSpeed(const Parameter::Value& value);
    int getCloseSpeed() const;
    void setCloseSpeed(const Parameter::Value& value);
	bool isBlue() const;
	void setBlue(bool state);
	bool isHalfOpen() const;
	void setHalfOpen(bool state);
	bool isFootSplashCancel() const;
	void setFootSplashCancel(const Parameter::Value& state);
	int getFootCancelClosingSpeed() const;
	void setFootCancelClosingSpeed(const Parameter::Value& value);
	int getFootCancelPos() const;
	void setFootCancelPos(const Parameter::Value& value);
	int getFootCancelPosDiff() const;
	void setFootCancelPosDiff(const Parameter::Value& value);
	int getFootCancelMaskTime() const;
	void setFootCancelMaskTime(const Parameter::Value& value);
	void connectFootCancelMaskTime(const OnFootCancelMaskTimeChanged::slot_function_type& slot) {_onFootCancelMaskTimeChanged.disconnect_all_slots();_onFootCancelMaskTimeChanged.connect(slot);}
	int getFootCancelVelocity() const;
	void setFootCancelVelocity(const Parameter::Value& value);
	void connectFootCancelVelocity(const OnFootCancelVelocityChanged::slot_function_type& slot) {_onFootCancelVelocityChanged.disconnect_all_slots(); _onFootCancelVelocityChanged.connect(slot);}
	int getFootCancelTimeLimit() const;
	void setFootCancelTimeLimit(int value);
	int getHalfOpenEnteringTime() const;
	void setHalfOpenEnteringTime(int value);
	int getPositionOnCloseBegin() const;
	int getPositionOnOpenBegin() const;
	void setCancelOpenHitThreshold(const Parameter::Value& value);
	void setCancelOpenHitVelocity(const Parameter::Value& value);
	int getCancelOpenHitThreshold() const;
	int getCancelOpenHitVelocity() const;
	bool isCancelHitWhileOpen() const;
	void setCancelHitWhileOpen(const Parameter::Value& value);
	int getSecurityPosition() const;
	void setSecurityPosition(const Parameter::Value& value);
	int getSecurityOpenPosition() const;
	void setSecurityOpenPosition(const Parameter::Value& value);
	int getHalfOpenMaximumPosition() const;
	void setHalfOpenMaximumPosition(const Parameter::Value& value);
	int getHalfOpenActivationTime() const;
	void setHalfOpenActivationTime(const Parameter::Value& value);

private:
	bool	_isBlue;
	bool	_isHalfOpen;
	int		_footCancelTimeLimit;
	int		_halfOpenEnteringTime;
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
	Parameter::Value	_isBlueDetectionByPosition;
	Parameter::Value	_isBlueDetectionBySpeed;
	Parameter::Value	_controlPosThreshold;
	Parameter::Value	_openSpeed;
	Parameter::Value	_closeSpeed;
	Parameter::Value	_isFootSplashCancel;
	Parameter::Value	_footCancelClosingSpeed;
	Parameter::Value	_footCancelPos;
	Parameter::Value	_footCancelPosDiff;
	Parameter::Value	_footCancelMaskTime;
	Parameter::Value	_footCancelVelocity;
	Parameter::Value	_isCancelHitWhileOpen;
	Parameter::Value	_cancelOpenHitThreshold;
	Parameter::Value	_cancelOpenHitVelocity;
	Parameter::Value	_securityPosition;
	Parameter::Value	_securityOpenPosition;
	Parameter::Value	_halfOpenMaximumPosition;
	Parameter::Value	_halfOpenActivationTime;

	OnFootCancelMaskTimeChanged	_onFootCancelMaskTimeChanged;
	OnFootCancelVelocityChanged _onFootCancelVelocityChanged;

private:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive & ar, const unsigned int)
	{
		// Base class serialization
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Pad);

		// Hi Hat control and pedal
		ar  & BOOST_SERIALIZATION_NVP(_isBlueDetectionByPosition);
		ar  & BOOST_SERIALIZATION_NVP(_isBlueDetectionBySpeed);
		ar  & BOOST_SERIALIZATION_NVP(_controlPosThreshold);
		ar  & BOOST_SERIALIZATION_NVP(_openSpeed);
		ar  & BOOST_SERIALIZATION_NVP(_closeSpeed);
		ar  & BOOST_SERIALIZATION_NVP(_isFootSplashCancel);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelClosingSpeed);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelPos);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelPosDiff);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelMaskTime);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelVelocity);
		ar  & BOOST_SERIALIZATION_NVP(_isCancelHitWhileOpen);
		ar  & BOOST_SERIALIZATION_NVP(_cancelOpenHitThreshold);
		ar  & BOOST_SERIALIZATION_NVP(_cancelOpenHitVelocity);
		ar  & BOOST_SERIALIZATION_NVP(_securityPosition);
		ar  & BOOST_SERIALIZATION_NVP(_securityOpenPosition);
		ar  & BOOST_SERIALIZATION_NVP(_halfOpenMaximumPosition);
		ar  & BOOST_SERIALIZATION_NVP(_halfOpenActivationTime);
	}
};

BOOST_CLASS_VERSION(HiHatPedalElement, 0)
