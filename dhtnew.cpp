//
//    FILE: dhtnew.cpp
//  AUTHOR: Rob.Tillaart@gmail.com
// VERSION: 0.3.0
// PURPOSE: DHT Temperature & Humidity Sensor library for Arduino
//     URL: https://github.com/RobTillaart/DHTNEW
//
// HISTORY:
// 0.1.0  2017-07-24 initial version based upon DHTStable
// 0.1.1  2017-07-29 add begin() to determine type once and for all instead of every call + refactor
// 0.1.2  2018-01-08 improved begin() + refactor()
// 0.1.3  2018-01-08 removed begin() + moved detection to read() function
// 0.1.4  2018-04-03 add get-/setDisableIRQ(bool b)
// 0.1.5  2019-01-20 fix negative temperature DHT22 - issue #120
// 0.1.6  2020-04-09 #pragma once, readme.md, own repo
// 0.1.7  2020-05-01 prevent premature read; add waitForReading flag (Kudo's to Mr-HaleYa),
// 0.2.0  2020-05-02 made temperature and humidity private (Kudo's to Mr-HaleYa),
// 0.2.1  2020-05-27 Fix #11 - Adjust bit timing threshold
// 0.2.2  2020-06-08 added ERROR_SENSOR_NOT_READY and differentiate timeout errors
// 0.3.0  2020-06-12 added getReadDelay & setReadDelay to tune reading interval
//

#include "dhtnew.h"

// these defines are implementation only, not for user
#define DHTLIB_DHT11_WAKEUP        18
#define DHTLIB_DHT_WAKEUP          1

// datasheet values 1000 and 2000,
// can be overruled with setReadDelay()
// experiments [Mr-HaleYa] indicate 1250 and 2250 to be robust.
// additional tests with ESP32 ==> 1250 - 2500
// DHT22 + ESP32 worked with 400 (2.5 reads/second)
#define DHTLIB_DHT11_READ_DELAY    1250
#define DHTLIB_DHT22_READ_DELAY    2500

// max timeout is 100 usec.
// loops using TIMEOUT use at least 4 clock cycli
// - read IO
// - compare IO
// - compare loopcounter
// - decrement loopcounter
//
// For a 16Mhz (UNO) 100 usec == 1600 clock cycles
// ==> 100 usec takes max 400 loops
// for a 240MHz (ESP32) 100 usec == 24000 clock cycles
// ==> 100 usec takes max 6000 loops
//
// By dividing F_CPU by 40000 we "fail" as fast as possible
#define DHTLIB_TIMEOUT (F_CPU/40000)


/////////////////////////////////////////////////////
//
// PUBLIC
//
DHTNEW::DHTNEW(uint8_t pin)
{
  _pin = pin;
  // Data-bus's free status is high voltage level.
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, HIGH);
  _readDelay = 0;
};

void DHTNEW::setType(uint8_t type)
{
  if ((type == 0) || (type == 11) || (type == 22))
  {
    _type = type;
  }
}

// return values:
// DHTLIB_OK
// DHTLIB_ERROR_CHECKSUM
// DHTLIB_ERROR_TIMEOUT_A
// DHTLIB_ERROR_BIT_SHIFT
// DHTLIB_ERROR_SENSOR_NOT_READY
// DHTLIB_ERROR_TIMEOUT_C
// DHTLIB_ERROR_TIMEOUT_D
int DHTNEW::read()
{
  if (_readDelay == 0)
  { 
    _readDelay = DHTLIB_DHT22_READ_DELAY;
    if (_type == 11) _readDelay = DHTLIB_DHT11_READ_DELAY;
  }
  if (_type != 0)
  {
    while (millis() - _lastRead < _readDelay)
    {
      if (!_waitForRead) return DHTLIB_OK;
      yield();
    }
    return _read();
  }

  _type = 22;
  _wakeupDelay = DHTLIB_DHT_WAKEUP;
  int rv = _read();
  if (rv == DHTLIB_OK) return rv;

  _type = 11;
  _wakeupDelay = DHTLIB_DHT11_WAKEUP;
  rv = _read();
  if (rv == DHTLIB_OK) return rv;

  _type = 0; // retry next time
  return rv;
}

// return values:
// DHTLIB_OK
// DHTLIB_ERROR_CHECKSUM
// DHTLIB_ERROR_TIMEOUT_A
// DHTLIB_ERROR_BIT_SHIFT
// DHTLIB_ERROR_SENSOR_NOT_READY
// DHTLIB_ERROR_TIMEOUT_C
// DHTLIB_ERROR_TIMEOUT_D
int DHTNEW::_read()
{
  // READ VALUES

  int rv = _readSensor();
  interrupts();

  // Data-bus's free status is high voltage level.
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, HIGH);
  _lastRead = millis();

  if (rv != DHTLIB_OK)
  {
    _humidity    = DHTLIB_INVALID_VALUE;
    _temperature = DHTLIB_INVALID_VALUE;
    return rv; // propagate error value
  }

  if (_type == 22) // DHT22, DHT33, DHT44, compatible
  {
    _humidity =    (_bits[0] * 256 + _bits[1]) * 0.1;
    _temperature = ((_bits[2] & 0x7F) * 256 + _bits[3]) * 0.1;
  }
  else // if (_type == 11)  // DHT11, DH12, compatible
  {
    _humidity = _bits[0] + _bits[1] * 0.1;
    _temperature = _bits[2] + _bits[3] * 0.1;
  }

  if (_bits[2] & 0x80)  // negative temperature
  {
    _temperature = -_temperature;
  }
  _humidity = constrain(_humidity + _humOffset, 0, 100);
  _temperature += _tempOffset;

  // TEST CHECKSUM
  uint8_t sum = _bits[0] + _bits[1] + _bits[2] + _bits[3];
  if (_bits[4] != sum)
  {
    return DHTLIB_ERROR_CHECKSUM;
  }
  return DHTLIB_OK;
}

/////////////////////////////////////////////////////
//
// PRIVATE
//


// return values:
// DHTLIB_OK
// DHTLIB_ERROR_CHECKSUM
// DHTLIB_ERROR_TIMEOUT_A
// DHTLIB_ERROR_BIT_SHIFT
// DHTLIB_ERROR_SENSOR_NOT_READY
// DHTLIB_ERROR_TIMEOUT_C
// DHTLIB_ERROR_TIMEOUT_D
int DHTNEW::_readSensor()
{
  // INIT BUFFERVAR TO RECEIVE DATA
  uint8_t mask = 128;
  uint8_t idx = 0;

  // EMPTY BUFFER
  for (uint8_t i = 0; i < 5; i++) _bits[i] = 0;

  yield();  // handle all pending IRQ's ?

  // REQUEST SAMPLE
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  delayMicroseconds(_wakeupDelay * 1100UL); // add 10% extra

  pinMode(_pin, INPUT_PULLUP);
  delayMicroseconds(55);  // was 40

  noInterrupts();  // FORCED CLI

  // GET ACKNOWLEDGE or TIMEOUT
  uint16_t loopCnt = DHTLIB_TIMEOUT;
  while(digitalRead(_pin) == LOW)
  {
    if (--loopCnt == 0) return DHTLIB_ERROR_TIMEOUT_A;
  }

  // If sensor stays HIGH >> 80 usec it is not ready yet.
  loopCnt = DHTLIB_TIMEOUT;
  while(digitalRead(_pin) == HIGH)
  {
    if (--loopCnt == 0) return DHTLIB_ERROR_SENSOR_NOT_READY;
  }

  // READ THE OUTPUT - 40 BITS => 5 BYTES
  for (uint8_t i = 40; i != 0; i--)
  {
    loopCnt = DHTLIB_TIMEOUT;
    while(digitalRead(_pin) == LOW)
    {
      if (--loopCnt == 0) return DHTLIB_ERROR_TIMEOUT_C;
    }

    uint32_t t = micros();

    loopCnt = DHTLIB_TIMEOUT;
    while(digitalRead(_pin) == HIGH)
    {
      if (--loopCnt == 0) return DHTLIB_ERROR_TIMEOUT_D;
    }

    // 26-28 us ==> 0
    //    70 us ==> 1
    if ((micros() - t) > DHTLIB_BIT_THRESHOLD)
    {
      _bits[idx] |= mask;
    }
    mask >>= 1;
    if (mask == 0)   // next byte?
    {
      mask = 0x80;
      idx++;
    }
  }
  // After the 40 bits the sensor pulls down the line for 50 usec
  // This library does not wait for that to happen.
  // loopCnt = DHTLIB_TIMEOUT;
  // while(digitalRead(_pin) == LOW)
  // {
  //   if (--loopCnt == 0) return DHTLIB_ERROR_TIMEOUTC;
  // }


  // CATCH RIGHTSHIFT BUG ESP (only 1 single bit)
  // humidity is max 1000 = 0x0E8 for DHT22 and 0x6400 for DHT11
  // so most significant bit may never be set.
  if (_bits[0] & 0x80) return DHTLIB_ERROR_BIT_SHIFT;

  return DHTLIB_OK;
}

// -- END OF FILE --
