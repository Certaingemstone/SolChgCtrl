#include "Protection.h"

using namespace Protection;

void Protection::disengage(uint8_t battEnable, uint8_t PWMpin, uint8_t* dutyPtr) {
	// disconnect battery
	digitalWrite(battEnable, LOW);
	// set so that PWMpin is always on, so MOSFET turns off
	*dutyPtr = 255;
	analogWrite(PWMpin, 255);
	delay(1000);
}

void Protection::engage(uint8_t battEnable, uint8_t PWMpin, uint8_t* dutyPtr)
{
	// set duty cycle to a guessed value
	*dutyPtr = 100;
	analogWrite(PWMpin, 100);
	delay(500);
	digitalWrite(battEnable, HIGH);
}

bool Protection::startOK(int cutoffLow, int cutoffHigh, float battADCscale, float panelADCscale, uint8_t battVpin, uint8_t panelVpin, uint8_t chargerFault)
{
	bool engage = 0;
	float Vbatt = analogRead(battVpin) * battADCscale;
  float Vpanel = analogRead(panelVpin) * panelADCscale;
	// if between cutoffLow-cutoffHigh V, and no fault during charger control,
	if ((Vbatt > cutoffLow) && (Vbatt < cutoffHigh) && (chargerFault == 0)) {
		// check Vds; if okay, engage
		if (Vpanel - Vbatt > 5) {
			engage = 1;
		}
	  else {
      Serial.println("Vds insufficient");
	  }
	}
	else {
    Serial.println("Output side voltage outside cutoff range or charger fault state");
	}
	Serial.print("Vin: "); Serial.println(Vpanel); Serial.print("Vout: "); Serial.println(Vbatt); 

	return engage;
}


uint8_t Protection::runtimeOK(Charger charger, uint8_t * VviolationsPtr, uint8_t * IviolationsPtr,
	uint16_t cutoffVLow, uint16_t cutoffVHigh, uint16_t cutoffI, 
	uint8_t VviolationsLim, uint8_t IviolationsLim, uint16_t Vdsmin)
{
	uint8_t chargerFault = 0;
	// voltages and currents in ADC units
	uint16_t Vin = charger.getInVoltage();
	uint16_t Vout = charger.getOutVoltage();
	uint16_t Iin = charger.getCurrent();

	// check for voltage out of range
	if (Vout < cutoffVLow || Vout > cutoffVHigh) {
		*VviolationsPtr = *VviolationsPtr + 1;
	}
	else {
		*VviolationsPtr = 0; // reset if back in range
	}
	if (*VviolationsPtr > VviolationsLim) {
		chargerFault = 1;
	}
	
	// check for Vds too low with significant current ( >= 6ADC, or 150mA; Arduino Pro Mini draws around 70mA by itself )
	if ((Vin - Vout < Vdsmin) && Iin > 5) {
		chargerFault = 2;
	}
	else if (Vin <= Vout) {
		chargerFault = 2; // needed to handle Vin < Vout, where Vin-Vout may be large due to uint subtraction
	}

	// check for current over limit (i.e. near Isc of array for MPPT mode, or specific limit for others)
	if (Iin > cutoffI) {
		*IviolationsPtr = *IviolationsPtr + 1;
	}
	else {
		*IviolationsPtr = 0; // reset if back in range
	}
	if (*IviolationsPtr > IviolationsLim) {
		chargerFault = 3;
	}

	return chargerFault;
}
