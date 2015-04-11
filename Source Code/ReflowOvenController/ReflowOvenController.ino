// Copyright Felix van Oost 2015.
// This documentation describes Open Hardware and is licensed under the CERN OHL v1.2. You may redistribute and modify this documentation under the terms of the CERN OHL v1.2. 
// This documentation is distributed WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING OF MERCHANTABILITY, SATISFACTORY QUALITY AND FITNESS FOR A PARTICULAR PURPOSE. Please see the CERN OHL v1.2 for applicable conditions.

// Reflow Oven Controller for the Arduino
// v1.0, Built 10/04/2015

void setup() 
{
  #define THERM        0                                         // Define thermocouple input pin (A0)
  #define JUNCTION     1                                         // Define cold junction input pin (A1)
  
  #define SETBUTTON    2                                         // Define pushbutton input pins (D2-D4)
  #define INCBUTTON    3
  #define DECBUTTON    4
  #define STATE1LED    5                                         // Define LED output pins (D5-D8)
  #define STATE2LED    6
  #define STATE3LED    7
  #define STATE4LED    8
  #define BUZZER       9                                         // Define buzzer output pin (D9)
  #define OVEN         10                                        // Define oven SSR pin (D10)
  
  pinMode(THERM, INPUT);                                         // Declare pins as inputs
  pinMode(JUNCTION, INPUT);
  pinMode(SETBUTTON, INPUT);
  pinMode(INCBUTTON, INPUT);
  pinMode(DECBUTTON, INPUT);
  
  pinMode(STATE1LED, OUTPUT);                                    // Declare pins as outputs
  pinMode(STATE2LED, OUTPUT);
  pinMode(STATE3LED, OUTPUT);
  pinMode(STATE4LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(OVEN, OUTPUT);
  
  Serial.begin(9600);                                            // Initialise serial port at 9600 baud
}

// Description:		Obtains an analog reading for the cold junction from the LM335 and converts it to a temperature in Celsius 
// Parameters:		-
// Returns:		The cold junction temperature in Celsius
int getJunctionTemp()
{
  int junctionReading = 0;
  int junctionTemp = 0;
  
  junctionReading = analogRead(JUNCTION);                        // Obtain 10-bit cold junction reading from the ADC
  junctionTemp = ((junctionReading / 1023) * 5 * 100) - 273;     // Convert reading to temperature in Celsius
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
