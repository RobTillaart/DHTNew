//
//    FILE: DHT_endless.ino
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.1
// PURPOSE: demo
//    DATE: 2020-06-04
//    (c) : MIT
//
// 0.1.0    2020-06-04 initial version
// 0.1.1    2020-06-15 match 0.3.0 error handling
//
// DHT PIN layout from left to right
// =================================
// FRONT : DESCRIPTION  
// pin 1 : VCC
// pin 2 : DATA
// pin 3 : Not Connected
// pin 4 : GND

#include <dhtnew.h>

DHTNEW mySensor(16);   // ESP 16  UNO 6

uint32_t count = 0;
uint32_t start, stop;

uint32_t errors[10] = { 0,0, 0,0, 0,0, 0,0, 0,0 };


void setup()
{
  Serial.begin(115200);
  Serial.println("DHT_endless.ino");
  Serial.print("LIBRARY VERSION: ");
  Serial.println(DHTNEW_LIB_VERSION);
  Serial.println();
}

void loop()
{
  count++;
  // show counters for OK and errors.
  if (count % 50 == 0)
  {
    Serial.println();
    Serial.println("OK \tCRC \tTOA \tTOAB \tTOC \tTOD \tSNR \tBS \tUNK");
    for (uint8_t i = 0; i < 9; i++)
    {
      Serial.print(errors[i]);
      Serial.print('\t');
    }
    Serial.println();
    Serial.println();
  }

  if (count % 10 == 0)
  {
    Serial.println();
    Serial.println("TIM\tCNT\tSTAT\tHUMI\tTEMP\tTIME\tTYPE");
  }
  Serial.print(millis());
  Serial.print("\t");
  Serial.print(count);
  Serial.print("\t");

  start = micros();
  int chk = mySensor.read();
  stop = micros();

  switch (chk)
  {
    case DHTLIB_OK:
      Serial.print("OK,\t");
      errors[0]++;
      break;
    case DHTLIB_ERROR_CHECKSUM:
      Serial.print("CRC,\t");
      errors[1]++;
      break;
    case DHTLIB_ERROR_TIMEOUT_A:
      Serial.print("TOA,\t");
      errors[2]++;
      break;
    case DHTLIB_ERROR_TIMEOUT_B:
      Serial.print("TOB,\t");
      errors[3]++;
      break;
    case DHTLIB_ERROR_TIMEOUT_C:
      Serial.print("TOC,\t");
      errors[4]++;
      break;
    case DHTLIB_ERROR_TIMEOUT_D:
      Serial.print("TOD,\t");
      errors[5]++;
      break;
    case DHTLIB_ERROR_SENSOR_NOT_READY:
      Serial.print("SNR,\t");
      errors[6]++;
      break;
    case DHTLIB_ERROR_BIT_SHIFT:
      Serial.print("BS,\t");
      errors[7]++;
      break;
    default:
      Serial.print("U");
      Serial.print(chk);
      Serial.print(",\t");
      errors[8]++;
      break;
  }
  // DISPLAY DATA
  Serial.print(mySensor.getHumidity(), 1);
  Serial.print(",\t");
  Serial.print(mySensor.getTemperature(), 1);
  Serial.print(",\t");
  Serial.print(stop - start);
  Serial.print(",\t");
  Serial.println(mySensor.getType());

  delay(1000);
}


// END OF FILE
