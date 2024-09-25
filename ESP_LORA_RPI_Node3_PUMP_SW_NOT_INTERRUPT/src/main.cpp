#include <Arduino.h>
#include <SPI.h>              
#include <LoRa.h>
#include <sensorRead.h>
#include <loraTxRx.h>
#include <Adafruit_INA219.h>
#include <EasyButton.h>
#include <EEPROM.h>
 

#define ONE               1
#define TWO               2
#define THREE             3
#define FOUR              4
#define FIVE              5
#define SIX               6
#define SEVEN             7
#define BUTTON_MODE       4
#define BUTTON_PUMP       15
#define LED_MODE_PUMP     33   
#define BUTTON_MODE_PUMP  25  
#define LED_STATUS        32
#define LED_PUMP          17
#define RelayOn              0
#define RelayOff              2
//====================================EEPROM Address==============================
#define addr_pumpMode        0x10
#define addr_mode            0x20
#define addr_pumpState       0x30
#define addr_statusNode      0x40
#define addr_timeNormalMode  0x50
#define addr_timeDebugMode   0x60

#define BUTTON_PIN_BITMASK 0x2008000 // GPIO 15 and GPIO 25 = (2^15+2^25) HEX


EasyButton buttonMode(BUTTON_MODE);
EasyButton buttonPump(BUTTON_PUMP);
EasyButton buttonModePump(BUTTON_MODE_PUMP);
gpio_num_t wakeup_gpio_button_mode = GPIO_NUM_4;
int wakeup_gpio_button_pump = 15;
gpio_num_t ledModePump_gpio = GPIO_NUM_33;
gpio_num_t ledStatus_gpio = GPIO_NUM_32;
gpio_num_t ledPump_gpio = GPIO_NUM_17;
gpio_num_t pump_gpio = GPIO_NUM_2;
// Adafruit_INA219 ina219;
///////////////////////////////////////////////
float overCurrent = 2.0;  // 2.0 A
bool pumpMode = false;
bool mode = false;
bool pumpState = false;
bool pumpStatus = false;
bool statusNode = false;
float timeNormalMode = 3;
float timeDebugMode = 1;

int state = 1;
bool stateSendData = false;
int countSendData = 0;
int countPumpMode = 0;
String Mymessage = "";
int swPumpPressed = 0;
int flag = 0;

unsigned long previousMillisCount = 0;
unsigned long previousMillisCheckPump = 0;
bool previousPumpStatus = false;
bool previousModePump = false;
void pumpOn(bool status);
void checkStatePump();
bool updateConfig(String receiveData);
void updateEEPROM();
void setMode(bool mode);
void setPump(bool pumpState);
void setPumpMode(bool pumpMode);
void updatePump(bool status);
void updateStatus(bool status);
void updateModePump(bool status);

void onModePressed(){
  Serial.println("onModePressed pressed");
  mode = !mode;
  swPumpPressed = 1;
}
void onPumpPressed(){
  Serial.println("onPumpPressed pressed");
  pumpState = !pumpState;
  swPumpPressed = 1;
  countSendData = 0;
  countPumpMode = 0;
  state = ONE;
}
void onPumpModePressed(){
  Serial.println("onPumpModePressed");
  pumpMode = !pumpMode;
  swPumpPressed = 1;
  countPumpMode = 0;
  countSendData = 0;
  state = ONE;
}
void check_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 :  Serial.println("Wakeup caused by external signal using RTC_IO GPIO -------> 4");
                                  //Serial.print("mode IN");Serial.println(mode); 
                                  //mode = !mode;
                                  //Serial.print("mode OUT");Serial.println(mode);
                                  //setMode(mode); 
                                  break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}
void check_GPIO_wake_up(){
  uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  Serial.print("GPIO that triggered the wake up: GPIO ");
  Serial.println((log(GPIO_reason))/log(2), 0);
   switch((uint32_t)(log2(GPIO_reason)))
  {
    case 15 : Serial.print("GPIO ------->"); Serial.println((uint32_t)(log2(GPIO_reason)));
              Serial.print("STATUS PUMP IN  ");Serial.println(pumpState); 
              pumpState = !pumpState;
              Serial.print("mode OUT");Serial.println(pumpState);
              setPump(pumpState);
              break;
    case 25 :Serial.print("GPIO ------->");  Serial.println((uint32_t)(log2(GPIO_reason)));
              Serial.print("pumpMode PUMP IN  ");Serial.println(pumpMode); 
              pumpMode = !pumpMode;
              Serial.print("pumpMode OUT");Serial.println(pumpMode);
              setPumpMode(pumpMode);   break;
    default : Serial.printf("GPIO NO!!! "); break;
  }
}
void setup() {
  Serial.begin(115200);
  EEPROM.begin(512); // You can change the size based on your requirements
  /////////////////////////////////////////
  //Set data to EEPROM
  // EEPROM.put(addr_pumpMode, pumpMode);
  // EEPROM.put(addr_mode, mode);
  // EEPROM.put(addr_pumpState, pumpState);
  // EEPROM.put(addr_statusNode, statusNode);
  // EEPROM.put(addr_timeNormalMode, timeNormalMode);
  // EEPROM.put(addr_timeDebugMode, timeDebugMode);
  // EEPROM.commit() ;
  EEPROM.get(addr_pumpMode, pumpMode);
  EEPROM.get(addr_mode, mode);
  EEPROM.get(addr_pumpState, pumpState);
  EEPROM.get(addr_statusNode, statusNode);
  EEPROM.get(addr_timeNormalMode, timeNormalMode);
  EEPROM.get(addr_timeDebugMode, timeDebugMode);
  //////////////////////////////////////////////////////////////////////////////
  pinMode(BUTTON_MODE,INPUT_PULLUP);
  // pinMode(BUTTON_PUMP,INPUT);
  // pinMode(BUTTON_MODE_PUMP,INPUT); 
  pinMode(LED_MODE_PUMP,OUTPUT);
  pinMode(LED_STATUS,OUTPUT);
  pinMode(LED_PUMP,OUTPUT);
  pinMode(RelayOn,OUTPUT);
  pinMode(RelayOff,OUTPUT);
  digitalWrite(LED_PUMP, HIGH);
  digitalWrite(LED_STATUS, HIGH);
  digitalWrite(LED_MODE_PUMP, HIGH);
  buttonMode.begin();
  buttonPump.begin();
  buttonModePump.begin();
  buttonMode.onPressed(onModePressed);
  buttonPump.onPressed(onPumpPressed);
  buttonModePump.onPressed(onPumpModePressed);
  check_wakeup_reason();
  check_GPIO_wake_up();
  esp_sleep_enable_ext0_wakeup(wakeup_gpio_button_mode,0); //1 = High, 0 = Low
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK,ESP_EXT1_WAKEUP_ANY_HIGH);
  setMode(mode);

  state = ONE;
  countPumpMode = 0;
  countSendData = 0;
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);
  LoRa.setTxPower(12);
  
  while (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    delay(500);
  }
  Serial.println("LoRa init succeeded.");

  while (!Serial) 
  {
    delay(1);
  }
 //////////////////////////////////////////////////////////////
  Wire.begin(21, 22); // SDA = 21, SCL = 22 (เปลี่ยนตามขาของ ESP32)
  bool statusBme;
  statusBme = bme.begin(0x76);  
  if (!statusBme) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  currentSensor.current(13, currCalibration);
  Serial.println("Startttttt");
  delay(1000);
}

void loop() {
  try {
    unsigned long currentMillisCheckPump = millis();
    if (currentMillisCheckPump - previousMillisCheckPump >= 50) {
      previousMillisCheckPump = currentMillisCheckPump;
      buttonMode.read();
      buttonPump.read();
      buttonModePump.read();
      updatePump(pumpState);
      updateModePump(pumpMode);
      updateStatus(statusNode);
      // if(pumpState == 1){
      //   //Serial.println(">>>>>>>>>>>>>>>>>>> pump onnnnnnnnnnnnnnnn <<<<<<<<<<<<<<<<<<<<<<");
      //   pumpState = 1;
      // }else if(pumpState == 0){
      //   //Serial.println(">>>>>>>>>>>>>>>>>>> pump offffffffffffffff <<<<<<<<<<<<<<<<<<<<<<");
      //   pumpState = 0;
      // }
      // updatePump(pumpState);
      // updateModePump(pumpMode);  
      //Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    }
    if(pumpState == 0){
      Serial.println(">>>>>>>>>>>>>>>>>>> pumpState 0 <<<<<<<<<<<<<<<<<<<<<<");
      
      if(state == ONE){                   //อ่านค่าเซ็นเซอร์
        Serial.println(state);
        updateSensorTemp_Hum();
        pumpStatus = 1;
        state = TWO;
      }else if(state == TWO){             //ส่งข้อมูลไปเกตเวย์พร้อมขอสถานะปั้มน้ำและค่าconfigต่างๆ
        Serial.println(state);// pumpMode  mode pumpState timeNormalMode timeDebugMode
        // temp hum pumpMode  mode  pumpState pumpStatus  statusNode  timeNormalMode  timeDebugMode
        Mymessage = String(temp)+","+String(hum)+","+String(pumpMode)+","+String(mode)+","+String(pumpState)+","+String(pumpStatus)+","+String(statusNode)+","+String(timeNormalMode)+","+String(timeDebugMode);
        Serial.print("Mymessage");
        Serial.println(Mymessage);
        sendMessage(Mymessage);
        Serial.println("Mymessage sendddd");
        state = THREE;
      }else if(state == THREE){            //รอรับการยืนยันความถูกต้องของข้อมูลจากเกตเวย์
        //Serial.println(state);
        onReceive();
        if(sendSuccess == true && resendData == false){
          state = FOUR;
        }else if(sendSuccess == false && resendData == true){
          if(countSendData >= 5){
            statusNode = 0;
            //digitalWrite(LED_STATUS,statusNode);
            //gpio_hold_dis(ledStatus_gpio);
            Serial.print("OFF LED_STATUS");
            // delay(10);
            pumpState = 0;
            state = FIVE;
          }else{
            countSendData ++;
            Serial.print("countSendData");
            Serial.println(countSendData);
            state = TWO;
          }
        }
      }else if(state == FOUR){            //Updateข้อมูลconfigต่างๆจากเกตเวย์
        if(updateConfig(receiveData)){
          Serial.print("statusNode: ");  Serial.println(statusNode);
          //digitalWrite(LED_STATUS,statusNode);
          //gpio_hold_en(ledStatus_gpio);
          if(pumpState == 1){//เช็คด้วยว่ามีการสั่งปั้มทำงานหรือไม่
              pumpState = 1;
              return;
            }else{
              state = FIVE;
            }
        }else{
          state = TWO;
        }
        // checkStatePump();
        //Serial.println(state);
        
        //state = FIVE;
      }else if(state == FIVE){            //รอรับการยืนยันความถูกต้องของข้อมูลจากเกตเวย์
        //Serial.println(state);
        Serial.println(">>>>>>>>>>>>>>>>>>>Before<<<<<<<<<<<<<<<<<<<<<<");
        Serial.print("pumpMode");
        Serial.println(pumpMode);
        Serial.print("mode");
        Serial.println(mode);
        Serial.print("pumpState");
        Serial.println(pumpState);
        Serial.print("statusNode");
        Serial.println(statusNode);
        Serial.print("timeNormalMode");
        Serial.println(timeNormalMode);
        Serial.print("timeDebugMode");
        Serial.println(timeDebugMode);
        Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<");
        updateEEPROM();
        state = SIX;
      }else if(state == SIX){             //บันทึกข้อมูลลงEEPROM
        //Serial.println(state);
        setMode(mode);
        Serial.println(">>>>>>>>>>>>>>>>>>>Before Deep Sleep <<<<<<<<<<<<<<<<<<<<<<");
        Serial.print("pumpMode");
        Serial.println(pumpMode);
        Serial.print("mode");
        Serial.println(mode);
        Serial.print("pumpState");
        Serial.println(pumpState);
        Serial.print("statusNode");
        Serial.println(statusNode);
        Serial.print("timeNormalMode");
        Serial.println(timeNormalMode);
        Serial.print("timeDebugMode");
        Serial.println(timeDebugMode);
        Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>esp_deep_sleep_start<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
        esp_deep_sleep_start(); // เข้าสู่โหมด Deep Sleep
        state = SEVEN;
      }
    }else if(pumpState == 1){
      // Serial.println(">>>>>>>>>>>>>>>>>>> pumpState 1 <<<<<<<<<<<<<<<<<<<<<<");
      unsigned long currentMillisCount = millis();
      if (currentMillisCount - previousMillisCount >= 1000) {
        previousMillisCount = currentMillisCount;
        countPumpMode ++;
        if(countPumpMode == 10){
          flag = 1;
        }
        Serial.print(">>>>>>>>>>>>>>>>>>> countPumpMode <<<<<<<<<<<<<<<<<<<<<<");
        Serial.println(countPumpMode);
        updateSensorCurrent();
      }
      if(flag == 1){
        if(state == ONE){                   //อ่านค่าเซ็นเซอร์
        Serial.println("------------------------------------------------"); 
        Serial.println(state);    
                                            //อ่านค่าเซ็นเซอร์วัดกระแสว่าปั้มทำงานจริงหรือไม่####################ยังไม่ได้ทำนะครับ
        updateSensorCurrent();
        if(Irms > 0.6){
          pumpStatus = 1;
          Serial.println("");
        }else if(Irms <= 0.5){
          pumpStatus = 0;
        }
        updateSensorTemp_Hum();
        state = TWO;
        }else if(state == TWO){             //ส่งข้อมูลไปเกตเวย์พร้อมขอสถานะปั้มน้ำและค่าconfigต่างๆ
        Serial.println("------------------------------------------------"); 
        Serial.println(state);
          //Serial.println(state);
          // pumpMode  mode  pumpState pumpStatus  statusNode  timeNormalMode  timeDebugMode
          Mymessage = String(temp)+","+String(hum)+","+String(pumpMode)+","+String(mode)+","+String(pumpState)+","+String(pumpStatus)+","+String(statusNode)+","+String(timeNormalMode)+","+String(timeDebugMode);
          Serial.print("Mymessage :"); 
          Serial.println(Mymessage);
          // delay(100);
          sendMessage(Mymessage);
          Serial.println("Sent !!:");
          state = THREE;
        }else if(state == THREE){            //รอรับการยืนยันความถูกต้องของข้อมูลจากเกตเวย์
          Serial.println("------------------------------------------------"); 
          Serial.println(state);
          //Serial.println(state);
          onReceive();
          if(sendSuccess == true && resendData == false){
            state = FOUR;
          }else if(sendSuccess == false && resendData == true){
            if(countSendData >= 5){
              statusNode = 0;
              //digitalWrite(LED_STATUS,statusNode);
              //gpio_hold_dis(ledStatus_gpio);
              Serial.print("OFF LED_STATUS");
              // delay(10);
              pumpState = 0;
              state = FIVE;
            }else{
              countSendData ++;
              Serial.print("countSendData");
              Serial.println(countSendData);
              state = TWO;
            }
          }
        }else if(state == FOUR){            //Updateข้อมูลconfigต่างๆจากเกตเวย์
          Serial.println("------------------------------------------------"); 
          Serial.println(state);
          if(updateConfig(receiveData)){
            Serial.print("statusNode: ");  Serial.println(statusNode);
            //digitalWrite(LED_STATUS,statusNode);
            //gpio_hold_en(ledStatus_gpio);
            if(pumpState == 0){//เช็คด้วยว่ามีการสั่งปั้มทำงานหรือไม่
              // pumpState = 0;
              // return;
            }else{
              state = FIVE;
            }
          }else{
            state = TWO;
          }
          // checkStatePump();
          //Serial.println(state);
          //state = FIVE;
        }else if(state == FIVE){            //รอรับการยืนยันความถูกต้องของข้อมูลจากเกตเวย์
          Serial.println("------------------------------------------------"); 
          Serial.println(state);
          //Serial.println(state);
          Serial.println(">>>>>>>>>>>>>>>>>>>Before<<<<<<<<<<<<<<<<<<<<<<");
          Serial.print("pumpMode");
          Serial.println(pumpMode);
          Serial.print("mode");
          Serial.println(mode);
          Serial.print("pumpState");
          Serial.println(pumpState);
          Serial.print("statusNode");
          Serial.println(statusNode);
          Serial.print("timeNormalMode");
          Serial.println(timeNormalMode);
          Serial.print("timeDebugMode");
          Serial.println(timeDebugMode);
          Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<");
          updateEEPROM();
          state = SIX;
        }else if(state == SIX){             //บันทึกข้อมูลลงEEPROM
          Serial.println("------------------------------------------------"); 
          Serial.println(state);
          //Serial.println(state);
          setMode(mode);
          Serial.println(">>>>>>>>>>>>>>>>>>>Before Deep Sleep <<<<<<<<<<<<<<<<<<<<<<");
          Serial.print("pumpMode");
          Serial.println(pumpMode);
          Serial.print("mode");
          Serial.println(mode);
          Serial.print("pumpState");
          Serial.println(pumpState);
          Serial.print("statusNode");
          Serial.println(statusNode);
          Serial.print("timeNormalMode");
          Serial.println(timeNormalMode);
          Serial.print("timeDebugMode");
          Serial.println(timeDebugMode);
          Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
          countPumpMode = 0;
          countSendData = 0;
          Mymessage = "";
          flag = 0;
          state = ONE;
          // return;
        }
      }
    }
  } catch (...) {
    Serial.println("An error ");
    // กระทำตามสถานการณ์เหตุการณ์ผิดปกติ
  }
  delay(0);

}
void updatePump(bool status){
  // if(status != previousPumpStatus){ // ตรวจสอบว่ามีการเปลี่ยนแปลงหรือไม่
  //   previousPumpStatus = status; // บันทึกค่าปัจจุบันเพื่อใช้ในการตรวจสอบในรอบถัดไป
  if(status){
    digitalWrite(LED_PUMP, LOW);
    digitalWrite(RelayOn, HIGH);
    delay(100);
    digitalWrite(RelayOn, LOW);
    // gpio_hold_en(pump_gpio);
    // Serial.println("-------------------------------------Pump ON");
  }else{
    digitalWrite(LED_PUMP, HIGH);
    digitalWrite(RelayOff, HIGH);
    delay(100);
    digitalWrite(RelayOff, LOW);
    // gpio_hold_dis(pump_gpio);
    // Serial.println("-------------------------------------Pump OFF");
  }
  // }
}
void updateModePump(bool status){
  // if(status != previousModePump){ // ตรวจสอบว่ามีการเปลี่ยนแปลงหรือไม่
  //   previousModePump = status; // บันทึกค่าปัจจุบันเพื่อใช้ในการตรวจสอบในรอบถัดไป
  if(status){
    digitalWrite(LED_MODE_PUMP, LOW);
    gpio_hold_en(ledModePump_gpio);
    
    // Serial.println("-------------------------------->>Pump Auto");
  }else{
    digitalWrite(LED_MODE_PUMP, HIGH);
    gpio_hold_dis(ledModePump_gpio);
    // Serial.println("-------------------------------->>Pump Manual");
  }
  // }
}
void updateStatus(bool status){
  if(status){
    digitalWrite(LED_STATUS, LOW);
    gpio_hold_en(ledStatus_gpio);
    //Serial.println("Status ON");
  }else{
    digitalWrite(LED_STATUS, HIGH);
    gpio_hold_dis(ledStatus_gpio);
    // Serial.println("Status OFF");
  }
}

bool updateConfig(String receiveData){
  String data  = receiveData;
  char delimiter = ',';
  // แบ่งข้อมูลโดยใช้ String::indexOf() และ String::substring()
  String dataValues[9]; // สร้างอาร์เรย์ String มีขนาด 7 สมาชิก
  int startIndex = 0;
  int endIndex;
  for (int i = 0; i < 9; i++) {
      endIndex = data.indexOf(delimiter, startIndex);
      dataValues[i] = data.substring(startIndex, endIndex);
      startIndex = endIndex + 1;
  }
  // แบ่งข้อมูลสุดท้าย
  dataValues[9] = data.substring(startIndex);
  // data = "161,187,10,Pass,0,5,1";
  if(dataValues[3] == "Pass"){
    // pumpMode  mode  pumpState pumpStatus  statusNode  timeNormalMode  timeDebugMode
    pumpMode = dataValues[4].toInt();
    mode = dataValues[5].toFloat();
    pumpState = dataValues[6].toFloat();
    statusNode = dataValues[7].toFloat();
    timeNormalMode = dataValues[8].toFloat();
    timeDebugMode = dataValues[9].toFloat();
    Serial.println("---------------------------------------------");
    Serial.print("pumpMode");
    Serial.println(pumpMode);
    Serial.print("mode");
    Serial.println(mode);
    Serial.print("pumpState");
    Serial.println(pumpState);
    Serial.print("statusNode");
    Serial.println(statusNode);
    Serial.print("timeNormalMode");
    Serial.println(timeNormalMode);
    Serial.print("timeDebugMode");
    Serial.println(timeDebugMode);
    Serial.println("---------------------------------------------");
    return true;
  }else{
     return false;
  }
}
void setMode(bool mode){
  if(mode){
    updateEEPROM();
    esp_sleep_enable_timer_wakeup(timeNormalMode * 60 * 1000000);  // ตื่นขึ้นทุก 5 นาที
    Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>NormalMode<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
  }else{
    updateEEPROM();
    esp_sleep_enable_timer_wakeup(timeDebugMode * 60 * 1000000);  // ตื่นขึ้นทุก 1 นาที
    Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>DebugMode<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
  }
}
void setPump(bool pumpState){
  if(pumpState){
    updateEEPROM();
    Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>NormalMode<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
  }else{
    updateEEPROM();
    Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>DebugMode<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
  }
}
void setPumpMode(bool pumpMode){
  if(pumpMode){
    updateEEPROM();
    Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>> pumpMode AUTO <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
  }else{
    updateEEPROM();
    Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>> pumpMode MANUAL <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
  }
}
void updateEEPROM(){
  EEPROM.put(addr_pumpMode, pumpMode);
  EEPROM.put(addr_mode, mode);
  EEPROM.put(addr_pumpState, pumpState);
  EEPROM.put(addr_statusNode, statusNode);
  EEPROM.put(addr_timeNormalMode, timeNormalMode);
  EEPROM.put(addr_timeDebugMode, timeDebugMode);
  EEPROM.commit();
  EEPROM.get(addr_pumpMode, pumpMode);
  EEPROM.get(addr_mode, mode);
  EEPROM.get(addr_pumpState, pumpState);
  EEPROM.get(addr_statusNode, statusNode);
  EEPROM.get(addr_timeNormalMode, timeNormalMode);
  EEPROM.get(addr_timeDebugMode, timeDebugMode);
  Serial.println(">>>>>>>>>>>>>>>>>>>updateEEPROM<<<<<<<<<<<<<<<<<<<<<<");
  Serial.print("pumpMode");
  Serial.println(pumpMode);
  Serial.print("mode");
  Serial.println(mode);
  Serial.print("pumpState");
  Serial.println(pumpState);
  Serial.print("statusNode");
  Serial.println(statusNode);
  Serial.print("timeNormalMode");
  Serial.println(timeNormalMode);
  Serial.print("timeDebugMode");
  Serial.println(timeDebugMode);
  Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<");
}