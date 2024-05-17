#include <LoRarxtx.h>
#include <LoRa.h>
#include <SPI.h>
#include <Arduino.h>

long lastSendTime = 0;        // last send time
int interval = 2000;        // interval between sends
void setup(){
    Serial.begin(115200);
    LoRa.setPins(SS, RST, DIO0);
    LoRa.setTxPower(12);
    SPI.begin(SCK, MISO, MOSI, SS);
    while(!LoRa.begin(923E6)){
        Serial.println("LoRa Initailization failed!");
        delay(500);
    }
    Serial.println("LoRa init succeeded.");
    Serial.println();
    Serial.println("LoRa Simple Node");
    Serial.println("Only receive messages from gateways");
    Serial.println("Tx: invertIQ disable");
    Serial.println("Rx: invertIQ enable");
    Serial.println();

    LoRa.onReceive(onReceive);
    LoRa.onTxDone(onTxDone);
    LoRa_rxMode();
}

void loop(){
    if (runEvery(1000)) { // repeat every 1000 millis

        String message = "HeLoRa World! ";
        message += "I'm a Node! ";
        message += millis();

        LoRa_sendMessage(message); // send a message

        Serial.println("Send Message!");
    }
    // // LoRa.onReceive(onReceive);
    // int ack = LoRa.parsePacket();
    // if(ack){
    //     Serial.println("Ack recieved");
    // }else{
    //     Serial.println("Ack not recieved");
    // }
    // int rssi = LoRa.packetRssi();
    // Serial.print("RSSI: ");
    // Serial.println(rssi);
    delay(5000);
}