/*
 * motor.h
 *
 *  Created on: Mar 20, 2024
 *      Author: Nico
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_
#include "main.h"
#include "utils.hpp"
#include "main.h"
#include "tim.h"

#define DMA_BUFFER_SIZE 20000 // max move distance onece is length/100



extern const uint32_t PulseDMABuff[DMA_BUFFER_SIZE];

class PulseMotor
{
private:
    TIM_HandleTypeDef *pTim;
    uint32_t timChannel;
    GPIO_TypeDef *pGPIO;
    uint16_t GPIO_Pin;
    uint32_t prescaler = 1000 - 1;

    const uint32_t inputFrequency = 72e6;
    const uint16_t CounterPeriod = 72 - 1;

protected:
    int32_t stepSum = 0;

public:
    PulseMotor(TIM_HandleTypeDef *pTim, uint32_t timChannel, GPIO_TypeDef *pGPIO, uint16_t GPIO_Pin)
    {
        this->pTim = pTim;
        this->timChannel = timChannel;
        this->pGPIO = pGPIO;
        this->GPIO_Pin = GPIO_Pin;
    }

    uint16_t getFrequency()
    {
        return inputFrequency / (prescaler + 1) / (CounterPeriod + 1);
    }

    // pulse sending frequency
    void setFrequency(uint16_t frequency)
    {
        if (frequency == 0)
        {
            prescaler = 500;
            return;
        }
        prescaler = inputFrequency / (CounterPeriod + 1) / frequency - 1;
        __HAL_TIM_SET_PRESCALER(pTim, prescaler);
    }

    // direction: 0 or 1
    void setDirection(uint8_t direction)
    {
        HAL_GPIO_WritePin(pGPIO, GPIO_Pin, direction ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }

    void pulse(uint16_t pulseNum)
    {
        // PulseDMABuff[pulseNum] = 0;
        HAL_TIM_PWM_Start_DMA(pTim, timChannel, (uint32_t *)PulseDMABuff, pulseNum);
    }

    void spinStart()
    {
        __HAL_TIM_SET_COMPARE(pTim, timChannel, 36);
        HAL_TIM_PWM_Start(pTim, timChannel);
    }
    void spinStop()
    {
        HAL_TIM_PWM_Stop(pTim, timChannel);
    }

    void emergencyStop()
    {
        HAL_TIM_PWM_Stop_DMA(pTim, timChannel);
        HAL_TIM_PWM_Stop(pTim, timChannel);
        __HAL_TIM_SET_COMPARE(pTim, timChannel, 0);
        HAL_TIM_PWM_Start(pTim, timChannel);
    }

    void pulse_wait(uint16_t pulseNum)
    {
        pulse(pulseNum);
        HAL_Delay((float)pulseNum / getFrequency() * 1000 + 10);
    }

    void step(uint8_t direction, uint32_t stepNum)
    {   
        if (stepNum == 0)
        {
            return;
        }
        if (stepNum > DMA_BUFFER_SIZE)
        {
            printToLCD("StepNum too large", 18);
            return;
        }
        stepSum += direction ? stepNum : -stepNum;
        setDirection(direction);
        pulse(stepNum);
    }


    void step_wait(uint8_t direction, uint32_t stepNum)
    {
        stepSum += direction ? stepNum : -stepNum;
        setDirection(direction);
        pulse_wait(stepNum);
    }

    // This function can go any number of steps
    void step_inf(uint8_t direction, uint32_t stepNum)
    {
        if (stepNum == 0)
        {
            return;
        }
        setDirection(direction);
        while (stepNum > 0)
        {
            uint16_t pulseNum = stepNum < DMA_BUFFER_SIZE ? stepNum : DMA_BUFFER_SIZE;
            stepSum+= direction ? pulseNum : -pulseNum;
            stepNum -= pulseNum;
            pulse(pulseNum);
            HAL_Delay((float)pulseNum / getFrequency() * 1000+5);
            // Problem! This will block the program! Not applicable for multi-motor control!
        }
    }
};

class SERVO42C_Pulse : public PulseMotor
{
    friend void setPosition3d(float x, float y, float z, float speed);
    friend void step3d(uint32_t xStepCount, uint8_t xDir, uint32_t yStepCount, uint8_t yDir, uint32_t zStepCount, uint8_t zDir, float speed);

protected:
    // configs
    const int maxDistance = 0; // 264 for x, 146 for z, 适用于用电机驱动丝杆
    uint8_t stepDivision = 1;
    float stepAngle = 1.8; // degree, depends on motor type, the version we are using is 1.8
    float mmPerLap = 2;    // mm, depends on the mechanical structure of 丝杆

    uint32_t getStepCountFromTargetPosition(float targetPosition, uint8_t &direction)
    {
        float currentPosition = getPosition();
        float error = targetPosition - currentPosition;
        direction = error > 0 ? 1 : 0;
        error = error > 0 ? error : -error;
        uint32_t stepCount = error / (mmPerLap) * (360 / stepAngle) * stepDivision;
        return stepCount;
    }

    float getRPMof(uint8_t speed)
    {
        float divisionCoefficient = ABS(stepAngle - 1.8) > 0.01 ? 400 : 200;
        return (speed * 30000) / (stepDivision * divisionCoefficient);
    }
    float getSpeedParamOfRPM(float rpm)
    {
        float divisionCoefficient = ABS(stepAngle - 1.8) > 0.01 ? 400 : 200;
        return rpm * stepDivision * divisionCoefficient / 30000;
    }

    float getLinearSpeedOf(uint8_t speed)
    {
        return getRPMof(speed) * mmPerLap / 60; // mm/s
    }

    uint8_t getSpeedParamOfLinearSpeed(float linearSpeed)
    {
        float rpm = linearSpeed * 60 / mmPerLap;
        return getSpeedParamOfRPM(rpm);
    }

    float frequencyToSpeed(uint16_t frequency)
    {
        // frequency是每秒脉冲数，speed是每秒毫米数
        // 所以需要返回一秒内走过的distance，一秒内的step数就是frequency
        return stepcountToDistance(frequency);
    }

    float speedToFrequency(float speed)
    {
        float rps = speed / mmPerLap;
        return rps * 360 / stepAngle * stepDivision;
    }

public:
    bool stepCompleted = false;
    float stepcountToDistance(uint32_t stepCount)
    {
        return stepCount / (float)stepDivision * stepAngle / 360 * mmPerLap;
    }
    
    SERVO42C_Pulse(TIM_HandleTypeDef *pTim, uint32_t timChannel, GPIO_TypeDef *pGPIO, uint16_t GPIO_Pin) : PulseMotor(pTim, timChannel, pGPIO, GPIO_Pin) {}

    // position: distance in mm from zero position
    // zero position: where the motor is set
    float getPosition()
    {
        return (float)stepSum / stepDivision * stepAngle / 360 * mmPerLap;
    }
    void setPosition(float position)
    {
        uint8_t direction = 0;
        uint32_t stepCount = getStepCountFromTargetPosition(position, direction);
        step(direction, stepCount);
    }

    float getSpeed()
    {
        return frequencyToSpeed(getFrequency());
    }
    void setSpeed(float speed)
    {
        if (speed > 0.001)
            setFrequency(speedToFrequency(speed));
    }
    void resetStepSum(){
        stepSum = 0;
    }

    // reset zero position by turning the motor CW/CCW(0/1)
    // once the motor stops turnning, it reaches the zero position
    // WARN: this will block the program until the motor stops
    void alignAbsolutePosition()
    {
        if (HAL_GPIO_ReadPin(SWITCH_X_0_GPIO_Port, SWITCH_X_0_Pin) == GPIO_PIN_SET)
        {
            setFrequency(1000);
            printToLCD("Waiting Limit Switch", 0);
            while (HAL_GPIO_ReadPin(SWITCH_X_0_GPIO_Port, SWITCH_X_0_Pin) == GPIO_PIN_SET)
            {
                step_wait(0, 20);
            }
        };
        printToLCD("Aligned!", 0);
        stepSum = 0;
    }
};

extern SERVO42C_Pulse xPulseMotor; // tim, tim channel, dir gpio, dir gpio pin
extern SERVO42C_Pulse yPulseMotor;
extern SERVO42C_Pulse zPulseMotor;

class SERVO42C_UART : public SERVO42C_Pulse{
    friend void step3d(uint32_t xStepCount, uint8_t xDir, uint32_t yStepCount, uint8_t yDir, uint32_t zStepCount, uint8_t zDir);
    friend void setPosition3d(float x, float y, float z);
private:
    UART_HandleTypeDef* pUART;
    uint8_t address;

    uint16_t encoder = 0;
    uint16_t zeroEncoder = 0;
    int32_t encoderCarry = 0;
    int32_t zeroEncoderCarry = 0;

    int16_t errorAngleRaw = 0; // 0~0xFFFF corresponds to 0~360 degree
    float errorAngle = 0; // 0~360 degree
    // bool isShaftProtected = false;
    bool enableAbsolutePosControl = false; // only allow pos control after zero position aligned

    // configs
    uint8_t stepSpeed = 50;

    static uint8_t getCRC(uint8_t instruction[], uint8_t len){
        uint16_t result = 0;
        for (int i=0; i<len; i++){
            result+=instruction[i];
        }
        return result & 0xFF;
    }

    // WARNING: remember to check for nullptr, which means receive failed
    uint8_t* receiveUART(uint8_t len){
        static uint8_t data[16] = {};
        HAL_UART_Receive(pUART, data, len+1, 700);
        uint8_t offset = 0;
        if (data[0] == address){
            offset = 0;
        }else if (data[1] == address){
            offset = 1;
        }else {
            return nullptr;
        }

        if (data[len-1+offset] == getCRC(data+offset, len-1)){
            // printToLCD("Check Success", 3);

            // move forward by the offset
            if (offset == 1){
                for (int i=0; i<len; i++){
                    data[i] = data[i+1];
                }
            }
            return data;
        }else{
            // printToLCD("Check fail : offset="+String(offset), 3);
            // printToLCD(String("dataCRC=")+String::toHexStr(data[len-1+offset])+" CalcCRC="+String::toHexStr(getCRC(data+offset, len-1)), 5);
            // printToLCD(String(data,9), 7);
            return nullptr;
        }
    }

    uint8_t* receiveUART(uint8_t len, uint16_t maxRetry){
        uint8_t* data = nullptr;
        while (data==nullptr){
            data = receiveUART(len);
            maxRetry--;
            if (maxRetry==0){
                break;
            }
            if (data==nullptr){
                // debugLog("Retry receiveUART, maxRetry="+String(maxRetry));
            }
        }
        return data;
    }

public:
    SERVO42C_UART(uint8_t address, UART_HandleTypeDef* pUART, const SERVO42C_Pulse& pulse) : address(address), pUART(pUART), SERVO42C_Pulse(pulse){}

    // position : distance from zero position
    float getPosition(){ // to test
        receiveEncoder();
        // if (enableAbsolutePosControl){
        float position = 0;
        position = ((encoder - zeroEncoder)/(float)0xFFFF+ (encoderCarry - zeroEncoderCarry))*mmPerLap;
        // }
        // return -1;
    }


    void setPosition(float position){ // to test
        receiveEncoder();
        uint8_t direction = 0;
        uint32_t stepCount = getStepCountFromTargetPosition(position, direction);
        step_UART(direction, stepSpeed, stepCount);
    }


    // direction : 1 or 0
    // speed: 0~7
    // stepCount: each stepCount/stepDivision for 1.8 deg, currently stepDivision=1
    void step_UART(uint8_t direction, uint8_t speed, uint32_t stepCount){
        uint8_t instruction[8] = {};
        instruction[0] = address;
        instruction[1] = 0xfd;
        instruction[2] = (direction<<7)|(speed & 0x7F);
        for (int i=0; i<4; i++){
            instruction[6-i] = (stepCount & (0xFF << 8*i))>>8*i;
        }
        instruction[7] = getCRC(instruction, 8);
        debugLog(String(instruction, 8)+" c:"+String(stepCount));
        HAL_UART_Transmit(pUART, instruction, 8, 100);
    }

    // step with block
    void step_UART_b(uint8_t direction, uint8_t speed, uint32_t stepCount){
        uint8_t instruction[8] = {};
        instruction[0] = address;
        instruction[1] = 0xfd;
        instruction[2] = (direction<<7)|(speed & 0x7F);
        for (int i=0; i<4; i++){
            instruction[6-i] = (stepCount & (0xFF << 8*i))>>8*i;
        }
        instruction[7] = getCRC(instruction, 8);
        debugLog(String(instruction, 8)+"c"+String(stepCount));
        HAL_UART_Transmit(pUART, instruction, 8, 100);
        debugLog("step command sent",19);
        uint8_t* data = receiveUART(3);
        debugLog("step started",19);
        data = receiveUART(3);
        debugLog("step finished",19);
    }

    void stepClockwise_UART(uint32_t stepCount){
        step_UART(0, stepSpeed, stepCount);
    }
    void stepCounterClockwise_UART(uint32_t stepCount){
        step_UART(1, stepSpeed, stepCount);
    }

    // speed: 0~0x7f i.e. 0~127
    void spin(uint8_t direction, uint8_t speed){
        uint8_t instruction[4] = {};
        instruction[0] = address;
        instruction[1] = 0xf6;
        instruction[2] = (direction<<7)|(speed & 0x7F);
        instruction[3] = getCRC(instruction, 3);
        HAL_UART_Transmit(pUART, instruction, 4, 100);
    }
    void spinClockwise(uint8_t speed){
        spin(0, speed);
    }
    void spinCounterClockwise(uint8_t speed){
        spin(1, speed);
    }

    void stop(){
        uint8_t instruction[3] = {};
        instruction[0] = address;
        instruction[1] = 0xf7;
        instruction[2] = getCRC(instruction, 2);
        HAL_UART_Transmit(pUART, instruction, 3, 50);
    }

    // torque: 0~0x4B0, i.e. 0~1200
    void setMaxTorque(uint16_t torque){
        uint8_t instruction[5] = {};
        instruction[0] = address;
        instruction[1] = 0xa5;
        instruction[2] = (torque<<8);
        instruction[3] = torque&0xFF;
        instruction[4] = getCRC(instruction, 4);
        HAL_UART_Transmit(pUART, instruction, 5, 100);
    }

    bool receiveEncoder(int retryCount = 3){
        // send instruction to request encoder
        uint8_t instruction[3] = {};
        instruction[0] = address;
        instruction[1] = 0x30;
        instruction[2] = getCRC(instruction, 2);
        while (retryCount>0){
            HAL_UART_Transmit(pUART, instruction, 3, 50);
            uint8_t* data = receiveUART(8, 3);
            if (data!=nullptr){
                encoder = data[5]<<8 | data[6];
                encoderCarry = data[1]<<24 | data[2]<<16 | data[3]<<8 | data[4];
                return true;
            }
            retryCount--;
        }
        return false;
    }

    void receiveErrorAngle(){ // todo
        uint8_t instruction[3] = {};
        instruction[0] = address;
        instruction[1] = 0x39;
        instruction[2] = getCRC(instruction, 2);
        HAL_UART_Transmit(pUART, instruction, 3, 50);

        uint8_t* data = receiveUART(4, 3);
        if (data==nullptr){
            debugLog("Failed to receive error angle");
            return;
        }

        errorAngleRaw = data[1]<<8 | data[2];
        errorAngle = errorAngleRaw/(float)0xFFFF*360;

    }

    bool isShaftProtected(){ //todo
        uint8_t instruction[3] = {};
        instruction[0] = address;
        instruction[1] = 0x3E;
        instruction[2] = getCRC(instruction, 2);
        HAL_UART_Transmit(pUART, instruction, 3, 50);
        
        uint8_t* data = receiveUART(3, 3);
        if (data==nullptr){
            debugLog("Failed to receive shaft protection state");
            return false;
        }
        return data[1] == 1;
    }

    void releaseProtection(){
        uint8_t instruction[3] = {};
        instruction[0] = address;
        instruction[1] = 0x3D;
        instruction[2] = getCRC(instruction, 2);
        HAL_UART_Transmit(pUART, instruction, 3, 50);
    }

    

    uint16_t getEncoder(){
        return encoder;
    }
    int32_t getEncoderCarry(){
        return encoderCarry;
    }
    int16_t getErrorAngleRaw(){
        return errorAngleRaw;
    }
    float getErrorAngle(){
        return errorAngle;
    }
    // bool getShaftProtectionState(){
    //     return isShaftProtected;
    // }

};

extern SERVO42C_UART xServo;
extern SERVO42C_UART yServo;
extern SERVO42C_UART zServo;

void step3d(uint32_t xStepCount, uint8_t xDir, uint32_t yStepCount, uint8_t yDir, uint32_t zStepCount, uint8_t zDir, float speed);

void setPosition3d(float x, float y, float z, float speed = 15);

#endif /* INC_MOTOR_H_ */
