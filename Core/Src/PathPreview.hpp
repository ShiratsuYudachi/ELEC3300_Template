#include <cstdint>
#include "lcdtp.h"
#include "trigo.h"
#include "EasyUI.hpp"
#include <cstdlib>
#include <cmath>



// 实现 sin 函数，考虑周期性和符号
float fastsin(int degree){
    degree = (degree % 360 + 360) % 360; // 首先将角度规约到 0-359 度
    if (degree < 90) {
        return sinTable[degree];
    } else if (degree < 180) {
        return sinTable[179 - degree];  // 90到179度
    } else if (degree < 270) {
        return -sinTable[degree - 180];  // 180到269度
    } else {
        return -sinTable[359 - degree];  // 270到359度
    }
}

// 实现 cos 函数，使用 sin 函数
float fastcos(int degree){
    return fastsin(degree + 90);  // 直接利用 sin 函数实现
}

struct Point3D {
    float x, y, z;
};

enum Axis {
    X, Y, Z
};

float sinUnderCurrentAngleX = 0;
float cosUnderCurrentAngleX = 0;
float sinUnderCurrentAngleY = 0;
float cosUnderCurrentAngleY = 0;
float sinUnderCurrentAngleZ = 0;
float cosUnderCurrentAngleZ = 0;

Point3D rotatePoint(const Point3D &point, Axis axis) {
    Point3D newPoint;
    switch (axis) {
        case X:
            newPoint.x = point.x;
            newPoint.y = point.y * cosUnderCurrentAngleX - point.z * sinUnderCurrentAngleX;
            newPoint.z = point.y * sinUnderCurrentAngleX + point.z * cosUnderCurrentAngleX;
            break;
        case Y:
            newPoint.x = point.x * cosUnderCurrentAngleY + point.z * sinUnderCurrentAngleY;
            newPoint.y = point.y;
            newPoint.z = -point.x * sinUnderCurrentAngleY + point.z * cosUnderCurrentAngleY;
            break;
        case Z:
            newPoint.x = point.x * cosUnderCurrentAngleZ - point.y * sinUnderCurrentAngleZ;
            newPoint.y = point.x * sinUnderCurrentAngleZ + point.y * cosUnderCurrentAngleZ;
            newPoint.z = point.z;
            break;
    }
    return newPoint;
}


float rotateAngleX = 0;
float rotateAngleY = 0;
float rotateAngleZ = 0;

const int r = 150;
uint8_t vRAM[r*r*2] = {};
void setPixel(int x, int y, uint16_t color, float alpha) {
    if (x < 0 || x >= r || y < 0 || y >= r) return;
    
    uint16_t bg_color = ((uint16_t*)vRAM)[x+y*r];
    uint8_t bg_red   = (bg_color >> 11) & 0x1F;
    uint8_t bg_green = (bg_color >> 5) & 0x3F;
    uint8_t bg_blue  = bg_color & 0x1F;

    uint8_t fg_red   = (color >> 11) & 0x1F;
    uint8_t fg_green = (color >> 5) & 0x3F;
    uint8_t fg_blue  = color & 0x1F;

    uint8_t red   = (uint8_t)(bg_red * (1 - alpha) + fg_red * alpha);
    uint8_t green = (uint8_t)(bg_green * (1 - alpha) + fg_green * alpha);
    uint8_t blue  = (uint8_t)(bg_blue * (1 - alpha) + fg_blue * alpha);

    ((uint16_t*)vRAM)[x+y*r] = (red << 11) | (green << 5) | blue;
}


void drawLineToVRAM(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;  // error value e_xy

    while (true) {
        setPixel(x0, y0, color, 1.0);  // Set the pixel with full color intensity
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) {  // e_xy + e_x > 0
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {  // e_xy + e_y < 0
            err += dx;
            y0 += sy;
        }
    }
}


// anti-aliased
void drawLineToVRAM_AA(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    bool steep = fabs(y1 - y0) > fabs(x1 - x0);
    if (steep) {
        uint16_t temp = x0;
        x0 = y0;
        y0 = temp;

        temp = x1;
        x1 = y1;
        y1 = temp;
    }
    if (x0 > x1) {
        uint16_t temp = x0;
        x0 = x1;
        x1 = temp;

        temp = y0;
        y0 = y1;
        y1 = temp;
    }

    float dx = x1 - x0;
    float dy = y1 - y0;
    float gradient = dy / dx;
    if (dx == 0.0) {
        gradient = 1;
    }

    float intery = y0 + gradient; // 初始交点的y坐标

    for (int x = x0 + 1; x <= x1; x++) {
        if (steep) {
            setPixel((int)intery, x, color, 1 - (intery - floor(intery)));
            setPixel((int)intery + 1, x, color, intery - floor(intery));
        } else {
            setPixel(x, (int)intery, color, 1 - (intery - floor(intery)));
            setPixel(x, (int)intery + 1, color, intery - floor(intery));
        }
        intery += gradient;
    }
}

void resetVRAM(){
    //memset(vRAM, 0, sizeof(vRAM));
    uint32_t color_32 = (CYAN << 16) | CYAN;
    for(int i = 0; i < r*r/2; i++)
    {
        ((uint32_t*)vRAM)[i] = color_32;
        i++;
        ((uint32_t*)vRAM)[i] = color_32;
        i++;
        ((uint32_t*)vRAM)[i] = color_32;
        i++;
        ((uint32_t*)vRAM)[i] = color_32;
        i++;
        ((uint32_t*)vRAM)[i] = color_32;
    }
}

void renderVRAM(int posX, int posY){
    LCD_OpenWindow(posX, posY, r, r);
    LCD_Write_Cmd ( CMD_SetPixel );	
    for(int i = 0; i < r*r; i++)
    {
        * ( __IO uint16_t * ) ( FSMC_Addr_LCD_DATA ) = ((uint16_t*)vRAM)[i];
        i++;
        * ( __IO uint16_t * ) ( FSMC_Addr_LCD_DATA ) = ((uint16_t*)vRAM)[i];
        i++;
        * ( __IO uint16_t * ) ( FSMC_Addr_LCD_DATA ) = ((uint16_t*)vRAM)[i];
        i++;
        * ( __IO uint16_t * ) ( FSMC_Addr_LCD_DATA ) = ((uint16_t*)vRAM)[i];
        i++;
        * ( __IO uint16_t * ) ( FSMC_Addr_LCD_DATA ) = ((uint16_t*)vRAM)[i];
    }
}


// const int r_bool = 150;
// bool VRAM_bool[r][r] = {};
// void drawLineToVRAM_bool(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2){
//     void drawLineToVRAM_bool(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2){
//     int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
//     int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1; 
//     int err = dx + dy, e2; /* error value e_xy */

//     while (true) {  // 循环直到到达终点
//         if (x1 >= 0 && x1 < r && y1 >= 0 && y1 < r) {
//             VRAM_bool[x1][y1] = true; // 标记当前点
//         }
//         if (x1 == x2 && y1 == y2) break;
//         e2 = 2 * err;
//         if (e2 >= dy) { err += dy; x1 += sx; } // e_xy+e_x > 0
//         if (e2 <= dx) { err += dx; y1 += sy; } // e_xy+e_y < 0
//     }
// }
// }


void drawReferenceLine(int posX, int posY){
    LCD_DrawLine(posX-100, posY, posX + 100, posY, RED);
    LCD_DrawLine(posX, posY, posX, posY + 100, RED);
    LCD_DrawLine(posX, posY, posX - 100, posY, RED);
    LCD_DrawLine(posX, posY, posX, posY - 100, RED);
}


int lastX = 0;
int lastY = 0;
int lastTick = 0;
class PreviewDisplay : public UIElement
{
private:
    uint16_t color;
    

public:
    float previewScale = 1.0;
    float xOffset = 0;
    float yOffset = 0;
    
    bool use3d = false;
    bool useAA = true; // anti-aliasing

    PreviewDisplay(Screen *screen, uint16_t x, uint16_t y, uint16_t color = CYAN)
        : UIElement(screen, x, y, width = r, height = r)
    {
        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;
        this->color = color;
    }

    void map2d(float posX, float posY, float speedInterval = -1)
    {
        bool enableDrawing = false;
        float lastPos[4] = {0, 0, 0};
        resetVRAM();

        for (float* cmd = (float*)targetGcode; cmd < (float*)targetGcode+targetGcodeLength*4; cmd+=4){
            float pos[3] = {cmd[0], cmd[1], cmd[2]};
            for (int i = 0; i < 3; i++){
                pos[i] *= previewScale;
            }
            pos[1] *= -1;
            pos[0] += xOffset;
            pos[1] += yOffset;
            enableDrawing = pos[2] <= 0.001;
            if (enableDrawing)
                if (useAA)
                    drawLineToVRAM_AA(lastPos[0], lastPos[1], pos[0], pos[1], RED);
                else
                    drawLineToVRAM(lastPos[0], lastPos[1], pos[0], pos[1], RED);
            for (int i = 0; i < 3; i++){
                lastPos[i] = pos[i];
            }
            if (speedInterval > 0){
                HAL_Delay(speedInterval);
            }
        }

        renderVRAM(posX, posY);
    }

    void map3d(int posX, int posY) {
        resetVRAM();
        Point3D lastPos = {0, 0, 0};

        int centerPosX = posX + r/2;
        int centerPosY = posY + r/2;
        int vCenterX = r/2; // vRAM center
        int vCenterY = r/2; // vRAM center

        // draw circle
        sinUnderCurrentAngleX = fastsin(rotateAngleX);
        cosUnderCurrentAngleX = fastcos(rotateAngleX);
        sinUnderCurrentAngleY = fastsin(rotateAngleY);
        cosUnderCurrentAngleY = fastcos(rotateAngleY);
        sinUnderCurrentAngleZ = fastsin(rotateAngleZ);
        cosUnderCurrentAngleZ = fastcos(rotateAngleZ);
        
        for (float* cmd = (float*)targetGcode; cmd < (float*)targetGcode+targetGcodeLength*4; cmd+=4){
            Point3D point = {cmd[0] - targetGcodeCenterOfMass[0], cmd[1] - targetGcodeCenterOfMass[1], cmd[2] - targetGcodeCenterOfMass[2]};
            point.x *= previewScale;
            point.y *= previewScale;
            point.z *= previewScale;
            point = rotatePoint(point, X);
            point = rotatePoint(point, Y);
            point = rotatePoint(point, Z);
            if (useAA)
                drawLineToVRAM_AA(vCenterX + lastPos.x, vCenterY - lastPos.z, vCenterX + point.x, vCenterY - point.z, RED);
            else
                drawLineToVRAM(vCenterX + lastPos.x, vCenterY - lastPos.z, vCenterX + point.x, vCenterY - point.z, RED);
            lastPos = point;
        }

        renderVRAM(posX, posY);
    }
    
    void render() override
    {
        if (use3d){
            map3d(x, y);
        }
        else{
            map2d(x, y);
        }
    }
    // void render2d() {
    //     // LCD_Clear(0,0,240,320);
    //     // operationScreen.renderAll();
    //     map2d(x, y);
    // }

    void update(uint16_t x, uint16_t y) override
    {
        return;   
    }


    
};