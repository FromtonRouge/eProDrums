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

	typedef boost::signals2::signal<void (bool)> OnFootCancelActivated;
	typedef boost::signals2::signal<void (int)> OnFootCancelMaskTimeChanged;
	typedef boost::signals2::signal<void (int)> OnFootCancelVelocityChanged;
	typedef boost::signals2::signal<void (bool)> OnFootCancelAfterPedalHitActivated;
	typedef boost::signals2::signal<void (int)> OnFootCancelAfterPedalHitMaskTimeChanged;
	typedef boost::signals2::signal<void (int)> OnFootCancelAfterPedalHitVelocityChanged;
	typedef boost::shared_ptr<HiHatPedalElement> Ptr;
	enum MovingState
	{
		MS_NO_CHANGE,
		MS_START_OPEN,
		MS_START_CLOSE
	};

	enum BlueStateChangeReason
	{
		INITIAL_STATE,
		POSITION_THRESHOLD,
		IN_SECURED_ZONE,
		OPENING_MOVEMENT,
		CLOSING_MOVEMENT
	};

public:
	HiHatPedalElement();
	virtual ~HiHatPedalElement();
	HiHatPedalElement(const HiHatPedalElement&);
	HiHatPedalElement& operator=(const HiHatPedalElement& rOther);

public:
	int getCurrentControlPos() const;
	void setCurrentControlPos(int value);
	int getCurrentDeltaPos() const;
	void setCurrentDeltaPos(int value);
	float getCurrentControlSpeed() const;
	MovingState setCurrentControlSpeed(float value);
	float getCurrentControlAcceleration() const;
	void setCurrentControlAcceleration(float value);
	float getCurrentJerk() const;
	void setCurrentJerk(float value);

	bool isBlueDetectionByAccent() const;
	void setBlueDetectionByAccent(const Parameter::Value& state);
	bool isBlueDetectionByPosition() const;
	void setBlueDetectionByPosition(const Parameter::Value& state);
	bool isBlueDetectionBySpeed() const;
	void setBlueDetectionBySpeed(const Parameter::Value& state);
	bool isBlueDetectionByAcceleration() const;
	void setBlueDetectionByAcceleration(const Parameter::Value& state);
	bool isHalfOpenModeEnabled() const;
	void setHalfOpenModeEnabled(const Parameter::Value& state);
	int getControlPosThreshold() const;
	void setControlPosThreshold(const Parameter::Value& value);
	int getControlPosDelayTime() const;
	void setControlPosDelayTime(const Parameter::Value& value);

	int getOpenSpeed() const;
	void setOpenSpeed(const Parameter::Value& value);
	int getCloseSpeed() const;
	void setCloseSpeed(const Parameter::Value& value);

	int getOpenAcceleration() const;
	void setOpenAcceleration(const Parameter::Value& value);
	int getCloseAcceleration() const;
	void setCloseAcceleration(const Parameter::Value& value);

	int getOpenPositionDelta() const;
	void setOpenPositionDelta(const Parameter::Value& value);
	int getClosePositionDelta() const;
	void setClosePositionDelta(const Parameter::Value& value);

	bool isBlue() const;
	BlueStateChangeReason getBlueStateChangeReason() const;
	void setBlueStateChangeReason(BlueStateChangeReason reason);
	void setBlue(bool state, BlueStateChangeReason reason);
	bool isHalfOpen() const;
	void setHalfOpen(bool state);

	bool isFootCancel() const;
	void setFootCancel(const Parameter::Value& state);
	void connectFootCancelActivated(const OnFootCancelActivated::slot_function_type& slot) {_onFootCancelActivated.disconnect_all_slots();_onFootCancelActivated.connect(slot);}

	bool isFootCancelAfterPedalHit() const;
	void setFootCancelAfterPedalHit(const Parameter::Value& state);
	void connectFootCancelAfterPedalHitActivated(const OnFootCancelAfterPedalHitActivated::slot_function_type& slot) {_onFootCancelAfterPedalHitActivated.disconnect_all_slots();_onFootCancelAfterPedalHitActivated.connect(slot);}

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

	int getFootCancelAfterPedalHitMaskTime() const;
	void setFootCancelAfterPedalHitMaskTime(const Parameter::Value& value);
	void connectFootCancelAfterPedalHitMaskTime(const OnFootCancelAfterPedalHitMaskTimeChanged::slot_function_type& slot) {_onFootCancelAfterPedalHitMaskTimeChanged.disconnect_all_slots();_onFootCancelAfterPedalHitMaskTimeChanged.connect(slot);}

	int getFootCancelAfterPedalHitVelocity() const;
	void setFootCancelAfterPedalHitVelocity(const Parameter::Value& value);
	void connectFootCancelAfterPedalHitVelocity(const OnFootCancelAfterPedalHitVelocityChanged::slot_function_type& slot) {_onFootCancelAfterPedalHitVelocityChanged.disconnect_all_slots(); _onFootCancelAfterPedalHitVelocityChanged.connect(slot);}

	int getFootCancelTimeLimit() const;
	void setFootCancelTimeLimit(int value);
	int getHalfOpenEnteringTime() const;
	void setHalfOpenEnteringTime(int value);
	int getBlueStateEnteringTime() const;
	void setBlueStateEnteringTime(int value);
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
	int getHalfOpenMaximumPosition() const;
	void setHalfOpenMaximumPosition(const Parameter::Value& value);
	int getHalfOpenActivationTime() const;
	void setHalfOpenActivationTime(const Parameter::Value& value);
	QPolygonF getBlueAccentFunctions() const;
	void setBlueAccentFunctions(const Parameter::Value& value);
	void setBlueAccentOverride(const Parameter::Value& value);
	bool isBlueAccentOverride() const;
	void setBowAlwaysYellow(const Parameter::Value& value);
	bool isBowAlwaysYellow() const;

private:
	bool	_isBlue;
	bool	_isHalfOpen;
	int		_footCancelTimeLimit;
	int		_halfOpenEnteringTime;
	int		_blueStateEnteringTime;
	int		_previousControlPos;
	int		_currentControlPos;
	int		_currentDeltaPos;
	float	_currentControlSpeed;			// in unit/s
	float	_previousControlSpeed;			// in unit/s
	float 	_currentControlAcceleration;	// in unit/s�
	float 	_currentJerk;					// in unit/s3
	BlueStateChangeReason _blueStateChangeReason;

	/** Position of the hh control at the very start of a closing movement. */
	int		_posOnCloseBegin; 

	/** Position of the hh control at the very start of a opening movement. */
	int		_posOnOpenBegin; 

	// Archived data
	Parameter::Value	_isBlueDetectionByAccent;
	Parameter::Value	_isBlueDetectionByPosition;
	Parameter::Value	_isBlueDetectionBySpeed;
	Parameter::Value	_isBlueDetectionByAcceleration;
	Parameter::Value	_isHalfOpenModeEnabled;
	Parameter::Value	_isFootCancel;
	Parameter::Value	_isFootCancelAfterPedalHit;
	Parameter::Value	_isCancelHitWhileOpen;
	Parameter::Value	_isBlueAccentOverride;
	Parameter::Value	_isBowAlwaysYellow;
	Parameter::Value	_controlPosThreshold;
	Parameter::Value	_controlPosDelayTime;
	Parameter::Value	_openSpeed;
	Parameter::Value	_closeSpeed;
	Parameter::Value	_openAcceleration;
	Parameter::Value	_closeAcceleration;
	Parameter::Value	_openPositionDelta;
	Parameter::Value	_closePositionDelta;
	Parameter::Value	_footCancelClosingSpeed;
	Parameter::Value	_footCancelPos;
	Parameter::Value	_footCancelPosDiff;
	Parameter::Value	_footCancelMaskTime;
	Parameter::Value	_footCancelVelocity;
	Parameter::Value	_footCancelAfterPedalHitMaskTime;
	Parameter::Value	_footCancelAfterPedalHitVelocity;
	Parameter::Value	_cancelOpenHitThreshold;
	Parameter::Value	_cancelOpenHitVelocity;
	Parameter::Value	_securityPosition;
	Parameter::Value	_halfOpenMaximumPosition;
	Parameter::Value	_halfOpenActivationTime;
	Parameter::Value	_blueAccentFunctions;

	OnFootCancelActivated						_onFootCancelActivated;
	OnFootCancelMaskTimeChanged					_onFootCancelMaskTimeChanged;
	OnFootCancelVelocityChanged 				_onFootCancelVelocityChanged;
	OnFootCancelAfterPedalHitActivated			_onFootCancelAfterPedalHitActivated;
	OnFootCancelAfterPedalHitMaskTimeChanged	_onFootCancelAfterPedalHitMaskTimeChanged;
	OnFootCancelAfterPedalHitVelocityChanged	_onFootCancelAfterPedalHitVelocityChanged;

private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int)
	{
		// Base class serialization
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Pad);

		// Hi Hat control and pedal
		ar  & BOOST_SERIALIZATION_NVP(_isBlueDetectionByAccent);
		ar  & BOOST_SERIALIZATION_NVP(_isBlueDetectionByPosition);
		ar  & BOOST_SERIALIZATION_NVP(_isBlueDetectionBySpeed);
		ar  & BOOST_SERIALIZATION_NVP(_isBlueDetectionByAcceleration);
		ar  & BOOST_SERIALIZATION_NVP(_isHalfOpenModeEnabled);
		ar  & BOOST_SERIALIZATION_NVP(_isFootCancel);
		ar  & BOOST_SERIALIZATION_NVP(_isFootCancelAfterPedalHit);
		ar  & BOOST_SERIALIZATION_NVP(_isCancelHitWhileOpen);
		ar  & BOOST_SERIALIZATION_NVP(_isBlueAccentOverride);
		ar  & BOOST_SERIALIZATION_NVP(_isBowAlwaysYellow);
		ar  & BOOST_SERIALIZATION_NVP(_controlPosThreshold);
		ar  & BOOST_SERIALIZATION_NVP(_controlPosDelayTime);
		ar  & BOOST_SERIALIZATION_NVP(_openSpeed);
		ar  & BOOST_SERIALIZATION_NVP(_closeSpeed);
		ar  & BOOST_SERIALIZATION_NVP(_openAcceleration);
		ar  & BOOST_SERIALIZATION_NVP(_closeAcceleration);
		ar  & BOOST_SERIALIZATION_NVP(_openPositionDelta);
		ar  & BOOST_SERIALIZATION_NVP(_closePositionDelta);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelClosingSpeed);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelPos);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelPosDiff);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelMaskTime);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelVelocity);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelAfterPedalHitMaskTime);
		ar  & BOOST_SERIALIZATION_NVP(_footCancelAfterPedalHitVelocity);
		ar  & BOOST_SERIALIZATION_NVP(_cancelOpenHitThreshold);
		ar  & BOOST_SERIALIZATION_NVP(_cancelOpenHitVelocity);
		ar  & BOOST_SERIALIZATION_NVP(_securityPosition);
		ar  & BOOST_SERIALIZATION_NVP(_halfOpenMaximumPosition);
		ar  & BOOST_SERIALIZATION_NVP(_halfOpenActivationTime);
		ar  & BOOST_SERIALIZATION_NVP(_blueAccentFunctions);
	}
};

BOOST_CLASS_VERSION(HiHatPedalElement, 0)
