// Copyright Felix van Oost 2015.
// This documentation describes Open Hardware and is licensed under the CERN OHL v1.2. You may redistribute and modify this documentation under the terms of the CERN OHL v1.2. 
// This documentation is distributed WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING OF MERCHANTABILITY, SATISFACTORY QUALITY AND FITNESS FOR A PARTICULAR PURPOSE. Please see the CERN OHL v1.2 for applicable conditions.

// Reflow Oven Controller for the Arduino Uno

#define thermPin            0                                                 // Thermocouple input pin (A0)
#define junctionPin         1                                                 // Cold junction input pin (A1)

#define setButtonPin        2                                                 // Pushbutton input pins (D2-D4)
#define incButtonPin        3
#define decButtonPin        4
#define LED1Pin             5                                                 // LED output pins (D5-D8)
#define LED2Pin             6
#define LED3Pin             7
#define LED4Pin             8
#define buzzerPin           9                                                 // Buzzer output pin (D9)
#define ovenPin             10                                                // Oven SSR pin (D10)

#define BUTTON_DELAY        120                                               // Pushbutton delay for increment / decrement (ms)
#define DEBOUNCE_DELAY      50                                                // Pushbutton debounce delay (ms)

#define LOW_BUZZER_FREQ     3000                                              // Buzzer frequencies (Hz)
#define MID_BUZZER_FREQ     4000
#define HIGH_BUZZER_FREQ    5000
#define SHORT_BEEP          100                                               // Short beep length (ms)
#define LONG_BEEP           400                                               // Long beep length (ms)

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

#define SOAK_TEMP_OFFSET    15                                                // Soak temperature offset (to account for embodied heat in oven at soak stage)
#define REFLOW_TEMP_OFFSET  8                                                 // Reflow temperature offset (to account for embodied heat in oven at reflow stage)

#define ERROR_TEMP          50                                                // Thermocouple error temperature threshold (stops reflow process if specified temperature is not reached within the first 30 seconds)
#define SAFE_TEMP           60                                                // Safe-to-handle temperature

byte state = OFF;

byte soakTemp = 150;                                                          // Declare thermal profile parameter variables and initialise to default values
byte soakTime = 60;
byte reflowTemp = 220;
byte reflowTime = 45;

float thermTemp = 0;                                                          // Declare temperature variables
float junctionTemp = 0;
float ovenTemp = 0;

volatile unsigned int processTime = 0;                                        // Declare time variables
volatile byte stateTime = 0;

// 0-255C LUT for K-type thermocouple (stored in program memory)
const int ThermLUT[] PROGMEM = {0, 18, 37, 56, 74, 93, 112, 130, 149, 168,
                                187, 205, 224, 243, 262, 281, 299, 318, 337, 356,
                                375, 394, 413, 432, 451, 470, 489, 508, 527, 547,
                                565, 585, 604, 623, 642, 661, 681, 700, 719, 738,
                                758, 777, 796, 815, 835, 854, 873, 893, 912, 932,
                                951, 970, 990, 1009, 1028, 1048, 1067, 1087, 1106, 1126,
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
  pinMode(LED1Pin, OUTPUT);
  pinMode(LED2Pin, OUTPUT);
  pinMode(LED3Pin, OUTPUT);
  pinMode(LED4Pin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ovenPin, OUTPUT);
  
  digitalWrite(LED1Pin, 1);                                                   // Turn off LEDs
  digitalWrite(LED2Pin, 1); 
  digitalWrite(LED3Pin, 1); 
  digitalWrite(LED4Pin, 1);
  
  Serial.begin(9600);                                                         // Initialise serial port at 9600 baud
  
  setParameters();
  Serial.println("");
  Serial.println("Press 'set' to begin reflow process");
}

// Description:		Obtains an analog reading for the cold junction from the LM35 and converts it to a temperature in Celsius 
// Parameters:		-
// Returns:		-
void getJunctionTemp()
{
  int junctionReading = 0;
  
  analogReference(INTERNAL);                                                  // Use 1.1V reference for ADC readings (LM35 outputs 0-1V)
  junctionReading = analogRead(junctionPin);                                  // Obtain 10-bit cold junction reading from the ADC
  junctionTemp = ((junctionReading * 1.1 * 100) / 1023);                      // Convert reading to temperature in Celsius
  return;
}

// Description:		Obtains an analog reading for the thermcouple and converts it to a temperature in Celsius using a LUT
// Parameters:		-
// Returns:		-
void getThermTemp()
{
  int thermReading = 0;
  int i = 0;
  
  analogReference(DEFAULT);                                                   // Use default 5V reference for ADC readings
  thermReading = analogRead(thermPin);                                        // Obtain 10-bit thermocouple reading from the ADC
  thermReading = ((thermReading * 5 * 1000) / 1023);                          // Convert reading to a voltage (in mV)
  
  while(thermReading < ThermLUT[i])
  {
    i++;
  }
  thermTemp = i;
  
  return;
}

// Description:		Interrupt Service Routine for Timer 1
//                   1. Increments the total process time and state time every second
//                   2. Obtains temperature readings for the thermocouple and cold junction and calculates the current oven temperature
//                   3. Displays the current oven temperature in the serial monitor
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
  cli();                                                                        // Disable global interrupts
  TCCR1A = 0;                                                                   // Reset Timer 1 registers
  TCCR1B = 0;
  TIMSK1 |= (1 << TOIE1);                                                       // Enable Timer 1 overflow interrupt
  TCNT1 = 0x0BDB;                                                               // Preload Timer 1 with 3035 to overflow at 1Hz
  TCCR1B |= (1 << CS12);                                                        // Select clock source as internal 12MHz oscillator with CLK / 256 prescaling and start Timer 1
  sei();                                                                        // Enable global interrupts
}

// Description:		Reads the status of the pushbuttons and increments or decrements the given initial value within the specified minimum and maximum ranges until the set button is pressed
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
  Serial.write("\x1b[2J");                                                    // ANSI sequence to clear screen (in HyperTerminal)
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

  return;
}

void loop() 
{
  switch(state)                                                               // Finite state machine
  {
    case OFF:
    {
      TCCR1B = 0;                                                             // Stop Timer 1
      processTime = 0;                                                        // Reset process time
      digitalWrite(LED1Pin, 1);                                               // Turn off LEDs
      digitalWrite(LED2Pin, 1);
      digitalWrite(LED3Pin, 1);
      digitalWrite(LED4Pin, 1);
      digitalWrite(ovenPin, 0);                                               // Turn off oven
      
      while(digitalRead(setButtonPin) != 0);                                  // Wait for set button to be pressed to begin the reflow process
      while(digitalRead(setButtonPin) == 0);                                  // Wait for set button to be released      
      delay(DEBOUNCE_DELAY);
     
      tone(buzzerPin, LOW_BUZZER_FREQ, LONG_BEEP);                            // Play melody
      delay(SHORT_BEEP);
      tone(buzzerPin, MID_BUZZER_FREQ, LONG_BEEP);
      delay(SHORT_BEEP);
      tone(buzzerPin, HIGH_BUZZER_FREQ, LONG_BEEP);
     
      initialiseTimer1();
      stateTime = 0;                                                          // Reset state time
      state = RAMP_TO_SOAK;                                                   // Enter ramp to soak state
      break;
    }
    case RAMP_TO_SOAK:
    { 
      if(digitalRead(setButtonPin) == 0)                                      // Stop reflow process if set button is pressed
      {
        while(digitalRead(setButtonPin) == 0);                                // Wait for set button to be released
        state = OFF;
        break;
      }
      if(stateTime == 30 && ovenTemp < 50)                                    // Stop reflow process if oven does not reach 50C in the first 30s (thermocouple error)
      {
        state = OFF;
        break;
      }
      
      // FOR DEBUGGING
      digitalWrite(LED1Pin, 0);
      digitalWrite(LED2Pin, 0);
      digitalWrite(LED3Pin, 0);
      digitalWrite(LED4Pin, 0);
      break;
    }
    case SOAK:
    {
      if(digitalRead(setButtonPin) == 0)                                      // Stop reflow process if set button is pressed
      {
        while(digitalRead(setButtonPin) == 0);                                // Wait for set button to be released
        state = OFF;
        break;
      }
      
      // Soak state code
      break;
    }
    case RAMP_TO_REFLOW:
    {
      if(digitalRead(setButtonPin) == 0)                                      // Stop reflow process if set button is pressed
      {
        while(digitalRead(setButtonPin) == 0);                                // Wait for set button to be released
        state = OFF;
        break;
      }
      
      // Ramp to reflow state code
      break;
    }
    case REFLOW:
    {
      if(digitalRead(setButtonPin) == 0)                                      // Stop reflow process if set button is pressed
      {
        while(digitalRead(setButtonPin) == 0);                                // Wait for set button to be released
        state = OFF;
        break;
      }
      
      // Reflow state code
      break;
    }
  }
}
