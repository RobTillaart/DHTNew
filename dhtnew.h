#pragma once
//
//    FILE: dhtnew.h
//  AUTHOR: Rob Tillaart
// VERSION: 0.3.0
// PURPOSE: DHT Temperature & Humidity Sensor library for Arduino
//     URL: https://github.com/RobTillaart/DHTNEW
//
// HISTORY:
// see dhtnew.cpp file

// DHT PIN layout from left to right
// =================================
// FRONT : DESCRIPTION  
// pin 1 : VCC
// pin 2 : DATA
// pin 3 : Not Connected
// pin 4 : GND

#include "Arduino.h"

#define DHTNEW_LIB_VERSION "0.3.0"

#define DHTLIB_OK                         0
#define DHTLIB_ERROR_CHECKSUM            -1
#define DHTLIB_ERROR_TIMEOUT_A           -2
#define DHTLIB_ERROR_BIT_SHIFT           -3
#define DHTLIB_ERROR_SENSOR_NOT_READY    -4
#define DHTLIB_ERROR_TIMEOUT_C           -5
#define DHTLIB_ERROR_TIMEOUT_D           -6
#define DHTLIB_ERROR_TIMEOUT_B           -7

#define DHTLIB_INVALID_VALUE    -999


// bits are timing based (datasheet)
// 26-28us ==> 0
// 70 us   ==> 1
// See https://github.com/RobTillaart/DHTNew/issues/11
#ifndef DHTLIB_BIT_THRESHOLD
#define DHTLIB_BIT_THRESHOLD          50
#endif


class DHTNEW
{
public:

    DHTNEW(uint8_t pin);

    // 0 = unknown, 11 or 22
    uint8_t getType()                 { return _type; };
    void setType(uint8_t type = 0);
    int read();

    // lastRead is in MilliSeconds since start sketch
    uint32_t lastRead()               { return _lastRead; };

    // preferred interface
    float getHumidity()               { return _humidity; };
    float getTemperature()            { return _temperature; };

    // adding offsets works well in normal range
    // might introduce under- or overflow at the ends of the sensor range
    void  setHumOffset(float offset)  { _humOffset = offset; };
    void  setTempOffset(float offset) { _tempOffset = offset; };
    float getHumOffset()              { return _humOffset; };
    float getTempOffset()             { return _tempOffset; };

    bool getWaitForReading()          { return _waitForRead; };
    void setWaitForReading(bool b )   { _waitForRead = b; };

    // set readDelay to 0 will reset to datasheet values
    uint16_t getReadDelay()           { return _readDelay; };
    void setReadDelay(uint16_t rd = 0){ _readDelay = rd; };

private:
    uint8_t  _pin = 0;
    uint8_t  _wakeupDelay = 0;
    uint8_t  _type = 0;
    float    _humOffset = 0.0;
    float    _tempOffset = 0.0;
    float    _humidity;
    float    _temperature;
    uint32_t _lastRead = 0;
    bool     _waitForRead = false;
    uint16_t _readDelay = 0;

    uint8_t  _bits[5];  // buffer to receive data
    int      _read();
    int      _readSensor();
};

// -- END OF FILE --
