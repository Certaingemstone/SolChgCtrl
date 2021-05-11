#ifndef SOLPROTECTION_H
#define SOLPROTECTION_H

#include <Arduino.h>
#include <stdint.h>

uint8_t disengage(uint8_t battEnable, uint8_t PWMpin, uint8_t * dutyPtr);
// shuts off relay pin and sets PWM duty cycle to 0

bool statusOK(int cutoffLow, int cutoffHigh, float battADCscale, uint8_t battVpin, bool chargerFault);
// decides whether or not to engage charger
// based on detecting battery connects and disconnects and voltage
// returns 0 if no, 1 if yes

#endif
