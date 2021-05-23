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
	// MPPT specific variables
	float prevPower;
	int8_t prevAdjustment;
	// state variables that will change when these functions are called, initialized to 0
	uint8_t panelV, panelI, battV;

	Charger(uint8_t*, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, float, float, float);

	// to make
	// functions that return the requested adjustment to duty cycle per a given algorithm, given state: 
	// algoCV, algoCC, algoMPPT
	// function to perform the adjustment safely: updateDuty
	// function to get the new current and voltage: updateState
	// function to check the validity of the state variables and track violations of voltage/current boundaries: 
	// runtimeOK, to be included in Protection
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

	void updateState();
	// measure and update object voltage and current values

	void resetMPPT();
	// reset MPPT state variables to 0

	bool updateDuty(int8_t adjustment);
	// updates the value stored at dutyPtr according to adjustment, returns 1 if success, 0 if hit duty deadzone
	// if duty deadzone was going to be reached (<=9 or >=245), will set to 10 or 244 instead
	// does NOT update the analogWrite

	uint8_t getInVoltage();
	// return currently stored voltage value, in ADC units

	uint8_t getOutVoltage();
	// return currently stored voltage value, in ADC units

	uint8_t getCurrent();
	// return currently stored current value, in ADC units
	
	int8_t algoCV(float Kp, uint8_t scaledVtarget, uint8_t scaledVtolerance, uint8_t scaledIlimit);
	// from current state variables, returns the requested adjustment to duty cycle
	// maximum size of requested adjustment is 20
	// no time delay
	// a modified proportional (P) controller

	int8_t algoCC(float Kp, uint8_t scaledItarget, uint8_t scaledItolerance);
	// from current state variables, returns the requested adjustment to duty cycle
	// maximum size of requested adjustment is 20
	// no time delay
	// a modified proportional (P) controller 
	
	int8_t algoMPPT(float Kp, uint8_t scaledSoftIlimit);
	// from current state variables, returns the requested adjustment to duty cycle
	// maximum size of requested adjustment is 1
	// a perturb-and-observe MPPT implementation
	// no time delay
	// updates prevPower and prevAdjustment for next iteration

	


	// TO BE REFACTORED USING ABOVE HELPERS
	// For all the following, if overcurrent condition persists, will disengage charger
	// and return 0 if nominal operation, 1 if load disconnected, 2 if Vds too low under load (e.g. <3.5V, >100mA), 3 if overcurrent
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