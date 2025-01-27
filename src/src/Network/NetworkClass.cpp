#include <string>
#include "NetworkClass.h"



NetworkClass::NetworkClass(std::shared_ptr<SharedResources> sharedResources) : resources(sharedResources) {

}
bool NetworkClass::is_credentials_set = false;
char NetworkClass::ssid[32] = {0};
char NetworkClass::password[64] = {0};

void NetworkClass::init() {
    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return;
    }
}
void NetworkClass::deinit() {
    cyw43_arch_disable_sta_mode();
    cyw43_arch_deinit();
}

void NetworkClass::setCredentials(char* SSID, char* PASSWORD) {
    strcpy(ssid, SSID);
    strcpy(password, PASSWORD);
    is_credentials_set = true;
}

int NetworkClass::connect() {
    //init();
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect\n");
        return 0;
    }
    return 1;
}

void NetworkClass::ap_init() {
    init();
    cyw43_arch_enable_ap_mode(ap_name, ap_pass, CYW43_AUTH_WPA2_AES_PSK);
    IP4_ADDR(&gw, 192, 168, 4, 1);
    IP4_ADDR(&mask, 255, 255, 255, 0);
    dhcp_server_init(&dhcp_server, &gw, &mask);
    httpd_init();
    httpd_cgi_init();
}

void NetworkClass::httpd_cgi_init() {
    http_set_cgi_handlers(cgi_handlers, sizeof(cgi_handlers) / sizeof(cgi_handlers[0]));
}

const char *NetworkClass::cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
    char gssid[32] = {0};
    char gpassword[64] = {0};

    for (int i = 0; i < iNumParams; i++) {
        if (strcmp(pcParam[i], "ssid") == 0) {
            strncpy(gssid, pcValue[i], sizeof(gssid) - 1);
        } else if (strcmp(pcParam[i], "password") == 0) {
            strncpy(gpassword, pcValue[i], sizeof(gpassword) - 1);
        }
    }
    if (gssid[0] != '\0' && gpassword[0] != '\0') {
        printf("Received SSID: %s, Password: %s\n", gssid, gpassword);
        // Set the SSID and password in the shared resources
        setCredentials(gssid, gpassword);
        return "/confirm.html";
    }

    return "/index.html";
}

bool NetworkClass::ap_disconnect() {
    cyw43_arch_disable_ap_mode();
    return true;
}


bool NetworkClass::areCredentialsSet() {
    return is_credentials_set;
}
