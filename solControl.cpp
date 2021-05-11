#include "solControl.h"
//WIP
bool runCharger(int iterations, uint8_t* dutyPtr, float Itarget, float Vtarget, float VtargetFC,
    uint8_t PWMpin, uint8_t panelVpin, uint8_t panelIpin, uint8_t battVpin, 
    float battADCscale, float panelADCscale, float currentADCscale) {
    // input number of times to run control loop
    // returns 0 if nominal function, returns 1 if large fluctuation in battery side voltage occurred

    // decide between CC, CV, or float

    bool state = 0; // flag for fault states
    // read battery state
    float Vbatt = analogRead(battVpin) * battADCscale;
    if (Vbatt < Vtarget && charged = 0) {
        state = 1;
    }
    else if ()

        // execute constrained MPPT
        for (int i = 0; i < iterations; i++) {
            // read system state
            float Vpanel = analogRead(panelVpin) * panelADCscale;
            float Ipanel = analogRead(panelIpin) * currentADCscale;
            // maintain constant voltage 13V

            analogWrite(PWMpin, duty)
        }

    return state;

}