#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define WIFISSID "WIFI_IOT53_plus"
#define WIFIPASS "belajariot"

//Wifi Start
const char* deviceid = "ID=Prototype1_pH";
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

const int potPin=34;
float ph;
float Value=0;
 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
    connectWifi();
    delay(10000);
  pinMode(potPin,INPUT);


}


 void loop(){
    //Value= analogRead(potPin);
    for(int i=0;i<800;i++){
      Value +=analogRead(potPin);
    }
    Value /=800;
    Serial.print(Value);
    Serial.print(" | ");
    //float voltage=Value*(3.3/4095.0);
    ph=15.5942-0.0051167*Value;
   // Serial.println("ini adalah ph ================ " + String(ph));

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
    url += "&ph=";
    url += String(ph);
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