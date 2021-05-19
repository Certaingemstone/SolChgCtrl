#include "Charger.h"

Charger::Charger(uint8_t* dutyPtrIn, uint8_t PWMpinIn, 
    uint8_t panelVpinIn, uint8_t panelIpinIn, uint8_t battVpinIn, uint8_t battEnableIn, uint8_t inputpinIn,
    float battADCscaleIn, float panelADCscaleIn, float currentADCscaleIn)
{
    dutyPtr = dutyPtrIn; PWMpin = PWMpinIn; panelVpin = panelVpinIn; 
    panelIpin = panelIpinIn; battVpin = battVpinIn; battEnable = battEnableIn; inputpin = inputpinIn;
    battADCscale = battADCscaleIn; panelADCscale = panelADCscaleIn; currentADCscale = currentADCscaleIn;
}


uint8_t Charger::runConstantVoltage(float Kp, float Vtarget, float Ilimit, float tolerance)
{
    uint8_t flag = 0;
    bool running = true;
    uint8_t IviolationsMax = 30; // max number of control loop iterations over current limit allowed (300ms)
    uint8_t Iviolations = 0; // for comparison with above
    uint8_t VviolationsMax = 30; // max number of control loop cycles outside voltage range allowed (3000ms)
    uint8_t Vviolations = 0;
    uint8_t margin = 10; // size of forbidden duty cycle gap on high (0) and low (255) ends
    float Vout = analogRead(battVpin) * battADCscale;
    float Vin = analogRead(panelVpin) * panelADCscale;
    float Iin = analogRead(panelIpin) * currentADCscale;

    float adjRaw = 0;
    int8_t adj = 0;

    engage(battEnable, PWMpin, dutyPtr);

    while (running) {
        // handle stopping via user input (runs every 10 iterations/100ms)
        if ((int)digitalRead(inputpin) == 1) {
            running = false; // stop after next control loop
        }

        // handle output voltage tolerance
        if (Vviolations > VviolationsMax) {
            running = false;
            flag = 1;
        }
        float absVdiff = ((Vout - Vtarget) < 0) ? (Vtarget - Vout) : (Vout - Vtarget); // if x < 0 return -x, else return x
        if (absVdiff > tolerance) {
            Vviolations++;
        }
        else {
            Vviolations = 0; // reset if no longer violating
        }

        // handle Vds protection (runs every 10 iterations/100ms)
        Vin = analogRead(panelVpin) * panelADCscale;
        if ((Vin - Vout) < 4f) {
            running = false;
            flag = 2;
        }
        
        
        // control loop, each loop (10 iterations) will run a bit over 100ms
        for (int i = 0; i < 10; i++) {
            // refresh output voltage and input current
            delay(10);
            Vout = analogRead(battVpin) * battADCscale;
            Iin = analogRead(panelIpin) * currentADCscale;

            // handle overcurrent protection
            if (Iviolations > IviolationsMax) {
                running = false;
                flag = 3;
                break;
            }
            if (Iin > Ilimit) {
                Iviolations++;
            }
            else {
                Iviolations = 0; // reset if no longer violating
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
                // avoid entering forbidden region when adjusting duty cycle down
                if ((uint8_t)(-adj) >= *dutyPtr - margin) {
                    // duty set to 100 by engage() initially; this won't underflow
                    *dutyPtr = margin + 1;
                }
                else {
                    *dutyPtr = *dutyPtr + adj; // note: this works
                }
            }
            else {
                // avoid entering forbidden region when adjusting it up
                if ((uint8_t)(adj) >= UINT8_MAX - *dutyPtr - margin) {
                    *dutyPtr = UINT8_MAX - margin - 1;
                }
                else {
                    *dutyPtr = *dutyPtr + adj;
                }
            }
        }
    }

    disengage(battEnable, PWMpin, dutyPtr);

    return flag;
}


uint8_t Charger::runConstantCurrent(float Kp, float Itarget, float Ilimit)
{
    uint8_t flag = 0;
    bool running = true;
    uint8_t IviolationsMax = 30; // max number of control loop iterations over current limit allowed (300ms)
    uint8_t Iviolations = 0; // for comparison with above
    uint8_t VviolationsMax = 30; // max number of control loop cycles outside voltage range allowed (3000ms)
    uint8_t Vviolations = 0;
    uint8_t margin = 10; // size of forbidden duty cycle gap on high (0) and low (255) ends
    float Vout = analogRead(battVpin) * battADCscale;
    float Vin = analogRead(panelVpin) * panelADCscale;
    float Iin = analogRead(panelIpin) * currentADCscale;

    float adjRaw = 0;
    int8_t adj = 0;

    engage(battEnable, PWMpin, dutyPtr);

    while (running) {
        // handle stopping via user input (runs every 10 iterations/100ms)
        if ((int)digitalRead(inputpin) == 1) {
            running = false; // stop after next control loop
        }

        // handle output voltage tolerance
        if (Vviolations > VviolationsMax) {
            running = false;
            flag = 1;
        }
        float absVdiff = ((Vout - Vtarget) < 0) ? (Vtarget - Vout) : (Vout - Vtarget); // if x < 0 return -x, else return x
        if (absVdiff > tolerance) {
            Vviolations++;
        }
        else {
            Vviolations = 0; // reset if no longer violating
        }

        // handle Vds protection (runs every 10 iterations/100ms)
        Vin = analogRead(panelVpin) * panelADCscale;
        if ((Vin - Vout) < 4f) {
            running = false;
            flag = 2;
        }


        // control loop, each loop (10 iterations) will run a bit over 100ms
        for (int i = 0; i < 10; i++) {
            // handle overcurrent protection
            if (Iviolations > IviolationsMax) {
                running = false;
                flag = 3;
                break;
            }
            if (Iin > Ilimit) {
                Iviolations++;
            }
            else {
                Iviolations = 0; // reset if no longer violating
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
                // avoid entering forbidden region when adjusting duty cycle down
                if ((uint8_t)(-adj) >= *dutyPtr - margin) {
                    // duty set to 100 by engage() initially; this won't underflow
                    *dutyPtr = margin + 1;
                }
                else {
                    *dutyPtr = *dutyPtr + adj; // note: this works
                }
            }
            else {
                // avoid entering forbidden region when adjusting it up
                if ((uint8_t)(adj) >= UINT8_MAX - *dutyPtr - margin) {
                    *dutyPtr = UINT8_MAX - margin - 1;
                }
                else {
                    *dutyPtr = *dutyPtr + adj;
                }
            }

            // refresh output voltage and input current
            delay(10);
            Vout = analogRead(battVpin) * battADCscale;
            Iin = analogRead(panelIpin) * currentADCscale;
        }
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


/*
uint8_t Charger::runLiIon(uint8_t stage, float Itarget, float Vtarget, float Ilimit)
{
    uint8_t flag = 0;
    disengage(battEnable, PWMpin, dutyPtr);
    return flag;
}
*/