#ifndef __controllerstate_hpp
#define __controllerstate_hpp
#include <Arduino.h>
#include "common.hpp"

enum ControllerButtons: uint16_t {
  ControllerButtonMask = 0x00FF,
  ControllerJoyMask    = 0xF000,
  ControllerMask       = ControllerButtonMask | ControllerJoyMask,

  ControllerButtonTop  = 0x0080,
  ControllerButtonBot  = 0x0040,

  ControllerButtonA    = 0x0010,
  ControllerButtonB    = 0x0001,
  ControllerButtonC    = 0x0008,
  ControllerButtonD    = 0x0002,

  ControllerJoyVert    = 0xC000,
  ControllerJoyUp      = 0x0000,
  ControllerJoyMid     = 0x4000,
  ControllerJoyDown    = 0x8000,

  ControllerJoyHori    = 0x3000,
  ControllerJoyLeft    = 0x0000,
  ControllerJoyCent    = 0x1000,
  ControllerJoyRight   = 0x2000,

  ControllerDefault    = ControllerJoyMid | ControllerJoyCent
};

/*
 * Up+Left  | 00 00
 * Up       | 00 10
 * Up+Right | 00 20
 * Right    | 00 40
 * Center   | 00 50
 * Left     | 00 60
 * Dn+left  | 00 80
 * Down     | 00 90
 * Dn+right | 00 A0
*/

typedef struct {
  ControllerButtons currentButtons;
  ControllerButtons changedButtons;
} ControllerState;

extern TypedQueue<ControllerState> controllerState;

#endif