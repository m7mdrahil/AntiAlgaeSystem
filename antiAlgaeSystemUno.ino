/* CONNECTIONS
   A0 - PO OF PH SENSOR
   A1 - T0 of PH SENSOR - TEMP SENSOR
   D1 - SCL of distance sensor (GREEN)
   D2 - SDA of distance sensor (BLUE)
   D3 -
   D7 - RECYCLE PUMP CONTROL PIN - WHITE
   D6 - REFILL PUMP CONTROL PIN - GREY
   D5 - COOLING FAN PUMP CONTROL PIN - PURPLE
*/

//******************* DATA TRANSFER
#include<SoftwareSerial.h>
SoftwareSerial Nodemcu(10, 11);//10 to D2 and 11 to D1

float waterLevel, waterTemp, pHLevel;

unsigned long lastRecycleTime;
unsigned long recycleTime = 120000;
unsigned long recycleDelay = 150000;
unsigned long coolingTime = 10000;
float waterLowerLimit = 32;//32
float waterUpperLimit = 37;//37
float tempUpperLimit = 30.0;

//******************* RELAYS
int recyclePumpPin = 7;
int refillPumpPin = 6;
int coolingFanPin = 5;

//******************* SENSORS
int pHSensorPin = A0;
int echoPin = 2; //BLUE
int trigPin = 3; //GREEN

//******************* DISTANCE SENSOR
#include<stdlib.h>
#include <Wire.h>

//******************* TEMP SENSOR
#include <OneWire.h>
#include <DallasTemperature.h>
int ONE_WIRE_BUS = 8;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

//SCL - D1
//SDA - D2
//3V - Vcc

float tankHeight = 37;

long duration;
int distance;


void setup()
{
  Serial.begin(115200);
  Wire.begin();
  pinMode(recyclePumpPin, OUTPUT);
  pinMode(refillPumpPin, OUTPUT);
  pinMode(coolingFanPin, OUTPUT);
  pinMode(pHSensorPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  turnOffRefillPump();
  turnOffCoolingFan();
  turnOffRecyclePump();
  Nodemcu.begin(115200);
  sensors.begin();
  delay(1000);
  lastRecycleTime = millis();
  Serial.println("Setup complete...");
}

void loop()
{
  getData();
  transferData();
  getWaterLevel();
  waterUpperLimit = tankHeight;
  while (waterLevel < waterLowerLimit) //3.7
  {
    getWaterLevel();
    turnOffRecyclePump();
    turnOnRefillPump();
    delay(100);
  }

  turnOffRefillPump();

  if (millis() - lastRecycleTime >  recycleTime) //need to check if any units are wrong
  {
    turnOnRecyclePump();
    delay(recycleDelay);
    turnOffRecyclePump();
    lastRecycleTime = millis();
  }
  if (waterTemp > tempUpperLimit)
  {
    turnOnCoolingFan();
    delay(coolingTime);
    turnOffCoolingFan();
  }
  else
  {
    turnOffCoolingFan();
    delay(1000);
  }

}

void getData()
{
  Serial.println("getData");
  getWaterTemp();
  getWaterLevel();
  getpHLevel();
  Serial.print("waterTemp");  Serial.println(waterTemp);
  Serial.print("waterLevel"); Serial.println(waterLevel);
  Serial.print("pHLevel"); Serial.println(pHLevel);

}

void transferData()
{
  //Serial.println("Data transfer started.....");
  Nodemcu.print(waterTemp); Nodemcu.print("A");
  Nodemcu.print(waterLevel); Nodemcu.print("B");
  Nodemcu.print(pHLevel); Nodemcu.print("C");
  Nodemcu.print("\n");
  delay(500);
  Serial.print("waterTemp "); Serial.println(waterTemp);
  Serial.print("waterLevel "); Serial.println(waterLevel);
  Serial.print("pHLevel "); Serial.println(pHLevel);
}


void turnOnRecyclePump()
{
  Serial.println("turnOnRecyclePump()");
  digitalWrite(recyclePumpPin, LOW);
  delay(100);
}

void turnOffRecyclePump()
{
  Serial.println("turnOffRecyclePump()");
  digitalWrite(recyclePumpPin, HIGH);
  delay(100);
}


void turnOffRefillPump()
{
  Serial.println("turnOffRefillPump()");
  digitalWrite(refillPumpPin, HIGH);
  delay(100);
}

void turnOnRefillPump()
{
  Serial.println("turnOnRefillPump()");
  digitalWrite(refillPumpPin, LOW);
  delay(100);
}

void turnOffCoolingFan()
{
  Serial.println("turnOffCoolingFan()");
  digitalWrite(coolingFanPin, HIGH);
  delay(100);
}

void turnOnCoolingFan()
{
  Serial.println("turnOnCoolingFan()");
  digitalWrite(coolingFanPin, LOW);
  delay(100);
}

void getWaterTemp()
{
  Serial.println("getWaterTemp()");
  for (int i = 0; i < 100; i++)
  { // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
    sensors.requestTemperatures();
    waterTemp = sensors.getTempCByIndex(0); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  }
}

void getWaterLevel()
{
  Serial.println("getWaterLevel()");
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
  Serial.print("Distance: ");
  Serial.print(distance);
  waterLevel = tankHeight - distance; // to get centimeter values

}


void getpHLevel()
{
  Serial.println("getpHLevel()");
  int pHSensorValue = analogRead(pHSensorPin);
  float Voltage = pHSensorValue * (5.0 / 1023.0);
  pHLevel = Voltage * 14.0 / 5.0;
}
