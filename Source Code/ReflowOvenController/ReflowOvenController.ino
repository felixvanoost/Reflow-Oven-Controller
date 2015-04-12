// Copyright Felix van Oost 2015.
// This documentation describes Open Hardware and is licensed under the CERN OHL v1.2. You may redistribute and modify this documentation under the terms of the CERN OHL v1.2. 
// This documentation is distributed WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING OF MERCHANTABILITY, SATISFACTORY QUALITY AND FITNESS FOR A PARTICULAR PURPOSE. Please see the CERN OHL v1.2 for applicable conditions.

// Reflow Oven Controller for the Arduino
// v1.0, Built 10/04/2015

#define thermPin        0                                           // Define thermocouple input pin (A0)
#define junctionPin     1                                           // Define cold junction input pin (A1)
  
#define setButtonPin    2                                           // Define pushbutton input pins (D2-D4)
#define incButtonPin    3
#define decButtonPin    4
#define LED1Pin         5                                           // Define LED output pins (D5-D8)
#define LED2Pin         6
#define LED3Pin         7
#define LED4Pin         8
#define buzzerPin       9                                           // Define buzzer output pin (D9)
#define ovenPin         10                                          // Define oven SSR pin (D10)

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
  
  int soakTemp = 150;                                               // Declare thermal profile variables and initialise to default values
  int soakTime = 60;
  int reflowTemp = 220;
  int reflowTime = 45;
  
  Serial.begin(9600);                                               // Initialise serial port at 9600 baud
}

// Description:		Obtains an analog reading for the cold junction from the LM335 and converts it to a temperature in Celsius 
// Parameters:		-
// Returns:		The cold junction temperature in Celsius
int getJunctionTemp()
{
  int junctionReading = 0;
  int junctionTemp = 0;
  
  junctionReading = analogRead(junctionPin);                        // Obtain 10-bit cold junction reading from the ADC
  junctionTemp = ((junctionReading / 1023) * 5 * 100) - 273;        // Convert reading to temperature in Celsius
  return junctionTemp;
}

void loop() 
{
  // FOR DEBUGGING
  int junctionTemp = 0;
  junctionTemp = getJunctionTemp();
  Serial.println(junctionTemp);
  delay(1000);
}
