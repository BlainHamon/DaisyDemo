#include "common.hpp"
#include <FreeRTOS.h>

int led = 13;
int errorNumber = 0;

void blinker(void * unused) {
  TickType_t xLastWakeTime;
  const TickType_t xDelay = pdMS_TO_TICKS(5000);
  const TickType_t xFlash = pdMS_TO_TICKS(500);
  xLastWakeTime = xTaskGetTickCount();

  for (;;) {
    digitalWrite(led, HIGH); // turn the LED on (HIGH is the voltage level)
    for (int i = 0; i < errorNumber; i++)
    {
      digitalWrite(led, HIGH); // turn the LED on (HIGH is the voltage level)
      vTaskDelayUntil( &xLastWakeTime, xFlash);
      digitalWrite(led, LOW); // turn the LED off
      vTaskDelayUntil( &xLastWakeTime, xFlash);
    }
    vTaskDelayUntil( &xLastWakeTime, xDelay);
    digitalWrite(led, LOW); // turn the LED off
    vTaskDelayUntil( &xLastWakeTime, xDelay);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  setupManagers();
  xTaskCreate(&blinker, "blinker", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
}

void loop() {
  taskYIELD();
}

/*
So now what? We should probably have several processes running independantly
  Bluetooth uplink?
  Wifi uplink?
  Messaging/threadsafe hub
  Mood controller
  Eye control
  Screen control
  LED control

  Text screen

  Text manager with 80*24 screen array? pipes to serial and screen

  [Input manager]
  |               +->[accessories]->[               ]
  v               |                 [               ]
[Emotion Manager]-+->[Eye manager]->[LEDGroupManager]
                  |
                  |->[Log Manager]----->[                 ]
                  |                     [ display manager ]
                  |->[Graphic Manager]->[                 ]

<->[network manager]

So how are these pipes set up? Are they general purpose?
*/
