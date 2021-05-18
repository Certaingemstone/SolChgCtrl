#include "Charger.h"

Charger::Charger(uint8_t* dutyPtrIn, uint8_t PWMpinIn, 
    uint8_t panelVpinIn, uint8_t panelIpinIn, uint8_t battVpinIn, uint8_t battEnableIn, uint8_t inputpinIn,
    float battADCscaleIn, float panelADCscaleIn, float currentADCscaleIn)
{
    dutyPtr = dutyPtrIn; PWMpin = PWMpinIn; panelVpin = panelVpinIn; 
    panelIpin = panelIpinIn; battVpin = battVpinIn; battEnable = battEnableIn; inputpin = inputpinIn;
    battADCscale = battADCscaleIn; panelADCscale = panelADCscaleIn; currentADCscale = currentADCscaleIn;
}


uint8_t Charger::runConstantVoltage(float Kp, float Vtarget, float Ilimit)
{
    uint8_t flag = 0;
    bool running = true;
    uint8_t tolerance = 30; // max number of control loop cycles over current limit allowed (300ms)
    uint8_t violations = 0; // for comparison with above
    uint8_t margin = 10; // size of forbidden duty cycle gap on high (0) and low (255) ends
    float Vout = analogRead(battVpin) * battADCscale;
    float Iin = analogRead(panelIpin) * currentADCscale;

    float adjRaw = 0;
    int8_t adj = 0;

    engage(battEnable, PWMpin, dutyPtr);

    while (running) {
        //TODO: handle Vds protection
        // handle stopping via user input
        if ((int)digitalRead(inputpin) == 1) {
            running = false;
        }
        // handle overcurrent protection
        if (violations > tolerance) {
            running = false;
            flag = 3;
        }
        if (Iin > Ilimit) {
            violations++;
        }
        else {
            violations = 0; // if violations continue 
        }
        
        // determine adjustment to duty cycle
        adjRaw = Kp * (Vout - Vtarget); // negative if output is less than target 
        // since we need to decrease signal duty cycle (i.e. from MCU) to increase output voltage
        // limit of step size for stability purposes
        if (adjRaw > 20f) { 
            adj = 20; 
        }
        else if (adjRaw < -20f) { 
            adj = -20; 
        }
        else {
            adj = (adjRaw >= 0) ? (int8_t)(adjRaw + 0.5) : (int8_t)(adjRaw - 0.5); // round
        }
        
        // apply adjustment to duty cycle
        if (adj < 0) {
            // avoid underflow when adjusting duty cycle down
            if ((uint8_t)(-adj) >= *dutyPtr - margin) {
                // duty set to 100 by engage() initially
                *dutyPtr = margin + 1; // make sure we don't go under margin
            }
            else {
                *dutyPtr = *dutyPtr + adj;
            }
        }
        else {
            // avoid overflow when adjusting it up
            if ((uint8_t)(adj) >= UINT8_MAX - *dutyPtr - margin) {
                *dutyPtr = UINT8_MAX - margin - 1;
            }
            else {
                *dutyPtr = *dutyPtr + adj;
            }
        }
        
        // check output voltage and input current again
        delay(10);
        Vout = analogRead(battVpin) * battADCscale;
        Iin = analogRead(panelIpin) * currentADCscale;
    }

    disengage(battEnable, PWMpin, dutyPtr);

    return flag;
}

uint8_t Charger::runSLA(uint8_t stage, float Itarget, float Vtarget, float VtargetFC, float Ilimit)
{
    uint8_t flag = 0;
    disengage(battEnable, PWMpin, dutyPtr);
    return flag;
}

uint8_t Charger::runLiIon(uint8_t stage, float Itarget, float Vtarget, float Ilimit)
{
    uint8_t flag = 0;
    disengage(battEnable, PWMpin, dutyPtr);
    return flag;
}
