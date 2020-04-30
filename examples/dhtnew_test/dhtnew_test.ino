//
//    FILE: dhtnew_test.ino
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.2
// PURPOSE: DHTNEW library test sketch for Arduino
//     URL:
// HISTORY:
//
// 0.1.0 - 2017-07-24 initial version
// 0.1.1 - 2017-07-29 added begin();
// 0.1.2 - 2018-01-08 removed begin();
//
// Released to the public domain
//
// FRONT left 2 right
// pin 1 : VCC
// pin 2 : DATA
// pin 3 : NC
// PIN 4 : GND

#include "dhtnew.h"

DHTNEW mySensor(18);



void setup()
{
  Serial.begin(115200);
  Serial.println("---Starting Up---");
  Serial.println("dhtnew_test.ino");
  Serial.print("LIBRARY VERSION: ");
  Serial.println(DHTNEW_LIB_VERSION);
  Serial.println();
  
  // Setting this to true will force the sketch to,
  // wait until the sensor can take another reading,
  // (this can add ~2-3 sec to .read() depending on sensor). 
  // If false, when a sensor is asked to read before it,
  // is ready the last read values will be returned.
  // It is set to false by default. 
  mySensor.setWaitForReading(true);
  
  Serial.println("\n1. Type detection test, first run might take longer to determine type");
  Serial.println("STAT\tHUMI\tTEMP\tTIME\tTYPE");
  test();
  test();
  test();
  test();

  Serial.println("\n2. Humidity offset test");
  Serial.println("STAT\tHUMI\tTEMP\tTIME\tTYPE");
  mySensor.setHumOffset(2.5);
  test();
  mySensor.setHumOffset(5.5);
  test();
  mySensor.setHumOffset(-5.5);
  test();
  mySensor.setHumOffset(0);
  test();

  Serial.println("\n3. Temperature offset test");
  Serial.println("STAT\tHUMI\tTEMP\tTIME\tTYPE");
  mySensor.setTempOffset(2.5);
  test();
  mySensor.setTempOffset(5.5);
  test();
  mySensor.setTempOffset(-5.5);
  test();
  mySensor.setTempOffset(0);
  test();

  
  Serial.println("\n4. Get LastRead test");
  delay(2250);
  for (int i = 0; i < 20; i++)
  {
    if (millis() - mySensor.lastRead() > 2000)
    {
      mySensor.read();
      Serial.println("-------------");
      Serial.println("Taking reading");
      Serial.print(mySensor.getHumidity(), 1);
      Serial.print(",\t");
      Serial.println(mySensor.getTemperature(), 1);
      Serial.println("-------------");
      Serial.println("LastRead results");
    }
    Serial.print(mySensor.getHumidity(), 1);
    Serial.print(",\t");
    Serial.println(mySensor.getTemperature(), 1);
    delay(250);
  }

  Serial.println("\nDone...");
}

void loop()
{

}

void test()
{
  // READ DATA
  uint32_t start = micros();
  int chk = mySensor.read();
  uint32_t stop = micros();

  switch (chk)
  {
    case DHTLIB_OK:
      Serial.print("OK,\t");
      break;
    case DHTLIB_ERROR_CHECKSUM:
      Serial.print("Checksum error,\t");
      break;
    case DHTLIB_ERROR_TIMEOUT:
      Serial.print("Time out error,\t");
      break;
    default:
      Serial.print("Unknown error,\t");
      break;
  }
  // DISPLAY DATA
  Serial.print(mySensor.getHumidity(), 1);
  Serial.print(",\t");
  Serial.print(mySensor.getTemperature(), 1);
  Serial.print(",\t");
  uint32_t duration = stop - start;
  Serial.print(duration);
  Serial.print(",\t");
  Serial.println(mySensor.getType());
  delay(500);
}


// END OF FILE