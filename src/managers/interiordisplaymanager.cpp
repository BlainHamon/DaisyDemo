#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <SSD1306_Shim.h>

#include <layer.h>

#include "common.hpp"
#include "impulse.hpp"
#include <FreeRTOS.h>

#include <daisyfont.h>

TypedQueue<uint8_t *> bufferQueue(1);
void outputScreen(void * ptr);

class InteriorDisplayManager : public Manager {
  friend void outputScreen(void * ptr);
  SSD1306_Shim display;
  uint8_t * buffer0;
  uint8_t * buffer1;
public:
  const char * name() {
    return "InteriorDisplayManager";
  }
  InteriorDisplayManager() : display(128, 32, &Wire, 14){};
  void setup() {
    uint32_t size = display.bufferSize();
    buffer0 = (uint8_t *)malloc(size);
    buffer1 = (uint8_t *)malloc(size);
    display.setBuffer(buffer0);
    //Because the output has to happen on the main thread, we spawn a second task for the actual display
    xTaskCreatePinnedToCore(&outputScreen, "Main core OLED",
      CONFIG_ESP32_PTHREAD_TASK_STACK_SIZE_DEFAULT,
      this,  ( 2 | portPRIVILEGE_BIT ), NULL, xPortGetCoreID());

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    }
    // put your setup code here, to run once:
  }

  void output() { //By doing it this way, the rendered buffer is always the one not being worked on
    uint8_t *outbound = display.getBuffer();
    bufferQueue.send(outbound);
    if (outbound == buffer0) {
      display.setBuffer(buffer1);
      memcpy(buffer1, buffer0, display.bufferSize());
    } else {
      display.setBuffer(buffer0);
      memcpy(buffer0, buffer1, display.bufferSize());
    }
  }

  void writeToLine(const uint8_t row, const char * text) {
    uint16_t index = row * 128;

  }

  void loop() {
    for(;;) { // Loop forever...

      display.clearDisplay(); // Clear the display buffer

      uint8_t *buffer = display.getBuffer();
      uint16_t col = daisyFont.start(' ');
      for (uint16_t i = 0; i < 512; i++)
      {
        buffer[i] = daisyFont.columnAt(col + i);
      }
      output(); // Show the display buffer on the screen
      vTaskDelay(pdMS_TO_TICKS(200));        // Pause for 1/10 second
    }
  }
};

InteriorDisplayManager interiorDisplayManager;

void outputScreen(void * ptr) {
  InteriorDisplayManager *manager = static_cast<InteriorDisplayManager*>(ptr);
  while(1) {
    //This buffer will be the opposite of the one being used for drawing.
    uint8_t *buffer = bufferQueue.receive();
    //So we can display it without fear of modification.
    manager->display.display(buffer);
  }
}
