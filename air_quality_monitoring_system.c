/*---------------------------------------------------------
 * Title: Air Quality Monitoring System
 * Author: Zaid Ali
 * Email: zaidali.za2635@gmail.com
 * Description:
 * This program reads air quality using the MQ135 sensor,
 * displays it on a 16x2 LCD, and sends real-time data
 * over Wi-Fi using the ESP8266 module. It also uses
 * a warning LED and a simple HTTP server.
 *---------------------------------------------------------*/

#include "MQ135.h"
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

#define DEBUG true

// Setup software serial pins for ESP8266
SoftwareSerial esp8266(9, 10); // RX, TX

// LCD pin mapping
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Sensor pin and variables
const int sensorPin = A0;
unsigned long systemStartTime = 0;
int air_quality;

void setup() {
  pinMode(sensorPin, INPUT);
  pinMode(8, OUTPUT); // Buzzer or LED indicator pin

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("circuitdigest");
  lcd.setCursor(0, 1);
  lcd.print("Sensor Warming...");
  delay(3000);

  // Start serial connections
  Serial.begin(115200);
  esp8266.begin(115200);

  // ESP8266 Configuration
  sendData("AT+RST\r\n", 2000, DEBUG);           // Reset module
  sendData("AT+CWMODE=2\r\n", 1000, DEBUG);      // Set to AP mode
  sendData("AT+CIFSR\r\n", 1000, DEBUG);         // Get IP address
  sendData("AT+CIPMUX=1\r\n", 1000, DEBUG);      // Enable multiple connections
  sendData("AT+CIPSERVER=1,80\r\n", 1000, DEBUG);// Start server on port 80

  lcd.clear();
  systemStartTime = millis(); // Record system start time
}

void loop() {
  MQ135 gasSensor = MQ135(sensorPin);
  float air_quality = gasSensor.getPPM();

  // Handle HTTP Requests
  if (esp8266.available()) {
    if (esp8266.find("+IPD,")) {
      delay(1000);
      int connectionId = esp8266.read() - 48;

      String webpage = "<html><head><title>Air Quality Monitor</title></head><body>";
      webpage += "<h1>IOT Air Pollution Monitoring</h1>";
      webpage += "<p><strong>Air Quality:</strong> ";
      webpage += air_quality;
      webpage += " PPM</p>";

      if (air_quality <= 1000) {
        webpage += "<p>Status: Fresh Air</p>";
      } else if (air_quality <= 2000) {
        webpage += "<p>Status: Poor Air</p>";
      } else {
        webpage += "<p>Status: <b>Danger!</b> Move to Fresh Air</p>";
      }

      // Show uptime
      unsigned long uptime = (millis() - systemStartTime) / 1000;
      webpage += "<p>System Uptime: ";
      webpage += uptime;
      webpage += " seconds</p>";

      webpage += "</body></html>";

      // Send response
      String cipSend = "AT+CIPSEND=" + String(connectionId) + "," + String(webpage.length()) + "\r\n";
      sendData(cipSend, 1000, DEBUG);
      sendData(webpage, 1000, DEBUG);

      // Close connection
      String closeCommand = "AT+CIPCLOSE=" + String(connectionId) + "\r\n";
      sendData(closeCommand, 1000, DEBUG);
    }
  }

  // LCD Output
  lcd.setCursor(0, 0);
  lcd.print("Air: ");
  lcd.print(air_quality);
  lcd.print(" PPM     ");

  lcd.setCursor(0, 1);
  if (air_quality <= 1000) {
    lcd.print("Status: Fresh Air ");
    digitalWrite(8, LOW);
  } else if (air_quality <= 2000) {
    lcd.print("Status: Poor Air  ");
    digitalWrite(8, HIGH);
  } else {
    lcd.print("!!!DANGEROUS!!!   ");
    digitalWrite(8, HIGH);
  }

  delay(1000); // Delay for 1 second
}

// Function to send AT command and get response from ESP8266
String sendData(String command, const int timeout, boolean debug) {
  String response = "";
  esp8266.print(command);
  long int time = millis();

  while ((millis() - time) < timeout) {
    while (esp8266.available()) {
      char c = esp8266.read();
      response += c;
    }
  }

  if (debug) {
    Serial.print("[ESP8266 Response] ");
    Serial.println(response);
  }

  return response;
}

/* 
 * Contact:
 * 👨‍💻 Zaid Ali
 * 📧 zaidali.za2635@gmail.com
 * 📞 +91 7275591869
 * 📍 Kanpur, India
 * 💻 Project: Air Quality Monitoring System (IoT + LCD + ESP8266)
 */
