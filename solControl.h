#ifndef SOLCONTROL_H
#define SOLCONTROL_H

#include <Arduino.h>
#include <stdint.h>

bool runCharger(int iterations, uint8_t * dutyPtr, float Itarget, float Vtarget, float VtargetFC,
	uint8_t PWMpin, uint8_t panelVpin, uint8_t panelIpin, uint8_t battVpin, 
	float battADCscale, float panelADCscale, float currentADCscale);
// Runs charge in three stages intended for lead acid: CC, CV, float

// Starts off taking measurements and defining state
// Initial connection starting voltage below 12.8 and CC complete flag not set and current voltage below Vtarget: CC
// Current voltage at ... and current above ... : CV
// Current voltage at ... and current below ... : drop to float charge mode

// Checks for overcurrent and or battery disconnect/connect events (large voltage fluctuations in a run)

// Returns flag for above events

#endif