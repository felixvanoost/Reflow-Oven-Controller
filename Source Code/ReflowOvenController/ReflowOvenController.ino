// Copyright Felix van Oost 2015.
// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
// See the GNU General Public License for more details.

// Reflow Oven Controller vX.XX

#include <avr/pgmspace.h>

#define thermPin                 A0                                           // Thermocouple input pin (A0)
#define junctionPin              A1                                           // Cold junction input pin (A1)

#define setButtonPin             2                                            // Pushbutton input pins (D2-D4)
#define Button1Pin               3
#define Button2Pin               4
#define ovenPin                  5                                            // Oven SSR pin (D5)
#define buzzerPin                6                                            // Buzzer output pin (D6)
#define LED1Pin                  7                                            // LED output pins (D7-D8)
#define ovenLEDPin               8

#define DEBOUNCE_DELAY           100                                          // Pushbutton debounce delay (ms)

#define LOW_BUZZER_FREQ          3000                                         // Buzzer frequencies (Hz)
#define MID_BUZZER_FREQ          4000
#define HIGH_BUZZER_FREQ         5000
#define SHORT_BEEP               100                                          // Buzzer beep lengths (ms)
#define MEDIUM_BEEP              400
#define LONG_BEEP                3000

#define OFF                      0                                            // Finite-state machine states
#define RAMP_TO_SOAK             1
#define SOAK                     2
#define RAMP_TO_REFLOW           3
#define REFLOW                   4
#define COOLING                  5

#define READING_FREQUENCY        5                                            // Frequency of temperature readings (1, 2, 4, 5, 10, 20, 25, 50, or 100Hz)

#define SOAK_DUTY_CYCLE          10                                           // Oven PWM duty cycle for soak state (%)
#define REFLOW_DUTY_CYCLE        20                                           // Oven PWM duty cycle for reflow state (%)

#define SOAK_TEMP_OFFSET         15                                           // Soak temperature offset (to account for embodied heat in oven at soak stage)
#define REFLOW_TEMP_OFFSET       8                                            // Reflow temperature offset (to account for embodied heat in oven at reflow stage)

#define ERROR_TEMP               50                                           // Thermocouple error temperature threshold (stops reflow process if not reached within the first 30 seconds)
#define SAFE_TO_HANDLE_TEMP      60                                           // Safe-to-handle temperature

byte state = OFF;                                                             // Declare finite-state machine state variable

byte soakTemp;                                                                // Declare thermal profile parameter variables
byte soakTime;
byte reflowTemp;
byte reflowTime;

int ovenTempSum = 0;                                                          // Declare temperature variables
int ovenTemp = 0;

volatile unsigned int cycleTime = 0;                                          // Declare time variables
volatile byte stateTime = 0;

volatile byte interruptCount = 0;                                             // Declare Timer 1 interrupt count variable
volatile byte readingIntervalCount = 0;                                       // Declare Timer 1 reading interval count variable
volatile byte PWMValue = 0;                                                   // Declare PWM duty cycle variable for oven SSR control

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
  pinMode(thermPin, INPUT);                                                   // Declare thermocouple and temperature sensor pins as inputs
  pinMode(junctionPin, INPUT);
  pinMode(setButtonPin, INPUT_PULLUP);                                        // Declare pushbutton pins as inputs with internal pull-up resistors enabled
  pinMode(Button1Pin, INPUT_PULLUP);
  pinMode(Button2Pin, INPUT_PULLUP);
  pinMode(ovenPin, OUTPUT);                                                   // Declare oven, buzzer, and LED pins as outputs
  pinMode(buzzerPin, OUTPUT);
  pinMode(LED1Pin, OUTPUT);
  pinMode(ovenLEDPin, OUTPUT);
  
  digitalWrite(LED1Pin, 0);                                                   // Turn off LEDs
  digitalWrite(ovenLEDPin, 0); 
  
  Serial.begin(9600);                                                         // Initialise serial port at 9600 baud
}

// Description:		Obtains an analog reading for the cold junction from the LM35Z and converts it to a temperature in Celsius.
// Parameters:		-
// Returns:		    A temperature reading for the cold junction in Celsius.
byte getJunctionTemp()
{
  double junctionReading = 0;
  
  junctionReading = analogRead(junctionPin);                                  // Obtain 10-bit cold junction reading from the ADC
  junctionReading = ((junctionReading * 5.0 * 100.0) / 1023.0);               // Convert reading to temperature in Celsius
  
  return byte(junctionReading);
}

// Description:		Obtains an analog reading for the thermcouple and converts it to a temperature in Celsius using a LUT.
// Parameters:		-
// Returns:		    A temperature reading for the thermocouple in Celsius.
byte getThermTemp()
{
  double thermReading = 0;
  byte i = 0;
  
  thermReading = analogRead(thermPin);                                        // Obtain 10-bit thermocouple reading from the ADC
  thermReading = ((thermReading * 5.0 * 1000.0) / 1023.0);                    // Convert reading to voltage (in mV)
  
  while(int(thermReading) > pgm_read_word_near(thermLUT + i))                 // Iterate through LUT to find the entry closest to the reading voltage
  {
    i++;
  }
  
  return i;                                                                   // Index of the LUT entry closest to the reading voltage is the temperature in Celsius
}

// Description:		CTC Interrupt Service Routine for Timer 1
//                      1. Obtains temperature readings for the thermocouple and cold junction at an defined frequency and calculates the average to smooth out irregularities in the data.
//                      2. Increments the total cycle time and state time every second.
//                      3. Calculates the current oven temperature every second and sends the current oven temperature to Python.
//                      4. Controls the oven using a slow form of PWM (due to switching speed limitations of the SSR).
ISR(TIMER1_COMPA_vect)
{
  interruptCount++;
  readingIntervalCount++;
  
  if(readingIntervalCount == (100 / READING_FREQUENCY))                       // Obtain temperature readings for the thermocouple and cold junction at regular intervals and calculate cumulative sum
  {
    ovenTempSum += /*getThermTemp() +*/ getJunctionTemp();
    readingIntervalCount = 0;
  }
  
  if(interruptCount == 100)                                                   // Execute code every second (100 timer overflows)
  {
    cycleTime++;                                                              // Increment total cycle and state time variables
    stateTime++;
  
    ovenTemp = ovenTempSum / READING_FREQUENCY;                               // Calculate current oven temperature by finding the average value of previously gathered temperature readings
    Serial.print(ovenTemp);                                                   // Display current oven temperature
    Serial.print("\n");
    
    interruptCount = 0;                                                       // Reset interrupt count and sum of oven temperatu"re readings
    ovenTempSum = 0;
  }
  digitalWrite(ovenPin, interruptCount > PWMValue ? 0:1);                     // Turn off oven if interruptCount > PWMValue or turn on if interruptCount < PWMValue
  digitalWrite(ovenLEDPin, interruptCount > PWMValue ? 0:1);                  // Toggle oven LED to mirror oven state
}

// Description:		Initialises Timer 1 to interrupt every 10ms.
// Parameters:		-
// Returns:		    -
void initialiseTimer1()
{
  cli();                                                                      // Disable global interrupts
  TCCR1A = 0;                                                                 // Reset Timer 1 registers
  TCCR1B = 0;
  TCNT1 = 0;                                                                  // Load Timer 1 with 0
  OCR1A = 0x4E1F;                                                             // Load output compare register with 19,999 to create a compare match interrupt every 10ms
  TIMSK1 |= (1 << OCIE1A);                                                    // Enable Timer 1 CTC (Clear Timer on Compare) interrupt
  TCCR1B |= (1 << WGM12);                                                     // Select CTC mode
  TCCR1B |= (1 << CS11);                                                      // Select internal 16MHz oscillator with CLK / 8 prescaling as clock source and start Timer 1
  sei();                                                                      // Enable global interrupts
}

// Description:		1. Receives a thermal profile parameter from Python via the serial port.
//                2. Echoes the received value back to Python to ensure that the correct data has been received.
//                3. Provides user feedback via the buzzer.
// Parameters:		-
// Returns:		    The value of the received thermal profile parameter.
int receiveParameter()
{
  int value = 0;
  
  while(value == 0)
  {
    if(Serial.available() > 0)                                                // Check if data has already arrived in the serial receive buffer
    {
      value = Serial.parseInt();                                              // Read data from the buffer and convert to an integer
      Serial.print(value);                                                    // Echo received value back to Python
      Serial.print("\n");
    }
  }
  tone(buzzerPin, HIGH_BUZZER_FREQ, SHORT_BEEP);
  
  return byte(value);
}

void loop() 
{
  switch(state)                                                               // Finite-state machine control logic
  {
    // Idle state
    case OFF:
    {
      cycleTime = 0;                                                          // Reset cycle time
      digitalWrite(LED1Pin, 0);
      TCCR1B = 0;                                                             // Stop Timer 1
      
      soakTemp = receiveParameter();                                          // Obtain thermal profile parameters
      soakTime = receiveParameter();
      reflowTemp = receiveParameter();
      reflowTime = receiveParameter();
      
      while(digitalRead(setButtonPin) != 0);                                  // Wait for set button to be pressed to begin the reflow process
      while(digitalRead(setButtonPin) == 0);                                  // Wait for set button to be released      
      delay(DEBOUNCE_DELAY);
      digitalWrite(LED1Pin, 1);                                               // Turn on LED1
      Serial.print("Start\n");                                                // Send 'start' flag to Python

      tone(buzzerPin, LOW_BUZZER_FREQ, MEDIUM_BEEP);                          // Play start melody
      delay(SHORT_BEEP);
      tone(buzzerPin, MID_BUZZER_FREQ, MEDIUM_BEEP);
      delay(SHORT_BEEP);
      tone(buzzerPin, HIGH_BUZZER_FREQ, MEDIUM_BEEP);
     
      initialiseTimer1();
      stateTime = 0;                                                          // Reset state time
      state = RAMP_TO_SOAK;                                                   // Enter ramp to soak state
      break;
    }
    // Ramp to soak state
    case RAMP_TO_SOAK:
    { 
      PWMValue = 100;                                                         // Set oven duty cycle to 100%
      
      if(digitalRead(setButtonPin) == 0)                                      // Stop reflow process if set button is pressed
      {
        while(digitalRead(setButtonPin) == 0);                                // Wait for set button to be released
        delay(DEBOUNCE_DELAY);
        PWMValue = 0;
        Serial.print("Stop\n");                                               // Send 'stop' flag to Python
        delay(10);                                                            // Ensure Timer 1 ISR is given enough time to execute and turn off the oven before it is stopped (upon returning to Idle state)
        state = OFF;
        break;
      }
      if(stateTime == 30 && ovenTemp < ERROR_TEMP)                            // Stop reflow process if oven does not reach the specified error temperature in the first 30s (thermocouple error)
      {
        PWMValue = 0;
        Serial.print("Therm\n");                                              // Send 'thermocouple error' flag to Python
        delay(10);                                                            // Ensure Timer 1 ISR is given enough time to execute and turn off the oven before it is stopped (upon returning to Idle state)
        state = OFF;
        break;
      }
      if(ovenTemp > soakTemp - SOAK_TEMP_OFFSET)                              // Turn off oven prematurely to account for embodied heat
      {
        PWMValue = 0;
      }
      if(ovenTemp > soakTemp)                                                 // Enter soak state once soak temperature has been reached
      {
        stateTime = 0;                                                        // Reset state time
        tone(buzzerPin, MID_BUZZER_FREQ, MEDIUM_BEEP);
        state = SOAK;
      }
      break;
    }
    // Soak state
    case SOAK:
    {
      PWMValue = SOAK_DUTY_CYCLE;                                             // Set corresponding oven PWM duty cycle
      
      if(digitalRead(setButtonPin) == 0)                                      // Stop reflow process if set button is pressed
      {
        while(digitalRead(setButtonPin) == 0);                                // Wait for set button to be released
        delay(DEBOUNCE_DELAY);
        PWMValue = 0;
        Serial.print("Stop\n");                                               // Send 'stop' flag to Python
        delay(10);                                                            // Ensure Timer 1 ISR is given enough time to execute and turn off the oven before it is stopped (upon returning to Idle state)
        state = OFF;
        break;
      }
      if(stateTime == soakTime)                                               // Enter ramp to reflow state once soak time has elapsed
      {
        stateTime = 0;                                                        // Reset state time
        tone(buzzerPin, MID_BUZZER_FREQ, MEDIUM_BEEP);
        state = RAMP_TO_REFLOW;
      }
      break;
    }
    // Ramp to reflow state
    case RAMP_TO_REFLOW:
    {
      PWMValue = 100;                                                         // Set oven PWM duty cycle to 100%
      
      if(digitalRead(setButtonPin) == 0)                                      // Stop reflow process if set button is pressed
      {
        while(digitalRead(setButtonPin) == 0);                                // Wait for set button to be released
        delay(DEBOUNCE_DELAY);
        PWMValue = 0;
        Serial.print("Stop\n");                                               // Send 'stop' flag to Python
        delay(10);                                                            // Ensure Timer 1 ISR is given enough time to execute and turn off the oven before it is stopped (upon returning to Idle state)
        state = OFF;
        break;
      }
      if(ovenTemp > reflowTemp - REFLOW_TEMP_OFFSET)                          // Turn off oven prematurely to account for embodied heat
      {
        PWMValue = 0;
      }
      if(ovenTemp > reflowTemp)                                               // Enter reflow state once reflow temperature has been reached
      {
        stateTime = 0;                                                        // Reset state time
        tone(buzzerPin, MID_BUZZER_FREQ, MEDIUM_BEEP);
        state = REFLOW;
      }
      break;
    }
    // Reflow state
    case REFLOW:
    {
      PWMValue = REFLOW_DUTY_CYCLE;                                           // Set corresponding oven PWM duty cycle
      
      if(digitalRead(setButtonPin) == 0)                                      // Stop reflow process if set button is pressed
      {
        while(digitalRead(setButtonPin) == 0);                                // Wait for set button to be released
        delay(DEBOUNCE_DELAY);
        PWMValue = 0;
        Serial.print("Stop\n");                                               // Send 'stop' flag to Python
        delay(10);                                                            // Ensure Timer 1 ISR is given enough time to execute and turn off the oven before it is stopped (upon returning to Idle state)
        state = OFF;
        break;
      }
      if(stateTime == reflowTime)                                             // Enter cooling state once reflow time has elapsed
      {
        stateTime = 0;                                                        // Reset state time
        PWMValue = 0;

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
      if(ovenTemp < SAFE_TO_HANDLE_TEMP)                                      // Finish reflow process once safe-to-handle temperature is reached
      {
        stateTime = 0;                                                        // Reset state time
        TCCR1B = 0;                                                           // Stop Timer 1
        
        digitalWrite(LED1Pin, 0);                                             // Turn off LED1
        Serial.print("Done\n");                                               // Send 'done' flag to Python
        
        tone(buzzerPin, HIGH_BUZZER_FREQ, MEDIUM_BEEP);                       // Play end melody
        delay(SHORT_BEEP);
        tone(buzzerPin, MID_BUZZER_FREQ, MEDIUM_BEEP);
        delay(SHORT_BEEP);
        tone(buzzerPin, LOW_BUZZER_FREQ, MEDIUM_BEEP);
        
        Serial.print(cycleTime);                                              // Send total reflow cycle time to Python
        state = OFF;
      }
      break;
    }
  }
}
