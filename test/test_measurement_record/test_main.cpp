#include <Arduino.h>
#include <unity.h>

#include "MeasurementRecord.h"

void test_record_crc_is_valid() {
  MeasurementRecord rec{};
  rec.timestamp = 123456;
  rec.temp_x100 = 2150;
  rec.hum_x100 = 5030;
  rec.weight_g = 42000;
  rec.seq = 7;
  rec.flags = 0;
  measurement::finalizeRecord(rec);

  TEST_ASSERT_TRUE(measurement::isRecordValid(rec));
}

void setup() {
  delay(1000);
  UNITY_BEGIN();
  RUN_TEST(test_record_crc_is_valid);
  UNITY_END();
}

void loop() {}
