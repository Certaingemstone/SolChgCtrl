#ifndef CHARGER_H
#define CHARGER_H

#include <Arduino.h>
#include <stdint.h>

#include "Protection.h"


class Charger {
private:
	// system parameters that won't change
	uint8_t* const dutyPtr;
	uint8_t const PWMpin, panelVpin, panelIpin, battVpin, battEnable, inputpin;
	float const battADCscale, panelADCscale, currentADCscale;
public:
	// state variables that will change when these functions are called
	//uint8_t 

	Charger(uint8_t*, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, float, float, float);

	// to make
	// functions that return the requested adjustment to duty cycle per a given algorithm, given state: 
	// algoCV, algoCC, algoMPPT
	// function to perform the adjustment safely: updateDuty
	// function to get the new current and voltage: updateState
	// function to check the validity of the state variables and track violations of voltage/current boundaries: runOK
	// larger routines: runCV, runCC, runMPPT, which make use of the above

	// Usage:
	// Main function initializes a Charger, not with a specific mode 
	// Main function, depending on mode, calls runConstantVoltage, runConstantCurrent, runMPPT
	// in a specific order, and uses the return results from each and the state to determine which to call next/with what parameters
	// runConstantVoltage will have different disengage conditions depending on how it's called, including;
	// - protection and manual interrupt only (the conditions in 5/18/2021 version)
	// - all of the above + a current (for CV) or voltage (for CC/MPPT) cutoff specified by caller; ending by the cutoff will NOT turn off battEnable, will only set duty cycle to 255.
	// runConstantVoltage updateState's, then algoCV's, then updateDuty's, and runOK's (and if flag is on, also checks cutoff)
	// - returns exit code flag from runOK or cutoff check if applicable
	// runConstantCurrent does more or less the same as runConstantVoltage, just with different cutoff and different variable
	// runMPPT also does the same as above, but with MPPT tracking, different cutoff and looser constraints on current/voltage


	// For all the following, if overcurrent condition persists, will disengage charger
	// and return 0 if nominal operation, 1 if load disconnected, 2 if Vds too low, 3 if overcurrent
	// Each adjusts the duty cycle of the PWM running in SolChgCtrlMain
	
	//uint8_t runConstantVoltage(float Kp, float Vtarget, float Ilimit);
	// attempts to produce a constant output voltage, will reduce voltage if current limit exceeded
	// uses proportional controller - maybe will implement a PID later

	//uint8_t runConstantCurrent(float Kp, float Itarget, float Ilimit);
	// attempts to produce a constant output voltage, will reduce voltage if current limit exceeded
	// uses proportional controller - maybe will implement a PID later

	//uint8_t runSLA(uint8_t stage, float Itarget, float Vtarget, float VtargetFC, float Ilimit);
	// charge procedure for sealed lead acid; stages are
	// 1 - Constant current at Itarget input current or lower, MPPT tracking enabled, transfer to 2 when reaching Vtarget
	// 2 - Constant voltage at Vtarget, transfer to 3 when current < 15% of Itarget
	// 3 - Float at VtargetFC until Vds threshold is reached (i.e. insufficient input voltage available)

	/*
	uint8_t runLiIon(uint8_t stage, float Itarget, float Vtarget, float Ilimit);
	// charge procedure for sealed lead acid; stages are
	// 1 - Constant current at Itarget input current or lower
	// 2 - Constant voltage at Vtarget until current < 5% of Itarget or Vds threshold is reached
	*/
};

#endif