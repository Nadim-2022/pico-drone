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
#include "lwip/dns.h"
#include "lwip/apps/mqtt.h"
#include "lwip/altcp_tls.h"
//#include "picow_access_point.c"

#define TCP_PORT 80
#define DEBUG_printf printf
#define POLL_TIME_S 5

const uint8_t cert[] =
        "-----BEGIN CERTIFICATE-----\n"
        "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\n"
        "-----END CERTIFICATE-----\n";

const uint8_t cert2[] =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIFCzCCA/OgAwIBAgISA7NXJMeI1If+CSICLyQe6/LYMA0GCSqGSIb3DQEBCwUA\n"
        "MDMxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQwwCgYDVQQD\n"
        "EwNSMTEwHhcNMjQxMjIzMjI0OTI0WhcNMjUwMzIzMjI0OTIzWjAfMR0wGwYDVQQD\n"
        "DBQqLnMxLmV1LmhpdmVtcS5jbG91ZDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCC\n"
        "AQoCggEBAKVuz2sMPmxx2w/f81/YAEKTbNZMJPk2+ooLFg5hxXvReF+AwIT4XvZ+\n"
        "MLhSKvFxmghJF+BB9WyhqrcJLGDCP4s6SOLWTYixEoTcaLUviqqn+06kYqDJ6E83\n"
        "NGsc7T42DlPnzqcZZjPRed9rt4CP3RgeZlWyYZgiD8FoJG9gie8ytihF/FkGZT8T\n"
        "N4Vkl2vQa3mfBWeeKrcuhcLPxqIWDz/30iYfLtEe5JYYScoCKTXcP9SUStjpR8pD\n"
        "vfOWdvasOAuBy7yBbx01/4lcQt50hfbhTR/K14/D4rNkuuvU7ktSQnoxVXC8YDwG\n"
        "zkny10DFt65mVYLNZcBQtOLHHOZGV30CAwEAAaOCAiswggInMA4GA1UdDwEB/wQE\n"
        "AwIFoDAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwDAYDVR0TAQH/BAIw\n"
        "ADAdBgNVHQ4EFgQUgsEjDU35+EWJKBsFxJ0lM0PXMi4wHwYDVR0jBBgwFoAUxc9G\n"
        "pOr0w8B6bJXELbBeki8m47kwVwYIKwYBBQUHAQEESzBJMCIGCCsGAQUFBzABhhZo\n"
        "dHRwOi8vcjExLm8ubGVuY3Iub3JnMCMGCCsGAQUFBzAChhdodHRwOi8vcjExLmku\n"
        "bGVuY3Iub3JnLzAzBgNVHREELDAqghQqLnMxLmV1LmhpdmVtcS5jbG91ZIISczEu\n"
        "ZXUuaGl2ZW1xLmNsb3VkMBMGA1UdIAQMMAowCAYGZ4EMAQIBMIIBAwYKKwYBBAHW\n"
        "eQIEAgSB9ASB8QDvAHYAzPsPaoVxCWX+lZtTzumyfCLphVwNl422qX5UwP5MDbAA\n"
        "AAGT9euKuwAABAMARzBFAiEA7kt5ecRQhyl1JsSPpgt4bN14o+/BZspQCq0d46Wy\n"
        "03wCIDZ17LnI6Hh+cIF6SlX64OB4pc18XilUqI7pffaEJEENAHUAzxFW7tUufK/z\n"
        "h1vZaS6b6RpxZ0qwF+ysAdJbd87MOwgAAAGT9euK3wAABAMARjBEAiAlffjlUAeU\n"
        "7T6o7ISkiFGXz5G9tx2BB2C5f+GQdqc59QIgEdxGXKjoAJYYlYeZqig2LQxxkdPZ\n"
        "JaYjkrdr9PBeVcYwDQYJKoZIhvcNAQELBQADggEBADKUM+E39KujX/GS+beQajyU\n"
        "/19CFjB+TXYoaXpRPbXTL9XvhCNWI5ZiGc+uGOFneNBED+24YoC1JTLW3a+bWfuJ\n"
        "hBl8bjJoxbP38MsffsWnQ3CGEO4EdcwqdyYf68qGY9FhxvVAx7nzf8qGzRuN0waQ\n"
        "INpYn6eTjZiCICHPdnQnntVSfTza+mzNEBYqZpHAqkpUywG8pEUytJG7ECw5Z79r\n"
        "bEo3gwI2XCfSfVS57aizBYWeZq0tvAmfy7YD3ubo/IIjB4WhxINwgVAPk5oqSToM\n"
        "ZzKtyDiKUcubGARwO0QPjufvvvmbKB56xNtKELvpSTOUCrei9HQcV+utJWvrZSA=\n"
        "-----END CERTIFICATE-----\n";

mqtt_client_t *client;

void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("MQTT connection established.\n");

        // Subscribe to a topic
        const char *sub_topic = "test/topic";
        mqtt_subscribe(client, sub_topic, 0, NULL, NULL); // QoS 0 for simplicity
        printf("Subscribed to topic: %s\n", sub_topic);
    } else {
        printf("MQTT connection failed: %d\n", status);
    }
}

void mqtt_publish_cb(void *arg, err_t err) {
    if (err == ERR_OK) {
        printf("Message published successfully.\n");
    } else {
        printf("Failed to publish message: %d\n", err);
    }
}

void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    printf("MQTT Data Received: %.*s\n", len, (const char *)data);
}

void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    printf("MQTT Topic Received: %s, Length: %d\n", topic, tot_len);
}





void NetworkTask(void *param);

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
    xTaskCreate(internet_task, "InternetTask", 2048, NULL, tskIDLE_PRIORITY + 1, nullptr);
    vTaskStartScheduler();
    while(true){

    };
}

void internet_task(void *pvParameters) {
    bool network_status = false;
    if (cyw43_arch_init()) {
        DEBUG_printf("failed to initialise\n");
        return;
    }
    const char *ap_name = "TP-Link_A2FC";
    const char *password = "nadimahmed";
    cyw43_arch_enable_sta_mode();
    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(ap_name, password, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Failed to connect to Wi-Fi.\n");
    } else {
        printf("Connected to Wi-Fi.\n");
        network_status = true;
        // Print the ip address of the device
        // Print the IP address
        // Print the IP address
        struct netif *netif = netif_list;
        if (netif_is_up(netif)) {
            printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
        } else {
            printf("Network interface is down\n");
        }
    }
    printf("Connected to Wi-Fi.\n");
    vTaskDelay(10000);
    //ip_addr_t broker_ip;
    //IP4_ADDR(&broker_ip, 192, 168, 0, 210); // Example IP address of HiveMQ
    //54.73.92.158
    const char *hostname = "d2ae37053fd34d8db5914cbef1ff7c95.s1.eu.hivemq.cloud";
    //const char *hostname = "broker.hivemq.com";
    ip_addr_t resolved_addr;
    bool ip_resolved = false;
    while (!ip_resolved) {
        if (dns_gethostbyname(hostname, &resolved_addr, NULL, NULL) == ERR_OK) {
            printf("Resolved IP: %s\n", ipaddr_ntoa(&resolved_addr));
            ip_resolved = true;
        } else {
            printf("DNS resolution failed or in progress.\n");
        }
        vTaskDelay(5000);
    }
    printf("resolved IP: %s\n", ipaddr_ntoa(&resolved_addr));
    //IP4_ADDR(&broker_ip, 46,137,47,218);


// Create TLS configuration
    struct altcp_tls_config *tls_config = altcp_tls_create_config_client(cert, sizeof(cert)+1);
    //struct altcp_tls_config *tls_config = altcp_tls_create_config_client(NULL,  0);
    //struct altcp_tls_config *tls_config = altcp_tls_create_config_client_2wayauth(NULL, 0, NULL, 0, NULL, 0, NULL, 0);
    /*if (!tls_config) {
        printf("Failed to create TLS configuration\n");
        return;
    }*/
    assert(tls_config);

    mbedtls_ssl_conf_authmode((mbedtls_ssl_config *)tls_config, MBEDTLS_SSL_VERIFY_REQUIRED);
    /*assert(tls_config);
    // Disable certificate verification (not secure)
    mbedtls_ssl_conf_authmode((mbedtls_ssl_config *)tls_config, MBEDTLS_SSL_VERIFY_NONE);
    //mbedtls_ssl_conf_authmode((mbedtls_ssl_config *)tls_config, MBEDTLS_SSL_VERIFY_OPTIONAL);
    //mbedtls_ssl_conf_authmode(&tls_config->conf, MBEDTLS_SSL_VERIFY_NONE);
    struct altcp_pcb *tls_conn = altcp_tls_new(tls_config, IPADDR_TYPE_V4);
    if (!tls_conn) {
        printf("Failed to create TLS connection\n");
        return ;
    }*/

    /* static const char *alpn_protocols[] = { "mqtt", NULL }; // Replace "mqtt" with the required protocol string.
     altcp_tls_configure_alpn_protocols(tls_config, alpn_protocols);*/

    struct mqtt_connect_client_info_t ci = {
            .client_id = "banglafin",
            .client_user = "nadim",  // Set to username if required
            .client_pass = "Nadim420",  // Set to password if required
            .keep_alive = 60,
            .tls_config = tls_config,
            .server_name = hostname
    };

    client = mqtt_client_new();
    if (client == NULL) {
        printf("Failed to create MQTT client.\n");
    }
    /* mbedtls_ssl_set_hostname((mbedtls_ssl_context *)altcp_tls_context(tls_conn), ipaddr_ntoa(&resolved_addr));

     printf("resolving %s\n", hostname);*/
    // Set incoming message and publish callbacks
    mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, NULL);

    // Connect to the broker
    mqtt_client_connect(client, &resolved_addr, 8883, mqtt_connection_cb, NULL, &ci);

    // Publish a message to the broker
    const char *pub_topic = "nadim";
    const char *message = "Hello from Pico W!";
    mqtt_publish(client, pub_topic, message, strlen(message), 0, 0, mqtt_publish_cb, NULL);
    mqtt_subscribe(client, pub_topic, 0, NULL, NULL);


    while (true){
        //mqtt_publish(client, pub_topic, message, strlen(message), 0, 0, mqtt_publish_cb, NULL);
        vTaskDelay(4000);
    }

}


