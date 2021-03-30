#ifndef _COMMON
#define _COMMON

#include <Arduino.h>
#include "manager.hpp"
#include "typedqueue.hpp"

extern int errorNumber;

template <class T>
struct Datagram {
  T type;
  union
  {
    uint8_t bytes[4];
    uint32_t value;
    uint8_t *pointer;
    struct {
      int8_t x;
      int8_t y;
      int8_t w;
      int8_t h;
    };
    char *text;
  };
};

#endif