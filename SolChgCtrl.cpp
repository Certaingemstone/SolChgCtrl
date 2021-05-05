#include <Arduino.h>

void setup() {
  // voltage at which to turn relay on or off
  const int cutoffLow = 9;
  const int cutoffHigh = 15;

  // target voltage for constant voltage output mode
  const float Vtarget = 14.1; // 2.35V per cell
  const float VtargetFC = 13.5; // float charge 2.25V per cell
  // target/limit current on input side
  const float Itarget = 1.0; // C/8 for 8Ah battery


  // ideal (uncalibrated) size of one ADC step (5/1024) since ADC goes from 0-1023
  const float ADCunit = 0.004888;

  // voltage calibration coefficients (e.g. 2 -> multiply by 2 to get true voltage)
  const float battVdivider = 4.379; // 300.1k and 1014k
  const float panelVdivider = 4.356; // 298k and 1000k
  // current shunt calibration coefficient
  const float currentdivider = 5; // 200x gain on the voltage drop; and V = 0.001Ohm * I

  // initial duty cycle out of 255
  volatile byte duty = 0;

  const byte PWMpin = 10;
  const byte panelVpin = A0;
  const byte panelIpin = A1;
  const byte battVpin = A2;
  const byte battEnable = 12;
  // initialize relay off
  digitalWrite(battEnable, LOW);
  pinMode(battEnable, OUTPUT);
  // initialize MOSFET off; will constantly drain some power through panel
  digitalWrite(PWMpin, HIGH);
  pinMode(PWMpin, OUTPUT);
  // 32kHz PWM using Timer 1
  TCCR1B = TCCR1B & 0b11111000 | 0x01;

  // some derived values
  const float battADCscale = ADCunit * battVdivider;
  const float panelADCscale = ADCunit * panelVdivider;
  const float currentADCscale = ADCunit * currentDivider;

  Serial.begin(9600);
}

// WORK IN PROGRESS
bool runCharger(int iterations) {
  // input number of times to run control loop
  // returns 0 if nominal function, returns 1 if large fluctuation in battery side voltage occurred

  // decide between CC, CV, or float
  int state = 0; // 0 for unknown, 1 for CC, 2 for CV, 3 for float
  // read battery state
  float Vbatt = analogRead(battVpin) * battADCscale;
  if (Vbatt < Vtarget && charged = 0) {
    state = 1;
  }
  else if ( )

  // execute constrained MPPT
  for (int i = 0; i < iterations; i++) {
    // read system state
    float Vpanel = analogRead(panelVpin) * panelADCscale;
    float Ipanel = analogRead(panelIpin) * currentADCscale;
    // maintain constant voltage 13V

    analogWrite(PWMpin, duty)
  }

}

void disengage() {
  digitalWrite(battEnable, LOW);
  duty = 0;
  analogWrite(PWMpin, 0);
  delay(5000);
}

void loop() {
  // PERFORM STATUS CHECKS
  bool engage = 0;
  // read battery side voltage
  float Vbatt = analogRead(battVpin) * battADCscale;

  // if between cutoffLow-cutoffHigh V, engage or remain engaged
  // if no battery or battery connected in reverse, voltage will be below cutoffLow; disengage
  if (Vbatt > cutoffLow && Vbatt < cutoffHigh) {
    engage = 1;
  } else {
    engage = 0;
  }
  // if battery is disconnected while engaged, large dV/dt would have been measured; disengage
  // then re-run status check to re-detect battery
  if (dVdtEvent) {
    engage = 0;
  }

  switch (engage) {
    case 0:
      disengage();
      break;
    case 1:
      digitalWrite(battEnable, HIGH);
      dVdtEvent = runCharger(10);
      break;
    default:
      disengage();
      break;
  }

}
