#ifndef __impulse_hpp
#define __impulse_hpp

#include <Arduino.h>
#include "common.hpp"
#include "typedqueue.hpp"

enum Impulse
{
  ImpulseEyeMovement,
  ImpulseBlink, //Sets it to closed immediately
  ImpulseText,
  ImpulseMood,
  ImpulseCross,
  ImpulseSquint,
};

extern TypedQueue<Datagram<Impulse>> impulseList;

#endif