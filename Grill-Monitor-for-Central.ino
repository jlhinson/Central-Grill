const int numReadings = 15;      // number of reading to average
int readIndex = 0;               // index of the current reading

int readings1[numReadings];      // readings from A0
int total1 = 0;                  // running total for A0
int averageGrill = 0;            // average for A0

int readings2[numReadings];      // readings from A1
int total2 = 0;                  // running total for A1
int averageMeat = 0;             // average for A1

int grillProbe = A0;             // define inputs
int meatProbe = A1;

float aVar = .0039083;           // define fixed variables
float bVar = -.0000005775;
float zeroOhm = 1000;
float ohmOffset = 8;

void setup() {
  Serial.begin(9600);            // initialize serial comms
  
  // initialize all readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings1[thisReading] = 0;
    readings2[thisReading] = 0;
  }
}

void loop() {
  int raw1 = analogRead(grillProbe);            // read raw
  float voltage1 = raw1 * (3.3 / 1023);         // convert to voltage
  float ratio1 = 3.3 / voltage1;
  float ohms1 = (1500 * (1 / (ratio1 - 1))) + ohmOffset;
  float cTemp1 = (-aVar + sqrt(sq(aVar) - (4 * bVar * (1 - (ohms1 / zeroOhm))))) / (2 * bVar);
  float fTemp1 = cTemp1 * 1.8 + 32;
  total1 = total1 - readings1[readIndex];
  readings1[readIndex] = fTemp1;
  total1 = total1 + readings1[readIndex];
  averageGrill = total1 / numReadings;
  
  int raw2 = analogRead(meatProbe);            // read raw
  float voltage2 = raw2 * (3.3 / 1023);        // convert to voltage
  float ratio2 = 3.3 / voltage2; 
  float ohms2 = (1500 * (1 / (ratio2 - 1))) + ohmOffset;
  float cTemp2 = (-aVar + sqrt(sq(aVar) - (4 * bVar * (1 - (ohms2 / zeroOhm))))) / (2 * bVar);
  float fTemp2 = cTemp2 * 1.8 + 32;
  total2 = total2 - readings2[readIndex];
  readings2[readIndex] = fTemp2;
  total2 = total2 + readings2[readIndex];
  averageMeat = total2 / numReadings;

  readIndex = readIndex + 1;
  if (readIndex >= numReadings) {
    readIndex = 0;
  }

  Serial.print("Grill: ");
  Serial.println(averageGrill);
  Serial.println(ohms1);
  Serial.print("Meat: ");
  Serial.println(averageMeat);
  Serial.println(ohms2);
  delay(2000);
}
