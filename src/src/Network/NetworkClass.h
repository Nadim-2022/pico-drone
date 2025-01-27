#ifndef RP2040_FREERTOS_IRQ_NETWORKCLASS_H
#define RP2040_FREERTOS_IRQ_NETWORKCLASS_H

#include <mbedtls/debug.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "lwip/apps/httpd.h"
#include "lwip/init.h"
#include "task.h"
#include "timers.h"
#include "src/SharedResources.h"
#include "dhcpserver.h"

class NetworkClass {
public:
  NetworkClass(std::shared_ptr<SharedResources> sharedResources );
    void init();
    void deinit();
    int connect();
    static void setCredentials(char* ssid, char* password);
    void httpd_cgi_init();
    void ap_init();
    static bool areCredentialsSet();
    bool ap_disconnect();

private:
    static char ssid[32];
    static char password[64];
    std::shared_ptr<SharedResources> resources;
    dhcp_server_t dhcp_server;
    uint32_t event;
    const char *ap_name = "picow_test";
    const char *ap_pass = "password";
    ip4_addr_t gw, mask;
    static const char *cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
    const tCGI cgi_handlers[1] = {
            {"/submit_form", cgi_handler}
    };
    static bool is_credentials_set;
};


#endif //RP2040_FREERTOS_IRQ_NETWORKCLASS_H
