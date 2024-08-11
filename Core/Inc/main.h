/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DHT11_Pin GPIO_PIN_6
#define DHT11_GPIO_Port GPIOE
#define XMOTOR_DIR_PIN_Pin GPIO_PIN_4
#define XMOTOR_DIR_PIN_GPIO_Port GPIOA
#define ZMOTOR_DIR_PIN_Pin GPIO_PIN_7
#define ZMOTOR_DIR_PIN_GPIO_Port GPIOA
#define SWITCH_X_0_Pin GPIO_PIN_8
#define SWITCH_X_0_GPIO_Port GPIOC
#define SWITCH_Y_0_Pin GPIO_PIN_9
#define SWITCH_Y_0_GPIO_Port GPIOC
#define SWITCH_Z_0_Pin GPIO_PIN_10
#define SWITCH_Z_0_GPIO_Port GPIOC
#define YMOTOR_DIR_PIN_Pin GPIO_PIN_7
#define YMOTOR_DIR_PIN_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
