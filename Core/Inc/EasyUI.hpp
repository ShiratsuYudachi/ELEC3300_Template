#ifndef __EASYUI_HPP__
#define __EASYUI_HPP__
extern "C"
{
#include "lcdtp.h"
#include "xpt2046.h"
}
#include <cstring>
#include <stdint.h>
#include "utils.hpp"

#define TEXT_CHAR_NUM 16
#define MAX_UI_ELEMENTS 32

class UIElement;
class Screen;

extern Screen *allScreens[MAX_UI_ELEMENTS];
extern uint8_t screenNum;

class Screen
{
private:
    // This class is used to manage the UI elements. Each screen contains a list of UI elements, and the screen will render and update all the elements inside it.
    // When one screen is active, it will render and update all the elements inside it.
    // When the screen is not active, it will not render or update the elements inside it.
public:
    static Screen* activeScreen;
    UIElement *elements[MAX_UI_ELEMENTS];
    uint8_t elementNum = 0;

    void (*onUpdate)() = nullptr;


    Screen(void (*onUpdate)() = nullptr) : onUpdate(onUpdate)
    {
        allScreens[screenNum++] = this;
        elementNum = 0;
    }

    void renderAll(); // See the implementation below
    void updateAll(); // See the implementation in EasyUI.cpp
    void setActive(){
        // LCD_Clear(0,0,240,320);
        activeScreen = this;
        // this->renderAll();
    }
};



class UIElement
{
public:
    static UIElement *allElements[MAX_UI_ELEMENTS];
    static uint8_t elementNum;

    virtual void render() = 0;
    virtual void update(uint16_t x, uint16_t y) = 0;

    static void updateAllElements(); 
    // DO NOT USE THIS FUNCTION WHEN USING MULTIPLE SCREENS

    bool isInvalidInput(uint16_t x, uint16_t y)
    {
        if (y > 500 || y <= 32)
            return true;
        else
            return false;
    }

    uint16_t x, y;
    uint16_t width, height;

protected:
    UIElement(Screen *screen, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
    {
        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;
        allElements[elementNum++] = this;
        screen->elements[screen->elementNum++] = this;
    }
    bool checkTouch(uint16_t x, uint16_t y)
    {
        return x >= this->x && x <= this->x + width && y >= this->y && y <= this->y + height;
    }
};

class Button : public UIElement
{
private:
    uint16_t last_color = 0;
    uint16_t initialColor;
    uint16_t color;
    uint16_t textColor;
    char text[TEXT_CHAR_NUM];

public:
    bool isPressed = false;
    void (*onPressed)() = nullptr;
    void (*whilePressing)() = nullptr;
    void (*onReleased)() = nullptr;

    Button(Screen *screen, uint16_t x, uint16_t y, char text[TEXT_CHAR_NUM], uint16_t width = 85, uint16_t height = 50, uint16_t color = CYAN, uint16_t textColor = BLACK)
        : UIElement(screen, x, y, width, height)
    {
        this->initialColor = color;
        this->color = color;
        this->textColor = textColor;
        strcpy(this->text, text);
    }

    void setOnPressed(void (*onPressed)())
    {
        this->onPressed = onPressed;
    }

    void setWhilePressing(void (*whilePressing)())
    {
        this->whilePressing = whilePressing;
    }

    void setOnReleased(void (*onReleased)())
    {
        this->onReleased = onReleased;
    }

    void setText(const char* text){
        strcpy(this->text, text);
    }

    void render() override
    {
        // render background
        // for (int i=y; height<y?i<y+height:i>y-height; height<y?i++:i--)
        // {
        //     LCD_DrawLine(x, i, x+width, i, color);
        // }
        LCD_OpenWindow(x, y, width, height);
        LCD_FillColor(width * height, color);

        // render text
        LCD_DrawString_Color(x + width / 7, y + height / 3, text, color, textColor);
    }

    void update(u_int16_t x, u_int16_t y) override
    {
        if (checkTouch(x, y))
        {
            color = YELLOW;
            // if (!isPressed)
            //{ // only trigger once
            isPressed = true;
            if (whilePressing)
                whilePressing();
            //}
        }
        else
        {
            color = initialColor;
            isPressed = false;
        }
        if (last_color != color)
        {
            if (isPressed && onPressed)
            {
                onPressed();
            }
            if (!isPressed && onReleased)
                onReleased();
            render();
            last_color = color;
        }
    }
};

class Slider : public UIElement
{
private:
    uint16_t barColor;
    float value;
    uint16_t maxValue;

    uint16_t draggerRadius = 20;
    bool isDragging = false;

    uint16_t draggerX;
    uint16_t draggerY;

public:
    Slider(
        Screen *screen, 
        uint16_t x,
        uint16_t y,
        uint16_t maxValue = 0,
        uint16_t width = 10,
        uint16_t height = 130,
        uint16_t barColor = CYAN) : UIElement(screen, x, y, width, height)
    {
        this->maxValue = maxValue;
        this->barColor = barColor;
        draggerX = x + width / 2;
        draggerY = y + height / 2;
    }

    void render() override
    {
        // render bar
        LCD_OpenWindow(x, y, width, height);
        LCD_FillColor(width * height, barColor);

        // render dragger
        LCD_OpenWindow(x, draggerY, width, draggerRadius);
        LCD_FillColor(width * draggerRadius, RED);
    }

    uint16_t wrapY(u_int16_t y)
    {
        if (y > 500 || y <= 32)
            return draggerY; // y=2048 if not touched
        if (y < this->y)
            return this->y;
        if (y > this->y + height - draggerRadius)
            return this->y + height - draggerRadius;
        return y;
    }

    void update(u_int16_t x, u_int16_t y) override
    {
        bool isDraggerTouched;
        if (isDragging)
        {
            LCD_OpenWindow(this->x, draggerY, width, draggerRadius);
            LCD_FillColor(width * draggerRadius, CYAN);
            draggerY = wrapY(y);
            LCD_OpenWindow(this->x, draggerY, width, draggerRadius);
            LCD_FillColor(width * draggerRadius, RED);
            isDraggerTouched = x >= draggerX - 6 * draggerRadius && x <= draggerX + 6 * draggerRadius && y >= draggerY - 6 * draggerRadius && y <= draggerY + 6 * draggerRadius;
        }
        else
        {
            isDraggerTouched = x >= draggerX - draggerRadius && x <= draggerX + draggerRadius && y >= draggerY - draggerRadius && y <= draggerY + draggerRadius;
        }
        isDragging = isDraggerTouched;
        value = (draggerY - this->y) * maxValue / height;
    }
    float getValue()
    {
        return value / maxValue;
    }
};

class TouchPad : public UIElement
{
private:
    uint16_t color;
    uint16_t dotRadius = 10;

public:
    uint16_t dotX;
    uint16_t dotY;
    uint16_t lastDotX = 0;
    uint16_t lastDotY = 0;
    void (*onPressed)(TouchPad *, int, int) = nullptr;

    TouchPad(Screen *screen, uint16_t x, uint16_t y, void (*onPressed)(TouchPad *, int, int) = nullptr, uint16_t width = 150, uint16_t height = 150, uint16_t color = CYAN)
        : UIElement(screen, x, y, width, height)
    {
        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;
        this->color = color;
        dotX = width / 2;
        dotY = height / 2;
        this->onPressed = onPressed;
    }

    void render() override
    {
        // render background
        LCD_OpenWindow(x, y, width, height);
        LCD_FillColor(width * height, color);

        // render dot
        renderDot();
    }

    void clearDot()
    {
        LCD_OpenWindow(x + dotX, y + dotY, dotRadius, dotRadius);
        LCD_FillColor(dotRadius * dotRadius, color);
    }

    void renderDot()
    {
        LCD_OpenWindow(x + dotX, y + dotY, dotRadius, dotRadius);
        LCD_FillColor(dotRadius * dotRadius, RED);
    }

    uint16_t wrapX(u_int16_t x)
    {
        if (x > 500)
            return dotX; // x=2048 if not touched
        if (x < this->x)
            return this->x;
        if (x > this->x + width - dotRadius)
            return this->x + width - dotRadius;
        return x;
    }

    u_int16_t wrapY(u_int16_t y)
    {
        if (y > 500)
            return dotY; // y=2048 if not touched
        if (y < this->y)
            return this->y;
        if (y > this->y + height - dotRadius)
            return this->y + height - dotRadius;
        return y;
    }

    void update(u_int16_t x, u_int16_t y) override
    {
        // if the touchpad is not touched, do nothing
        if (x > 500 || y > 500)
            return;
        if (x < this->x || x > this->x + width || y < this->y || y > this->y + height)
            return;

        int _dotX = wrapX(x) - this->x;
        int _dotY = wrapY(y) - this->y;
        if (lastDotX != _dotX || lastDotY != _dotY)
        {
            clearDot();
            dotX = _dotX;
            dotY = _dotY;
            renderDot();
            lastDotX = dotX;
            lastDotY = dotY;
        } // render only when the coordinate changes

        // execute the function
        if (onPressed)
            onPressed(this, dotX, dotY);
    }

    float getXRatio()
    {
        return (float)dotX / width;
    }

    float getYRatio()
    {
        return (float)dotY / height;
    }
};
 
class Joystick : public UIElement
{
private:
    uint16_t color;
    bool isDragging = false;

public:
    uint16_t dotRadius = 20;
    int deadzoneSideLength = 40; 

    bool performanceMode = false; // disable dragging animation

    uint16_t dotX;
    uint16_t dotY;
    uint16_t lastDotX = 0;
    uint16_t lastDotY = 0;
    void (*whilePressing)() = nullptr;
    void (*onPressed)() = nullptr;

    uint32_t lastTick = 0;

    Joystick(Screen *screen,uint16_t x, uint16_t y, uint16_t width = 150, uint16_t height = 150, uint16_t color = CYAN, void (*whilePressing)() = nullptr)
        : UIElement(screen, x, y, width, height)
    {
        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;
        this->color = color;
        dotX = getInitialDotX();
        dotY = getInitialDotY();
        this->whilePressing = whilePressing;
    }

    void render() override
    {
        // render background
        LCD_OpenWindow(x, y, width, height);
        LCD_FillColor(width * height, color);

        // render dot
        renderDot();

        renderDeadZone();
    }

    void clearDot()
    {
        LCD_OpenWindow(x + dotX, y + dotY, dotRadius, dotRadius);
        LCD_FillColor(dotRadius * dotRadius, color);
    }

    void renderDot()
    {
        LCD_OpenWindow(x + dotX, y + dotY, dotRadius, dotRadius);
        LCD_FillColor(dotRadius * dotRadius, RED);
    }
    void renderDeadZone()
    {
        int centerX = getInitialDotX() + dotRadius/2;
        int centerY = getInitialDotY() + dotRadius/2;
        LCD_DrawLine(x + centerX - deadzoneSideLength/2, y + centerY - deadzoneSideLength/2, x + centerX + deadzoneSideLength/2, y + centerY - deadzoneSideLength/2, RED);
        LCD_DrawLine(x + centerX - deadzoneSideLength/2, y + centerY - deadzoneSideLength/2, x + centerX - deadzoneSideLength/2, y + centerY + deadzoneSideLength/2, RED);
        LCD_DrawLine(x + centerX + deadzoneSideLength/2, y + centerY + deadzoneSideLength/2, x + centerX - deadzoneSideLength/2, y + centerY + deadzoneSideLength/2, RED);
        LCD_DrawLine(x + centerX + deadzoneSideLength/2, y + centerY + deadzoneSideLength/2, x + centerX + deadzoneSideLength/2, y + centerY - deadzoneSideLength/2, RED);
    }
    bool insideDeadZone_X(){
        return dotX > getInitialDotX() - deadzoneSideLength/2 && dotX < getInitialDotX() + deadzoneSideLength/2;
    }
    bool insideDeadZone_Y(){
        return dotY > getInitialDotY() - deadzoneSideLength/2 && dotY < getInitialDotY() + deadzoneSideLength/2;
    }

    uint16_t wrapX(u_int16_t x)
    {
        if (x > 500)
            return dotX; // x=2048 if not touched
        if (x < this->x)
            return this->x;
        if (x > this->x + width - dotRadius)
            return this->x + width - dotRadius;
        return x;
    }

    u_int16_t wrapY(u_int16_t y)
    {
        if (y > 500)
            return dotY; // y=2048 if not touched
        if (y < this->y)
            return this->y;
        if (y > this->y + height - dotRadius)
            return this->y + height - dotRadius;
        return y;
    }

    uint16_t getInitialDotX()
    {
        return width / 2 - dotRadius/2;
    }

    uint16_t getInitialDotY()
    {
        return height / 2 - dotRadius/2;
    }


    void update(u_int16_t x, u_int16_t y) override
    {
        // if the touchpad is not touched, do nothing
        if (isDragging && isInvalidInput(x, y)){
            clearDot();
            dotX = getInitialDotX();
            dotY = getInitialDotY();
            renderDot();
            renderDeadZone();
            isDragging = false;
            lastTick = HAL_GetTick();
            return;
        }
        if (x < this->x || x > this->x + width || y < this->y || y > this->y + height){
            lastTick = HAL_GetTick();
            return;
        }

        if (!isDragging){
            if (onPressed) onPressed();
            if (performanceMode){
                clearDot();
            }
        }
        
        isDragging = true;

        if (performanceMode){
            dotX = wrapX(x) - this->x;
            dotY = wrapY(y) - this->y;
        }else{
            int _dotX = wrapX(x) - this->x;
            int _dotY = wrapY(y) - this->y;

            if (lastDotX != _dotX || lastDotY != _dotY)
            {
                clearDot();
                dotX = _dotX;
                dotY = _dotY;
                renderDot();
                lastDotX = dotX;
                lastDotY = dotY;
            } // render only when the coordinate changes
        }
        // execute the function
        if (whilePressing)
            whilePressing();
        lastTick = HAL_GetTick();
    }

    float get_dX()
    {
        if (insideDeadZone_X())
            return 0;
        return (float)dotX / width - 0.5;
    }

    float get_dX_dt()
    {
        float dTime = (HAL_GetTick() - lastTick) / 1000.0;
        return get_dX() * dTime;
    }

    float get_dY()
    {
        if (insideDeadZone_Y())
            return 0;
        return (float)dotY / height - 0.5;
    }

    float get_dY_dt()
    {
        float dTime = (HAL_GetTick() - lastTick) / 1000.0;
        return get_dY() * dTime;
    }
};





#endif