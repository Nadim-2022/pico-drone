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


void rtc(void *param);
void internet_task(void *pvParameters);
int main()
{
    stdio_init_all();
    printf("\nBoot\n");

    SharedResources sharedResourcesPtr;
    //auto sharedResourcesPtr = std::make_shared<SharedResources>();
    sharedResourcesPtr.mutex = xSemaphoreCreateMutex();
    // Network task
    // xTaskCreate(NetworkTask, "NetworkTask", 6000, &sharedResourcesPtr,
    //           tskIDLE_PRIORITY + 1, nullptr);
    sleep_ms(2000); // Allow console to settle
    //xTaskCreate(wifi_task, "WiFiTask", 2048, NULL, tskIDLE_PRIORITY + 1, nullptr);
    xTaskCreate(rtc, "Real time clock", 2048, NULL, 1, NULL);
    xTaskCreate(internet_task, "InternetTask", 2048, &sharedResourcesPtr, tskIDLE_PRIORITY + 1, nullptr);
    vTaskStartScheduler();
    while(true){

    };
}
void rtc(void *param){
    auto i2cbus{std::make_shared<PicoI2C>(0, 400000)};
    RTC rtc(i2cbus, 0x6F);
    uint8_t *seconds = 0;
    uint8_t *minutes = 0;
    uint8_t *hours = 0;
    uint8_t *day = 0;
    uint8_t *month = 0;
    uint8_t *year = 0;

    while (true) {
        rtc.get_time( seconds, minutes, hours, day, month, year);
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


