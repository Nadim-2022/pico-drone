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
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/httpd.h"
#include "lwip/init.h"
//#include "picow_access_point.c"

#define TCP_PORT 80
#define DEBUG_printf printf
#define POLL_TIME_S 5

const char *cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
    char ssid[32] = {0};
    char password[64] = {0};

    for (int i = 0; i < iNumParams; i++) {
        if (strcmp(pcParam[i], "ssid") == 0) {
            strncpy(ssid, pcValue[i], sizeof(ssid) - 1);
        } else if (strcmp(pcParam[i], "password") == 0) {
            strncpy(password, pcValue[i], sizeof(password) - 1);
        }
    }
    if (ssid[0] != '\0' && password[0] != '\0') {
        printf("Received SSID: %s, Password: %s\n", ssid, password);
        return "/confirm.html";
    }

    return "/index.html"; // Redirect to a confirmation page or back to the form
}

const tCGI cgi_handlers[] = {
        {"/submit_form", cgi_handler}
};

void httpd_cgi_init(void) {
    http_set_cgi_handlers(cgi_handlers, sizeof(cgi_handlers) / sizeof(cgi_handlers[0]));
}





void NetworkTask(void *param);
void access_point_task(void *pvParameters);
void wifi_task(void *pvParameters);
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
    xTaskCreate(wifi_task, "WiFiTask", 2048, NULL, tskIDLE_PRIORITY + 1, nullptr);
    vTaskStartScheduler();
    while(true){

    };
}


void wifi_task(void *pvParameters) {

    if (cyw43_arch_init()) {
        DEBUG_printf("failed to initialise\n");
        return ;
    }
    const char *ap_name = "picow_test";
#if 1
    const char *password = "password";
#else
    const char *password = NULL;
#endif

    cyw43_arch_enable_ap_mode(ap_name, password, CYW43_AUTH_WPA2_AES_PSK);

    ip4_addr_t gw, mask;
    IP4_ADDR(&gw, 192, 168, 4, 1);
    IP4_ADDR(&mask, 255, 255, 255, 0);

    // Start the dhcp server
    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &gw, &mask);

    // Start the dns server
    //dns_server_t dns_server;
    //dns_server_init(&dns_server, &state->gw);
    httpd_init();
    httpd_cgi_init();
    while (1) {
#if PICO_CYW43_ARCH_POLL
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
#else
        sleep_ms(1000);
#endif
    }

    //dns_server_deinit(&dns_server);
    dhcp_server_deinit(&dhcp_server);
    cyw43_arch_deinit();
    printf("Test complete\n");
    return ;
}

/**
 * @brief Task to handle network operations.
 *
 * This task manages network connectivity and data transmission.
 * It sets network credentials, connects to the network, and sends/receives data.
 * The task runs indefinitely with a fixed delay between iterations.
 *
 * @param param Pointer to shared resources.
 */

void NetworkTask(void *param) {
    auto sharedResources = static_cast<SharedResources *>(param);
    NetworkClass network(static_cast<std::shared_ptr<SharedResources>>(sharedResources));
    bool network_status = false;
    bool transmit = false;
    while (true) {

        if (network_status) {
            network.connect();
            transmit = true;
            network.transmit = true;
            network_status = false;
        }
        printf("Network Task");
        vTaskDelay(1000);
    }
}

/**
 * @brief Task to handle interrupt events.
 *
 * This task initializes interrupt handlers for rotary encoder and buttons.
 * It continuously checks the event queue for any interrupt events and updates
 * the shared resources accordingly. The task runs indefinitely with a fixed delay
 * between iterations.
 *
 * @param param Pointer to shared resources.
 */

void InterruptHandler(void *param){
    /*
     auto sharedResources = static_cast<SharedResources *>(param);
     irqs rothandlerA(ROT_A_PIN);
    // irqs rothandlerB(rotB);
     irqs rothandlerP(ROT_SW_PIN);
     irqs sw0(BUTTON_0);
     irqs sw1(BUTTON_1);
     irqs sw2(BUTTON_2);


     while(true){

         // Queue  receive the events
         if (xQueueReceive(rothandlerA.eventQueue, &rothandlerA.event, portMAX_DELAY) == pdTRUE){
             switch (rothandlerA.event) {
                 case irqs::ROTARY_ENCODER_CW_EVENT:
                     sharedResources->isRotaryClockwise = true;
                     break;
                 case irqs::ROTARY_ENCODER_CCW_EVENT:
                     sharedResources->isRotaryCounterClockwise = true;
                     break;
                 case irqs::BUTTON_PRESS_EVENT:
                     sharedResources->isRotaryPressed = true;
                     break;
                 case irqs::BUTTON_PRESS_EVENT_0:
                     sharedResources->isSW0Pressed = true;
                     break;
                 case irqs::BUTTON_PRESS_EVENT_1:
                     sharedResources->isSW1Pressed = true;
                     break;
                 case irqs::BUTTON_PRESS_EVENT_2:
                     sharedResources->isSW2Pressed = true;
                     break;
                 default:
                     break;
             }
         }

         vTaskDelay(10);
     }
 */
}

