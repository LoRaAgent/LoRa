#include "Adafruit_VL53L1X.h"

#define IRQ_PIN 2
#define XSHUT_PIN 3
int setZero = 0 ;
double adjustSensor ;
Adafruit_VL53L1X vl53 = Adafruit_VL53L1X(XSHUT_PIN, IRQ_PIN);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println(F("Adafruit VL53L1X sensor demo"));

  Wire.begin();
  if (!vl53.begin(0x29, &Wire)) {
    Serial.print(F("Error on init of VL sensor: "));
    Serial.println(vl53.vl_status);
    while (1) delay(10);
  }
  Serial.println(F("VL53L1X sensor OK!"));

  Serial.print(F("Sensor ID: 0x"));
  Serial.println(vl53.sensorID(), HEX);

  if (!vl53.startRanging()) {
    Serial.print(F("Couldn't start ranging: "));
    Serial.println(vl53.vl_status);
    while (1) delay(10);
  }
  Serial.println(F("Ranging started"));

  // Valid timing budgets: 15, 20, 33, 50, 100, 200, and 500ms!
  vl53.setTimingBudget(200); // เอาไว้ปรับความเร็ว ในการ อ่านของ seensor ยิ่งมากยิ่งช้า ค่าหน่วงมาก ความแม่นยำสูง
  Serial.print(F("Timing budget (ms): "));
  Serial.println(vl53.getTimingBudget());
  vl53.VL53L1X_SetOffset(0); // เอาไว้ใช้ offset ค่า โดยค่าที่ใส่มันจะเป็นหน่วย มิลลิ 20 = 2 cm
  vl53.VL53L1X_SetXtalk(0); // เอาไว้ ตั้งค่า crosstalk
  vl53.VL53L1X_SetDistanceMode(2); // ปรับ ประสิทธิภาพระยะการอ่านของ sensor โดยที่ 1 = Short , 2 = Long
  if (vl53.VL53L1X_SetROI(16,16) == 0) {   //(ROIcenter,ROIsize)
    Serial.println(F("ตั้งค่า ROI สำเร็จ"));
  } else {
    Serial.println(F("ตั้งค่า ROI ล้มเหลว"));
  }

  /*
  vl53.VL53L1X_SetDistanceThreshold(100, 300, 3, 1);
  vl53.VL53L1X_SetInterruptPolarity(0);
  */
}

void loop() {
  int16_t distance;

  if (vl53.dataReady()) {
    // มีข้อมูลใหม่ที่สามารถอ่านได้!
    distance = vl53.distance();
    if (distance == -1) {
      // มีบางอย่างผิดพลาด!
      Serial.print(F("Couldn't get distance: "));
      Serial.println(vl53.vl_status);
      return;
    }
    adjustSensor = (distance - setZero)/10; // หาร 10 เพื่อแปลง mm -> cm
    Serial.print(F("Distance: "));
    Serial.print(adjustSensor);
    Serial.println(" cm");
    vl53.clearInterrupt();// อ่านข้อมูลเสร็จแล้ว พร้อมสำหรับการอ่านครั้งต่อไป!
  }
}
