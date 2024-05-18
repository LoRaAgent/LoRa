#include <LoRarxtx.h>
#include <LoRa.h>
#include <SPI.h>
#include <Arduino.h>
long lastSendTime = 0;        // last send time
int interval = 2000;        // interval between sends
int state = 1;
void setup(){
    Serial.begin(115200);
    LoRa.setPins(SS, RST, DIO0);
    LoRa.setTxPower(18);
    SPI.begin(SCK, MISO, MOSI, SS);
    while(!LoRa.begin(915E6)){
        Serial.println("LoRa Initailization failed!");
        delay(500);
    }
    LoRa.setSpreadingFactor(7);          //testing Spreading factors can chage 7-12
    Serial.println("LoRa init succeeded.");
}

void loop(){
    if(state == 1){
        sendMessage("hello world");
        state = 2;
    }else if(state == 2){
        onReceive();
        Serial.println("state2");
        if(SuccessReceived == true){
            state = 1;
            Serial.println("change");
            SuccessReceived = false;
        }else if(SuccessReceived == false){
            state = 2;
        }
        delay(1000);
    }
}