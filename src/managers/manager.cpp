#include <Arduino.h>
#include "common.hpp"
#include <FreeRTOS.h>

struct ManagerNode
{
  Manager *value;
  ManagerNode *next;
};

ManagerNode *startupList = 0;

extern "C"
void runManagerLoop(void * payload) {
  Manager *manager = static_cast<Manager *>(payload);
  for (;;)
  {
    manager->loop();
  }
}

Manager::Manager() {
  ManagerNode *last = new ManagerNode();
  last->value = this;
  last->next = startupList;
  startupList = last;
}

const char *unnamedManager = "UNNAMED MANAGER";
const char *Manager::name() {
  return unnamedManager;
}

void setupManagers() {
  ManagerNode *last = startupList;
  //First pass: run setups
  while ( last != 0 ) {
    last->value->setup();
    last = last->next;
  }
  //Second pass: start loops
  last = startupList;
  while (last != 0) {
    xTaskCreate(
      &runManagerLoop,
      last->value->name(),
      CONFIG_ESP32_PTHREAD_TASK_STACK_SIZE_DEFAULT,
      last->value,
      1,
      NULL
    );
    startupList = last->next;
    delete last;
    last = startupList;
  }
}

