//
// Created by iamna on 14/01/2025.
//

#ifndef SMART_PLUG_RTC_H
#define SMART_PLUG_RTC_H

#include <cstdint>
#include "stdio.h"
#include <memory>
#include "PicoI2C.h"
#include "FreeRTOS.h"
#include "semphr.h"

// I2C Address of MCP79410
#define MCP79410_ADDR 0x6F

// MCP79410 Registers
#define RTCSEC 0x00
#define RTCMIN 0x01
#define RTCHOUR 0x02
#define RTCWKDAY 0x03
#define RTCDATE 0x04
#define RTCMTH 0x05
#define RTCYEAR 0x06
#define CONTROL 0x07

#define I2C_WAIT_TIME 5

class RTC {
public:
    explicit RTC(std::shared_ptr<PicoI2C> i2c, uint16_t device_address);
    ~RTC();
    void init();
    void set_time(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day, uint8_t month, uint8_t year);
    void get_time(uint8_t* seconds, uint8_t* minutes, uint8_t* hours, uint8_t* day, uint8_t* month, uint8_t* year);
    void set_alarm(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day, uint8_t date);
    void get_alarm(uint8_t* seconds, uint8_t* minutes, uint8_t* hours, uint8_t* day, uint8_t* date);
    void enable_alarm();
    void disable_alarm();
    void clear_alarm();
    SemaphoreHandle_t i2c_mutex;
private:
    std::shared_ptr<PicoI2C> i2c;
    uint16_t device_address;
    uint8_t dec_to_bcd(uint8_t dec);
    uint8_t bcd_to_dec(uint8_t bcd);
};


#endif //SMART_PLUG_RTC_H
