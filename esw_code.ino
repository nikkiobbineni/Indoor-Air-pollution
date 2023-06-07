#include <WiFi.h>
#include <WebServer.h>
#include <ThingSpeak.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include "Adafruit_SGP40.h"
#include "Adafruit_SHT4x.h"
#include "HTTPClient.h"
#include "time.h"
#include <ArduinoJson.h>

Adafruit_SGP40 sgp;
Adafruit_SHT4x sht4 = Adafruit_SHT4x();

const char* ssid = "Galaxy";   // your network SSID (name) 
const char* password = "12345678";   // your network password

WiFiClient  client;

String cse_ip = "192.168.209.51"; // YOUR IP from ipconfig/ifconfig
String cse_port = "8080";
String server = "http://" + cse_ip + ":" + cse_port + "/~/in-cse/in-name/";
String ae = "CO2";
String cnt = "Node_1";

void createCI(String &val){
HTTPClient http;
http.begin(server + ae + "/" + cnt + "/");
http.addHeader("X-M2M-Origin", "admin:admin");
http.addHeader("Content-Type", "application/json;ty=4");
int code = http.POST("{\"m2m:cin\": {\"cnf\":\"application/json\",\"con\": \"" + String(val) + "\"}}");
if(code==-1)
{
   Serial.println("Not correct");
}
http.end();
}
  
unsigned long myChannelNumber = 1916314;
const char * myWriteAPIKey = "GLWI4CVJWF9V935D";

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

// Variable to hold temperature readings
int val;
//uncomment if you want to get temperature in Fahrenheit
//float temperatureF;

#include <SPI.h>
#include <Wire.h>

unsigned long duration, th, tl;
int ppm;
#define PIN 14

unsigned long previous_loop, previous_10, previous_25, prev_time;

byte command_frame[9] = {0xAA, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x67, 0xBB};
byte received_data[9];
int sum = 0;
void send_command(byte command)
{
  command_frame[1] = command;
  int sum = command_frame[0] + command_frame[1] + command_frame[2] + command_frame[3] + command_frame[4] + command_frame[5] + command_frame[8];
  int rem = sum % 256;
  command_frame[6] = (sum - rem) / 256;
  command_frame[7] = rem;
  delay(1000);
  Serial.write(command_frame, 9);

}

bool checksum()
{
  sum = int(received_data[0]) + int(received_data[1]) + int(received_data[2]) + int(received_data[3]) + int(received_data[4]) + int(received_data[5]) + int(received_data[8]);
  if (sum == ((int(received_data[6]) * 256) + int(received_data[7])))
  {
    return true;
  }
  else
    return false;
}
void calculate_pm()
{
  int pm2 = int(received_data[4]) * 256 + int(received_data[5]);
  delay(2000);
  int pm10 = int(received_data[2]) * 256 + int(received_data[3]);
  ThingSpeak.setField(5, pm2);
  ThingSpeak.setField(6, pm10);
  Serial.print("PM2.5: ");
  Serial.println(pm2);
  Serial.print("PM10: ");
  Serial.println(pm10);
}

void PM_setup() {
  send_command(0x01);
}

void PM_Reading() {
  delay(5000);
  if (millis() - prev_time > 5000)
  {
    send_command(0x02);
    prev_time = millis();
  }
  if (Serial.available())
  {
 Serial.println("SErial inpu/t available");
    Serial.readBytes(received_data, 9);
//    if (checksum())/
//    {/
      calculate_pm();
//    }/
  }
}

void setup() {
  Serial.begin(9600);  //Initialize serial
  
  WiFi.mode(WIFI_STA);   
  if (!sgp.begin()){
    
    Serial.println("SGP40 sensor not found :(");
    while (1);
    
  }

  if (!sht4.begin()) {
    
    Serial.println("Couldn't find SHT4x");
    while (1)
    delay(1);
    
  }
  Serial.print("Found SHT4x + SGP40 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
  ThingSpeak.begin(client);  // Initialize ThingSpeak
   PM_setup();
}

void loop() {
   PM_Reading();
  if ((millis() - lastTime) > timerDelay) {
    
    // Connect or reconnect to WiFi
    if(WiFi.status() != WL_CONNECTED){
      Serial.print("Attempting to connect");
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password); 
        delay(5000);     
      } 
      Serial.println("\nConnected.");
    }

    th = pulseIn(PIN, HIGH, 2008000) / 1000;
    tl = 1004 - th;
    ppm = 2000 * (th - 2) / (th + tl - 4);
    float h =ppm;
    String ans=String(h);
    createCI(ans);
    if (ppm > 2000) {
    Serial.println(ppm);
    ThingSpeak.setField(1,h);
     
  }
  else {
    Serial.print(" Co2 Concentration: ");
    Serial.println(ppm);
    ThingSpeak.setField(1,h);
    
  }
  if (isnan(h)) {
    Serial.println("Failed!");
    return;
  }
  uint16_t sraw;
  int32_t voc_index;
  
  sensors_event_t humidity, temp;
  sht4.getEvent(&humidity, &temp);

  if (isnan(temp.temperature) || isnan(humidity.relative_humidity)) {
    return;
  }
    voc_index = sgp.measureVocIndex(temp.temperature, humidity.relative_humidity);
    float a,b,c;
    a=voc_index;
    b=temp.temperature;
    c=humidity.relative_humidity;
    sraw = sgp.measureRaw(temp.temperature, humidity.relative_humidity);   
    Serial.print("Raw measurement: ");
    Serial.println(sraw); 
    Serial.print("VOC");
    Serial.println(a);
    ThingSpeak.setField(2,a);
    Serial.print("Temperature");
    Serial.println(b);
    ThingSpeak.setField(3,b);
    Serial.print("Humidity");
    Serial.println(c);
    ThingSpeak.setField(4,c);
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
//   -
ThingSpeak.writeField(myChannelNumber, 2, a, myWriteAPIKey);
 
    
    if(x== 200){
      Serial.println("Channel update successful.");
    }
    else{
//      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    lastTime = millis();
  }

}