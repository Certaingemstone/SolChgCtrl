#include <stdint.h>
#include <Arduino.h>

#include "solProtection.h"
#include "solControl.h"
#include "constants.h"

void setup() {
    // initial duty cycle out of 255
    volatile uint8_t duty = 0;

    // initial charger state set to 
    // 0 = nominal, 1 = overcurrent detected, or iteration dVdt too large
    volatile bool chargerFault = 0;

    // initialize relay off
    digitalWrite(battEnable, LOW);
    pinMode(battEnable, OUTPUT);
    // initialize MOSFET off; will constantly drain some power through panel
    digitalWrite(PWMpin, HIGH);
    pinMode(PWMpin, OUTPUT);
    // 32kHz PWM using Timer 1
    TCCR1B = TCCR1B & 0b11111000 | 0x01;

    Serial.begin(9600);
}


void loop() {
    // check status
    bool engage = statusOK(cutoffLow, cutoffHigh, battADCscale, battVpin, chargerFault);
    // either disengage or engage charger; disengaging will delay for 5 seconds before returning to this loop
    switch (engage) {
        case 0:
            disengage(battEnable, PWMpin, &duty);
            // clear charger fault if not already
            chargerFault = 0;
            break;
        case 1:
            digitalWrite(battEnable, HIGH);
            duty = 128; // start at around 50%
            chargerFault = runCharger(10);
            break;
        default:
            // something weird happened, just stop 
            disengage(battEnable, PWMpin, &duty);
            // and do not clear fault
            chargerFault = 1;
            break;
    }

}
