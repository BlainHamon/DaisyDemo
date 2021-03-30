//
//  layer.hpp
//
//
//  Created for Scaleworx on 11/17/15.
//
//

#ifndef layer_hpp
#define layer_hpp

#include <inttypes.h>
#include "valuemap.h"
#include "cubicbeziercurve.h"
#include "font.h"

template <class T>
class Layer {
protected:
    Layer<T> * next;
public:
    Layer() : next(0) {};
    inline void setLowerLayer(Layer<T> & newNext) {next = &newNext;};
    void add(Layer<T> &newNext);
    virtual T colorAtLocation(uint16_t x, uint16_t y) = 0;
    virtual void applyToBuffer(ValueMap<T> &buffer) {
      uint16_t y = 0;
      while ( y < buffer.height ) {
        uint16_t x = 0;
        while ( x < buffer.width ) {
          buffer.setValueAtIndex(colorAtLocation(x,y), x, y );
          x++;
        }
        y++;
      }
    }
};

template <class T>
class OpaqueColorLayer : public Layer<T> {
    T color;
public:
    OpaqueColorLayer(T c) : color(c) {};
    T colorAtLocation(uint16_t x, uint16_t y) { return color; };
};

template <class T>
class PatternLayer : public Layer<T> {
    ValueMap<T> pattern;
public:
    PatternLayer(ValueMap<T> p) : pattern(p) {};
    inline void setPattern(ValueMap<T> p) { pattern = p; }
    T colorAtLocation(uint16_t x, uint16_t y) {
      return pattern.unsafeValueAtIndex(
        x % pattern.width, y % pattern.height
      );
    };
};

template <class T>
class SpriteLayer : public Layer<T> {
public:
    ValueMap<T> sprite;
    int16_t offsetx;
    int16_t offsety;
    SpriteLayer(ValueMap<T> s, int16_t x = 0, int16_t y = 0) : sprite(s), offsetx(x), offsety(y) {};
    T colorAtLocation(uint16_t x, uint16_t y) {
      uint16_t sx = x - offsetx;
      uint16_t sy = y - offsety;
      if ((sx < 0) || (sx >= sprite.width) || (sy < 0) || (sy >= sprite.height)) {
        return this->next->colorAtLocation(x,y);
      }
      return sprite.unsafeValueAtIndex(sx, sy);
    };
    void applyToBuffer(ValueMap<T> &buffer) {
      this->next->applyToBuffer(buffer);
      uint16_t sy = 0;
      while ( sy < sprite.height ) {
        int16_t y = sy + offsety;
        if (y < 0 ) {
          sy++;
          continue;
        }
        if (y >= buffer.height) { break; }
        uint16_t sx = 0;
        while ( sx < sprite.width ) {
          int16_t x = sx + offsetx;
          if (x < 0 ) {
            sx++;
            continue;
          }
          if (x >= buffer.width) { break; }
          buffer.setValueAtIndex(sprite.unsafeValueAtIndex(sx,sy), x, y );
          sx++;
        }
        sy++;
      }
    }

};

template <class T>
class CurveLayer : public Layer<T> {
  xypoint p0;
  xypoint p1;
  xypoint p2;
  xypoint p3;
  CubicBezierCurve curve;
  T aboveColor;
  T lineColor;
  T belowColor;
  bool drawAbove;
  bool drawLine;
  bool drawBelow;
public:
  inline void setAboveColor(T c) { aboveColor = c; drawAbove = 1; }
  inline void setLineColor(T c) { lineColor = c; drawLine = 1; }
  inline void setBelowColor(T c) { belowColor = c; drawBelow = 1; }
  inline void clearAbove() { drawAbove = 0; }
  inline void clearLine() { drawLine = 0; }
  inline void clearBelow() { drawBelow = 0; }
  inline void setControlPoints(xypoint pA, xypoint pB, xypoint pC, xypoint pD) {
    p0 = pA; p1 = pB; p2 = pC; p3 = pD; refreshCurve();
  }
  CurveLayer(uint16_t w, uint16_t h = 0) : p0({0,0}), p1({w,0}), p2({0,h}), p3({w,h}), curve(w), drawAbove(0), drawLine(0), drawBelow(0) {}
  void refreshCurve() { curve.recalculateCurve(p0, p1, p2, p3); }
  T colorAtLocation(uint16_t x, uint16_t y) {
    uint16_t lineY = curve.YAtX(x, p1.y, p2.y);
    if (drawAbove && (lineY > y)) {
      return aboveColor;
    } else if (drawLine && (lineY == y)) {
      return lineColor;
    } else if (drawBelow && (lineY < y)) {
      return belowColor;
    }
    return this->next->colorAtLocation(x,y);
  }
  uint16_t lineYAtX(uint16_t x) {
    return curve.YAtX(x, p1.y, p2.y);
  }
  void adjustControlHeights(int16_t y1, int16_t y2) { // Does not force recalculation
    p1.y = y1;
    p2.y = y2;
  }

  void applyToBuffer(ValueMap<T> &buffer) {
    // If we're not completely opaque
    if (!drawAbove || !drawBelow || !drawLine || (buffer.width > curve.getWidth())) {
      this->next->applyToBuffer(buffer);
    }
    for (uint16_t x = 0; x < curve.getWidth(); x++)
    {
      uint16_t lineY = lineYAtX(x);
      if (drawAbove) {
        uint16_t maxY = min(lineY, buffer.height);
        for (uint y = 0; y < maxY; y++)
        {
          buffer.setValueAtIndex(aboveColor, x, y);
        }
      }
      if (drawLine && (lineY >= 0) && (lineY < buffer.height)) {
        buffer.setValueAtIndex(lineColor, x, lineY);
        if ( x + 1 < curve.getWidth() ) {
          uint16_t nextY = lineYAtX(x+1);
          if ( lineY - nextY > 1 ) {
            for (nextY++; nextY < lineY; nextY++)
            {
              buffer.setValueAtIndex(lineColor, x, nextY);
            }
          }
        }
        if (x > 0) {
          uint16_t lastY = curve.YAtX(x - 1, p1.y, p2.y);
          if ( lineY - lastY > 1) {
            for (lastY++; lastY < lineY; lastY++)
            {
              buffer.setValueAtIndex(lineColor, x, lastY);
            }
          }
        }
      }
      if (drawBelow) {
        uint16_t minY = max(lineY + 1, 0);
        uint16_t maxY = buffer.height;
        for (uint16_t y = minY; y < maxY; y++)
        {
          buffer.setValueAtIndex(belowColor, x, y);
        }
      }
    }
  }
};

template <class T, class M>
class MaskLayer : public Layer<T> {
  M *mask;
  Layer<T> maskedLayer;
  xypoint maskOffset;
public:
  MaskLayer(){};
};

template <class T, class M> //M is a type of LayerFont
class TextLayer : public Layer<T> {
  M *font;
  Layer<T> *textureLayer;
  xypoint textPosition;
  // Thing to track font
  char *text;
  uint16_t start; // Where in the string to start
  uint16_t end; // Typically string length

  int16_t firstLetterOffset;
  // How many columns to remove (or if start == 0, offset < 0, to pad) for scrolling

  void adjustOffset(uint16_t *i, int16_t *o) {
    uint16_t index = 0;
    if (i != NULL) {
      index = *i;
    }
    int16_t offset = 0;
    if (o != NULL) {
      offset = *o;
    }
    int16_t tempWidth = width(index);
    while ((offset >= tempWidth) && (index < end)) {
      index += 1;
      offset -= tempWidth;
      tempWidth = width(index);
    }
    while ((offset < 0) && (index > 0)) {
      index -= 1;
      tempWidth = width(index);
      offset += tempWidth;
    }
    if (i != NULL) {
      *i = index;
    }
    if (o != NULL) {
      *o = offset;
    }
  }

public:
  TextLayer(M *f, char *t = NULL): Layer<T>(), font(f), textPosition(makeXYPoint(0,0)) {setText(t);};
  T colorAtLocation(uint16_t x, uint16_t y) {
    int16_t fontx = x + firstLetterOffset - textPosition.x;
    int16_t fonty = y - textPosition.y;
    if (isTextAt(fontx,fonty)) {
      return this->textureLayer->colorAtLocation(x, y);
    }
    return this->next->colorAtLocation(x, y);
  }

  uint8_t isTextAt(int16_t x, int16_t y) {
    uint16_t letter = start;
    if (y < 0 || x < 0) {
      return 0;
    }
    adjustOffset(&letter, &x);
    if (letter >= end) {
      return 0;
    }
    x += font->start(text[letter]);
    return font->valueAt(x,y);
  }

  void setText(char * newText) {
    text = newText;
    start = 0;
    firstLetterOffset = 0;
    if (newText == NULL) {
      end = 0;
    } else {
      end = strlen(newText);
    }
  };

  void setTexture(Layer<T> *texture) {
    textureLayer = texture;
  }

  inline int16_t width(uint16_t index) {
    return font->charWidth(text[index]);
  }

  void addOffset(int16_t offset) {
    firstLetterOffset += offset;
    adjustOffset(&start, &firstLetterOffset);
  }
};


#endif /* layer_hpp */
