/*
 * input.h
 *
 *  Created on: Jun 8, 2021
 *      Author: Stek4
 */

#ifndef INC_INPUT_H_
#define INC_INPUT_H_

#ifdef __cplusplus
 extern "C" {
#endif

 // Function declarations here
#include <stdint.h>
#include "stm32f7xx.h"

void init_input(UART_HandleTypeDef * huart);
void proc_input(void);


#ifdef __cplusplus
}
#endif


#endif /* INC_INPUT_H_ */
