//
//  valuemap.hpp
//
//
//  Created for Scaleworx on 9/25/15.
//
//

#ifndef valuemap_hpp
#define valuemap_hpp

#include <inttypes.h>
#include <Arduino.h>

template <class T>
class ValueMap {
public:
    uint16_t width;
    uint16_t height;
    T * values;

    ValueMap(uint16_t w, uint16_t h, T value): width(w), height(h),
      values(0) {
        uint16_t count = h * w;
        values = (T*) malloc(count * sizeof(T));
        fill(value);
      };

    ValueMap(uint16_t w, uint16_t h): width(w), height(h),
      values(0) {
        uint16_t size = sizeof(T) * h * w;
        values = (T*) malloc(size);
      };

    ValueMap(uint16_t w, uint16_t h, T v[]):
      width(w), height(h), values(v) {};
    
    void fill(T value) {
        uint16_t count = width * height;
        for (uint16_t i = 0; i < count; i++)
        {
          values[i] = value;
        }
    }

    T valueAtIndex(uint16_t x, uint16_t y) {
      if (x>=width) x = width - 1;
      if (y>=height) y = height - 1;
      return unsafeValueAtIndex(x,y);
    }

    void setValueAtIndex(T value, uint16_t x, uint16_t y) {
      if (x>=width) return;
      if (y>=height) return;
      unsafeSetValueAtIndex(value, x, y);
    }

    inline T unsafeValueAtIndex(uint16_t x, uint16_t y) {
      return values[x + width * y];
    }

    inline void unsafeSetValueAtIndex(T value, uint16_t x, uint16_t y) {
      values[x + width * y] = value;
    }
};

typedef ValueMap<uint32_t> LongMap;
typedef ValueMap<uint16_t> ShortMap;
typedef ValueMap<uint8_t> ByteMap;

#endif /* valuemap_cpp */
