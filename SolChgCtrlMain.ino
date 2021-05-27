#include <stdint.h>
#include <Arduino.h>

#include "Protection.h"
#include "Charger.h"
#include "constants.h"

void setup() {
    //setup input; assumes pull down
    pinMode(default_inputPin, INPUT);
    pinMode(default_modePin, INPUT);


    // initialize relay off
    digitalWrite(default_battEnable, LOW);
    pinMode(default_battEnable, OUTPUT);
    // initialize MOSFET off by having PWM high; will constantly drain some power through panel
    digitalWrite(default_PWMpin, HIGH);
    pinMode(default_PWMpin, OUTPUT);
    // 32kHz PWM using Timer 1 for later calls to analogWrite
    TCCR1B = TCCR1B & 0b11111000 | 0x01;

    Serial.begin(9600);
}


void loop() {
    // to store duty cycle out of 255
    // when PWM is high, MOSFET is off: bigger duty = lower output voltage
    static uint8_t volatile duty = 255;

    // initialize charger object
    static Charger charger = Charger(&duty, default_panelVpin, default_panelIpin, default_battVpin);
    
    // 0 = nominal or manual exit, 1 = battery disconnected unexpectedly/voltage anomaly, 
    // 2 = Vds below safety limit, 3 = overcurrent/fatal
    static uint8_t chargerFault;
    // mode, by default is 0 = Lead acid charging, 1 = Manual override as constant voltage source
    static uint8_t chargerMode;

    chargerMode = 1; //TEMPORARY
    
    // reset charger state
    charger.resetMPPT();
    charger.updateState();

    // check status
    bool engage = Protection::startOK(cutoffLow[chargerMode], cutoffHigh[chargerMode], default_battADCscale, default_panelADCscale, default_battVpin, default_panelVpin, chargerFault);
    switch (engage) {
        case false:
            Serial.println("INFO: Not starting");
            Protection::disengage(default_battEnable, default_PWMpin, &duty); // includes 1 second delay
            Serial.print("Charger state ");
            Serial.println(chargerFault);
            // clear charger fault except for fault state 3
            if (chargerFault != 3) {
                chargerFault = 0;
                delay(10000); // re run check 10 second later
            } 
            else {
                Serial.println("WARNING: Overcurrent");
                delay(30000);
            }
            break;
        case true:
            // to track faults
            chargerFault = 0;
            uint8_t Vviolations = 0;
            uint8_t Iviolations = 0;
            
            bool running = true;
            
            //Sealed Lead Acid
            if (chargerMode == 0) {
              // tracking which stage of charging we're in
              uint8_t chargeStage = 1; // 0 = CC , 1 = CV, 2 = FLOAT
              charger.updateState(); // get updated state measurements
              if (charger.// check OCV
              Protection::engage(default_battEnable, default_PWMpin, &duty);
              while (running) {
                charger.updateState(); 
                
              }
            }
            
            //CV
            if (chargerMode == 1) {
                Protection::engage(default_battEnable, default_PWMpin, &duty);
                while (running) {
                    charger.updateState();
                    charger.updateDuty(charger.stepCV(CV_Kp, VADC_12), true); // get and apply proposed duty cycle step size
                    analogWrite(default_PWMpin, duty);
                    // 100 -> 2.14V, 1000 -> 21.14V (tolerate basically all output voltages)
                    // 50 -> 1.2A max
                    // 160 -> 3.4Vds min
                    chargerFault = Protection::runtimeOK(charger, &Vviolations, &Iviolations, 
                        100, 1000, 50, 50, 30, 160);
                    if (chargerFault != 0) {
                        running = false;
                    }
                    if (digitalRead(default_inputPin) == HIGH) {
                        running = false;
                    }
                    
                }
                
            }
            Serial.println("INFO: Disengaging");
            Protection::disengage(default_battEnable, default_PWMpin, &duty);
            delay(10000);
            break;
    }

}
