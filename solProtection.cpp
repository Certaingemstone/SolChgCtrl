#include "solProtection.h"

uint8_t disengage(uint8_t battEnable, uint8_t PWMpin, uint8_t * dutyPtr) {
	// disconnect battery
	digitalWrite(battEnable, LOW);
	// set so that PWMpin is always on, so MOSFET turns off
	*dutyPtr = 255;
	analogWrite(PWMpin, 255);
	delay(5000);
}

bool statusOK(int cutoffLow, int cutoffHigh, float battADCscale, uint8_t battVpin, bool chargerFault)
{
	bool engage = 0;
	float Vbatt = analogRead(battVpin) * battADCscale;
	// if between cutoffLow-cutoffHigh V, and no fault during charger control, engage or remain engaged
	if (Vbatt > cutoffLow && Vbatt < cutoffHigh && !chargerFault) {
		engage = 1;
	}

	return engage;
}
