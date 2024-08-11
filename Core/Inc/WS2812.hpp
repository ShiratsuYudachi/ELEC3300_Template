/**
 * @file not in buzzer.h
 * @brief A complex driver of the ws2812 LED strips driven by PWM
 * @version 0.1
 * @date 2023-10-4, 2024-3-30
 * @author Based on RM2024 tutorial template, 95% Assisted by baoqi, the rest 5% junk by Nico
 * @copyright Copyleft (d) 2023
 *
 */

#include <cstdint>
#pragma once

#define LED_NUM 84
#define WS2812_TIM htim8
#define WS2812_TIM_CHANNEL TIM_CHANNEL_1

#define RESET_COUNT 80
// if the LEDs is blinking during refresh or NEVER refresh, increase this

// NOTE:
// ioc need to be modified by yourself, note the following things:
// 1. For ABP1 Timer Clock = 84MHz, prescaler=40-1, CounterPeriod=5-1
// 2. Corresponding DMA should be enabled, and Mode=Circular, DataWidth=Word, Direction=MemoryToPeripheral


extern bool isInited;


/**
 * @brief The structure that contains the rgb value of a single ws2812 unit
 */
struct RGB
{
    uint8_t green;
    uint8_t red;
    uint8_t blue;

    RGB(uint8_t r, uint8_t g, uint8_t b) : red(r ), green(g ), blue(b ) {}
    RGB(uint8_t r, uint8_t g, uint8_t b, float luminance) : red(r * luminance), green(g * luminance), blue(b * luminance) {}

    void toBitSequence(uint32_t *bitSequence) const;
};

void setColor(int index, unsigned char r, unsigned char g, unsigned char b);
void setColor(int index, RGB color);

void blank(int index);

void blankAll();



extern uint32_t CCRDMABuff[LED_NUM * 24 + RESET_COUNT];
