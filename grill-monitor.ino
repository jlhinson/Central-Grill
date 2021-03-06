// libraries
// =================================================================
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <Adafruit_MAX31865.h>

// wifi intialize
// =================================================================
char ssid[] = "ssid"; //  your network SSID (name)
char pass[] = "pass"; // your network password
//int status = WL_IDLE_STATUS; // the Wifi radio's status
boolean wifiNotify = false; // track wifi connection notification
unsigned long lastWifiAttempt = 0; // track connect attempt to delay repeated attempts
bool errorWifi = false; // track wifi error
const int wifiLed = 5;

// temperature reading initialize
// =================================================================
const int numReadings = 10;      // number of readings to average
int readIndex = 0;               // index of the current reading

int grillArray[numReadings];     // readings from grill probe
int grillTotal = 0;              // running total for grill probe
int grillAverage = 0;            // average for grill probe
String grillString;              // convert to string for display

int meatArray[numReadings];      // readings from meat probe
int meatTotal = 0;               // running total for meat probe
int meatAverage = 0;             // average for meat probe
String meatString;               // convert to string for display

Adafruit_MAX31865 max1 = Adafruit_MAX31865(2, 23, 19, 18);   // Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31865 max2 = Adafruit_MAX31865(15, 23, 19, 18);
#define RREF      4300.0    // Rref resistor value. Use 430.0 for PT100, 4300.0 for PT1000.
#define RNOMINAL  1000.0    // 'nominal' 0-degrees-C resistance of the sensor. 100.0 for PT100, 1000.0 for PT1000
/*int grillProbe = A4;             // define inputs
int meatProbe = A5;

float aVar = .0039083;           // define fixed variables
float bVar = -.0000005775;
float zeroOhm = 1000;
float ohmOffset = 0;

float fullScale = 3.3;           // chip ADC variables
float resolution = 1023;
float voltageIn = 3.3;*/

// display initialize
// =================================================================
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();
char displaybuffer[4];           // buffer to hold display characters
const int tempSwitchPin = 13;    // set switch pin
int tempSwitchState = 0;         // variable for storing switch state

// =================================================================
void setup() {
  Serial.begin(9600);            // initialize serial comms
  delay(5000);

  alpha4.begin(0x70);  // pass in the address and clear display
  alpha4.clear();
  alpha4.writeDisplay();
  
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {     // initialize all readings to 0:
    grillArray[thisReading] = 0;
    meatArray[thisReading] = 0;
  }

  pinMode(tempSwitchPin, INPUT);    // declare switch as input
  pinMode(wifiLed, OUTPUT);         // declare wifi led as output and ensure it starts off
  digitalWrite(wifiLed, HIGH);

  max1.begin(MAX31865_2WIRE);  // set to 2WIRE, 3WIRE, or 4WIRE as necessary
  max2.begin(MAX31865_2WIRE);
}

// =================================================================
void loop() {
  connections();       // check wifi connection

  temperatures();      // get tempertaure probe readings

  updateDisplay();
  
  delay(5000);
}

// wifi connection
// =====================================================================
void connections() {
  if (WiFi.status() != WL_CONNECTED) {
    errorWifi = true;                 // wifi not connected
    digitalWrite(wifiLed, HIGH);       // turn off led
    unsigned long now = millis();
    if (now - lastWifiAttempt > 10000) {
      lastWifiAttempt = now;
     Serial.println("WiFi disconnected.");
      Serial.print("Connecting to SSID: ");
      Serial.println(ssid);
      Serial.println(" ");
      wifiNotify = false;
      WiFi.begin(ssid, pass);
    }
  }
  // notify ip address on new connection
  if (WiFi.status() == WL_CONNECTED && wifiNotify == false){
    errorWifi = false;                 // wifi now connected
    digitalWrite(wifiLed, LOW);       // turn on led
    Serial.print("Wifi connected, IP: ");
    Serial.println(WiFi.localIP());
    Serial.println(" ");
    wifiNotify = true;
    lastWifiAttempt = 0;
  }
  
}

// check temperature probes
// =====================================================================
void temperatures() {
  /*int grillRaw = analogRead(grillProbe);            // read raw
  float voltage1 = grillRaw * (fullScale / resolution);         // convert to voltage
  float ratio1 = voltageIn / voltage1;
  float ohms1 = (1500 * (1 / (ratio1 - 1))) + ohmOffset;
  float cTemp1 = (-aVar + sqrt(sq(aVar) - (4 * bVar * (1 - (ohms1 / zeroOhm))))) / (2 * bVar);
  float fTemp1 = cTemp1 * 1.8 + 32;*/

  float cTemp1 = max1.temperature(RNOMINAL, RREF);
  float fTemp1 = cTemp1 * 1.8 + 32;
  grillTotal = grillTotal - grillArray[readIndex];
  grillArray[readIndex] = fTemp1;
  grillTotal = grillTotal + grillArray[readIndex];
  grillAverage = grillTotal / numReadings;
  grillString = String(grillAverage);
  Serial.print("Grill: ");
  Serial.println(grillAverage);
  
  /*int meatRaw = analogRead(meatProbe);               // read raw
  float voltage2 = meatRaw * (fullScale / resolution);           // convert to voltage
  float ratio2 = voltageIn / voltage2; 
  float ohms2 = (1500 * (1 / (ratio2 - 1))) + ohmOffset;
  float cTemp2 = (-aVar + sqrt(sq(aVar) - (4 * bVar * (1 - (ohms2 / zeroOhm))))) / (2 * bVar);
  float fTemp2 = cTemp2 * 1.8 + 32;*/

  float cTemp2 = max2.temperature(RNOMINAL, RREF);
  float fTemp2 = cTemp2 * 1.8 + 32;
  meatTotal = meatTotal - meatArray[readIndex];
  meatArray[readIndex] = fTemp2;
  meatTotal = meatTotal + meatArray[readIndex];
  meatAverage = meatTotal / numReadings;
  meatString = String(meatAverage);
  Serial.print("Meat: ");
  Serial.println(meatAverage);

  Serial.println(" ");
  
  readIndex = readIndex + 1;
  if (readIndex >= numReadings) {
    readIndex = 0;
  }
  
}

// update display
// =====================================================================
void updateDisplay() {
  tempSwitchState = digitalRead(tempSwitchPin);    // read temp switch
  
  if (tempSwitchState == HIGH) {                   // show grill temp if switch is high
    if (grillString.length() == 2) {
      grillString = "0" + grillString;
    }
    grillString.toCharArray(displaybuffer, 4);
    Serial.println("Displaying grill temperature.");
    Serial.println(" ");
  } else {                                         // show meat temp if switch is low
    if (meatString.length() == 2) {
      meatString = "0" + meatString;
    }
    meatString.toCharArray(displaybuffer, 4);
    Serial.println("Displaying meat termperature.");
    Serial.println(" ");
  }
 
  alpha4.writeDigitAscii(0, displaybuffer[0]);        // set every digit to the buffer
  alpha4.writeDigitAscii(1, displaybuffer[1]);
  alpha4.writeDigitAscii(2, displaybuffer[2]);
  alpha4.writeDigitAscii(3, 'F');
 
  alpha4.writeDisplay();                             // write it out
}

