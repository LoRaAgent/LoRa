#include <Arduino.h>
#include <LoRa.h>
#include <EasyButton.h>
#ifndef loratx_rx
#define loratx_rx
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26
#define BAND 923E6
extern int swPumpPressed ;
extern bool pumpState ;
extern EasyButton buttonMode;
extern EasyButton buttonPump;
extern EasyButton buttonModePump;
#endif

extern const int csPin = 7;          // LoRa radio chip select
extern const int resetPin = 6;       // LoRa radio reset
extern const int irqPin = 1;         // change for your board; must be a hardware interrupt pin

byte localAddress = 0xC1;     // address of this device
byte destination = 0xBB;      // destination to send to
extern bool sendSuccess = false;
extern bool resendData = false;

unsigned long previousMillis = 0;
extern String pumpData = "";
extern String receiveData = "";

bool isAlphaNumericOrComma(char c);
bool hasNonAlphaNumericChars(String str);

void sendMessage(String outgoing) {//บางครั้งไม่ส่งมาแก้ด้วย เช็คtry cath
  // delay(10);
  Serial.println("Make send LoRa");
  if (LoRa.beginPacket() == 0) {
    Serial.println("Error: Begin packet failed");
    return;
  }
  if (LoRa.write(destination) != 1) {
    Serial.println("Error: Write destination failed");
    LoRa.endPacket(); // Make sure to end the packet before returning
    return;
  }
  if (LoRa.write(localAddress) != 1) {
    Serial.println("Error: Write local address failed");
    LoRa.endPacket();
    return;
  }
  if (LoRa.write(outgoing.length()) != 1) {
    Serial.println("Error: Write payload length failed");
    LoRa.endPacket();
    return;
  }
  if (LoRa.print(outgoing) != outgoing.length()) {
    Serial.println("Error: Write payload failed");
    LoRa.endPacket();
    return;
  }
  if (LoRa.endPacket() == false) {
    Serial.println("Error: End packet failed");
    return;
  }
  Serial.println("Message sent.");
  // Serial.print(" Make  sendMessage :");
  // delay(500);
  // LoRa.beginPacket();                   // start packet
  // LoRa.write(destination);              // add destination address
  // LoRa.write(localAddress);             // add sender address
  // LoRa.write(outgoing.length());        // add payload length
  // LoRa.print(outgoing);                 // add payload
  // LoRa.endPacket();                     // finish packet and send it                          // increment message ID
  // Serial.print(" destination :");
  // Serial.println(destination);
  // Serial.print(" localAddress :");
  // Serial.println(localAddress);
  // Serial.print(" outgoing.length :");
  // Serial.println(outgoing.length());
  // Serial.print(" outgoing :");
  // Serial.println(outgoing);
}

void onReceive() {
    int count = 0;
    while(true){
        int packetSize = LoRa.parsePacket();
        if (packetSize) {          // if there's no packet, return
            String rawPayload = "";
            for (int i = 0; i < packetSize; i++) {
                char currentChar = (char)LoRa.read();
                if (currentChar == ' ') {
                rawPayload += ",";
                } else {
                rawPayload += currentChar;
                }
            }
            Serial.print("rawPayload");
            Serial.println(rawPayload);
            if (hasNonAlphaNumericChars(rawPayload)) {
              Serial.println("String มีตัวอักขระที่ไม่ใช่ A-Z, 0-9 และ ,");
            } else {
              Serial.println("String เป็น A-Z, 0-9 และ , เท่านั้น");
              Serial.println(rawPayload);
              // Split the rawPayload into an array using " " (space) as a delimiter
              int arraySize = 0;
              String payloadArray[200]; // Assuming a maximum of 20 elements
              payloadArray[arraySize++] = ""; // Initialize the first element
              for (int i = 0; i < rawPayload.length(); i++) {
                  char currentChar = rawPayload.charAt(i);
                  if (currentChar == ',') {
                  payloadArray[arraySize++] = ""; // Move to the next element
                  } else {
                  payloadArray[arraySize - 1] += currentChar; // Add the character to the current element
                  }
              }
              String recipient = "0x"+String(payloadArray[0].toInt(), HEX);          // recipient address
              String sender = "0x"+String(payloadArray[1].toInt(), HEX);            // sender address
              String incomingLength = payloadArray[2];    // incoming msg length
              if(recipient == "0xc1" && sender == "0xbb"){
                receiveData = rawPayload;
                sendSuccess = true;
                resendData = false;
                break;
              }
            }
            
            //Serial.println("RSSI: " + String(LoRa.packetRssi()));
            //Serial.println("Snr: " + String(LoRa.packetSnr()));  
        }else {
            unsigned long currentMillis = millis();
            if (currentMillis - previousMillis >= 1000) {
                previousMillis = currentMillis;
                count++;
                Serial.println("count");
                Serial.println(count);
            }
            if(count == 10){
                Serial.println("break");
                //Serial.println("This message is not for me.");
                sendSuccess = false;
                resendData = true;
                break;
            }
        // }else if(sendSuccess == true && resendData == false){
        //     // break;
        // }
        }
      buttonMode.read();
      buttonPump.read();
      buttonModePump.read();
      if(swPumpPressed == 1){
        swPumpPressed = 0;
        sendSuccess = false;
        resendData = true;
        Serial.println("---------------------------------swPumpPressed-----------------------------------");
        break;
      }
    }
  
}

bool hasNonAlphaNumericChars(String str) {
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