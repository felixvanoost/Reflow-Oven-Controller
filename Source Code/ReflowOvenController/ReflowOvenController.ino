// Copyright Felix van Oost 2015.
// This documentation describes Open Hardware and is licensed under the CERN OHL v1.2. You may redistribute and modify this documentation under the terms of the CERN OHL v1.2. 
// This documentation is distributed WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING OF MERCHANTABILITY, SATISFACTORY QUALITY AND FITNESS FOR A PARTICULAR PURPOSE. Please see the CERN OHL v1.2 for applicable conditions.

// Reflow Oven Controller for the Arduino Uno
// v1.0, Built 12/04/2015

#define thermPin            0                                       // Define thermocouple input pin (A0)
#define junctionPin         1                                       // Define cold junction input pin (A1)

#define setButtonPin        2                                       // Define pushbutton input pins (D2-D4)
#define incButtonPin        3
#define decButtonPin        4
#define LED1Pin             5                                       // Define LED output pins (D5-D8)
#define LED2Pin             6
#define LED3Pin             7
#define LED4Pin             8
#define buzzerPin           9                                       // Define buzzer output pin (D9)
#define ovenPin             10                                      // Define oven SSR pin (D10)

#define OFF                 0                                       // Define finite state machine states
#define RAMP_TO_SOAK        1
#define SOAK                2
#define RAMP_TO_REFLOW      3
#define REFLOW              4

#define BUTTON_SPEED        20                                      // Pushbutton delay (ms)
#define BUZZER_FREQUENCY    5000                                    // Buzzer frequency (Hz)
#define SOAK_TEMP_OFFSET    15                                      // Soak temperature offset (to account for embodied heat in oven at soak stage)
#define REFLOW_TEMP_OFFSET  8                                       // Reflow temperature offset (to account for embodied heat in oven at reflow stage)
#define SAFE_TEMP           60                                      // Safe-to-handle temperature

char state = OFF;

char soakTemp = 150;                                                // Declare thermal profile parameter variables and initialise to default values
char soakTime = 60;
char reflowTemp = 220;
char reflowTime = 45;

float thermTemp = 0;                                                // Declare temperature variables
float junctionTemp = 0;

int processTime = 0;                                                // Declare time variables
char stateTime = 0;

void setup() 
{
  pinMode(thermPin, INPUT);                                         // Declare digital pins as inputs / outputs
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
  
  Serial.begin(9600);                                               // Initialise serial port at 9600 baud
}

// Description:		Obtains an analog reading for the cold junction from the LM35 and converts it to a temperature in Celsius 
// Parameters:		-
// Returns:		-
void getJunctionTemp()
{
  int junctionReading = 0;
  
  analogReference(INTERNAL);                                        // Use 1.1V reference for ADC readings (LM35 outputs 0-1V)
  junctionReading = analogRead(junctionPin);                        // Obtain 10-bit cold junction reading from the ADC
  junctionTemp = ((junctionReading * 1.1 * 100) / 1023);            // Convert reading to temperature in Celsius
  return;
}

// Description:		Obtains an analog reading for the thermcouple and converts it to a temperature in Celsius using a LUT
// Parameters:		-
// Returns:		-
void getThermTemp()
{
  int thermReading = 0;
  
  analogReference(DEFAULT);                                         // Use default 5V reference for ADC readings
  return;
}

// Description:		Reads the status of the pushbuttons and increments or decrements the current thermal profile parameter accordingly until the set button is pressed
// Parameters:		profileParameter - Thermal profile parameter currently being set
// Returns:		-
void readButtons(int profileParameter)
{
  while(digitalRead(setButtonPin) != 0)
  {
    if(digitalRead(incButtonPin) == 0)
    {
      profileParameter++;                                           // Increment parameter if increment button is pressed
    }
    if(digitalRead(decButtonPin) == 0)
    {
      profileParameter--;                                           // Decrement parameter if decrement button is pressed
      
    }
    Serial.print(profileParameter);                                 // Display parameter in serial monitor
    Serial.print("\r");                                             // Return to start of line in serial monitor
    delay(BUTTON_SPEED);
  }
}

// Description:		Allows user to set the four thermal profile parameters using pusbuttons and provides user feedback via the buzzer
// Parameters:		-
// Returns:		-
void setParameters()
{
  Serial.println("Soak Temperature:");
  readButtons(soakTemp);                                            // Set soak temperature
  tone(buzzerPin, BUZZER_FREQUENCY, 200);
  Serial.println("Soak Time:");
  readButtons(soakTime);                                            // Set soak time
  tone(buzzerPin, BUZZER_FREQUENCY, 200);
  Serial.println("Reflow Temperature:");
  readButtons(reflowTemp);                                          // Set reflow temperature
  tone(buzzerPin, BUZZER_FREQUENCY, 200);
  Serial.println("Reflow Time:");
  readButtons(reflowTime);                                          // Set reflow time
  tone(buzzerPin, BUZZER_FREQUENCY, 1000);
}

void loop() 
{
  // FOR DEBUGGING
  getJunctionTemp();
  Serial.println(junctionTemp);
  
  switch(state)                                                     // Finite state machine
  {
    case OFF:
    {
      // Off state code
      break;
    }
    case RAMP_TO_SOAK:
    {
      // Ramp to soak state code
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
  delay(1000);                                                      // Set frequency of measurements and decisions to 1Hz
}
