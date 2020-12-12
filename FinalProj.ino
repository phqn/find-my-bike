/*
 * Final Project - CS 147
 * Peter Nguyen
 * Austin Vigo
 */



#include <TinyGPS++.h>
#include "SparkFunLSM6DS3.h"
#include "Wire.h"
#include "SPI.h"
#include "rgb_lcd.h"
#include "WiFiEsp.h"

#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(10, 11);
#endif

TinyGPSPlus gps;
SoftwareSerial Serial2(4, 3);

//char ssid[] = "Rio Home 2G";
//char pass[] = "2000766957";
char ssid[] = "Nguyen_Family";
char pass[] = "Poppy11628";
int status = WL_IDLE_STATUS;
//char server[] = "3.21.230.178";
char server[] = "18.222.172.137";
char get_request[200];
WiFiEspClient client;

// LCD, Button, Buzzer, Accelerometer
rgb_lcd lcd;
const int buttonPin = 2;
int buttonState = 0;
LSM6DS3 myIMU;
int thresholdValue = 2;

void setup()
{
  // Open serial communications
  Serial.begin(9600);

  // Start software serial ports, last initialized port is listening
  Serial1.begin(115200);
  Serial2.begin(9600);


//  WiFi.init(&Serial1);
//
//  if (WiFi.status() == WL_NO_SHIELD) {
//    Serial.println("WiFi shield not present");
//    while (true);
//  }
//
//  while (status != WL_CONNECTED) {
//    Serial.print("Attempting to connect to WPA SSID: ");
//    Serial.println(ssid);
//    status = WiFi.begin(ssid, pass);
//  }
//  Serial.println("You're connected to the network");
//  printWifiStatus();

   // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  
  // Print a message to the LCD.
  lcd.print("Alarm Set");

  // Button
  pinMode(buttonPin, INPUT);
  
  // Buzzer
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);

  Serial.println("Starting Project");
}


bool sending = false;
bool request = false;
bool buttonPressed = false;
int parameter;
int longitude = 0;
int latitude = 0;
void loop()
{ 
  // Listen to GPS
  Serial2.listen();
  while (Serial2.available() > 0) {
    gps.encode(Serial2.read());
    if (gps.location.isUpdated()) {
      latitude = gps.location.lat();
      longitude = gps.location.lng();
      break;
    }
  }
    
  // Read Accelerometer values:
  float x = myIMU.readFloatAccelX();
  float y = myIMU.readFloatAccelY();
  
  //LCD
  lcd.setCursor(0, 1);

  // Sound alarm if accelerometer is moved
  if (x > 2 || y > 2 && sending == false) {
    // Sounds alarm
    Serial.println("ALARM SOUND");
    sending = true;
    lcd.print("Moved");
    digitalWrite(6, HIGH);
    digitalWrite(8, HIGH);
    parameter = 1;
    request = true;
    sending = false;
  }
  // Use button to turn off alarm
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH && sending == false) {
    sending = true;
    digitalWrite(6, LOW);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Alarm Set");
    digitalWrite(8, LOW);
    parameter = 2;
    request = true;
    sending = false;
    buttonPressed = true;
  }

  // Send request
  if (request == true) {
    //Listen to WiFi
    Serial1.listen();
    WiFi.init(&Serial1);
  
    if (WiFi.status() == WL_NO_SHIELD) {
      Serial.println("WiFi shield not present");
    }
  
    while (status != WL_CONNECTED) {
      Serial.print("Attempting to connect to WPA SSID: ");
      Serial.println(ssid);
      status = WiFi.begin(ssid, pass);
    }
    Serial.println("You're connected to the network");
    printWifiStatus();

    Serial.println("SENDING REQUEST");
    request = false;
    if (!client.connected()) {
      Serial.println("Starting connection to server..");
      client.connect(server, 5000);
    }
    Serial.println("Connected to server");

    // Handle Get Requests for button & alarms
    if (buttonPressed == true) {
      Serial.println("Button pressed");
      Serial.println(latitude);
      Serial.println(longitude);
      //sprintf(get_request,"GET /?type=%d HTTP/1.1\r\nHost: 18.221.147.67\r\nConnection: close\r\n\r\n", lng);
      sprintf(get_request,"GET /?type=%d&lat=%d&lon=%d HTTP/1.1\r\nHost: 18.221.147.67\r\nConnection: close\r\n\r\n", 2, latitude, longitude);
      buttonPressed = false;
    } else {
      sprintf(get_request,"GET /?type=%d HTTP/1.1\r\nHost: 18.221.147.67\r\nConnection: close\r\n\r\n", parameter);
    }
    Serial.println(get_request);
    client.print(get_request);
    delay(20000);
    Serial.println("Done with request");
  }
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
