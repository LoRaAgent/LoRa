#include <LoRarxtx.h>
#include <LoRa.h>
#include <SPI.h>
#include <Arduino.h>

long lastSendTime = 0;        // last send time
int interval = 2000;        // interval between sends
void setup(){
    Serial.begin(115200);
    LoRa.setPins(SS, RST, DIO0);
    LoRa.setTxPower(18);
    SPI.begin(SCK, MISO, MOSI, SS);
    while(!LoRa.begin(915E6)){
        Serial.println("LoRa Initailization failed!");
        delay(500);
    }
    LoRa.onReceive(onReceive);
    LoRa.receive();
    Serial.println("LoRa init succeeded.");
}

void loop(){
    LoRa.receive();
    delay(2000);
    sendMessage("test");
    delay(2000);
}