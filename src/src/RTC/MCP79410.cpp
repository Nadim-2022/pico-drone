#include <iostream>
#include <sstream>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hardware/gpio.h"
#include "PicoOsUart.h"
#include "pico/stdlib.h"
#include "semphr.h"

#include "hardware/timer.h"
#include "src/SharedResources.h"
#include "src/Network/NetworkClass.h"
#include "dnsserver.h"
#include "dhcpserver.h"
/**
 * @brief Reads the runtime counter.
 *
 * This function reads the current value of the runtime counter from the hardware timer.
 *
 * @return uint32_t The current value of the runtime counter.
 */
extern "C" {
uint32_t read_runtime_ctr(void) {
    return timer_hw->timerawl;
}
}

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

// Helper to convert decimal to BCD
uint8_t dec_to_bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

// Helper to convert BCD to decimal
uint8_t bcd_to_dec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

// Initialize MCP79410
void mcp79410_init(i2c_inst_t *i2c) {
    uint8_t buffer[2];

    // Enable oscillator by setting ST bit (bit 7 of RTCSEC register)
    buffer[0] = RTCSEC;
    buffer[1] = 0x80; // Set ST bit to 1
    i2c_write_blocking(i2c, MCP79410_ADDR, buffer, 2, false);

    // Enable battery backup by setting VBATEN bit (bit 3 of RTCWKDAY register)
    buffer[0] = RTCWKDAY;
    buffer[1] = 0x08; // Set VBATEN bit to 1
    i2c_write_blocking(i2c, MCP79410_ADDR, buffer, 2, false);
}

// Set time (hh:mm:ss) and date (yy-mm-dd)
void mcp79410_set_time(i2c_inst_t *i2c, uint8_t hh, uint8_t mm, uint8_t ss,
                       uint8_t yy, uint8_t month, uint8_t day) {
    uint8_t buffer[8];

    // Set time and date
    buffer[0] = RTCSEC; // Start at seconds register
    buffer[1] = dec_to_bcd(ss) | 0x80; // Set ST bit
    buffer[2] = dec_to_bcd(mm);
    buffer[3] = dec_to_bcd(hh);
    buffer[4] = 0x08 | 0x01; // VBATEN + Weekday (1 for Monday)
    buffer[5] = dec_to_bcd(day);
    buffer[6] = dec_to_bcd(month);
    buffer[7] = dec_to_bcd(yy);

    i2c_write_blocking(i2c, MCP79410_ADDR, buffer, 8, false);
}

/*// Read time and date
void mcp79410_get_time(i2c_inst_t *i2c) {
    uint8_t reg = RTCSEC;
    uint8_t buffer[7];

    // Request time data
    i2c_write_blocking(i2c, MCP79410_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c, MCP79410_ADDR, buffer, 7, false);

    uint8_t ss = bcd_to_dec(buffer[0] & 0x7F); // Mask ST bit
    uint8_t mm = bcd_to_dec(buffer[1]);
    uint8_t hh = bcd_to_dec(buffer[2]);
    uint8_t day = bcd_to_dec(buffer[4] & 0x07); // Mask other bits
    uint8_t month = bcd_to_dec(buffer[5] & 0x1F);
    uint8_t year = bcd_to_dec(buffer[6]);

    printf("Time: %02u:%02u:%02u, Date: %02u/%02u/20%02u\n", hh, mm, ss, day, month, year);
}*/
// Read time and date
void mcp79410_get_time(PicoI2C *i2cbus) {
    uint8_t reg = RTCSEC;
    uint8_t buffer[7];

    // Request time data
    //i2cbus->read(MCP79410_ADDR, buffer, 7);
    i2cbus->transaction(MCP79410_ADDR, &reg, 1, buffer, 7);
    //i2c_write_blocking(i2c, MCP79410_ADDR, &reg, 1, true);
    //i2c_read_blocking(i2c, MCP79410_ADDR, buffer, 7, false);

    uint8_t ss = bcd_to_dec(buffer[0] & 0x7F); // Mask ST bit
    uint8_t mm = bcd_to_dec(buffer[1]);
    uint8_t hh = bcd_to_dec(buffer[2]);
    uint8_t day = bcd_to_dec(buffer[4] & 0x3F); // Mask other bits
    uint8_t month = bcd_to_dec(buffer[5] & 0x1F);
    uint8_t year = bcd_to_dec(buffer[6]);

    printf("Time: %02u:%02u:%02u, Date: %02u/%02u/20%02u\n", hh, mm, ss, day, month, year);
}

void rtc(void *param);
void eeprom_task(void *param);
void eeprom(void *param);
int main()
{
    stdio_init_all();
    printf("\nBoot\n");

    SharedResources sharedResourcesPtr;
    //auto sharedResourcesPtr = std::make_shared<SharedResources>();
    sharedResourcesPtr.mutex = xSemaphoreCreateMutex();
    xTaskCreate(rtc, "Real time clock", 2048, NULL, 1, NULL);
    // xTaskCreate(eeprom_task, "EEPROM", 2048, &sharedResourcesPtr, 1, NULL);
    vTaskStartScheduler();
    while(true){

    };
}

// Function to config and run Real time clock
void rtc(void *param){
    // I2C initialize
    /*i2c_init(i2c0, 400 * 1000); // 100 kHz I2C frequency
    gpio_set_function(8, GPIO_FUNC_I2C); // SDA
    gpio_set_function(9, GPIO_FUNC_I2C); // SCL
    gpio_pull_up(8);
    gpio_pull_up(9);*/
    auto i2cbus{std::make_shared<PicoI2C>(0, 400000)};

    //mcp79410_init(i2c0);
    //mcp79410_set_time(i2c0, 8, 52, 10, 24, 1, 5); // Set time to 10:30:45 on 3rd Jan 2024

    while (true) {
        //mcp79410_get_time(i2c0); // Read and print time
        mcp79410_get_time(i2cbus.get());
        sleep_ms(1000);
    }
}
#include "src/eeprom/EEPROM.h"
#include "src/i2c/PicoI2C.h"
#define SSID_ADDR 0
#define PASSWORD_ADDR 64
#define CO2SP_ADDR 128
#define READ_WRITE_SIZE 62

void eeprom_task(void *param){

    auto i2cbus{std::make_shared<PicoI2C>(0, 400000)};
    /* gpio_set_function(8, GPIO_FUNC_I2C); // SDA
     gpio_set_function(9, GPIO_FUNC_I2C); // SCL
     gpio_pull_up(8);
     gpio_pull_up(9);*/
    EEPROM eeprom (i2cbus, 0x50);
    char readbuffer[64] = {0};
    char ssidbuffer[64] = {0};
    char passbuffer[64] = {0};
    char SSID[6]= "nadim";
    char PASSWORD[6] = "ahmed";
    bool boot = true;
    eeprom.writeToMemory(SSID_ADDR, (uint8_t*)SSID, READ_WRITE_SIZE);
    eeprom.writeToMemory(PASSWORD_ADDR, (uint8_t*)PASSWORD, READ_WRITE_SIZE);


    while(true){
        if(boot){
            eeprom.readFromMemory(SSID_ADDR, (uint8_t*)ssidbuffer, READ_WRITE_SIZE);
            eeprom.readFromMemory(PASSWORD_ADDR, (uint8_t*)passbuffer, READ_WRITE_SIZE);
            printf("SSID: %s\n", ssidbuffer);
            printf("Password: %s\n", passbuffer);
            boot = false;
        }
        vTaskDelay(100);
    }

}
