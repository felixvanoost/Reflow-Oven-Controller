// Reflow Oven Controller Shield for the Arduino
// v1.0, Built 10/04/2015

void setup() 
{
  
  #define INCBUTTON    3
  #define DECBUTTON    4
  #define STATE2LED    6
  #define STATE3LED    7
  #define STATE4LED    8
  
  pinMode(JUNCTION, INPUT);
  pinMode(SETBUTTON, INPUT);
  pinMode(INCBUTTON, INPUT);
  pinMode(DECBUTTON, INPUT);
  
  pinMode(STATE2LED, OUTPUT);
  pinMode(STATE3LED, OUTPUT);
  pinMode(STATE4LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(OVEN, OUTPUT);
  
}

// Description:		Obtains an analog reading for the cold junction from the LM335 and converts it to a temperature in Celsius 
// Parameters:		-
// Returns:		The cold junction temperature in Celsius
int getJunctionTemp()
{
  int junctionReading = 0;
  int junctionTemp = 0;
  
  junctionReading = analogRead(JUNCTION);                        // Obtain 10-bit cold junction reading from the ADC
  return junctionTemp;
}

void loop() 
{

}
