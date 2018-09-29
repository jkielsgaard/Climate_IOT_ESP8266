
#include "DHT.h"
#include <EasyNTPClient.h>
#include <WiFiUdp.h>
#include <HttpClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>


// DHT22 data pin on digitalpin 5(14)
#define DHTPIN 14
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);


// Wifi credentials, Cloud host url, API key to the cloud API and unitID
char ssid[] = "<SSID>";
char pass[] = "<PASSWORD>";
String datadbURL= = "<HOSTURL>";
String apikey = "<APIKEY>";
String unitid = "<UNITID>";


// WiFiUDP and EasyNTPC is to collect the unix timestamp
WiFiUDP ntpUDP;
EasyNTPClient ntpClient(ntpUDP, "pool.ntp.org", 7200);


// Setup LED and serial monitor port
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  dht.begin();
  delay(100);

  ConnectWifi();
}


// Setup the Wifi connection and await to connect 
void ConnectWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  delay(100);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("."); 
    digitalWrite(LED_BUILTIN, HIGH); 
    delay(1000); 
    digitalWrite(LED_BUILTIN, LOW);  
    delay(1000);
  }

  Serial.print("WiFi connected - IP: ");
  Serial.println(WiFi.localIP());
}


// Start loop, will blink with the LED se user can see the unit active and in the loop
void loop(){
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000); 
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(2000); 

  // check wifi status, to ensure no error when sending climatedata 
  if(WiFi.status() == WL_CONNECTED) { post(); }
  else { Serial.println("Lost connection... ☠"); }

  delay(300000);
}


// The HTTP post function to send the climatedata to the cloud
void post()
{
  int attempt = 0;

  while (attempt < 3)
  {
    HTTPClient http; 
    http.begin(datadbURL, "2D:A5:86:8A:33:0B:81:91:EB:58:55:D7:BD:C6:3D:1B:EB:B3:92:B3");
    http.addHeader("x-api-key", apikey);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(ClimateDATA());
    if(httpCode > 0) {
      if(httpCode == HTTP_CODE_OK) {
        Serial.println("✉+☁=✔");
        attempt = 3;
      } else { Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str()); delay(2000); }
    } else { Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str()); delay(2000); }
    attempt++;
    http.end();
  }
}


// The DHT22 ClimateDATA function to collect the data
String ClimateDATA()
{
  long datetime = ntpClient.getUnixTime();
  float t = 0;
  float h = 0;
  
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  
  JSONencoder["TableName"] = "climatedata";
  JsonObject& Item = JSONencoder.createNestedObject("Item");
  Item["id"] = unitid + "_" + datetime;
  Item["datestamp"] = datetime;
  Item["unit"] = unitid;
  JsonObject& climatedata = Item.createNestedObject("climatedata");
  climatedata["humidity"] = h = dht.readHumidity();
  climatedata["temperature"] = t = dht.readTemperature();
  climatedata["heatindex"] = dht.computeHeatIndex(t, h, false);
  
  char JSONmessageBuffer[300];
  JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));

  return JSONmessageBuffer;
}
