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
	static const int MAX_ALLOWED_ACCELERATION = 2500000;
	static const int MIN_FOOT_SPEED = -8000;

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
	bool	isBlue() const;
	void	setBlue(bool state, BlueStateChangeReason reason);
	bool	isHalfOpen() const;
	void	setHalfOpen(bool state);
	int		getFootCancelTimeLimit() const;
	void	setFootCancelTimeLimit(int value);
	int		getHalfOpenEnteringTime() const;
	void	setHalfOpenEnteringTime(int value);
	int		getBlueStateEnteringTime() const;
	void	setBlueStateEnteringTime(int value);
	int		getCurrentControlPos() const;
	void	setCurrentControlPos(int value);
	int		getCurrentDeltaPos() const;
	void	setCurrentDeltaPos(int value);
	float	getCurrentControlSpeed() const;
	MovingState setCurrentControlSpeed(float value);
	float	getCurrentControlAcceleration() const;
	void	setCurrentControlAcceleration(float value);
	float	getCurrentJerk() const;
	void	setCurrentJerk(float value);
	BlueStateChangeReason getBlueStateChangeReason() const;
	void setBlueStateChangeReason(BlueStateChangeReason reason);
	int		getPositionOnCloseBegin() const;
	int		getPositionOnOpenBegin() const;


	/* TODO
	bool isBlueDetectionByPosition() const;
	void setBlueDetectionByPosition(const Property::Value& state);
	*/

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
	float 	_currentControlAcceleration;	// in unit/s²
	float 	_currentJerk;					// in unit/s3
	BlueStateChangeReason _blueStateChangeReason;

	/** Position of the hh control at the very start of a closing movement. */
	int		_posOnCloseBegin; 

	/** Position of the hh control at the very start of a opening movement. */
	int		_posOnOpenBegin; 

public:
	Property<bool>::Ptr	isBlueDetectionByAccent;
	Property<bool>::Ptr	isBlueDetectionByPosition;
	Property<bool>::Ptr	isBlueDetectionBySpeed;
	Property<bool>::Ptr	isBlueDetectionByAcceleration;
	Property<bool>::Ptr	isHalfOpenModeEnabled;
	Property<bool>::Ptr	isFootCancel;
	Property<bool>::Ptr	isFootCancelAfterPedalHit;
	Property<bool>::Ptr	isCancelHitWhileOpen;
	Property<bool>::Ptr	isBlueAccentOverride;
	Property<bool>::Ptr	isBowAlwaysYellow;
	Property<int>::Ptr	controlPosThreshold;
	Property<int>::Ptr	controlPosDelayTime;
	Property<int>::Ptr	openSpeed;
	Property<int>::Ptr	closeSpeed;
	Property<int>::Ptr	openAcceleration;
	Property<int>::Ptr	closeAcceleration;
	Property<int>::Ptr	openPositionDelta;
	Property<int>::Ptr	closePositionDelta;
	Property<int>::Ptr	footCancelClosingSpeed;
	Property<int>::Ptr	footCancelPos;
	Property<int>::Ptr	footCancelPosDiff;
	Property<int>::Ptr	footCancelMaskTime;
	Property<int>::Ptr	footCancelVelocity;
	Property<int>::Ptr	footCancelAfterPedalHitMaskTime;
	Property<int>::Ptr	footCancelAfterPedalHitVelocity;
	Property<int>::Ptr	cancelOpenHitThreshold;
	Property<int>::Ptr	cancelOpenHitVelocity;
	Property<int>::Ptr	securityPosition;
	Property<int>::Ptr	halfOpenMaximumPosition;
	Property<int>::Ptr	halfOpenActivationTime;

	Property<QPolygonF>::Ptr	funcBlueAccent;

private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int)
	{
		// Base class serialization
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Pad);

		// Hi Hat control and pedal
		ar  & BOOST_SERIALIZATION_NVP(isBlueDetectionByAccent);
		ar  & BOOST_SERIALIZATION_NVP(isBlueDetectionByPosition);
		ar  & BOOST_SERIALIZATION_NVP(isBlueDetectionBySpeed);
		ar  & BOOST_SERIALIZATION_NVP(isBlueDetectionByAcceleration);
		ar  & BOOST_SERIALIZATION_NVP(isHalfOpenModeEnabled);
		ar  & BOOST_SERIALIZATION_NVP(isFootCancel);
		ar  & BOOST_SERIALIZATION_NVP(isFootCancelAfterPedalHit);
		ar  & BOOST_SERIALIZATION_NVP(isCancelHitWhileOpen);
		ar  & BOOST_SERIALIZATION_NVP(isBlueAccentOverride);
		ar  & BOOST_SERIALIZATION_NVP(isBowAlwaysYellow);
		ar  & BOOST_SERIALIZATION_NVP(controlPosThreshold);
		ar  & BOOST_SERIALIZATION_NVP(controlPosDelayTime);
		ar  & BOOST_SERIALIZATION_NVP(openSpeed);
		ar  & BOOST_SERIALIZATION_NVP(closeSpeed);
		ar  & BOOST_SERIALIZATION_NVP(openAcceleration);
		ar  & BOOST_SERIALIZATION_NVP(closeAcceleration);
		ar  & BOOST_SERIALIZATION_NVP(openPositionDelta);
		ar  & BOOST_SERIALIZATION_NVP(closePositionDelta);
		ar  & BOOST_SERIALIZATION_NVP(footCancelClosingSpeed);
		ar  & BOOST_SERIALIZATION_NVP(footCancelPos);
		ar  & BOOST_SERIALIZATION_NVP(footCancelPosDiff);
		ar  & BOOST_SERIALIZATION_NVP(footCancelMaskTime);
		ar  & BOOST_SERIALIZATION_NVP(footCancelVelocity);
		ar  & BOOST_SERIALIZATION_NVP(footCancelAfterPedalHitMaskTime);
		ar  & BOOST_SERIALIZATION_NVP(footCancelAfterPedalHitVelocity);
		ar  & BOOST_SERIALIZATION_NVP(cancelOpenHitThreshold);
		ar  & BOOST_SERIALIZATION_NVP(cancelOpenHitVelocity);
		ar  & BOOST_SERIALIZATION_NVP(securityPosition);
		ar  & BOOST_SERIALIZATION_NVP(halfOpenMaximumPosition);
		ar  & BOOST_SERIALIZATION_NVP(halfOpenActivationTime);
		ar  & BOOST_SERIALIZATION_NVP(funcBlueAccent);
	}
};

BOOST_CLASS_VERSION(HiHatPedalElement, 0)
