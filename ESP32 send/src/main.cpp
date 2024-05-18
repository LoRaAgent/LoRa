#include <LoRarxtx.h>
#include <LoRa.h>
#include <SPI.h>
#include <Arduino.h>
long lastSendTime = 0;        // last send time
int interval = 1000;        // interval between sends
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
    //onReceive();
        if(state == 1){
            onReceive();
            if(SuccessReceived == true){
                state = 2;
                Serial.println("change");
                SuccessReceived = false;
            }else{
                if (millis() - lastSendTime > interval) {
                    lastSendTime = millis();
                    Serial.println("Waiting");
                    state = 1;
                }
            }
        }else if(state == 2){
            for(int i=0;i<1;i++)
                sendMessage("hello world");
                delay(1000);
            state = 1;
        }
}