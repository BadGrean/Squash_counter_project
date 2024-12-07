#ifndef DISPLAY_CONTROL_H
#define DISPLAY_CONTROL_H


#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

// Function declaration
void displayScore(uint8_t num1, uint8_t num2);
void setupDisplayGPIO();

#ifdef __cplusplus
}
#endif

#endif