// Copyright Felix van Oost 2015.
// This documentation describes Open Hardware and is licensed under the CERN OHL v1.2. You may redistribute and modify this documentation under the terms of the CERN OHL v1.2. 
// This documentation is distributed WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING OF MERCHANTABILITY, SATISFACTORY QUALITY AND FITNESS FOR A PARTICULAR PURPOSE. Please see the CERN OHL v1.2 for applicable conditions.

// Reflow Oven Controller for the Arduino Uno
// v1.0, Built 20/04/2015

#define thermPin            0                                                 // Define thermocouple input pin (A0)
#define junctionPin         1                                                 // Define cold junction input pin (A1)

#define setButtonPin        2                                                 // Define pushbutton input pins (D2-D4)
#define incButtonPin        3
#define decButtonPin        4
#define LED1Pin             5                                                 // Define LED output pins (D5-D8)
#define LED2Pin             6
#define LED3Pin             7
#define LED4Pin             8
#define buzzerPin           9                                                 // Define buzzer output pin (D9)
#define ovenPin             10                                                // Define oven SSR pin (D10)

#define BUTTON_DELAY        120                                               // Pushbutton delay (ms)
#define BUZZER_FREQUENCY    5000                                              // Buzzer frequency (Hz)
#define SHORT_BEEP          100                                               // Short beep length (ms)
#define LONG_BEEP           400                                               // Long beep length (ms)

#define MIN_SOAK_TEMP       120                                               // Define minimum values for thermal profile parameters
#define MIN_SOAK_TIME       30
#define MIN_REFLOW_TEMP     200
#define MIN_REFLOW_TIME     15

#define MAX_SOAK_TEMP       180                                               // Define maximum values for thermal profile parameters
#define MAX_SOAK_TIME       90
#define MAX_REFLOW_TEMP     240
#define MAX_REFLOW_TIME     60

#define OFF                 0                                                 // Define finite state machine states
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

unsigned int processTime = 0;                                                 // Declare time variables
unsigned char stateTime = 0;

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
  tone(buzzerPin, BUZZER_FREQUENCY, SHORT_BEEP);
  Serial.println("Soak Time:");
  soakTime = readButtons(soakTime, MIN_SOAK_TIME, MAX_SOAK_TIME);             // Set soak time
  Serial.println(soakTime);                                                   // Display entered soak time
  tone(buzzerPin, BUZZER_FREQUENCY, SHORT_BEEP);
  Serial.println("Reflow Temperature:");
  reflowTemp = readButtons(reflowTemp, MIN_REFLOW_TEMP, MAX_REFLOW_TEMP);     // Set reflow temperature
  Serial.println(reflowTemp);                                                 // Display entered reflow temperature
  tone(buzzerPin, BUZZER_FREQUENCY, SHORT_BEEP);
  Serial.println("Reflow Time:");
  reflowTime = readButtons(reflowTime, MIN_REFLOW_TIME, MAX_REFLOW_TIME);     // Set reflow time
  Serial.println(reflowTime);                                                 // Display entered reflow time
  tone(buzzerPin, BUZZER_FREQUENCY, SHORT_BEEP);
  return;
}

void loop() 
{
  switch(state)                                                               // Finite state machine
  {
    case OFF:
    {
      while(digitalRead(setButtonPin) != 0);                                  // Wait for set button to be pressed to begin the reflow process
      state = RAMP_TO_SOAK;                                                   // Enter ramp to soak state
      break;
    }
    case RAMP_TO_SOAK:
    {
      // FOR DEBUGGING
      tone(buzzerPin, BUZZER_FREQUENCY, LONG_BEEP);
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
      // Ramp to reflow state code
      break;
    }
    case REFLOW:
    {
      // Reflow state code
      break;
    }
  }
  delay(1000);                                                                // Set frequency of measurements and decisions to 1Hz
}
