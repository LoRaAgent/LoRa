#include <Arduino.h>
#include <LoRa.h>
#ifndef loratx_rx
#define loratx_rx
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26
#define BAND 915E6
#endif

extern const int csPin = 7;          // LoRa radio chip select
extern const int resetPin = 6;       // LoRa radio reset
extern const int irqPin = 1;         // change for your board; must be a hardware interrupt pin

byte localAddress = 0xaf;     // address of this device
String localAddressString = "0xaf";
byte destination = 0x23;      // destination to send to
String destinationAddressString = "0x23";
extern bool SuccessReceived = false;
extern bool resendData = false;
unsigned long previousMillis = 0;
extern bool receiveModeData = false;
extern String receiveData = "";

bool isAlphaNumericOrComma(char c);
bool hasNonAlphaNumericChars(String str);

void sendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packet
  // LoRa.write(destination);              // add destination address
  // LoRa.write(localAddress);             // add sender address
  // LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it                          // increment message ID
  // Serial.print(" destination :");
  // Serial.println(destination,HEX);
  // Serial.print(" localAddress :");
  // Serial.println(localAddress,HEX);
  // Serial.print(" outgoing.length :");
  // Serial.println(outgoing.length());
  Serial.print(" outgoing :");
  Serial.println(outgoing);
}


void onReceive() {
  String saveData = "";
  int packetSize = LoRa.parsePacket();
  if (packetSize) { // if there's a packet, process it
    // read packet
    for (int i = 0; i < packetSize; i++) {
      char data = (char)LoRa.read();
      saveData += data;
    }
    // print RSSI of packet
    Serial.print("Received packet '");
    Serial.print(saveData);
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
    SuccessReceived = true; // Set SuccessReceived to true if a packet was processed
  }
}

bool hasNonAlphaNumericChars(String str){
  for (int i = 0; i < str.length(); i++) {
    char currentChar = str[i];
    if (!isAlphaNumericOrComma(currentChar)) {
      return true;  // พบตัวอักขระที่ไม่ใช่ A-Z, 0-9 และ ,
    }
  }
  return false;  // ไม่พบตัวอักขระที่ไม่ใช่ A-Z, 0-9 และ ,
}

bool isAlphaNumericOrComma(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c == ',') || (c == '.');
}