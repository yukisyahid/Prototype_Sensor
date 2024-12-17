#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <OneWire.h>
#include <DallasTemperature.h>  //DS18B20

#include <SPI.h>
#include <LoRa.h>

//BME280 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;  // I2C

float bmeTemperature;
float bmePressure;
float bmeHumidity;
float bmeAltitude;

void setBMEValues() {
  bmeTemperature = bme.readTemperature();                //Celcius
  bmePressure = bme.readPressure() / 100.0F;             // hPa
  bmeHumidity = bme.readHumidity();                      // %
  bmeAltitude = bme.readAltitude(SEALEVELPRESSURE_HPA);  // meter
}


void printBMEValues() {
  Serial.print("Temperature = ");
  Serial.print(bmeTemperature);
  Serial.println(" *C");
  
  Serial.print("Pressure = ");
  Serial.print(bmePressure);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(bmeAltitude);
  Serial.println(" m");

  Serial.print("Humidity = ");
  Serial.print(bmeHumidity);
  Serial.println(" %");

  Serial.println();
}


//DS18B20 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// GPIO where the DS18B20 is connected to
const int waterTemperaturePin = 4;
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(waterTemperaturePin);
// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

float waterTemperature;

void setWaterTemperature() {
  sensors.requestTemperatures();
  waterTemperature = sensors.getTempCByIndex(0);
}


void printWaterTemperature(){
  Serial.print("Water Temperature = ");
  Serial.print(waterTemperature);
  Serial.println("ºC");
}


//pH Sensor +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
const int PH_PIN = 32;
float ph;
float PH_ADC_RAW = 0;

void setPH() {
  for (int i = 0; i < 800; i++) {
    PH_ADC_RAW += analogRead(PH_PIN);
  }
  PH_ADC_RAW /= 800;
  ph = 15.5942 - 0.0051167 * PH_ADC_RAW;
}


 void printPH(){
  Serial.print("pH = ");
  Serial.println(ph);
 }
 

//DO Sensor +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define DO_PIN 33

#define VREF 3300     //VREF (mv)
#define ADC_RES 4095  //ADC Resolution

//Single-point calibration Mode=0
//Two-point calibration Mode=1
#define TWO_POINT_CALIBRATION 0

#define READ_TEMP (25)  //Current water temperature ℃, Or temperature sensor function

//Single point calibration needs to be filled CAL1_V and CAL1_T
#define CAL1_V (1328)  //mv
#define CAL1_T (25)   //℃
//Two-point calibration needs to be filled CAL2_V and CAL2_T
//CAL1 High temperature point, CAL2 Low temperature point
#define CAL2_V (1300)  //mv
#define CAL2_T (15)    //℃

const uint16_t DO_Table[41] = {
  14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530,
  11260, 11010, 10770, 10530, 10300, 10080, 9860, 9660, 9460, 9270,
  9080, 8900, 8730, 8570, 8410, 8250, 8110, 7960, 7820, 7690,
  7560, 7430, 7300, 7180, 7070, 6950, 6840, 6730, 6630, 6530, 6410
};

uint8_t Temperaturet;
uint16_t DO_ADC_RAW;
uint16_t DO_ADC_Voltage;
uint16_t DO;

int16_t readDO(uint32_t voltage_mv, uint8_t temperature_c) {
#if TWO_POINT_CALIBRATION == 00
  uint16_t V_saturation = (uint32_t)CAL1_V + (uint32_t)35 * temperature_c - (uint32_t)CAL1_T * 35;
  return (voltage_mv * DO_Table[temperature_c] / V_saturation);
#else
  uint16_t V_saturation = (int16_t)((int8_t)temperature_c - CAL2_T) * ((uint16_t)CAL1_V - CAL2_V) / ((uint8_t)CAL1_T - CAL2_T) + CAL2_V;
  return (voltage_mv * DO_Table[temperature_c] / V_saturation);
#endif
}

void setDO() {
  Temperaturet = (uint8_t)READ_TEMP;
  DO_ADC_RAW = analogRead(DO_PIN);
  DO_ADC_Voltage = uint32_t(VREF) * DO_ADC_RAW / ADC_RES;

  DO = readDO(DO_ADC_Voltage, Temperaturet);
}


void printDO(){
  //Serial.print("DO Temperaturet:\t" + String(Temperaturet) + "\t");
  //Serial.print("DO ADC RAW:\t" + String(DO_ADC_RAW) + "\t");
  //Serial.print("DO ADC Voltage:\t" + String(DO_ADC_Voltage) + "\t");
  Serial.println("DO:\t" + String(DO));
}


//LoRa +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// - Pin configs -
#define ss 5
#define rst 14
#define dio0 2

void startLora() {
  Serial.println("LoRa Sender");
  // Setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(62.5E3);

  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  } else {
    Serial.println("Starting LoRa successful");
  }
}


void setup() {
  Serial.begin(9600);

  //LoRa +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  startLora();

  //BME280 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Serial.println("Starting BME280");
  bool BMEstatus;
  BMEstatus = bme.begin(0x76);
  if (!BMEstatus) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
      ;
  }
  Serial.println();

  //DS18B20 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // Start the DS18B20 sensor
  pinMode(waterTemperaturePin, INPUT);
  sensors.begin();

  //pH Sensor +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  pinMode(PH_PIN, INPUT);
}


void loop() {
  //BME280 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  setBMEValues();
  printBMEValues(); //remove comment on the function to use it.

  delay(1000);
  //DS18B20 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  setWaterTemperature();
  printWaterTemperature(); //remove comment on the function to use it.

  delay(1000);
  //pH Sensor +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  setPH();
  printPH(); //remove comment on the function to use it.
  
  delay(1000);
  //DO Sensor +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  setDO();
  printDO(); //remove comment on the function to use it.

  delay(1000);

  String message = "ID=PA01";
  message += ";eT=" + String(bmeTemperature);
  message += ";eH=" + String(bmeHumidity);
  message += ";eP=" + String(bmePressure);
  message += ";eA=" + String(bmeAltitude);
  message += ";wT=" + String(waterTemperature);
  message += ";pH=" + String(ph);
  message += ";DO=" + String(DO);
  Serial.print("Sending packet: ");
  Serial.println(message);
  // send packet
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();

  delay(10000);
}
