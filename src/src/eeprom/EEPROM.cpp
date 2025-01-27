#include "EEPROM.h"
#include "PicoI2C.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <cstdio>
#include <cstring>

#define EEPROM_SIZE 32768  // 32 KB total size
#define I2C_WAIT_TIME 5


EEPROM::EEPROM(std::shared_ptr<PicoI2C> i2c, uint16_t device_address)
        : i2c(i2c), device_address(device_address) {
    i2c_mutex = xSemaphoreCreateMutex();
}

EEPROM::~EEPROM() {}

uint16_t EEPROM::crc16(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// Write to memory with size and CRC
void EEPROM::writeToMemory(uint16_t memory_address, const uint8_t* data, size_t length) {
    if (length + 2 > 64) {  // Assuming 64-byte block size, 2 bytes reserved for CRC
        printf("Data length exceeds block size.\n");
        return;
    }

    uint16_t crc = crc16(data, length);  // Calculate CRC for the data

    if (xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
        uint8_t buffer[64];  // Buffer size to hold the data and CRC
        buffer[0] = (memory_address >> 8) & 0xFF; // High byte of memory address
        buffer[1] = memory_address & 0xFF;        // Low byte of memory address
        memcpy(buffer + 2, data, length);         // Copy data after address
        buffer[length + 2] = (crc >> 8) & 0xFF;   // Store high byte of CRC
        buffer[length + 3] = crc & 0xFF;          // Store low byte of CRC

        i2c->write(this->device_address, buffer, length + 4);  // Write memory address + data + CRC
        vTaskDelay(pdMS_TO_TICKS(I2C_WAIT_TIME));              // FreeRTOS-compliant delay

        xSemaphoreGive(i2c_mutex);  // Release mutex
    } else {
        printf("Failed to take mutex for EEPROM write operation.\n");
    }
}

// Read from memory with size and CRC verification
void EEPROM::readFromMemory(uint16_t memory_address, uint8_t* data, size_t length) {
    if (length + 2 > 64) {  // Assuming 64-byte block size, 2 bytes reserved for CRC
        printf("Data length exceeds block size.\n");
        return;
    }

    if (xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE) {
        uint8_t buffer[64];
        uint8_t address_buffer[2];
        address_buffer[0] = (memory_address >> 8) & 0xFF;  // High byte of memory address
        address_buffer[1] = memory_address & 0xFF;         // Low byte of memory address

        // First, send the memory address to the EEPROM
        i2c->write(this->device_address, address_buffer, 2);

        // Now, read the data starting at that address (including CRC)
        i2c->read(this->device_address, buffer, length + 2);
        // Extract the CRC from the last 2 bytes
        uint16_t stored_crc = (buffer[length] << 8) | buffer[length + 1];
        uint16_t calculated_crc = crc16(buffer, length);

        if (stored_crc != calculated_crc) {
            printf("CRC mismatch! Data is corrupted.\n");
            memset(data, 0, length);  // Clear the data buffer if corrupted
        } else {
            memcpy(data, buffer, length);  // Copy valid data to the output buffer
            printf("Read data from EEPROM.\n");
        }

        xSemaphoreGive(i2c_mutex);  // Release mutex
    } else {
        printf("Failed to take mutex for EEPROM read operation.\n");
    }
}