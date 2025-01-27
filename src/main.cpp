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
#include "src/MQTT/MQTTClient.h"
#include "src/RTC/RTC.h"
#include "src/eeprom/EEPROM.h"

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

void eeprom_task(void *param);
void rtc(void *param);
void internet_task(void *pvParameters);
void blink_led(void *pvParameters);
int main()
{
    stdio_init_all();
    printf("\nBoot\n");

    SharedResources sharedResourcesPtr;
    //auto sharedResourcesPtr = std::make_shared<SharedResources>();
    sharedResourcesPtr.mutex = xSemaphoreCreateMutex();
   // xTaskCreate(eeprom_task, "EEPROM", 2048, &sharedResourcesPtr, 1, NULL);
    //xTaskCreate(rtc, "Real time clock", 2048, NULL, 1, NULL);
    //xTaskCreate(internet_task, "InternetTask", 2048, &sharedResourcesPtr, tskIDLE_PRIORITY + 1, nullptr);
    xTaskCreate(blink_led, "Blink LED", 2048, NULL, 1, NULL);
    vTaskStartScheduler();
    while(true){

    };
}

void blink_led(void *pvParameters){
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    while(true){
        gpio_put(25, 1);
        printf("LED ON\n");
        sleep_ms(1000);
        gpio_put(25, 0);
        printf("LED OFF\n");
        sleep_ms(1000);
    }
}

#define SSID_ADDR 0
#define PASSWORD_ADDR 64
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

void rtc(void *param){
    auto i2cbus{std::make_shared<PicoI2C>(0, 400000)};
    RTC rtc(i2cbus, 0x6F);
    uint8_t seconds = 0;
    uint8_t minutes = 0;
    uint8_t hours = 0;
    uint8_t day = 0;
    uint8_t month = 0;
    uint8_t year = 0;

    while (true) {
        rtc.get_time( &seconds, &minutes, &hours, &day, &month, &year);
        printf("Time: %02u:%02u:%02u, Date: %02u/%02u/20%02u\n",hours,minutes,seconds, day, month, year);
        sleep_ms(5000);
    }
}
void internet_task(void *pvParameters) {
    NetworkClass network(static_cast<std::shared_ptr<SharedResources>>((SharedResources *) pvParameters));
    network.ap_init();
    while(!network.areCredentialsSet()){
        vTaskDelay(1000);
    }
    if (network.areCredentialsSet()){
        network.ap_disconnect();
        network.connect();
    }
    MQTTClient mqttClient;
    mqttClient.connect();
    mqttClient.subscribe("me");
    while (true){
        if(mqttClient.is_connected()){
            mqttClient.publish("me2",  "Hello from Pico");
        } else{
            printf("MQTT not connected");
        }
        vTaskDelay(10000);
    }

}


