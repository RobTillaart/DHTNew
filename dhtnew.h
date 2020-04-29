#pragma once
//
//    FILE: dhtnew.h
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.6
// PURPOSE: DHT Temperature & Humidity Sensor library for Arduino
//     URL: https://github.com/RobTillaart/DHTNEW
//
// HISTORY:
// see dhtnew.cpp file
//

#include <Arduino.h>

#define DHTNEW_LIB_VERSION "0.1.6"

#define DHTLIB_OK                0
#define DHTLIB_ERROR_CHECKSUM   -1
#define DHTLIB_ERROR_TIMEOUT    -2
#define DHTLIB_INVALID_VALUE    -999

#define DHTLIB_DHT11_WAKEUP     18
#define DHTLIB_DHT_WAKEUP       1

#define DHTLIB_DHT_READ_DELAY   2000

// max timeout is 100usec.
// For a 16Mhz proc that is max 1600 clock cycles
// loops using TIMEOUT use at least 4 clock cycli
// so 100 us takes max 400 loops
// so by dividing F_CPU by 40000 we "fail" as fast as possible

#define DHTLIB_TIMEOUT (F_CPU/40000)

class DHTNEW
{
public:

    DHTNEW(uint8_t pin);

    // 0 = unknown, 11 or 22
    int   getType()                   { return _type; };
    int   read();

    // lastRead is in MilliSeconds since start sketch
    uint32_t lastRead()               { return _lastRead; };

	// preferred interface
	float getHumidity()               { return humidity; };
	float getTemperature()            { return temperature; };

	// these values should be private in future ==> read only
    float humidity;
    float temperature;

    // adding offsets works well in normal range 
    // but can introduce under- or overflow at the ends of the sensor range
    void  setHumOffset(float offset)  { _humOffset = offset; };
    void  setTempOffset(float offset) { _tempOffset = offset; };
    float getHumOffset()              { return _humOffset; };
    float getTempOffset()             { return _tempOffset; };

    bool getDisableIRQ()              { return _disableIRQ; };
    void setDisableIRQ(bool b )       { _disableIRQ = b; };

private:
    uint8_t  _pin = 0;
    uint8_t  _wakeupDelay = 0;
    uint8_t  _type = 0;
    float    _humOffset = 0.0;
    float    _tempOffset = 0.0;
    uint32_t _lastRead = 0;
    bool     _disableIRQ = false;

    uint8_t  _bits[5];  // buffer to receive data
    int      _read();
    int      _readSensor();
};

// -- END OF FILE --
