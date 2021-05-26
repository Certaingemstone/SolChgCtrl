#include <stdint.h>
#include <Arduino.h>

#include "Protection.h"
#include "Charger.h"
#include "constants.h"

void setup() {
    // initial duty cycle out of 255
    // when PWM is high, MOSFET is off: bigger duty = lower output voltage
    uint8_t volatile duty = 255;

    // initial charger state set to
    // 0 = nominal or manual exit, 1 = battery disconnected unexpectedly/voltage anomaly, 
    // 2 = Vds below safety limit, 3 = overcurrent/fatal
    uint8_t chargerFault = 0;

    // mode, by default is 0 = Lead acid charging, 1 = Manual override as constant voltage source
    uint8_t chargerMode = 1;

    //setup input; assumes pull down
    pinMode(default_inputPin, INPUT);
    pinMode(default_modePin, INPUT);

    // initialize relay off
    digitalWrite(default_battEnable, LOW);
    pinMode(default_battEnable, OUTPUT);
    // initialize MOSFET off by having PWM high; will constantly drain some power through panel
    digitalWrite(default_PWMpin, HIGH);
    pinMode(default_PWMpin, OUTPUT);
    // 32kHz PWM using Timer 1
    TCCR1B = TCCR1B & 0b11111000 | 0x01;
    analogWrite(default_PWMpin, duty);

    Serial.begin(9600);
}


void loop() {
    // check mode; Pb (default), manual override, or other?
    // TODO, will set chargerMode to appropriate value based on user input

    // check status
    bool engage = Protection::startOK(cutoffLow[chargerMode], cutoffHigh[chargerMode], default_battADCscale, default_battVpin, chargerFault);
    // either disengage or engage charger; disengaging will delay for 5 seconds before returning to this loop
    // engage will transfer control to charging, which will return to this loop when it decides to disconnect battery
    switch (engage) {
        case false:
            Serial.println("INFO: Disengaging charger");
            Protection::disengage(default_battEnable, default_PWMpin, &duty);
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
        case true:
            Protection::engage(default_battEnable, default_PWMpin, &duty);
            
            // check 
            Serial.println("INFO:");
            chargerFault = runCharger(); //WIP
            break;
    }

}
