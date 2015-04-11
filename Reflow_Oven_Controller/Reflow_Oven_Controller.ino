// Reflow Oven Controller Shield for the Arduino
// v1.0, Built 10/04/2015

void setup() 
{
  #define THERM        0            // Define thermocouple input pin (A0)
  #define JUNCTION     1            // Define cold junction input pin (A1)
  
  #define SETBUTTON    2            // Define pushbutton input pins (D2-D4)
  #define INCBUTTON    3
  #define DECBUTTON    4
  #define STATE1LED    5            // Define LED output pins (D5-D8)
  #define STATE2LED    6
  #define STATE3LED    7
  #define STATE4LED    8
  #define BUZZER       9            // Define buzzer output pin (D9)
  #define OVEN         10           // Define oven SSR pin (D10)
  
  pinMode(THERM, INPUT);            // Declare pins as inputs
  pinMode(JUNCTION, INPUT);
  pinMode(SETBUTTON, INPUT);
  pinMode(INCBUTTON, INPUT);
  pinMode(DECBUTTON, INPUT);
  
  pinMode(STATE1LED, OUTPUT);       // Declare pins as outputs
  pinMode(STATE2LED, OUTPUT);
  pinMode(STATE3LED, OUTPUT);
  pinMode(STATE4LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(OVEN, OUTPUT);
  
  Serial.begin(9600);               // Initialise serial port at 9600 baud
}

void loop() 
{

}
