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
//#include "picow_access_point.c"

#define TCP_PORT 80
#define DEBUG_printf printf
#define POLL_TIME_S 5
#define HTTP_GET "GET"
#define HTTP_POST "POST"
#define HTTP_RESPONSE_HEADERS "HTTP/1.1 %d OK\nContent-Length: %d\nContent-Type: text/html; charset=utf-8\nConnection: close\n\n"
//#define CONNECT_FORM_BODY "<html><body><h1>WiFi Config</h1><form method=\"POST\" action=\"/connect\"><label for=\"ssid\">SSID:</label><input type=\"text\" id=\"ssid\" name=\"ssid\"><br><label for=\"password\">Password:</label><input type=\"password\" id=\"password\" name=\"password\"><br><input type=\"submit\" value=\"Connect\"></form></body></html>"
//#define CONNECT_FORM_BODY "<html><head><style>" \
                            "body { font-family: Arial, sans-serif; background-color: #f4f4f9; margin: 0; padding: 0; }" \
                            "h1 { text-align: center; color: #333; padding-top: 20px; }" \
                            "form { width: 300px; margin: 20px auto; padding: 20px; border: 1px solid #ccc; background-color: #fff; border-radius: 8px; box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1); }" \
                            "label { display: block; margin-bottom: 8px; font-size: 14px; color: #333; }" \
                            "input[type='text'], input[type='password'] { width: 100%; padding: 8px; margin-bottom: 15px; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; }" \
                            "input[type='submit'] { width: 100%; padding: 10px; background-color: #4CAF50; color: white; border: none; border-radius: 4px; cursor: pointer; font-size: 16px; }" \
                            "input[type='submit']:hover { background-color: #45a049; }" \
                          "</style></head><body>" \
                            "<h1>WiFi Config</h1>" \
                            "<form method=\"POST\" action=\"/connect\">" \
                            "<label for=\"ssid\">SSID:</label>" \
                            "<input type=\"text\" id=\"ssid\" name=\"ssid\"><br>" \
                            "<label for=\"password\">Password:</label>" \
                            "<input type=\"password\" id=\"password\" name=\"password\"><br>" \
                            "<input type=\"submit\" value=\"Connect\">" \
                            "</form>" \
                          "</body></html>"

//#define CONNECT_RESPONSE_BODY "<html><body><h1>Connecting to WiFi...</h1></body></html>"
#define CONNECT_RESPONSE_BODY "<html><head><style>" \
                                "body { font-family: Arial, sans-serif; background-color: #f4f4f9; margin: 0; padding: 0; }" \
                                "h1 { text-align: center; color: #333; padding-top: 50px; }" \
                                ".message { text-align: center; font-size: 20px; color: #333; padding-top: 20px; }" \
                                "body { display: flex; justify-content: center; align-items: center; height: 100vh; }" \
                                ".container { text-align: center; }" \
                                "h1 { color: #4CAF50; font-size: 32px; }" \
                              "</style></head><body>" \
                                "<div class=\"container\">" \
                                "<h1>Connecting to WiFi...</h1>" \
                                "<div class=\"message\">Please wait while we establish the connection.</div>" \
                                "</div>" \
                              "</body></html>"
#define CONNECT_PATH "/connect"
#define LED_GPIO 0
#define CONNECT_FORM_BODY "<html><head><style>" \
                            "body { font-family: Arial, sans-serif; background-color: #f4f4f9; margin: 0; padding: 0; }" \
                            "h1 { text-align: center; color: #333; padding-top: 20px; }" \
                            "form { width: 300px; margin: 20px auto; padding: 20px; border: 1px solid #ccc; background-color: #fff; border-radius: 8px; box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1); }" \
                            "label { display: block; margin-bottom: 8px; font-size: 14px; color: #333; }" \
                            "input[type='text'], input[type='password'] { width: 100%; padding: 8px; margin-bottom: 15px; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; }" \
                            "input[type='submit'] { width: 100%; padding: 10px; background-color: #4CAF50; color: white; border: none; border-radius: 4px; cursor: pointer; font-size: 16px; }" \
                            "input[type='submit']:hover { background-color: #45a049; }" \
                            "@media (max-width: 600px) { " \
                            "form { width: 90%; margin: 10px auto; padding: 15px; }" \
                            "h1 { font-size: 24px; padding-top: 15px; }" \
                            "label { font-size: 16px; }" \
                            "input[type='text'], input[type='password'], input[type='submit'] { font-size: 14px; padding: 12px; }" \
                            "}" \
                          "</style></head><body>" \
                            "<h1>WiFi Config</h1>" \
                            "<form method=\"POST\" action=\"/connect\">" \
                            "<label for=\"ssid\">SSID:</label>" \
                            "<input type=\"text\" id=\"ssid\" name=\"ssid\"><br>" \
                            "<label for=\"password\">Password:</label>" \
                            "<input type=\"password\" id=\"password\" name=\"password\"><br>" \
                            "<input type=\"submit\" value=\"Connect\">" \
                            "</form>" \
                          "</body></html>"


typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;
    bool complete;
    ip_addr_t gw;
} TCP_SERVER_T;

typedef struct TCP_CONNECT_STATE_T_ {
    struct tcp_pcb *pcb;
    int sent_len;
    char headers[2048];
    char result[2048];
    int header_len;
    int result_len;
    ip_addr_t *gw;
} TCP_CONNECT_STATE_T;

static err_t tcp_close_client_connection(TCP_CONNECT_STATE_T *con_state, struct tcp_pcb *client_pcb, err_t close_err) {
    if (client_pcb) {
        assert(con_state && con_state->pcb == client_pcb);
        tcp_arg(client_pcb, NULL);
        tcp_poll(client_pcb, NULL, 0);
        tcp_sent(client_pcb, NULL);
        tcp_recv(client_pcb, NULL);
        tcp_err(client_pcb, NULL);
        err_t err = tcp_close(client_pcb);
        if (err != ERR_OK) {
            DEBUG_printf("close failed %d, calling abort\n", err);
            tcp_abort(client_pcb);
            close_err = ERR_ABRT;
        }
        if (con_state) {
            free(con_state);
        }
    }
    return close_err;
}

static void tcp_server_close(TCP_SERVER_T *state) {
    if (state->server_pcb) {
        tcp_arg(state->server_pcb, NULL);
        tcp_close(state->server_pcb);
        state->server_pcb = NULL;
    }
}

static err_t tcp_server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    DEBUG_printf("tcp_server_sent %u\n", len);
    con_state->sent_len += len;
    if (con_state->sent_len >= con_state->header_len + con_state->result_len) {
        DEBUG_printf("all done\n");
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }
    return ERR_OK;
}

static void connect_to_wifi(const char *ssid, const char *password) {
    cyw43_arch_lwip_begin();
    cyw43_arch_disable_ap_mode();
    cyw43_arch_deinit();
    cyw43_arch_init();
    cyw43_arch_enable_sta_mode();
    cyw43_arch_wifi_connect_async(ssid, password, CYW43_AUTH_WPA2_AES_PSK);
    cyw43_arch_lwip_end();
}

static int handle_request(const char *request, const char *body, char *result, size_t max_result_len) {
    int len = 0;
    DEBUG_printf("Request data : %s\n", request);
    //DEBUG_printf("CONNECT_PATH: %s\n", CONNECT_PATH);
    char path[128] = {0};
    sscanf(request, "%127s", path); // Extract the first word (the path)
    DEBUG_printf("Extracted path: %s\n", path);
    if (strcmp(path, CONNECT_PATH) == 0) {
        DEBUG_printf("Body data : %s\n", body);
        if (body) {
            char ssid[32];
            char password[64];
            sscanf(body, "ssid=%31[^&]&password=%63s", ssid, password);
            //connect_to_wifi(ssid, password);
            DEBUG_printf("WiFI SSID: %s\n", ssid);
            DEBUG_printf("WiFI Password: %s\n", password);
            len = snprintf(result, max_result_len, CONNECT_RESPONSE_BODY);
        } else {
            len = snprintf(result, max_result_len, CONNECT_FORM_BODY);
        }
    } else {
        len = snprintf(result, max_result_len, CONNECT_FORM_BODY);
    }
    return len;
}

err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (!p) {
        DEBUG_printf("connection closed\n");
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }
    assert(con_state && con_state->pcb == pcb);
    if (p->tot_len > 0) {
        DEBUG_printf("tcp_server_recv %d err %d\n", p->tot_len, err);

        // Copy the request into the buffer
        pbuf_copy_partial(p, con_state->headers, p->tot_len > sizeof(con_state->headers) - 1 ? sizeof(con_state->headers) - 1 : p->tot_len, 0);

        // Handle GET or POST request
        if (strncmp(HTTP_GET, con_state->headers, sizeof(HTTP_GET) - 1) == 0 || strncmp(HTTP_POST, con_state->headers, sizeof(HTTP_POST) - 1) == 0) {
            char *request = con_state->headers + sizeof(HTTP_GET); // + space
            char *body = NULL;
            if (strncmp(HTTP_POST, con_state->headers, sizeof(HTTP_POST) - 1) == 0) {
                body = strstr(con_state->headers, "\r\n\r\n");
                if (body) {
                    body += 4; // Skip the "\r\n\r\n"
                }
            }

            // Generate content
            con_state->result_len = handle_request(request, body, con_state->result, sizeof(con_state->result));
            //DEBUG_printf("Request: %s\n", request);
            // DEBUG_printf("Result: %d\n", con_state->result_len);

            // Check we had enough buffer space
            if (con_state->result_len > sizeof(con_state->result) - 1) {
                DEBUG_printf("Too much result data %d\n", con_state->result_len);
                return tcp_close_client_connection(con_state, pcb, ERR_CLSD);
            }

            // Generate web page
            if (con_state->result_len > 0) {
                con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_HEADERS,
                                                 200, con_state->result_len);
                if (con_state->header_len > sizeof(con_state->headers) - 1) {
                    DEBUG_printf("Too much header data %d\n", con_state->header_len);
                    return tcp_close_client_connection(con_state, pcb, ERR_CLSD);
                }
            }

            // Send the headers to the client
            con_state->sent_len = 0;
            err_t err = tcp_write(pcb, con_state->headers, con_state->header_len, 0);
            if (err != ERR_OK) {
                DEBUG_printf("failed to write header data %d\n", err);
                return tcp_close_client_connection(con_state, pcb, err);
            }

            // Send the body to the client
            if (con_state->result_len) {
                err = tcp_write(pcb, con_state->result, con_state->result_len, 0);
                if (err != ERR_OK) {
                    DEBUG_printf("failed to write result data %d\n", err);
                    return tcp_close_client_connection(con_state, pcb, err);
                }
            }
        }
        tcp_recved(pcb, p->tot_len);
    }
    pbuf_free(p);
    return ERR_OK;
}

static err_t tcp_server_poll(void *arg, struct tcp_pcb *pcb) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    DEBUG_printf("tcp_server_poll_fn\n");
    return tcp_close_client_connection(con_state, pcb, ERR_OK); // Just disconnect clent?
}

static void tcp_server_err(void *arg, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (err != ERR_ABRT) {
        DEBUG_printf("tcp_client_err_fn %d\n", err);
        tcp_close_client_connection(con_state, con_state->pcb, err);
    }
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        DEBUG_printf("failure in accept\n");
        return ERR_VAL;
    }
    DEBUG_printf("client connected\n");

    // Create the state for the connection
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T *)calloc(1, sizeof(TCP_CONNECT_STATE_T));
    if (!con_state) {
        DEBUG_printf("failed to allocate connect state\n");
        return ERR_MEM;
    }
    con_state->pcb = client_pcb; // for checking
    con_state->gw = &state->gw;

    // setup connection to client
    tcp_arg(client_pcb, con_state);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_server_err);

    return ERR_OK;
}

static bool tcp_server_open(void *arg, const char *ap_name) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    DEBUG_printf("starting server on port %d\n", TCP_PORT);

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        DEBUG_printf("failed to create pcb\n");
        return false;
    }

    err_t err = tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT);
    if (err) {
        DEBUG_printf("failed to bind to port %d\n",TCP_PORT);
        return false;
    }

    state->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!state->server_pcb) {
        DEBUG_printf("failed to listen\n");
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }

    tcp_arg(state->server_pcb, state);
    tcp_accept(state->server_pcb, tcp_server_accept);

    printf("Try connecting to '%s' (press 'd' to disable access point)\n", ap_name);
    return true;
}

void key_pressed_func(void *param) {
    assert(param);
    TCP_SERVER_T *state = (TCP_SERVER_T*)param;
    int key = getchar_timeout_us(0); // get any pending key press but don't wait
    if (key == 'd' || key == 'D') {
        cyw43_arch_lwip_begin();
        cyw43_arch_disable_ap_mode();
        cyw43_arch_lwip_end();
        state->complete = true;
    }
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
    TCP_SERVER_T *state = (TCP_SERVER_T*)calloc(1, sizeof(TCP_SERVER_T));
    if (!state) {
        DEBUG_printf("failed to allocate state\n");
        return ;
    }

    if (cyw43_arch_init()) {
        DEBUG_printf("failed to initialise\n");
        return ;
    }

    // Get notified if the user presses a key
    stdio_set_chars_available_callback(key_pressed_func, state);

    const char *ap_name = "picow_test";
#if 1
    const char *password = "password";
#else
    const char *password = NULL;
#endif

    cyw43_arch_enable_ap_mode(ap_name, password, CYW43_AUTH_WPA2_AES_PSK);

    ip4_addr_t mask;
    IP4_ADDR(ip_2_ip4(&state->gw), 192, 168, 4, 1);
    IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 0);

    // Start the dhcp server
    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &state->gw, &mask);

    // Start the dns server
    //dns_server_t dns_server;
    //dns_server_init(&dns_server, &state->gw);

    if (!tcp_server_open(state, ap_name)) {
        DEBUG_printf("failed to open server\n");
        return ;
    }

    state->complete = false;
    while (!state->complete) {
#if PICO_CYW43_ARCH_POLL
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
#else
        //printf("Waiting for connection\n");
        sleep_ms(1000);
#endif
    }
    tcp_server_close(state);
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

