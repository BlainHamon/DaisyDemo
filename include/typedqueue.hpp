#ifndef __typedqueue_hpp
#define __typedqueue_hpp

#include <Arduino.h>
#include <FreeRTOS.h>

template <class T>
class TypedQueue {
public:
  QueueHandle_t queue;
  uint8_t length;
  TypedQueue(uint8_t l = 1) : length(l) {
    queue = xQueueCreate(l, sizeof(T));
  }
  T receive(portTickType duration = portMAX_DELAY) {
    T result;
    xQueueReceive(queue, &result, duration);
    return result;
  };
  uint8_t recievedTo(T &result, portTickType duration = 0) {
    return xQueueReceive(queue, &result, duration);
  }
  void send(T t, portTickType duration = portMAX_DELAY){
    xQueueSend(queue, &t, duration);
  };
  T peek(portTickType duration = 0) {
    T result;
    xQueuePeek(queue, &result, duration);
    return result;
  };
  void poke(T t) {
    if (length == 1) {
      xQueueOverwrite(queue, &t);
    } else {
      send(t, 0);
    }
  };
};

#endif