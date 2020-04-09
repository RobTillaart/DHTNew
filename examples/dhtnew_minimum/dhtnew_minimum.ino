//
//    FILE dhtnew_minimum.ino
//  AUTHOR Rob Tillaart
// VERSION 0.1.0
// PURPOSE DHTNEW library test sketch for Arduino
//     URL
// HISTORY
// 0.1.0 - 2018-01-08 initial version
//
// Released to the public domain
//

#include <dhtnew.h>

DHTNEW mySensor(6);

void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.println();

  mySensor.setHumOffset(10);
  mySensor.setTempOffset(-1.5);
}

void loop()
{
  if (millis() - mySensor.lastRead() > 2000)
  {
    mySensor.read();
    Serial.print(mySensor.humidity, 1);
    Serial.print("\t");
    Serial.println(mySensor.temperature, 1);
  }
}

// END OF FILE