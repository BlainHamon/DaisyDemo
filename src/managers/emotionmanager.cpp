#include "common.hpp"
#include "impulse.hpp"
#include "controllerState.hpp"

typedef enum
{
  EmotionEnumEmbarrased = 0,
  EmotionEnumSad,
  EmotionEnumConfused,
  EmotionEnumSleepy,
  EmotionEnumHappy,
  EmotionEnumSuspicious,
  EmotionEnumAngry,
  EmotionEnumDead,
  EmotionEnumTotalEnums,
} EmotionEnum;

void doBlink(void * unused) {
  while (1) {
    Datagram<Impulse> result;
    result.type = ImpulseBlink;
    result.value = 90;
    impulseList.send(result, pdMS_TO_TICKS(60));
    vTaskDelay(pdMS_TO_TICKS(100000));
  }
}

class EmotionManager : public Manager {
  EmotionEnum proposedMood;
public:
  EmotionManager(){};
  void setup(){
    xTaskCreate(
      &doBlink,
      "Blink",
      CONFIG_ESP32_PTHREAD_TASK_STACK_SIZE_DEFAULT,
      NULL,
      1,
      NULL
    );
  };
  void loop();
  void adjustMood(int8_t delta) {
    proposedMood = static_cast<EmotionEnum>((proposedMood + delta + EmotionEnumTotalEnums) % EmotionEnumTotalEnums);
  }
  void applyMood() {
    switch (proposedMood) {

    }
  }
  inline ~EmotionManager(){};
};

TypedQueue<Datagram<Impulse>> impulseList(15);

#define RATIO_ADJUST(foo, bar, ratio)  foo = (foo * (100-ratio) + bar * ratio) / 100

void EmotionManager::loop()
{
  ControllerState controls = controllerState.receive();
  Datagram<Impulse> result;
  result.type = ImpulseEyeMovement;
  result.x = 0;
  result.y = 0;
  switch (controls.currentButtons & ControllerJoyVert)
  {
  case ControllerJoyUp:
    result.y = 100;
    break;

  case ControllerJoyDown:
    result.y = -100;
    break;
  }

  switch (controls.currentButtons & ControllerJoyHori)
  {
  case ControllerJoyLeft:
    result.x = -100;
    break;

  case ControllerJoyRight:
    result.x = 100;
    break;
  }
  impulseList.send(result, pdMS_TO_TICKS(60));

  ControllerButtons newDowns = static_cast<ControllerButtons>
    (controls.currentButtons & controls.changedButtons);

  if (newDowns & ControllerButtonA ) {
    result.type = ImpulseBlink;
    result.value = 90;
    impulseList.send(result, pdMS_TO_TICKS(60));
  }

  if (newDowns & ControllerButtonB) {
    result.type = ImpulseText;
    result.text = "Hello, world!";
    impulseList.send(result, pdMS_TO_TICKS(60));
  }

  if (newDowns & ControllerButtonB) {
    result.type = ImpulseText;
    result.text = "Hello, world!";
    impulseList.send(result, pdMS_TO_TICKS(60));
  }

  // RATIO_ADJUST(currentEyeState.leftEye.pupilX, deltaX, 20);
  // RATIO_ADJUST(currentEyeState.leftEye.pupilX, deltaX, 20);
  // RATIO_ADJUST(currentEyeState.rightEye.pupilY, deltaY, 20);
  // RATIO_ADJUST(currentEyeState.rightEye.pupilY, deltaY, 20);
  // setEyeState(currentEyeState);
}

 EmotionManager emotionManager;
