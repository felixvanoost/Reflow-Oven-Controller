// Copyright Felix van Oost 2015.
// This documentation describes Open Hardware and is licensed under the CERN OHL v1.2. You may redistribute and modify this documentation under the terms of the CERN OHL v1.2. 
// This documentation is distributed WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING OF MERCHANTABILITY, SATISFACTORY QUALITY AND FITNESS FOR A PARTICULAR PURPOSE. Please see the CERN OHL v1.2 for applicable conditions.

// Reflow Oven Controller for the Arduino Uno
// v1.0, Built 20/04/2015

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
#define SAFE_TEMP           60                                                // Safe-to-handle temperature

unsigned char state = OFF;

unsigned char soakTemp = 150;                                                 // Declare thermal profile parameter variables and initialise to default values
unsigned char soakTime = 60;
unsigned char reflowTemp = 220;
unsigned char reflowTime = 45;

float thermTemp = 0;                                                          // Declare temperature variables
float junctionTemp = 0;
float ovenTemp = 0;

volatile unsigned int processTime = 0;                                        // Declare time variables
volatile unsigned char stateTime = 0;

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
  
  analogReference(DEFAULT);                                                   // Use default 5V reference for ADC readings
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
      digitalWrite(LED1Pin, 1);                                               // Turn off LEDs
      digitalWrite(LED2Pin, 1);
      digitalWrite(LED3Pin, 1);
      digitalWrite(LED4Pin, 1);
      digitalWrite(ovenPin, 0);                                               // Turn off oven
      
      while(digitalRead(setButtonPin) != 0);                                  // Wait for set button to be pressed to begin the reflow process
      while(digitalRead(setButtonPin) == 0);                                  // Wait for set button to be released      
      delay(DEBOUNCE_DELAY);
     
      tone(buzzerPin, LOW_BUZZER_FREQ, LONG_BEEP);                            // Play start process melody
      delay(SHORT_BEEP);
      tone(buzzerPin, MID_BUZZER_FREQ, LONG_BEEP);
      delay(SHORT_BEEP);
      tone(buzzerPin, HIGH_BUZZER_FREQ, LONG_BEEP);
     
      initialiseTimer1();
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
      
      // FOR DEBUGGING
      digitalWrite(LED1Pin, 0);
      digitalWrite(LED2Pin, 0);
      digitalWrite(LED3Pin, 0);
      digitalWrite(LED4Pin, 0);
      break;
    }
    case SOAK:
    {
      // Soak state code
      break;
    }
    case RAMP_TO_REFLOW:
    {
      // Ramp to reflow state code0
      break;
    }
    case REFLOW:
    {
      // Reflow state code
      break;
    }
  }
}
