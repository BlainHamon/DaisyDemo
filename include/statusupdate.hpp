#ifndef __statusupdate_hpp
#define __statusupdate_hpp

#include <Arduino.h>
#include "common.hpp"
#include "typedqueue.hpp"

enum StatusUpdate
{
  StatusUpdateAddLine,
  StatusUpdateBattery,
  StatusUpdateClearLines,
  StatusUpdateReplaceLine1,
  StatusUpdateReplaceLine2,
  StatusUpdateReplaceLine3,
  StatusUpdateReplaceLine4,
};

extern TypedQueue<Datagram<StatusUpdate>> updateList;

#endif