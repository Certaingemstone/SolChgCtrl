#ifndef PROTECTION_H
#define PROTECTION_H

#include <Arduino.h>
#include <stdint.h>

#include "Charger.h"

namespace Protection
{
	// sets battEnable low and sets PWM duty cycle to 255 out of 255 (always high, i.e. MOSFET off)
	void disengage(uint8_t battEnable, uint8_t PWMpin, uint8_t* dutyPtr);

	void engage(uint8_t battEnable, uint8_t PWMpin, uint8_t* dutyPtr);
	// ! analogWrite PWM on gate driver needs to be running before using this !
	// sets duty cycle to 100 out of 255 to pre-charge output stage, waits for 500ms, then sets battEnable high
	// transfer control to some charging algorithm immediately after
	
	// assumes charger is not engaged (relay off)
	// decides whether or not to engage charger
	// based on detecting battery connects and disconnects and voltage
	// returns 0 if no, 1 if yes
	bool startOK(int cutoffLow, int cutoffHigh, float battADCscale, float panelADCscale, uint8_t battVpin, uint8_t panelVpin, uint8_t chargerFault);
	
	// must run on each control iteration (to keep track of current and voltage violations)
	// assumes charger is engaged (relay on, FET active)
	// returns 0 or fault states 1, 2, or 3, as described in SolChgCtrlMain setup()
	uint8_t runtimeOK(Charger charger, uint8_t* VviolationsPtr, uint8_t* IviolationsPtr,
		uint16_t cutoffVLow, uint16_t cutoffVHigh, uint16_t cutoffI,
		uint8_t VviolationsLim, uint8_t IviolationsLim, uint16_t Vdsmin);
	
}

#endif
