#include "WS2812.hpp"

#include "string.h"
#include "tim.h"



bool isInited = false;

void RGB::toBitSequence(uint32_t *bitSequence) const
{
    for (int i = 0; i < 8; i++)
    {
        // Total counter for timer is 5 with period around 1us
        // hence 4/5 will be recognized as 1, 1/5 will be recognized as 0 (4/5 and 1/5 for duty cycle)
        // check manuals of ws2812 for more details
        bitSequence[i]      = (green & (1 << (7 - i))) ? 4 : 1;
        bitSequence[i + 8]  = (red & (1 << (7 - i))) ? 4 : 1;
        bitSequence[i + 16] = (blue & (1 << (7 - i))) ? 4 : 1;
    }
}

void init() { HAL_TIM_PWM_Start_DMA(&WS2812_TIM, WS2812_TIM_CHANNEL, CCRDMABuff, LED_NUM * 24 + RESET_COUNT); }


void setColor(int index, unsigned char r, unsigned char g, unsigned char b)
{
    if (!isInited)
    {
        init();
        isInited = true;
    }
    RGB rgb(r, g, b);
    rgb.toBitSequence(CCRDMABuff + index * 24);
}
void setColor(int index, RGB color)
{
    if (!isInited)
    {
        init();
        isInited = true;
    }
    color.toBitSequence(CCRDMABuff + index * 24);
}

void blank(int index)
{
    for (int i = 0; i < 24; i++)
    {
        RGB rgb(0, 0, 0);
        rgb.toBitSequence(CCRDMABuff + index * 24);
    }
}

void blankAll()
{
    if (!isInited)
    {
        init();
        isInited = true;
    }
    for (int i = 0; i < LED_NUM; i++)
    {
        blank(i);
    }
}




uint32_t CCRDMABuff[24 * LED_NUM + RESET_COUNT] = {};
