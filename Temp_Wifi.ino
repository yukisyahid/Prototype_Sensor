#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>


#define WIFISSID "WIFI_IOT53_plus"
#define WIFIPASS "belajariot"

// GPIO where the DS18B20 is connected to
const int oneWireBus = 4;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);


// do not touch below
unsigned long startTime = millis();

const int watchdog = 10000;  // delay before pushing data
unsigned long previousMillis = millis();

//wifi set
const char* deviceid = "ID=Prototype1_WaterTemp";
const char* groupid = "GROUP=Prototype_1";
//server to receive the data
const char* host = "pond.weather.id";
const int port = 80;

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
//wifi end

void setup() {
  // Start the Serial Monitor
  Serial.begin(115200);
  connectWifi(); //wifistart
  // Start the DS18B20 sensor
  sensors.begin();
  delay(1000);
}

void loop() {
  sensors.requestTemperatures(); 
  float t = sensors.getTempCByIndex(0);
  //float temperatureF = sensors.getTempFByIndex(0);
  Serial.print(t);
  Serial.println("ºC");
  //Serial.print(temperatureF);
  //Serial.println("ºF");

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
    url += "&t=";
    url += String(t);
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
  
  delay(300000);
}