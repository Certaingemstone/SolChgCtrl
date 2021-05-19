#include "Protection.h"

void disengage(uint8_t battEnable, uint8_t PWMpin, uint8_t* dutyPtr) {
	// disconnect battery
	digitalWrite(battEnable, LOW);
	// set so that PWMpin is always on, so MOSFET turns off
	*dutyPtr = 255;
	analogWrite(PWMpin, 255);
	delay(5000);
}

void engage(uint8_t battEnable, uint8_t PWMpin, uint8_t* dutyPtr)
{
	// set duty cycle to a guessed value
	*dutyPtr = 100;
	delay(500);
	digitalWrite(battEnable, HIGH);
}

bool startOK(int cutoffLow, int cutoffHigh, float battADCscale, float panelADCscale, uint8_t battVpin, uint8_t panelVpin, uint8_t chargerFault)
{
	bool engage = 0;
	float Vbatt = analogRead(battVpin) * battADCscale;
	// if between cutoffLow-cutoffHigh V, and no fault during charger control,
	if (Vbatt >= cutoffLow && Vbatt < cutoffHigh && chargerFault == 0) {
		// check Vds; if okay, engage
		float Vpanel = analogRead(panelVpin) * panelADCscale;
		if (Vpanel - Vbatt > 5) {
			engage = 1;
		}
	}
	return engage;
}

/*
uint8_t runtimeOK(int cutoffLow, int cutoffHigh, float Vtarget, float Vbatt, float Vpanel, float Ipanel, uint8_t chargerMode)
{
	uint8_t chargerFault = 0;
	// TODO
	return chargerFault;
}
*/

