//
// Created by iamna on 14/01/2025.
//

#include "RTC.h"

RTC::RTC(std::shared_ptr<PicoI2C> i2c, uint16_t device_address)
        : i2c(i2c), device_address(device_address) {
    i2c_mutex = xSemaphoreCreateMutex();
}

RTC::~RTC() {}

void RTC::init() {
    if (xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
        uint8_t buffer[2];
        // Enable oscillator by setting ST bit (bit 7 of RTCSEC register)
        buffer[0] = RTCSEC;
        buffer[1] = 0x80; // Set ST bit to 1

        i2c->write(this->device_address, buffer, 2);
        vTaskDelay(pdMS_TO_TICKS(I2C_WAIT_TIME));

        // Enable battery backup by setting VBATEN bit (bit 3 of RTCWKDAY register)
        buffer[0] = RTCWKDAY;
        buffer[1] = 0x08; // Set VBATEN bit to 1

        i2c->write(this->device_address, buffer, 2);
        xSemaphoreGive(i2c_mutex);
    } else {
        printf("Failed to take mutex for RTC init operation.\n");
    }
}

uint8_t RTC::dec_to_bcd(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

uint8_t RTC::bcd_to_dec(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

void RTC::set_time(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day, uint8_t month, uint8_t year) {
    if (xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
        uint8_t buffer[8];
        // Set time and date
        buffer[0] = RTCSEC; // Start at seconds register
        buffer[1] = dec_to_bcd(seconds) | 0x80; // Set ST bit
        buffer[2] = dec_to_bcd(minutes);
        buffer[3] = dec_to_bcd(hours);
        buffer[4] = 0x08 | 0x01; // VBATEN + Weekday (1 for Monday)
        buffer[5] = dec_to_bcd(day);
        buffer[6] = dec_to_bcd(month);
        buffer[7] = dec_to_bcd(year);

        i2c->write(this->device_address, buffer, 8);
        vTaskDelay(pdMS_TO_TICKS(I2C_WAIT_TIME));

        xSemaphoreGive(i2c_mutex);
    } else {
        printf("Failed to take mutex for RTC set time operation.\n");
    }
}

void RTC::get_time(uint8_t* seconds, uint8_t* minutes, uint8_t* hours, uint8_t* day, uint8_t* month, uint8_t* year) {
    if (xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
        uint8_t reg = RTCSEC;
        uint8_t buffer[7];
        // Request time data
        i2c->transaction(MCP79410_ADDR, &reg, 1, buffer, 7);

        *seconds = bcd_to_dec(buffer[0] & 0x7F); // Mask ST bit
        *minutes = bcd_to_dec(buffer[1]);
        *hours = bcd_to_dec(buffer[2]);
        *day = bcd_to_dec(buffer[4] & 0x3F); // Mask other bits
        *month = bcd_to_dec(buffer[5] & 0x1F);
        *year = bcd_to_dec(buffer[6]);
        /*
        printf("Time: %02u:%02u:%02u, Date: %02u/%02u/20%02u\n",
               bcd_to_dec(buffer[2]),
               bcd_to_dec(buffer[1]),
               bcd_to_dec(buffer[0] & 0x7F),
               bcd_to_dec(buffer[4] & 0x3F),
               bcd_to_dec(buffer[5] & 0x1F),
               bcd_to_dec(buffer[6]));
        */
        xSemaphoreGive(i2c_mutex);
    } else {
        printf("Failed to take mutex for RTC get time operation.\n");
    }
}

void RTC::set_alarm(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day, uint8_t date) {
    if (xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
        uint8_t buffer[6];
        // Set alarm time and date
        buffer[0] = 0x0A; // Start at ALM0SEC register
        buffer[1] = dec_to_bcd(seconds) | 0x80; // Set ST bit
        buffer[2] = dec_to_bcd(minutes);
        buffer[3] = dec_to_bcd(hours);
        buffer[4] = dec_to_bcd(day);
        buffer[5] = dec_to_bcd(date);

        i2c->write(this->device_address, buffer, 6);
        vTaskDelay(pdMS_TO_TICKS(I2C_WAIT_TIME));

        xSemaphoreGive(i2c_mutex);
    } else {
        printf("Failed to take mutex for RTC set alarm operation.\n");
    }
}


void RTC::clear_alarm() {
if (xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
        uint8_t buffer[2];
        // Clear alarm flag by setting ALMPOL bit (bit 7 of CONTROL register)
        buffer[0] = CONTROL;
        buffer[1] = 0x00; // Clear ALMPOL bit

        i2c->write(this->device_address, buffer, 2);
        vTaskDelay(pdMS_TO_TICKS(I2C_WAIT_TIME));

        xSemaphoreGive(i2c_mutex);
    } else {
        printf("Failed to take mutex for RTC clear alarm operation.\n");
    }
}


