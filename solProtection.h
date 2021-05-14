#ifndef SOLPROTECTION_H
#define SOLPROTECTION_H

#include <Arduino.h>
#include <stdint.h>

uint8_t disengage(uint8_t battEnable, uint8_t PWMpin, uint8_t * dutyPtr);
// sets battEnable low and sets PWM duty cycle to 255 out of 255 (always high, i.e. MOSFET off)

bool startOK(int cutoffLow, int cutoffHigh, float battADCscale, float panelADCscale, uint8_t battVpin, uint8_t panelVpin, uint8_t chargerFault);
// assumes charger is not engaged (relay off)
// decides whether or not to engage charger
// based on detecting battery connects and disconnects and voltage
// returns 0 if no, 1 if yes

uint8_t runtimeOK(int cutoffLow, int cutoffHigh, float Vtarget, float Vbatt, float Vpanel, float Ipanel, uint8_t chargerMode);
// assumes charger is engaged (relay on, FET active)
// based on measured current, input voltage, and output voltage,
// determines whether to continue charger operation after startup.
// will return 0 or fault states 1, 2, or 3, as described in SolChgCtrlMain setup()
// Vtarget is used if charger is operating in manual override
// will return 1 if too far from Vtarget in that case

#endif
