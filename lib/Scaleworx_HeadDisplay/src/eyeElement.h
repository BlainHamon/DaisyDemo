#include <Arduino.h>
#include <layer.h>

template <class T>
class EyeElement
{
protected:
  PatternLayer<T> background;
  SpriteLayer<T> iris;
  CurveLayer<T> lowerLid;
  CurveLayer<T> upperLid;
  Layer<T> *topLayer;
  int16_t high, medium, low;
  int8_t squint, tilt;
  uint16_t width;
  uint16_t height;
public:
  ValueMap<T> buffer;
  EyeElement(uint16_t w, uint16_t h, ValueMap<T> g, ValueMap<T> i) :
    background(g),
    iris(i),
    lowerLid(w,h), upperLid(w,h),
    topLayer(&upperLid),
    high(-h/4), medium(h/2), low(h),
    squint(0), tilt(0),
    width(w), height(h),
    buffer(w,h)
  {
    xypoint p0 = makeXYPoint(0,h);

    xypoint p1a = makeXYPoint(0,high);
    xypoint p1b = makeXYPoint(0,low);

    xypoint p2a = makeXYPoint(w,high);
    xypoint p2b = makeXYPoint(w,low);

    xypoint p3 = makeXYPoint(w,h);

    upperLid.setControlPoints(p0, p1a, p2a, p3);
    lowerLid.setControlPoints(p0, p1b, p2b, p3);

    iris.setLowerLayer(background);
    lowerLid.setLowerLayer(iris);
    upperLid.setLowerLayer(lowerLid);
  }
  void setFocus(int8_t x, int8_t y) {
        int16_t ix = 100L + x; // 0 = left, 200 = right
        int16_t iy = 100L - y; // 0 = top, 200 = bottom
        iris.offsetx = (width * ix - 100 * iris.sprite.width) / 200;
        iris.offsety = (height * iy - 100 * iris.sprite.height) / 200;
    }
  void render() {
    int16_t delta = medium - high;
    upperLid.adjustControlHeights(
      high + (squint - tilt) * delta / 100,
      high + (squint + tilt) * delta / 100
    );
    delta = medium - low;
    lowerLid.adjustControlHeights(
      low + (squint - tilt) * delta / 100,
      low + (squint + tilt) * delta / 100
    );
    topLayer->applyToBuffer(buffer);
  }
  void addLayer(Layer<T> *layer) {
    layer->setLowerLayer(*topLayer);
    topLayer = layer;
  }
  inline void setSquint(int8_t s) { squint = s; }
  inline void setTilt(int8_t t) { tilt = t; }
  inline void setEyelid(T color) {
    upperLid.setAboveColor(color);
    lowerLid.setBelowColor(color);
  }
  inline void setEyeLine(T color) {
    upperLid.setLineColor(color);
  }

};
