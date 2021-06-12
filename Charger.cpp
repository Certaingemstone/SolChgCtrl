#include "Charger.h"

Charger::Charger(uint8_t* dutyPtrIn, uint8_t panelVpinIn, uint8_t panelIpinIn, uint8_t battVpinIn) : dutyPtr(dutyPtrIn), panelVpin(panelVpinIn), panelIpin(panelIpinIn), battVpin(battVpinIn)
{
    prevPower = 0; prevAdjustment = false; panelV = 0; panelI = 0; battV = 0;
}


void Charger::updateState()
{
    panelV = (uint16_t)(analogRead(panelVpin));
    panelI = (uint16_t)(analogRead(panelIpin));
    battV = (uint16_t)(analogRead(battVpin));
}

void Charger::resetMPPT()
{
    prevPower = 0.0f;
    prevAdjustment = false;
}

bool Charger::updateDuty(int8_t adjustmentIn, bool invert)
{   
    int8_t adjustment = (invert) ? -adjustmentIn : adjustmentIn;
    uint8_t margin = 10;
    bool success = false;
    // apply adjustment to duty cycle
    if (adjustment < 0) {
        // avoid entering forbidden region when adjusting duty cycle down
        if ((uint8_t)(-adjustment) >= *dutyPtr - margin) {
            // duty set to 100 by engage() initially; this won't underflow
            *dutyPtr = margin + 1;
        }
        else {
            *dutyPtr = *dutyPtr + adjustment; // note: this works
            success = true;
        }
    }
    else {
        // avoid entering forbidden region when adjusting it up
        if ((uint8_t)(adjustment) >= UINT8_MAX - *dutyPtr - margin) {
            *dutyPtr = UINT8_MAX - margin - 1;
        }
        else {
            *dutyPtr = *dutyPtr + adjustment;
            success = true;
        }
    }
    return success;
}

uint16_t Charger::getInVoltage()
{
    return panelV;
}

uint16_t Charger::getOutVoltage()
{
    return battV;
}

uint16_t Charger::getCurrent()
{
    return panelI;
}

int8_t Charger::stepCV(float Kp, uint16_t scaledVtarget)
{
    int8_t adj = 0;
    // determine adjustment; battV is also a uint16_t
    int16_t adjRaw = (battV > scaledVtarget) ? (int16_t)(Kp * (battV - scaledVtarget) * -1) : (int16_t)(Kp * (scaledVtarget - battV));
    // If output > target, then give negative adjustment. Otherwise, output < target, give positive adjustment.
    // limit of step size for stability purposes
    if (adjRaw > 20) {
        adj = 20;
    }
    else if (adjRaw < -20) {
        adj = -20;
    }
    else {
        adj = (adjRaw >= 0) ? (int8_t)(adjRaw + 0.5) : (int8_t)(adjRaw - 0.5); // round, by this point adjRaw should fit in int8
    }
    return adj;
}

int8_t Charger::stepCC(float Kp, uint16_t scaledItarget)
{
    int8_t adj = 0;
    // determine adjustment; battV is also a int16_t
    int16_t adjRaw = (panelI > scaledItarget) ? (int16_t)(Kp * (panelI - scaledItarget) * -1) : (int16_t)(Kp * (scaledItarget - panelI));
    // If output > target, then give negative adjustment. Otherwise, output < target, give positive adjustment.
    // limit of step size for stability purposes
    if (adjRaw > 20) {
        adj = 20;
    }
    else if (adjRaw < -20) {
        adj = -20;
    }
    else {
        adj = (adjRaw >= 0) ? (int8_t)(adjRaw + 0.5) : (int8_t)(adjRaw - 0.5); // round, by this point adjRaw should fit
    }
    return adj;
}

//this is sketchy, should test
int8_t Charger::stepMPPT(uint16_t scaledSoftIlimit)
{
    int8_t adj = 0;
    // calculate current power (in ADC units)
    uint32_t currPower = panelV * panelI;
    // compare to prevPower
    // if current > previous make adjustment in same direction indicated by prevAdjustment
    if (currPower > prevPower) {
        adj = (prevAdjustment) ? 1 : -1;
        // prevAdjustment remains the same
    }
    // otherwise, switch direction
    else {
        adj = (prevAdjustment) ? -1 : 1;
        //prevAdjustment switches
        prevAdjustment = !prevAdjustment;
    }
    // check current limit
    if (panelI > scaledSoftIlimit) {
        adj = -1;
        prevAdjustment = false;
    }
    // update prevPower
    prevPower = currPower;

    return adj;

}
