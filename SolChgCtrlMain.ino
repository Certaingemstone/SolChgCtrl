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
    static uint8_t chargerFault = 0;
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

            // For all the following, if overcurrent condition persists, will disengage charger (logic to be in Protection)
            // and set chargerFault 0 if nominal, 1 if load disconnected, 2 if Vds too low under load (e.g. <3.5V, >100mA), 3 if overcurrent

            //uint8_t runSLA(uint8_t stage, float Itarget, float Vtarget, float VtargetFC, float Ilimit);
            // charge procedure for sealed lead acid; stages are
            // 1 - Constant current at Itarget input current or lower, MPPT tracking enabled, transfer to 2 when reaching Vtarget
            // 2 - Constant voltage at Vtarget, transfer to 3 when current < 15% of Itarget
            // 3 - Float at VtargetFC until Vds threshold is reached (i.e. insufficient input voltage available)

            //uint8_t runLiIon(uint8_t stage, float Itarget, float Vtarget, float Ilimit);
            // charge procedure for sealed lead acid; stages are
            // 1 - Constant current at Itarget input current or lower
            // 2 - Constant voltage at Vtarget until current < 5% of Itarget or Vds threshold is reached
            
            //Sealed Lead Acid
            if (chargerMode == 0) {
                // deciding which stage of charging we're starting on
                uint8_t chargeStage = 1; // 0 = CC , 1 = CV, 2 = FLOAT, 3 = IDLE
                charger.updateState(); // get updated state measurements
                // do OCV checks: 
                // if starting below fast charge cutoff, start on current-limited MPPT and go through whole process
                // if starting between that and float target, start on constant-voltage float
                // if starting above float target, don't engage until below float
                if (charger.battV < default_VfastChargeCutoff) {
                    chargeStage = 0;
                    Serial.println("INFO: Charger engaging with mode MPPT");
                }
                else if (charger.battV < default_VtargetFC) {
                    chargeStage = 2;
                    Serial.println("INFO: Charger engaging with mode FLOAT");
                }
                else {
                    chargeStage = 3;
                }
                
                
                
                Protection::engage(default_battEnable, default_PWMpin, &duty); 
                while (running) {
                    
                    switch (chargeStage) {
                        case 0:
                            // TODO: Implement MPPT client
                            // TODO: Implement condition for transitioning
                                chargeStage = 1;
                                Serial.println("INFO: Charger switching to mode CV");
                        case 1:
                            // TODO: Implement CV client
                            // TODO: Implement condition for transitioning
                                chargeStage = 2;
                                Serial.println("INFO: Charger switching to mode FLOAT");
                        case 2:
                            charger.updateState();
                            Serial.println(charger.getCurrent());
                            charger.updateDuty(charger.stepCV(CV_Kp, default_VtargetFC), true);
                            analogWrite(default_PWMpin, duty);
                            // cutoffLow: 560 / 12V    cutoffHigh: 660 / 14.1V
                            // cutoffI: 105 / 2.5A
                            chargerFault = Protection::runtimeOK(charger, &Vviolations, &Iviolations,
                                560, 660, 105, 300, 30, 160);
                            break;
                        case 3:
                            // Idle until at or below float again
                            Serial.println("INFO: Charger idle, initial voltage over float target");
                            delay(5000);
                            if (charger.battV <= default_VtargetFC) {
                                chargeStage = 2;
                                Serial.println("INFO: Charger switching to mode FLOAT");
                            }

                        default:
                            Serial.println("ERROR: Undefined charge stage");
                            running = false;
                            break;
                    }
                    
                    if (chargerFault != 0) {
                        running = false;
                    }
                    if (digitalRead(default_inputPin) == HIGH) {
                        running = false;
                    }
                }
            }
            
            
            if (chargerMode == 1) {
                Protection::engage(default_battEnable, default_PWMpin, &duty);
                while (running) {
                    charger.updateState();
                    //Serial.println("Duty: ");
                    //Serial.println(duty);
                    //Serial.println("In voltage: ");
                    //Serial.println(charger.getInVoltage());
                    //Serial.println("Out voltage: ");
                    //Serial.println(charger.getOutVoltage());
                    Serial.println(charger.getCurrent());
                    charger.updateDuty(charger.stepCV(CV_Kp, VADC_12), true); // get and apply proposed duty cycle step size
                    analogWrite(default_PWMpin, duty);
                    // 100 -> 2.14V, 1000 -> 21.14V (tolerate basically all output voltages)
                    // 105 -> 2.5A max
                    // 160 -> 3.4Vds min
                    chargerFault = Protection::runtimeOK(charger, &Vviolations, &Iviolations, 
                        100, 1000, 105, 300, 30, 160);
                    
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
