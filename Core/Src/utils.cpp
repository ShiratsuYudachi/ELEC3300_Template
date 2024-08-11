#include "utils.hpp"
extern "C"{
#include "lcdtp.h"
}
String operator+(const char* s, const String& str){
    String newStr;
    newStr.len = strlen(s) + str.len;
    strcpy(newStr.str, s);
    strcat(newStr.str, str.str);
    return newStr;
};
void printToLCD(const String& string, uint16_t row){
  char str[20];
  sprintf(str, "                   ");
  LCD_DrawString(0, row*15, str);
  LCD_DrawString(0, row*15, string.str);
}

void printArray(const uint8_t* arr, int len, int row){
    char str[20];
    sprintf(str, "                   ");
    LCD_DrawString(0, row*15, str);
    for (int i=0; i<len; i++){
        sprintf(str, "%02X", arr[i]);
        LCD_DrawString(i*15, row*15, str);
    }
}

void debugLog(const String& string, uint16_t row){
    printToLCD(string, row);
}