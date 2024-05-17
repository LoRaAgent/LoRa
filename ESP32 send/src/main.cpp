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
    // LoRa.onReceive(onReceive);
    // LoRa.receive();
    Serial.println("LoRa init succeeded.");
}

void loop(){
    //LoRa.receive();
    // if(onReceive()== true){
    // }
    // onReceive();
    // delay(1000);
    if(state == 1){
        // LoRa.receive();
        // Serial.println("state1");
        // Serial.println(SuccessReceived);
        sendMessage("hello world");
        if(SuccessReceived == true){
            state = 2;
            Serial.println("chage");
            SuccessReceived = false;
        }else{
            state = 1;
        }
    }else if(state == 2){
        onReceive();
        Serial.println("state2");
        state = 1;
        delay(1000);
    }
    // sendMessage("test");
    // delay(1000);
}