//
//  CubicBezierCurve.hpp
//
//
//  Created for Scaleworx on 2/26/16.
//
//

#ifndef CubicBezierCurve_hpp
#define CubicBezierCurve_hpp

#include <inttypes.h>
#include <stdlib.h>
#include <Arduino.h>

inline uint8_t enforceByte(int16_t val) {
  if (val < 0 ) return 0;
  if (val > 255) return 255;
  return val;
}

typedef struct {
    int16_t x;
    int16_t y;
} xypoint;

inline xypoint makeXYPoint(int16_t x, int16_t y) {
    xypoint result;
    result.x = x; result.y = y;
    return result;
}

class CubicBezierCurve {
protected:
// The bytes here are meant to be fractional. Or rather, table 0 includes
// the half bit, and table 1 and 2 include the quarter bit.
    uint16_t width;
    uint8_t * table0; //0-255 -> 0-127.5, shift 1 down for actual value.
    uint8_t * table1; //0-255 -> 0-63.75, shift down 2
    uint8_t * table2; //See table 1
public:

    CubicBezierCurve(uint16_t w) : width(w) {
      table0 = (uint8_t*)malloc(w*sizeof(uint8_t));
      table1 = (uint8_t*)malloc(w*sizeof(uint8_t));
      table2 = (uint8_t*)malloc(w*sizeof(uint8_t));
    };

    inline uint16_t getWidth() { return width; }

    int16_t YAtX(uint8_t x, int16_t y1, int16_t y2) {
        int32_t result = (table0[x] << 8) + // decimal point is 10 values in
            table1[x] * y1 + table2[x] * y2;
        return result >> 9;
    };

    /// This partially computes a curve given end points p0 and p4. The y value in p1 and p2 are not used, but used at YatX, allowing
    /// P1 and P2 to slide vertically without doing the full computation
  void recalculateCurve(xypoint p0, xypoint p1, xypoint p2, xypoint p3) {
      float t;
      float nextX = 0.5; /// So we only deal with it once
      for (t = 0.0; t<1.0; t+= 0.001) { ///Travel along the curve using time
          float t2 = t*t;
          float notT = 1.0-t; /// The amount of time left
          float notT2 = notT * notT;
          float x = notT2*(notT * p0.x + 3 * t * p1.x) + t2 * (3 * notT * p2.x + t * p3.x);
          if (x < nextX) {
              continue;
          }
          if (x >= width) {
              break;
          }
          nextX = nextX+1;
          uint16_t intx = floorf(x);
          table0[intx] = enforceByte(2.0 * ( notT2 * notT * p0.y + t2 * t * p3.y ));
          table1[intx] = enforceByte(1536.0 * notT2 * t );
          table2[intx] = enforceByte(1536.0 * notT * t2 );
      }
  }
};

#endif /* CubicBezierCurve_hpp */
