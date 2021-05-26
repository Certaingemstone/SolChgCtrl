#ifndef PROTECTION_H
#define PROTECTION_H

#include <Arduino.h>
#include <stdint.h>

#include "Charger.h"

namespace Protection
{
	void disengage(uint8_t battEnable, uint8_t PWMpin, uint8_t* dutyPtr);
	// sets battEnable low and sets PWM duty cycle to 255 out of 255 (always high, i.e. MOSFET off)

	void engage(uint8_t battEnable, uint8_t PWMpin, uint8_t* dutyPtr);
	// ! analogWrite PWM on gate driver needs to be running before using this !
	// sets duty cycle to 100 out of 255 to pre-charge output stage, waits for 500ms, then sets battEnable high
	// transfer control to some charging algorithm immediately after to avoid damage

	bool startOK(int cutoffLow, int cutoffHigh, float battADCscale, float panelADCscale, uint8_t battVpin, uint8_t panelVpin, uint8_t chargerFault);
	// assumes charger is not engaged (relay off)
	// decides whether or not to engage charger
	// based on detecting battery connects and disconnects and voltage
	// returns 0 if no, 1 if yes

	// must run on each control iteration (to keep track of current and voltage violations)
	uint8_t runtimeOK(uint8_t chargerMode, Charger charger, uint8_t* VviolationsPtr, uint8_t* IviolationsPtr,
		uint8_t cutoffVLow, uint8_t cutoffVHigh, uint8_t cutoffI,
		uint8_t VviolationsLim, uint8_t IviolationsLim, uint8_t Vdsmin);
	// assumes charger is engaged (relay on, FET active)
	// returns 0 or fault states 1, 2, or 3, as described in SolChgCtrlMain setup()
}

#endif
