#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>

char ssid[] = "wifi-id";   // your network SSID (name)
char pass[] = "password";  // your network password

#define PIEZO_PIN 5

#include "ServoEasing.hpp"
#define SERVO1_PIN 19

ServoEasing Servo1;
int startPos = 1;


WiFiUDP Udp;
OSCErrorCode error;
const unsigned int localPort = 8001;     // local port to listen for UDP packets
const unsigned int outPort = 8000;       // port we send packets to
const IPAddress outIp(192, 168, 1, 36);  // IP address of your computer
int pbuttval;                            // variable to hold the value of a button

void setup() {
  Serial.begin(115200);

  if (Servo1.attach(SERVO1_PIN, startPos) == INVALID_SERVO) {
    Serial.println("Error attaching servo");
  }
  pinMode(LED_BUILTIN, OUTPUT);

  delay(1000);

  pinMode(21, INPUT_PULLUP);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  if(WiFi.status() == WL_CONNECTED){
    digitalWrite(LED_BUILTIN, HIGH);
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");

  Serial.println(localPort);
}

void loop() {
  int buttval = digitalRead(21);
  OSCMessage msg;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      msg.fill(Udp.read());
    }
    if (!msg.hasError()) {
      msg.dispatch("/servoVal", servoControl);  //send to Servo Control
      msg.dispatch("/freq", playBuzzer);
    } else {
      error = msg.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }


  if (buttval != pbuttval) {
    sendMessage(buttval);
  }
  pbuttval = buttval;
}



void sendMessage(int value) {
  OSCMessage msg("/touched");
  msg.add(value);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}

void servoControl(OSCMessage &msg) {
  int value = msg.getInt(0);
  Serial.print("/servoVal: ");
  Serial.println(value);
  Servo1.setEasingType(EASE_BOUNCE_OUT);
  Servo1.startEaseTo(value, 100);
  delay(100);
}

void playBuzzer(OSCMessage &msg) {
  float freqVal = msg.getFloat(0);
  Serial.print("/freq: ");
  Serial.println(freqVal);
  tone(PIEZO_PIN, freqVal, 1000);
}