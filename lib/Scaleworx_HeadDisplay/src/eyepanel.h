#include <Arduino.h>
#include "eyeElement.h"

#include <FastLED.h>
#include <SPI.h>

#include <layer.h>

CRGB gradientVals[] = {
    {0, 80, 80},
    {0, 70, 70},
    {0, 60, 60},
    {0, 50, 50},
    {0, 40, 40},
    {0, 30, 30},
    {0, 20, 20},
    {0, 10, 10},
};
ValueMap<CRGB> gradient(1, 8, gradientVals);
CRGB irisImageVals[] = {
  {0,0,30}, {0,0,25},
  {0,0,25}, {0,0,20},
  {0,0,20}, {0,0,15},
};
ValueMap<CRGB> irisImage(2, 3, irisImageVals);
CRGB white = {30,30,30};
CRGB red = {50,0,0};
CRGB green = {0,50,0};
CRGB blue = {0,0,50};
CRGB black = {0,0,0};

class EyePanel
{
public:
    EyeElement<CRGB> left;
    EyeElement<CRGB> right;
    uint8_t blink;
    uint8_t squint;
    int8_t mood; // -100 for angry, 100 for sad
    int8_t cross;
    EyePanel() :
      left(8, 8, gradient, irisImage),
      right(8, 8, gradient, irisImage),
      blink(0), squint(0), mood(0), cross(0)
    {
      left.setEyelid(black);
      left.setEyeLine(blue);
      right.setEyelid(black);
      right.setEyeLine(blue);
    };
    void render() {
      int16_t eyeWide = (100 - squint) * (100 - blink); //wide eyed, from 0 to 10k
      int16_t squint = abs(mood) / 2 + 100 - eyeWide / 100;
      left.setSquint(squint);
      right.setSquint(squint);
      left.setTilt(-mood);
      right.setTilt(mood);
      left.render();
      right.render();
    }
    void setFocus(int8_t x, int8_t y) {
      int8_t calcX = min(100, max(-100, x + cross));
      left.setFocus(calcX, y);
      calcX = min(100, max(-100, x - cross));
      right.setFocus(calcX, y);
    }
};
