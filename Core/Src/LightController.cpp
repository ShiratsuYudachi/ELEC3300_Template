#include "LightController.hpp"
extern "C"{
  #include "main.h"
}
void playStartAnimation(){
  // LightStream stream1(0, 84, 3, 3, RGB(10, 0, 0));
  // stream1.start();
  // while (!stream1.isFinished()){
  //   stream1.setToColor();
  // }
  
  int current = 0;
  int length = 10;

  RGB color = RGB(0, 255, 0);

  while (current < LED_NUM){
    auto max = [](int a, int b){return a > b ? a : b;};
    auto min = [](int a, int b){return a < b ? a : b;};
    for (int i = current; i < min(current + length, LED_NUM); i++){
      setColor(i, color);
    }
    if (current > 0){
      setColor(current - 1, 0, 0, 0);
    }
    
    for (int i = LED_NUM - current - 1; i > max(LED_NUM - current - length, 0); i--){
      setColor(i, color);
    }
    if (current > 0){
      setColor(LED_NUM - current, 0, 0, 0);
    }
    HAL_Delay(12);
    current++;
  }
}


struct Counter{
  bool enable = false;
  int count = 0;
};


void updateBreathAnimation(int period, RGB color){
    // period = 3s
    static int startTick = 0;

    if (startTick == 0){
      startTick = HAL_GetTick();
    }
    int tick = HAL_GetTick();
    float progress = (tick - startTick)%period / (float)period;
    progress = progress < 0.01 ? 0.01 : progress;
    for (int i = 0; i < LED_NUM; i++){
      if (progress < 0.5){
        
        setColor(i, progress * color.red, progress * color.green, progress * color.blue);
      } else {
        
        setColor(i, color.red/2 - (progress-0.5)*color.red, color.green/2 - (progress-0.5)*color.green, color.blue/2 - (progress-0.5)*color.blue);
      }
    }
}


// dir: 1 for increasing index, 0 for decreasing index
int length = 7;
RGB bodyRGB = RGB(255, 0, 0, 0.3);
RGB headRGB = RGB(255, 140, 0, 0.3);
void updateLightStream(int start, int end, bool isDirIncresingIndex, int& current){
    // if (count < interval){
    //     count++;
    //     return;
    // }
    // count = 0;
    auto max = [](int a, int b){return a > b ? a : b;};
    auto min = [](int a, int b){return a < b ? a : b;};
    if (isDirIncresingIndex){
        for (int i = current; i < min(current + length, end); i++){
            if (i>=0 && i < LED_NUM)
            setColor(i, i==min(current + length, end)-1 ? headRGB : bodyRGB);
        }
        if (current > 0){
            if (current - 1 >= 0 && current - 1 < LED_NUM)
                setColor(current - 1, 0, 0, 0);
        }
    }else{
        for (int i = end - current - 1 + start; i > max(end - current - length + start, start); i--){
            if (i>= 0 && i < LED_NUM)
                setColor(i, i == max(end - current - length + start, start) +1 ? headRGB : bodyRGB);
        }
        if (current > 0){
            if (end - current + start >= 0 && end - current + start < LED_NUM)
                setColor(end - current + start, 0, 0, 0);
        }
    }
    
    current++;
    if (current > end){
        current = start;
    }
}


int current_l = 0 - 1;
int current_l_2 = 20 -1;

int current_mid = 30;
int current_mid_2 = 48;

int current_r = 54 + length ;
int current_r_2 = 72 + length;

void updateResettingAnimation(){
    HAL_Delay(50);
    if (isResetComplete_Y){
      for (int i = 0; i < 30; i++){
        setColor(i, 0, 255, 0);
      }
      for (int i = 54; i < LED_NUM; i++){
        setColor(i, 0, 255, 0);
      }
    }else{
      updateLightStream(0 - length, 30, 1, current_l);// 实现从0冒出来，而不是直接0～5生成一整条
      updateLightStream(0 - length, 30, 1, current_l_2);
      updateLightStream(54-1, LED_NUM+length-1, 0, current_r);
      updateLightStream(54-1, LED_NUM+length-1, 0, current_r_2);
    }

    if (isResetComplete_X){
      for (int i = 30; i < 54; i++){
        setColor(i, 0, 255, 0);
      }
      
    }else{
      updateLightStream(30 - length, 54 + length, 0, current_mid);
      updateLightStream(30- length, 54 + length, 0, current_mid_2);
      
    }

}

STATUS lightStatus = COMPLETE;
bool isResetComplete_X = false;
bool isResetComplete_Y = false;


Counter completeCounter;
void updateLightEffect(){
    switch (lightStatus){
        case STANDBY:
            updateBreathAnimation(3000, RGB(0, 255, 0));
            break;
        case OPERATING:
            updateBreathAnimation(1500, RGB(0, 80, 128));
            break;
        case WARNING:
          break;
        case FATAL:
          updateBreathAnimation(500, RGB(255, 0, 0));
          break;

        case RESETTING:
          // blankAll();
          updateResettingAnimation();
          break;
        case COMPLETE:
          const int blinkCount = 3;
          if (completeCounter.enable == false){
            completeCounter.enable = true;
            completeCounter.count = 0;
          }
          if (completeCounter.count < 100){
            completeCounter.count++;
          }else{
            completeCounter.enable = false;
            lightStatus = STANDBY;
          }
          for (int i = 0; i < LED_NUM; i++){
            if (completeCounter.count % 20 < 10){
              setColor(i, 0, 80, 0);
            }else{
              setColor(i, 0, 0, 0);
            }
          }
          break;
    }
};