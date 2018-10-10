
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
String datadbURL = "<HOSTURL>";
String apikey = "<APIKEY>";
String unitid = "<UNITID>";
String SHA1fingerprint = "<SHA1KEY>"

int counter = 0;

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
    http.begin(datadbURL, SHA1fingerprint);
    http.addHeader("x-api-key", apikey);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(ClimateDATA());
    if(httpCode > 0) {
      if(httpCode == HTTP_CODE_OK) {
        Serial.print(counter);
        Serial.println("✉+☁=✔");
        attempt = 3;
      } else { Serial.printf("[HTTP_CODE_FAIL] POST... failed, error: %s\n", http.errorToString(httpCode).c_str()); delay(2000); }
    } else { Serial.printf("[HTTP_ERROR] POST... failed, error: %s\n", http.errorToString(httpCode).c_str()); delay(2000); }
    attempt++;
    http.end();
  }
  counter++;
}


// The DHT22 ClimateDATA function to collect the data
String ClimateDATA()
{
  long datetime = 0;
  while(datetime == 0) { datetime = ntpClient.getUnixTime(); }
  
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if(isnan(t)) { t = dht.readTemperature(); }
  if(isnan(h)) { h = dht.readHumidity(); }
  
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  JSONencoder["datestamp"] = datetime;
  JSONencoder["unitid"] = unitid;
  JsonObject& climatedata = JSONencoder.createNestedObject("climatedata");
  climatedata["temperature"] = t;
  climatedata["humidity"] = h;
  climatedata["heatindex"] = dht.computeHeatIndex(t, h, false);
  
  char JSONmessageBuffer[300];
  JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));

  //Serial.print(JSONmessageBuffer); for testing

  return JSONmessageBuffer;
}
