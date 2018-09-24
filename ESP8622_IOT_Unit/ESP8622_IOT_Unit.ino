#include <EasyNTPClient.h>
#include <WiFiUdp.h>

#include <HttpClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>


char ssid[] = "<SSID>";
char pass[] = "<PASSWORD>";
String hostURL = "<HOSTURL>";
String Apikey = "<APIKEY>";


WiFiUDP ntpUDP;
EasyNTPClient ntpClient(ntpUDP, "pool.ntp.org", 7200);


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(74880);
  delay(100);
  
  ConnectWifi();
}

void ConnectWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  delay(100);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void loop(){
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000); 
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(2000); 
   
  post();

  delay(10000); 
  //delay(600000);   
}

void post()
{
  String unitID = "7560f9";
  long datetime = ntpClient.getUnixTime();
  
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  
  JSONencoder["TableName"] = "climatedata";
  JsonObject& Item = JSONencoder.createNestedObject("Item");
  Item["id"] = unitID + "_" + datetime;
  Item["datestamp"] = datetime;
  Item["unit"] = unitID;
  JsonObject& climatedata = Item.createNestedObject("climatedata");
  climatedata["humidity"] = 50;
  climatedata["temperature"] = 23;
  
  char JSONmessageBuffer[300];
  JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  
  HTTPClient http; 
  http.begin(hostURL, "2D:A5:86:8A:33:0B:81:91:EB:58:55:D7:BD:C6:3D:1B:EB:B3:92:B3");
  http.addHeader("x-api-key", Apikey);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(JSONmessageBuffer);
  if(httpCode > 0) {
    //Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    if(httpCode == HTTP_CODE_OK) {
      //String payload = http.getString();
      Serial.println("✉+☁=✔");
    }
  } else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}
