//****************************************************************************//
// animation_action.cpp                                                       //
// Copyright (C) 2001, 2002 Bruno 'Beosil' Heidelberger                       //
//****************************************************************************//
// This library is free software; you can redistribute it and/or modify it    //
// under the terms of the GNU Lesser General Public License as published by   //
// the Free Software Foundation; either version 2.1 of the License, or (at    //
// your option) any later version.                                            //
//****************************************************************************//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//****************************************************************************//
// Includes                                                                   //
//****************************************************************************//

#include "cal3d/animation_action.h"
#include "cal3d/error.h"
#include "cal3d/coreanimation.h"

 /*****************************************************************************/
/** Constructs the animation action instance.
  *
  * This function is the default constructor of the animation action instance.
  *****************************************************************************/

CalAnimationAction::CalAnimationAction(CalCoreAnimation* pCoreAnimation)
: CalAnimation(pCoreAnimation)
{
  setType(TYPE_ACTION);
}


 /*****************************************************************************/
/** Executes the animation action instance.
  *
  * This function executes the animation action instance.
  *
  * @param delayIn The time in seconds until the animation action instance
  *                reaches the full weight from the beginning of its execution.
  * @param delayOut The time in seconds in which the animation action instance
  *                 reaches zero weight at the end of its execution.
  * @param weightTarget No doxygen comment for this. FIXME.
  * @param autoLock     This prevents the Action from being reset and removed
  *                     on the last keyframe if true.
  *
  * @return One of the following values:
  *         \li \b true if successful
  *         \li \b false if an error happend
  *****************************************************************************/

bool CalAnimationAction::execute(float delayIn, float delayOut, float weightTarget,bool autoLock)
{
  setState(STATE_IN);
  setWeight(0.0f);
  m_delayIn = delayIn;
  m_delayOut = delayOut;
  setTime(0.0f);
  m_weightTarget = weightTarget;
  m_autoLock = autoLock;

  m_total_duration = getCoreAnimation()->getDuration();

  return true;
}

 /*****************************************************************************/
/** Updates the animation action instance.
  *
  * This function updates the animation action instance for a given amount of
  * time.
  *
  * @param deltaTime The elapsed time in seconds since the last update.
  *
  * @return One of the following values:
  *         \li \b true if the animation action instance is still active
  *         \li \b false if the execution of the animation action instance has
  *             ended
  *****************************************************************************/

bool CalAnimationAction::update(float deltaTime)
{
  // update animation action time

  if(getState() != STATE_STOPPED)
  {
    setTime(getTime() + deltaTime * getTimeFactor());
  }

  // handle IN phase
  if(getState() == STATE_IN)
  {
    // check if we are still in the IN phase
    if(getTime() < m_delayIn)
    {
      setWeight(getTime() / m_delayIn * m_weightTarget);
      //m_weight = m_time / m_delayIn;
    }
    else
    {
      setState(STATE_STEADY);
      setWeight(m_weightTarget);
    }
  }

  // handle STEADY
  if(getState() == STATE_STEADY)
  {
    // check if we reached OUT phase
    if (!m_autoLock && getTime() >= m_total_duration - m_delayOut)
    {
      setState(STATE_OUT);
    }
    // if the anim is supposed to stay locked on last keyframe, reset the time here.
    else if (m_autoLock && getTime() > m_total_duration)
    {
      setState(STATE_STOPPED);
      setTime(m_total_duration);
    }      
  }

  // handle OUT phase
  if(getState() == STATE_OUT)
  {
    // check if we are still in the OUT phase
    if (getTime() < m_total_duration)
    {
      setWeight((m_total_duration - getTime()) / m_delayOut * m_weightTarget);
    }
    else
    {
      // we reached the end of the action animation
      setWeight(0.0f);
      return false;
    }
  }

  return true;

}

// ----------------------------------------------------
// MCV
bool CalAnimationAction::remove(float timeout) {
  if (getState() == STATE_STOPPED || timeout == 0.f)
    return true;

  setState(STATE_OUT);

  // Do we have enough timeout time?
  if (getTime() + timeout > m_total_duration)
    timeout = m_total_duration - getTime();

  // Modify official duration
  m_total_duration = getTime() + timeout;

  // Update timeout
  m_delayOut = timeout;

  // To avoid discontinuities if cancelled during IN/OUT stages
  m_weightTarget = getWeight();

  // Returns false to indicate the animation should NOT be removed
  // from the list of active animations.
  return false;
}


//****************************************************************************//
