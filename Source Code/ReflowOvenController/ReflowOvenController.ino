// Copyright Felix van Oost 2015.
// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
// See the GNU General Public License for more details.

// Reflow Oven Controller for the Arduino Uno

#include <avr/pgmspace.h>

#define thermPin            A0                                                // Thermocouple input pin (A0)
#define junctionPin         A1                                                // Cold junction input pin (A1)

#define setButtonPin        2                                                 // Pushbutton input pins (D2-D4)
#define incButtonPin        3
#define decButtonPin        4
#define ovenPin             5                                                 // Oven SSR pin (D5)
#define buzzerPin           6                                                 // Buzzer output pin (D6)
#define LED1Pin             7                                                 // LED output pins (D7-D8)
#define LED2Pin             8

#define BUTTON_DELAY        120                                               // Pushbutton delay for increment / decrement (ms)
#define DEBOUNCE_DELAY      50                                                // Pushbutton debounce delay (ms)

#define LOW_BUZZER_FREQ     3000                                              // Buzzer frequencies (Hz)
#define MID_BUZZER_FREQ     4000
#define HIGH_BUZZER_FREQ    5000
#define SHORT_BEEP          100                                               // Buzzer beep lengths (ms)
#define MEDIUM_BEEP         400
#define LONG_BEEP           3000

#define MIN_SOAK_TEMP       120                                               // Minimum values for thermal profile parameters
#define MIN_SOAK_TIME       30
#define MIN_REFLOW_TEMP     200
#define MIN_REFLOW_TIME     15

#define MAX_SOAK_TEMP       180                                               // Maximum values for thermal profile parameters
#define MAX_SOAK_TIME       90
#define MAX_REFLOW_TEMP     240
#define MAX_REFLOW_TIME     60

#define OFF                 0                                                 // Finite state machine states
#define RAMP_TO_SOAK        1
#define SOAK                2
#define RAMP_TO_REFLOW      3
#define REFLOW              4
#define COOLING             5

#define SOAK_DUTY_CYCLE     25                                                // Oven PWM duty cycle for soak state (0-255)
#define REFLOW_DUTY_CYCLE   51                                                // Oven PWM duty cycle for reflow state (0-255)

#define SOAK_TEMP_OFFSET    15                                                // Soak temperature offset (to account for embodied heat in oven at soak stage)
#define REFLOW_TEMP_OFFSET  8                                                 // Reflow temperature offset (to account for embodied heat in oven at reflow stage)

#define ERROR_TEMP          50                                                // Thermocouple error temperature threshold (stops reflow process if specified temperature is not reached within the first 30 seconds)
#define SAFE_TO_HANDLE_TEMP 60                                                // Safe-to-handle temperature

byte state = OFF;

byte soakTemp = 150;                                                          // Declare thermal profile parameter variables and initialise to default values
byte soakTime = 60;
byte reflowTemp = 220;
byte reflowTime = 45;

byte thermTemp = 0;                                                           // Declare temperature variables
byte junctionTemp = 0;
byte ovenTemp = 0;

volatile unsigned int processTime = 0;                                        // Declare time variables
volatile byte stateTime = 0;

// 0-255C LUT for K-type thermocouple (stored in program memory)
const int thermLUT[] PROGMEM = {0,    18,   37,   56,   74,   93,   112,  130,  149,  168,
                                187,  205,  224,  243,  262,  281,  299,  318,  337,  356,
                                375,  394,  413,  432,  451,  470,  489,  508,  527,  547,
                                565,  585,  604,  623,  642,  661,  681,  700,  719,  738,
                                758,  777,  796,  815,  835,  854,  873,  893,  912,  932,
                                951,  970,  990,  1009, 1028, 1048, 1067, 1087, 1106, 1126,
                                1145, 1165, 1184, 1204, 1223, 1243, 1262, 1282, 1301, 1321,
                                1340, 1360, 1379, 1399, 1418, 1438, 1457, 1477, 1496, 1516,
                                1535, 1555, 1575, 1594, 1614, 1633, 1653, 1672, 1692, 1711,
                                1731, 1750, 1770, 1789, 1809, 1828, 1848, 1867, 1886, 1906,
                                1925, 1945, 1964, 1983, 2003, 2022, 2042, 2061, 2081, 2100,
                                2119, 2139, 2158, 2178, 2197, 2216, 2235, 2255, 2274, 2293,
                                2312, 2332, 2351, 2370, 2389, 2408, 2428, 2447, 2466, 2485,
                                2504, 2523, 2543, 2562, 2581, 2600, 2619, 2638, 2657, 2676,
                                2695, 2714, 2733, 2752, 2771, 2790, 2809, 2828, 2847, 2866,
                                2885, 2904, 2923, 2942, 2961, 2979, 2999, 3017, 3036, 3055,
                                3074, 3093, 3111, 3130, 3149, 3168, 3187, 3206, 3225, 3243,
                                3262, 3281, 3300, 3318, 3337, 3356, 3375, 3393, 3412, 3431,
                                3450, 3469, 3487, 3506, 3525, 3544, 3562, 3581, 3600, 3619,
                                3637, 3656, 3675, 3694, 3713, 3731, 3750, 3769, 3788, 3807,
                                3825, 3844, 3862, 3881, 3900, 3919, 3938, 3956, 3975, 3995,
                                4013, 4032, 4051, 4070, 4089, 4107, 4126, 4145, 4164, 4183,
                                4202, 4221, 4239, 4259, 4277, 4296, 4315, 4334, 4353, 4372,
                                4391, 4410, 4429, 4448, 4467, 4486, 4505, 4524, 4543, 4562,
                                4581, 4600, 4619, 4638, 4657, 4677, 4696, 4715, 4734, 4753,
                                4772, 4791, 4810, 4830, 4849, 4868};

void setup() 
{
  pinMode(thermPin, INPUT);                                                   // Declare digital pins as inputs / outputs
  pinMode(junctionPin, INPUT);
  pinMode(setButtonPin, INPUT);
  pinMode(incButtonPin, INPUT);
  pinMode(decButtonPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ovenPin, OUTPUT);
  pinMode(LED1Pin, OUTPUT);
  pinMode(LED2Pin, OUTPUT);
  
  digitalWrite(LED1Pin, 0);                                                   // Turn off LEDs
  digitalWrite(LED2Pin, 0); 
  
  TCCR0B = _BV(CS00) | _BV(CS02);                                             // Set Timer 0 prescaling to CLK / 1024 (reduces PWM frequency to 60Hz for oven SSR output)
  
  Serial.begin(9600);                                                         // Initialise serial port at 9600 baud
}

// Description:		Obtains an analog reading for the cold junction from the LM35Z and converts it to a temperature in Celsius 
// Parameters:		-
// Returns:		-
void getJunctionTemp()
{
  double junctionReading = 0;
  
  junctionReading = analogRead(junctionPin);                                  // Obtain 10-bit cold junction reading from the ADC
  junctionReading = ((junctionReading * 5.0 * 100.0) / 1023.0);               // Convert reading to temperature in Celsius
  junctionTemp = byte(junctionReading);
  
  return;
}

// Description:		Obtains an analog reading for the thermcouple and converts it to a temperature in Celsius using a LUT
// Parameters:		-
// Returns:		-
void getThermTemp()
{
  double thermReading = 0;
  byte i = 0;
  
  thermReading = analogRead(thermPin);                                        // Obtain 10-bit thermocouple reading from the ADC
  thermReading = ((thermReading * 5.0 * 1000.0) / 1023.0);                    // Convert reading to voltage (in mV)
  
  while(int(thermReading) > pgm_read_word_near(thermLUT + i))                 // Iterate through LUT to find the entry closest to the reading voltage
  {
    i++;
  }
  thermTemp = i;                                                              // Index of the LUT entry closest to the reading voltage is the temperature in Celsius
  
  return;
}

// Description:		Interrupt Service Routine for Timer 1
//                      1. Increments the total process time and state time every second
//                      2. Obtains temperature readings for the thermocouple and cold junction and calculates the current oven temperature
//                      3. Displays the current oven temperature in HyperTerminal
ISR(TIMER1_OVF_vect)
{
  processTime++;                                                              // Increment total process and state time
  stateTime++;
  
  getThermTemp();                                                             // Obtain temperature readings for the thermocouple and cold junction
  getJunctionTemp();
  ovenTemp = thermTemp + junctionTemp;                                        // Calculate current oven temperature
  Serial.println(ovenTemp);                                                   // Display current oven temperature
}

// Description:		Initialises Timer 1 to interrupt every second
// Parameters:		-
// Returns:		-
void initialiseTimer1()
{
  cli();                                                                      // Disable global interrupts
  TCCR1A = 0;                                                                 // Reset Timer 1 registers
  TCCR1B = 0;
  TIMSK1 |= (1 << TOIE1);                                                     // Enable Timer 1 overflow interrupt
  TCNT1 = 0x0BDB;                                                             // Preload Timer 1 with 3035 to produce overflow at 1Hz
  TCCR1B |= (1 << CS12);                                                      // Select clock source as internal 12MHz oscillator with CLK / 256 prescaling and start Timer 1
  sei();                                                                      // Enable global interrupts
}

// Description:		Reads the status of the pushbuttons and increments / decrements the given value within the specified minimum / maximum ranges until the set button is pressed
// Parameters:		value - Initial value of the thermal profile parameter currently being set
//                      minimum - Minimum acceptable value for specified parameter
//                      maximum - Maximum acceptable value for specified parameter
// Returns:	        The final thermal profile parameter value entered by the user
int readButtons(int value, int minimum, int maximum)
{
  while(digitalRead(setButtonPin) != 0)
  {
    if(digitalRead(incButtonPin) == 0 && value < maximum)
    {
      value++;                                                                // Increment parameter if increment button is pressed
    }
    if(digitalRead(decButtonPin) == 0 && value > minimum)
    {
      value--;                                                                // Decrement parameter if decrement button is pressed
    }
    Serial.print(value);                                                      // Display parameter in serial monitor
    Serial.print("\r");                                                       // Return to start of line in serial monitor
    delay(BUTTON_DELAY);
  }
  while(digitalRead(setButtonPin) == 0);                                      // Wait for set button to be released
  delay(DEBOUNCE_DELAY);
  
  return value;
}

// Description:		Allows user to set the four thermal profile parameters using pusbuttons and provides user feedback via the buzzer
// Parameters:		-
// Returns:		-
void setParameters()
{
  Serial.write("\x1b[2J");                                                    // ANSI sequence to clear screen in HyperTerminal
  Serial.println("Soak Temperature:");
  soakTemp = readButtons(soakTemp, MIN_SOAK_TEMP, MAX_SOAK_TEMP);             // Set soak temperature
  Serial.println(soakTemp);                                                   // Display entered soak temperature
  tone(buzzerPin, HIGH_BUZZER_FREQ, SHORT_BEEP);

  Serial.println("Soak Time:");
  soakTime = readButtons(soakTime, MIN_SOAK_TIME, MAX_SOAK_TIME);             // Set soak time
  Serial.println(soakTime);                                                   // Display entered soak time
  tone(buzzerPin, HIGH_BUZZER_FREQ, SHORT_BEEP);

  Serial.println("Reflow Temperature:");
  reflowTemp = readButtons(reflowTemp, MIN_REFLOW_TEMP, MAX_REFLOW_TEMP);     // Set reflow temperature
  Serial.println(reflowTemp);                                                 // Display entered reflow temperature
  tone(buzzerPin, HIGH_BUZZER_FREQ, SHORT_BEEP);

  Serial.println("Reflow Time:");
  reflowTime = readButtons(reflowTime, MIN_REFLOW_TIME, MAX_REFLOW_TIME);     // Set reflow time
  Serial.println(reflowTime);                                                 // Display entered reflow time
  tone(buzzerPin, HIGH_BUZZER_FREQ, SHORT_BEEP);
  
  Serial.println();
  Serial.println("Press 'set' to begin reflow process");

  return;
}

void loop() 
{
  switch(state)                                                               // Finite state machine control logic
  {
    // Idle state
    case OFF:
    {
      TCCR1B = 0;                                                             // Stop Timer 1
      processTime = 0;                                                        // Reset process time
      digitalWrite(ovenPin, 0);                                               // Turn off oven
      digitalWrite(LED1Pin, 0);                                               // Turn off LEDs
      digitalWrite(LED2Pin, 0);
      setParameters();
      
      while(digitalRead(setButtonPin) != 0);                                  // Wait for set button to be pressed to begin the reflow process
      while(digitalRead(setButtonPin) == 0);                                  // Wait for set button to be released      
      delay(DEBOUNCE_DELAY);
      Serial.println();
      Serial.println("Starting reflow process");

      tone(buzzerPin, LOW_BUZZER_FREQ, MEDIUM_BEEP);                          // Play start melody
      delay(SHORT_BEEP);
      tone(buzzerPin, MID_BUZZER_FREQ, MEDIUM_BEEP);
      delay(SHORT_BEEP);
      tone(buzzerPin, HIGH_BUZZER_FREQ, MEDIUM_BEEP);
     
      initialiseTimer1();
      stateTime = 0;                                                          // Reset state time
      Serial.println();                                                       // Display new state
      Serial.println("RAMP TO SOAK");
      Serial.println();
      state = RAMP_TO_SOAK;                                                   // Enter ramp to soak state
      break;
    }
    // Ramp to soak state
    case RAMP_TO_SOAK:
    { 
      analogWrite(ovenPin, 255);                                              // Set oven duty cycle to 100%
      digitalWrite(LED2Pin, 1);
      
      if(digitalRead(setButtonPin) == 0)                                      // Stop reflow process if set button is pressed
      {
        while(digitalRead(setButtonPin) == 0);                                // Wait for set button to be released
        state = OFF;
        break;
      }
      if(stateTime == 30 && ovenTemp < ERROR_TEMP)                            // Stop reflow process if oven does not reach the specified error temperature in the first 30s (thermocouple error)
      {
        state = OFF;
        break;
      }
      if(ovenTemp > soakTemp - SOAK_TEMP_OFFSET)                              // Turn off oven prematurely to account for embodied heat
      {
        digitalWrite(ovenPin, 0);
        digitalWrite(LED2Pin, 0);
      }
      if(ovenTemp > soakTemp)                                                 // Enter soak state once soak temperature has been reached
      {
        stateTime = 0;                                                        // Reset state time
        Serial.println();                                                     // Display new state
        Serial.println("SOAK");
        Serial.println();
        tone(buzzerPin, MID_BUZZER_FREQ, MEDIUM_BEEP);
        state = SOAK;
      }
      break;
    }
    // Soak state
    case SOAK:
    {
      analogWrite(ovenPin, SOAK_DUTY_CYCLE);                                  // Set corresponding oven PWM duty cycle
      
      if(digitalRead(setButtonPin) == 0)                                      // Stop reflow process if set button is pressed
      {
        while(digitalRead(setButtonPin) == 0);                                // Wait for set button to be released
        state = OFF;
        break;
      }
      if(stateTime == soakTime)                                               // Enter ramp to reflow state once soak time has elapsed
      {
        stateTime = 0;                                                        // Reset state time
        Serial.println();                                                     // Display new state
        Serial.println("RAMP TO REFLOW");
        Serial.println();
        tone(buzzerPin, MID_BUZZER_FREQ, MEDIUM_BEEP);
        state = RAMP_TO_REFLOW;
      }
      break;
    }
    // Ramp to reflow state
    case RAMP_TO_REFLOW:
    {
      analogWrite(ovenPin, 255);                                              // Set oven PWM duty cycle to 100%
      
      if(digitalRead(setButtonPin) == 0)                                      // Stop reflow process if set button is pressed
      {
        while(digitalRead(setButtonPin) == 0);                                // Wait for set button to be released
        state = OFF;
        break;
      }
      if(ovenTemp > reflowTemp - REFLOW_TEMP_OFFSET)                          // Turn off oven prematurely to account for embodied heat
      {
        digitalWrite(ovenPin, 0);
        digitalWrite(LED2Pin, 0);
      }
      if(ovenTemp > reflowTemp)                                               // Enter reflow state once reflow temperature has been reached
      {
        stateTime = 0;                                                        // Reset state time
        Serial.println();                                                     // Display new state
        Serial.println("REFLOW");
        Serial.println();
        tone(buzzerPin, MID_BUZZER_FREQ, MEDIUM_BEEP);
        state = REFLOW;
      }
      break;
    }
    // Reflow state
    case REFLOW:
    {
      analogWrite(ovenPin, REFLOW_DUTY_CYCLE);                                // Set corresponding oven PWM duty cycle
      
      if(digitalRead(setButtonPin) == 0)                                      // Stop reflow process if set button is pressed
      {
        while(digitalRead(setButtonPin) == 0);                                // Wait for set button to be released
        state = OFF;
        break;
      }
      if(stateTime == reflowTime)                                             // Enter cooling state once reflow time has elapsed
      {
        stateTime = 0;                                                        // Reset state time
        Serial.println();                                                     // Display new state
        Serial.println("COOLING");
        Serial.println();
        Serial.println("Please open oven door");
        Serial.println("Press 'set' to confirm door has been opened");
        
        while(digitalRead(setButtonPin) != 0);
        {
          tone(buzzerPin, MID_BUZZER_FREQ);                                   // Alert user to open oven door
        }
        while(digitalRead(setButtonPin) == 0);                                // Wait for set button to be released before turning off buzzer
        noTone(buzzerPin);
        state = COOLING;
      }
      break;
    }
    // Cooling state
    case COOLING:
    {
      digitalWrite(ovenPin, 0);                                               // Turn oven off
      digitalWrite(LED2Pin, 0);
      
      if(ovenTemp < SAFE_TO_HANDLE_TEMP)                                      // Finish reflow process once safe-to-handle temperature is reached
      {
        stateTime = 0;                                                        // Reset state time
        TCCR1B = 0;                                                           // Stop Timer 1
        
        Serial.println();
        Serial.println("Reflow process complete");
        
        tone(buzzerPin, HIGH_BUZZER_FREQ, MEDIUM_BEEP);                       // Play end melody
        delay(SHORT_BEEP);
        tone(buzzerPin, MID_BUZZER_FREQ, MEDIUM_BEEP);
        delay(SHORT_BEEP);
        tone(buzzerPin, LOW_BUZZER_FREQ, MEDIUM_BEEP);
        
        Serial.print("Total process time: ");                                 // Display total process time
        Serial.print(processTime);
        Serial.print(" seconds");
        Serial.println();
        Serial.println("Press 'set' to return to start");
        
        while(digitalRead(setButtonPin) != 0);                                // Wait for set button to be pressed
        while(digitalRead(setButtonPin) == 0);                                // Wait for set button to be released
        state = OFF;                                                          // Return to off state
      }
      break;
    }
  }
}
