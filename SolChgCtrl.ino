void setup() { 
  // voltage at which to turn relay on or off
  int cutoffLow = 9;
  int cutoffHigh = 14;

  // ideal (uncalibrated) size of one ADC step (5/1023) since ADC goes from 0-1023
  float ADCunit = 0.004888;
  
  // voltage calibration coefficients (e.g. 2 -> multiply by 2 to get true voltage)
  float battVdivider = 4.379; // 300.1k and 1014k
  float panelVdivider = 4.356; // 298k and 1000k
  // current shunt calibration coefficient
  float currentdivider = 5; // 200x gain on the voltage drop; and V = 0.001Ohm * I
  
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

  Serial.begin(9600);

  // some derived values
  float battADCscale = ADCunit * battVdivider;
  float panelADCscale = ADCunit * panelVdivider;
  float currentADCscale = ADCunit * currentDivider;
}

bool runCharger(int iterations) {
  // input number of times to run control loop
  // returns 0 if nominal function, returns 1 if large fluctuation in battery side voltage occurred
  for (int i = 0; i < iterations; i++) {
    // read panel state
    float Vpanel = analogRead(panelVpin) * panelADCscale;
    float Ipanel = analogRead(panelIpin) * currentADCscale;
  }
    
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
      digitalWrite(battEnable, LOW);
      delay(5000);
      break;
    case 1:
      digitalWrite(battEnable, HIGH);
      dVdtEvent = runCharger(10);
      break;
    default:
      digitalWrite(battEnable, LOW);
      delay(5000);
      break;
  }
  
}
