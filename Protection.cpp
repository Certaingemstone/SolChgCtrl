#include "Protection.h"

using namespace Protection;

void Protection::disengage(uint8_t battEnable, uint8_t PWMpin, uint8_t* dutyPtr) {
	// disconnect battery
	digitalWrite(battEnable, LOW);
	// set so that PWMpin is always on, so MOSFET turns off
	*dutyPtr = 255;
	analogWrite(PWMpin, 255);
	delay(5000);
}

void Protection::engage(uint8_t battEnable, uint8_t PWMpin, uint8_t* dutyPtr)
{
	// set duty cycle to a guessed value
	*dutyPtr = 100;
	delay(500);
	digitalWrite(battEnable, HIGH);
}

bool Protection::startOK(int cutoffLow, int cutoffHigh, float battADCscale, float panelADCscale, uint8_t battVpin, uint8_t panelVpin, uint8_t chargerFault)
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


uint8_t Protection::runtimeOK(uint8_t chargerMode, Charger charger, uint8_t * VviolationsPtr, uint8_t * IviolationsPtr,
	uint8_t cutoffVLow, uint8_t cutoffVHigh, uint8_t cutoffI, 
	uint8_t VviolationsLim, uint8_t IviolationsLim, uint8_t Vdsmin)
{
	uint8_t chargerFault = 0;
	// voltages and currents in ADC units
	uint8_t Vin = charger.getInVoltage();
	uint8_t Vout = charger.getOutVoltage();
	uint8_t Iin = charger.getCurrent();

	// check for voltage out of range
	if (Vout < cutoffVLow || Vout > cutoffVHigh) {
		*VviolationsPtr = *VviolationsPtr + 1;
	}
	if (*VviolationsPtr > VviolationsLim) {
		chargerFault = 1;
	}
	
	// check for Vds too low
	if (Vin - Vout < Vdsmin) {
		chargerFault = 2;
	}
	if (Vin <= Vout) {
		chargerFault = 2; // needed to handle Vin < Vout, where Vin-Vout may be large due to uint subtraction
	}

	// check for current over limit (i.e. near Isc of array for MPPT mode, or specific limit for others)
	if (Iin > cutoffI) {
		*IviolationsPtr = *IviolationsPtr + 1;
	}
	if (*IviolationsPtr > IviolationsLim) {
		chargerFault = 3;
	}

	return chargerFault;
}


