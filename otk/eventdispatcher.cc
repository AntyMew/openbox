// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "eventdispatcher.hh"
#include "display.hh"
#include <iostream>

namespace otk {

OtkEventDispatcher::OtkEventDispatcher()
  : _fallback(0), _master(0), _focus(None)
{
  _focus_e.xfocus.display = OBDisplay::display;
  _focus_e.xfocus.mode = NotifyNormal;
  _focus_e.xfocus.detail = NotifyNonlinear;

  _crossing_e.xcrossing.display = OBDisplay::display;
  _crossing_e.xcrossing.mode = NotifyNormal;
  _crossing_e.xcrossing.detail = NotifyNonlinear;
}

OtkEventDispatcher::~OtkEventDispatcher()
{
}

void OtkEventDispatcher::clearAllHandlers(void)
{
  _map.clear();
}

void OtkEventDispatcher::registerHandler(Window id, OtkEventHandler *handler)
{
  _map.insert(std::pair<Window, OtkEventHandler*>(id, handler));
}

void OtkEventDispatcher::clearHandler(Window id)
{
  _map.erase(id);
}

void OtkEventDispatcher::dispatchEvents(void)
{
  XEvent e;
  Window focus = None, unfocus = None;
  Window enter = None, leave = None;
  Window enter_root = None, leave_root = None;

  while (XPending(OBDisplay::display)) {
    XNextEvent(OBDisplay::display, &e);

#if 0//defined(DEBUG)
    printf("Event %d window %lx\n", e.type, e.xany.window);
#endif

    Window win;

    // pick a window
    switch (e.type) {
    case UnmapNotify:
      win = e.xunmap.window;
      break;
    case DestroyNotify:
      win = e.xdestroywindow.window;
      break;
    case ConfigureRequest:
      win = e.xconfigurerequest.window;
      break;
    default:
      win = e.xany.window;
    }
    
    // grab the lasttime and hack up the modifiers
    switch (e.type) {
    case ButtonPress:
    case ButtonRelease:
      _lasttime = e.xbutton.time;
      e.xbutton.state &= ~(LockMask | OBDisplay::numLockMask() |
                           OBDisplay::scrollLockMask());
      break;
    case KeyPress:
      e.xkey.state &= ~(LockMask | OBDisplay::numLockMask() |
                        OBDisplay::scrollLockMask());
      break;
    case MotionNotify:
      _lasttime = e.xmotion.time;
      e.xmotion.state &= ~(LockMask | OBDisplay::numLockMask() |
                           OBDisplay::scrollLockMask());
      break;
    case PropertyNotify:
      _lasttime = e.xproperty.time;
      break;
    case EnterNotify:
    case LeaveNotify:
      _lasttime = e.xcrossing.time;
      break;
    }

    // madly compress all focus events
    if (e.type == FocusIn) {
      // any other types are not ones we're interested in
      if (e.xfocus.detail == NotifyNonlinear) {
        focus = e.xfocus.window;
        unfocus = None;
        printf("FocusIn focus=%lx unfocus=%lx\n", focus, unfocus);
      }
    } else if (e.type == FocusOut) {
      // any other types are not ones we're interested in
      if (e.xfocus.detail == NotifyNonlinear) {
        unfocus = e.xfocus.window;
        focus = None;
        printf("FocusOut focus=%lx unfocus=%lx\n", focus, unfocus);
      }
    // madly compress all crossing events
    } else if (e.type == EnterNotify) {
      // any other types are not ones we're interested in
      if (e.xcrossing.mode == NotifyNormal) {
        // any other types are not ones we're interested in
        enter = e.xcrossing.window;
        enter_root = e.xcrossing.root;
        printf("Enter enter=%lx leave=%lx\n", enter, leave);
      }
    } else if (e.type == LeaveNotify) {
      // any other types are not ones we're interested in
      if (e.xcrossing.mode == NotifyNormal) {
        leave = e.xcrossing.window;
        leave_root = e.xcrossing.root;
        printf("Leave enter=%lx leave=%lx\n", enter, leave);
      }
    } else {
      // normal events
      dispatch(win, e);
    }
  }

  if (unfocus != None) {
    // the last focus event was an FocusOut, so where the hell is the focus at?
    printf("UNFOCUSING: %lx\n", unfocus);
    _focus_e.xfocus.type = FocusOut;
    _focus_e.xfocus.window = unfocus;
    dispatch(_focus_e.xfocus.window, _focus_e);

    _focus = None;
  } else if (focus != None && focus != _focus) {
    // the last focus event was a FocusIn, so unfocus what used to be focus and
    // focus this new target
    printf("FOCUSING: %lx\n", focus);
    _focus_e.xfocus.type = FocusIn;
    _focus_e.xfocus.window = focus;
    dispatch(_focus_e.xfocus.window, _focus_e);

    if (_focus != None) {
      printf("UNFOCUSING: %lx\n", _focus);
      _focus_e.xfocus.type = FocusOut;
      _focus_e.xfocus.window = _focus;
      dispatch(_focus_e.xfocus.window, _focus_e);
    }
    
    _focus = focus;
  }
  
  if (leave != None) {
    _crossing_e.xcrossing.type = LeaveNotify;
    _crossing_e.xcrossing.window = leave;
    _crossing_e.xcrossing.root = leave_root;
    dispatch(_crossing_e.xcrossing.window, _crossing_e);
  }
  if (enter != None) {
    _crossing_e.xcrossing.type = EnterNotify;
    _crossing_e.xcrossing.window = enter;
    _crossing_e.xcrossing.root = enter_root;
    dispatch(_crossing_e.xcrossing.window, _crossing_e);
  }
}

void OtkEventDispatcher::dispatch(Window win, const XEvent &e) {
  OtkEventHandler *handler = 0;
  OtkEventMap::iterator it;

  if (_master)
    _master->handle(e);

  it = _map.find(win);
  
  if (it != _map.end())
    handler = it->second;
  // these ConfigureRequests require some special attention
  else if (e.type == ConfigureRequest) {
    // unhandled configure requests must be used to configure the window
    // directly
    XWindowChanges xwc;
      
    xwc.x = e.xconfigurerequest.x;
    xwc.y = e.xconfigurerequest.y;
    xwc.width = e.xconfigurerequest.width;
    xwc.height = e.xconfigurerequest.height;
    xwc.border_width = e.xconfigurerequest.border_width;
    xwc.sibling = e.xconfigurerequest.above;
    xwc.stack_mode = e.xconfigurerequest.detail;
      
    XConfigureWindow(otk::OBDisplay::display, e.xconfigurerequest.window,
                     e.xconfigurerequest.value_mask, &xwc);
  } else
    handler = _fallback;

  if (handler)
    handler->handle(e);
}

OtkEventHandler *OtkEventDispatcher::findHandler(Window win)
{
  OtkEventMap::iterator it = _map.find(win);
  if (it != _map.end())
    return it->second;
  return 0;
}

}
