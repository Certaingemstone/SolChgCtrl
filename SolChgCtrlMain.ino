#include <stdint.h>
#include <Arduino.h>

#include "solProtection.h"
#include "solControl.h"
#include "constants.h"

void setup() {
    // initial duty cycle out of 255
    // when PWM is high, MOSFET is off: bigger duty = lower output voltage
    volatile uint8_t duty = 255;

    // initial charger state set to
    // 0 = nominal, 1 = battery disconnected unexpectedly/voltage anomaly, 2 = Vds below safety limit, 3 = overcurrent/fatal
    volatile uint8_t chargerFault = 0;

    // mode, by default is 0 = Lead acid charging, 1 = Manual override as constant voltage source
    uint8_t chargerMode = 0;

    // initialize relay off
    digitalWrite(battEnable, LOW);
    pinMode(battEnable, OUTPUT);
    // initialize MOSFET off by having PWM high; will constantly drain some power through panel
    digitalWrite(PWMpin, HIGH);
    pinMode(PWMpin, OUTPUT);
    // 32kHz PWM using Timer 1
    TCCR1B = TCCR1B & 0b11111000 | 0x01;

    Serial.begin(9600);
}


void loop() {
    // check mode; Pb (default), manual override, or other?
    // TODO, will set chargerMode to appropriate value based on user input

    // check status
    bool engage = startOK(cutoffLow[chargerMode], cutoffHigh[chargerMode], battADCscale, battVpin, chargerFault);
    // either disengage or engage charger; disengaging will delay for 5 seconds before returning to this loop
    // engage will transfer control to charging, which will return to this loop when it decides to disconnect battery
    switch (engage) {
        case 0:
            Serial.println("INFO: Disengaging charger")
            disengage(battEnable, PWMpin, &duty);
            // clear charger fault except for fault state 3
            if (chargerFault != 3) {
                chargerFault = 0;
            } 
            else {
                //display error message
                Serial.println("WARNING: Overcurrent occurred");
                delay(30000);
            }
            break;
        case 1:
            digitalWrite(battEnable, HIGH);
            duty = 128; // start at around 50%
            //need to rewrite this section to handle different modes
            Serial.println("INFO:")
            chargerFault = runCharger(10);
            break;
        default:
            // something weird happened, just stop
            Serial.println("WARNING: Undefined behavior")
            disengage(battEnable, PWMpin, &duty);
            // and stop everything afterwards
            chargerFault = 3;
            break;
    }

}
