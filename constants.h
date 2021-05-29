#ifndef CONSTANTS_H
#define CONSTANTS_H

// MODE-SPECIFIC CONSTANTS
// pin on which to take user input
static const uint8_t default_inputPin = 6;
static const uint8_t default_modePin = 7;

// voltage at which to turn relay on or off
static const float cutoffLow[] = { 9, -1 }; // [0] for lead acid, [1] for manual
static const float cutoffHigh[] = { 15, 15 }; // no minimum on OCV at output during manual override

// target voltage for lead acid charge constant voltage output
static const float default_Vtarget = 14.1; // 2.35V per cell
static const float default_VtargetFC = 13.5; // float charge 2.25V per cell

// target/limit current on input side for lead acid
static const float default_Itarget = 1.0; // a bit above C/8 for 8Ah battery

// control loop tuning parameters
static const float CV_Kp = 0.05;
static const float CC_Kp = 0.05;

// SYSTEM CONSTANTS
// ideal (uncalibrated) size of one ADC step (5/1024) since ADC goes from 0-1023
static const float ADCunit = 0.004888;

// voltage calibration coefficients (e.g. 2 -> multiply by 2 to get true voltage)
static const float battVdivider = 4.379; // 300.1k and 1014k
static const float panelVdivider = 4.356; // 298k and 1000k

// current shunt calibration coefficient
static const float currentdivider = 5; // 200x gain on the voltage drop; and V = 0.001Ohm * I

// define pins
static const uint8_t default_PWMpin = 10;
static const uint8_t default_panelVpin = 0;
static const uint8_t default_panelIpin = 1;
static const uint8_t default_battVpin = 2;
static const uint8_t default_battEnable = 12;

// some useful values
static const float default_battADCscale = ADCunit * battVdivider; // multiply by ADC reading on batt divider to get full voltage (V)
static const float default_panelADCscale = ADCunit * panelVdivider; // " " but with panel divider, to get full voltage across panel
static const float default_currentADCscale = ADCunit * currentdivider; // multiply by ADC reading on shunt differential amplifier to get current (A) through input side

// (0.0213Vin/ADCunit)
// (.0244Ain/ADCunit)
static const uint16_t VADC_12 = 700; // for 12V output: 12V/battADCscale = 559 (0.0214Vout/ADCunit)


#endif
