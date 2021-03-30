#include <Arduino.h>

#include <SPI.h>
#include <FastLED.h>

#include <layer.h>
#include <eyepanel.h>

#include "common.hpp"
#include "impulse.hpp"
#include <FreeRTOS.h>

#include <daisyfont.h>

// #include <Adafruit_RA8875.h>

#ifdef ESP32
const int pinEyeData = MOSI;
const int pinEyeClock = SCK;
#else
const int pinEyeData = 4;
const int pinEyeClock = 5;
#endif

#define NUM_LEDS 128 // Number of LEDs in strip

#define LED_TYPE    DOTSTAR
#define COLOR_ORDER BGR

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

class ExteriorDisplayManager : public Manager {
  CRGB leds[NUM_LEDS];
  EyePanel eyePanel;
  TextLayer<CRGB, DaisyFontClass> leftText;
  TextLayer<CRGB, DaisyFontClass> rightText;
  OpaqueColorLayer<CRGB> whiteLayer;
  uint32_t ticks;

  int8_t currentX;
  int8_t currentY;

  int8_t desiredX;
  int8_t desiredY;
  uint8_t desiredBlink;
  uint8_t desiredSquint;
  int8_t desiredMood; // -100 for angry, 100 for sad
  int8_t desiredCross;

public:
  const char * name() {
    return "ExteriorDisplayManager";
  }
  ExteriorDisplayManager() :  eyePanel(),
  leftText(&daisyFont), rightText(&daisyFont), whiteLayer(white),
  ticks(0), currentX(0), currentY(0), desiredX(0), desiredY(0),
  desiredBlink(0), desiredSquint(0), desiredMood(0),
  desiredCross(0) {}

  void setup() {
    static char *a = "D4";
    static char *b = "1Z";
    leftText.setText(a);
    leftText.setTexture(&whiteLayer);
    rightText.setText(b);
    rightText.setTexture(&whiteLayer);
    desiredMood = -70;
    FastLED.addLeds<LED_TYPE,pinEyeData,pinEyeClock,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    eyePanel.left.addLayer(&leftText);
    eyePanel.right.addLayer(&rightText);
  }
  void render() {
    eyePanel.render();
    for (uint8_t y = 0; y < 8; y++) {
      for (uint8_t x = 0; x < 8; x++)
      {
        leds[x + 8 * y] = eyePanel.right.buffer.valueAtIndex(x, 7 - y);
        leds[x + 8 * y + 64] = eyePanel.left.buffer.valueAtIndex(7 - x,y);
      }
    }
    FastLED.show();
  }
  void processImpulse(Datagram<Impulse> action) {
    switch (action.type)
    {

    case ImpulseEyeMovement:
      desiredX = action.x;
      desiredY = action.y;
      break;

    case ImpulseBlink:
      eyePanel.squint = action.value;
      break;

    case ImpulseMood:
      desiredMood = action.value;
      break;

    case ImpulseCross:
      desiredCross = action.value;
      break;

    case ImpulseSquint:
      desiredSquint = action.value;
      break;

    case ImpulseText:
      leftText.setText(action.text);
      rightText.setText(action.text);
      leftText.addOffset(-16);
      rightText.addOffset(-8);

    default:
      break;
    }
  }

  void applyTick() {
    ticks++;

    if (ticks & 0x01) {
      leftText.addOffset(1);
      rightText.addOffset(1);
    }
    currentX = (7 * currentX + desiredX) / 8;
    currentY = (7 * currentY + desiredY) / 8;
    eyePanel.setFocus(currentX, currentY);
    eyePanel.squint = (7 * eyePanel.squint + desiredSquint) / 8;
    eyePanel.mood = (7 * eyePanel.mood + desiredMood) / 8;
    eyePanel.cross = (7 * eyePanel.cross + desiredCross) / 8;
  };

  void loop() {
    desiredCross = 10;
    desiredSquint = 0;
    
    while (!xQueueIsQueueEmptyFromISR(impulseList.queue)) {
      Datagram<Impulse> action = impulseList.receive();
      processImpulse(action);
    }
    applyTick();
    render();
    vTaskDelay(pdMS_TO_TICKS(200));
  }
};

ExteriorDisplayManager exteriorDisplayManager;
