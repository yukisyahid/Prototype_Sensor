#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define WIFISSID "WIFI_IOT53_plus"
#define WIFIPASS "belajariot"
 
#define DO_PIN A0
 
#define VREF 3300    //VREF (mv)
#define ADC_RES 1024 //ADC Resolution
 
//Single-point calibration Mode=0
//Two-point calibration Mode=1
#define TWO_POINT_CALIBRATION 0
 
#define READ_TEMP (25) //Current water temperature ℃, Or temperature sensor function
 
//Single point calibration needs to be filled CAL1_V and CAL1_T
#define CAL1_V (947) //mv
#define CAL1_T (25)   //℃
//Two-point calibration needs to be filled CAL2_V and CAL2_T
//CAL1 High temperature point, CAL2 Low temperature point
#define CAL2_V (1300) //mv
#define CAL2_T (15)   //℃

//Wifi Start
const char* deviceid = "ID=Prototype1_DO";
const char* groupid = "GROUP=Prototype_1";
//server to receive the data
const char* host = "pond.weather.id";
const int port = 80;

// do not touch below
unsigned long startTime = millis();

const int watchdog = 10000;  // delay before pushing data
unsigned long previousMillis = millis();

void connectWifi() {
  Serial.println("Connecting to access point");
  WiFi.disconnect();
  if (WiFi.getMode() != WIFI_STA) {  //WIFI_STA= station mode (connect ke akses point)
    WiFi.mode(WIFI_STA);
  }
  WiFi.begin(WIFISSID, WIFIPASS);
  //...Give ESP 10 seconds to connect to station.
  startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  // Check Connection
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Wifi connected; IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.print("WiFi connect failed to ssid: ");
    Serial.println(WIFISSID);
  }
}

//Wifi End
 
const uint16_t DO_Table[41] = {
    14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530,
    11260, 11010, 10770, 10530, 10300, 10080, 9860, 9660, 9460, 9270,
    9080, 8900, 8730, 8570, 8410, 8250, 8110, 7960, 7820, 7690,
    7560, 7430, 7300, 7180, 7070, 6950, 6840, 6730, 6630, 6530, 6410};
 
uint8_t Temperaturet;
uint16_t ADC_Raw;
uint16_t ADC_Voltage;
uint16_t DO;
 
int16_t readDO(uint32_t voltage_mv, uint8_t temperature_c)
{
#if TWO_POINT_CALIBRATION == 00
  uint16_t V_saturation = (uint32_t)CAL1_V + (uint32_t)35 * temperature_c - (uint32_t)CAL1_T * 35;
  return (voltage_mv * DO_Table[temperature_c] / V_saturation);
#else
  uint16_t V_saturation = (int16_t)((int8_t)temperature_c - CAL2_T) * ((uint16_t)CAL1_V - CAL2_V) / ((uint8_t)CAL1_T - CAL2_T) + CAL2_V;
  return (voltage_mv * DO_Table[temperature_c] / V_saturation);
#endif
}
 
void setup()
{
  Serial.begin(115200);
  connectWifi();
  delay(1000);
}
 
void loop()
{
  Temperaturet = (uint8_t)READ_TEMP;
  ADC_Raw = analogRead(DO_PIN);
  ADC_Voltage = uint32_t(VREF) * ADC_Raw / ADC_RES;
 
  DO = readDO(ADC_Voltage, Temperaturet)/1000;
  Serial.print("Temperaturet:\t" + String(Temperaturet) + "\t");
  Serial.print("ADC RAW:\t" + String(ADC_Raw) + "\t");
  Serial.print("ADC Voltage:\t" + String(ADC_Voltage) + "\t");
  Serial.println("DO:\t" + String(DO) + "\t");
  delay(1000);

  //kirim data
  WiFiClient client;  //deklarasi method client
    HTTPClient http;    //deklarasi method http

    if (!client.connect(host, port)) {
      Serial.print("cannot connect to web server ");
      Serial.println(host);
      return;
    }

    //String url="/weather/poller.php?";
    String url = "http://";
    url += host;
    url += ":";
    url += port;
    url += "/?";
    //url += "ID=YOURID";
    url += deviceid;
    url += "&";
    url += groupid;
    url += "&DO=";
    url += String(DO);
    //url += "&humidity=";
    //url += String(h);
    url += "&softwaretype=Prototype1";

    // send it

    client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

    // wait for respond
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.print("timeout");
        client.stop();
        return;
      }
    }

    // ok, we got the reply
    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
  //wifi end
  delay(300000);
}