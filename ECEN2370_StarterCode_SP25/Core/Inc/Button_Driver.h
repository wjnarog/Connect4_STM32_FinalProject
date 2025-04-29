/*
 * Button_Driver.h
 *
 *  Created on: Apr 21, 2025
 *      Author: wjnar
 */

#ifndef INC_BUTTON_DRIVER_H_
#define INC_BUTTON_DRIVER_H_

#include <stdbool.h>
#include "stm32f4xx_hal.h"

#define BUTTON_PORT_ADDRESS 0x0
#define BUTTON_PIN 0x0
#define BUTTON_PRESSED 1
#define BUTTON_UNPRESSED 0


void ButtonInitInterruptMode();
bool IsButtonPressed();

#endif /* INC_BUTTON_DRIVER_H_ */
