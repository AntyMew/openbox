// -*- mode: C++; indent-tabs-mode: nil; -*-
#ifndef __timerqueuemanager_hh
#define __timerqueuemanager_hh

#include "timerqueue.hh"

namespace otk {

//! Manages a queue of OBTimer objects
/*!
  All OBTimer objects add themself to an OBTimerQueueManager. The manager is
  what fires the timers when their time has elapsed. This is done by having the
  application call the OBTimerQueueManager::fire class in its main event loop.
*/
class OBTimerQueueManager {
private:
  //! A priority queue of all timers being managed by this class.
  TimerQueue timerList;
public:
  //! Constructs a new OBTimerQueueManager
  OBTimerQueueManager() {}
  //! Destroys the OBTimerQueueManager
  virtual ~OBTimerQueueManager() {}

  //! Fire the next timer in the queue.
  /*!
    @param wait If true, this function will wait for the next timer, breaking
                on any events from the X server.
  */
  virtual void fire(bool wait = true);

  //! Adds a new timer to the queue
  /*!
    @param timer An OBTimer to add to the queue
  */
  virtual void addTimer(OBTimer* timer);
  //! Removes a timer from the queue
  /*!
    @param timer An OBTimer already in the queue to remove
  */
  virtual void removeTimer(OBTimer* timer);
};

}

#endif // __timerqueuemanager_hh
