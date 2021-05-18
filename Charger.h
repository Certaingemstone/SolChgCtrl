#ifndef CHARGER_H
#define CHARGER_H

#include <Arduino.h>
#include <stdint.h>

#include "Protection.h"


class Charger {
	uint8_t* dutyPtr;
	uint8_t PWMpin, panelVpin, panelIpin, battVpin, battEnable, inputpin;
	float battADCscale, panelADCscale, currentADCscale;
public:
	Charger(uint8_t*, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, float, float, float);

	// For all the following, if overcurrent condition persists, will disengage charger
	// and return 0 if nominal operation, 1 if load disconnected, 2 if Vds too low, 3 if overcurrent
	// Each adjusts the duty cycle of the PWM running in SolChgCtrlMain
	
	uint8_t runConstantVoltage(float Kp, float Vtarget, float Ilimit);
	// attempts to produce a constant output voltage, will reduce voltage if current limit exceeded
	// uses proportional controller - maybe will implement a PID later


	uint8_t runSLA(uint8_t stage, float Itarget, float Vtarget, float VtargetFC, float Ilimit);
	// charge procedure for sealed lead acid; stages are
	// 1 - Constant current at Itarget or lower, MPPT tracking enabled, transfer to 2 when reaching Vtarget
	// 2 - Constant voltage at Vtarget, transfer to 3 when current < 15% of Itarget
	// 3 - Float at VtargetFC until Vds threshold is reached (i.e. insufficient input voltage available)

	uint8_t runLiIon(uint8_t stage, float Itarget, float Vtarget, float Ilimit);
	// charge procedure for sealed lead acid; stages are
	// 1 - Constant current at Itarget or lower
	// 2 - Constant voltage at Vtarget until current < 5% of Itarget or Vds threshold is reached
};

#endif