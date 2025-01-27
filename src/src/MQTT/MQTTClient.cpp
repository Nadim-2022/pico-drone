//
// Created by iamna on 18/01/2025.
//

#include "MQTTClient.h"

MQTTClient::MQTTClient(){
};

MQTTClient::~MQTTClient(){
};

int MQTTClient::init() {
    client = mqtt_client_new();
    if (client == NULL) {
        printf("Failed to create MQTT client.\n");
        return 0;
    }
    return 1;
}

void MQTTClient::mqtt_dns_found(const char *name, ip_addr_t &ipaddr) {
    bool ip_resolved = false;
    while (!ip_resolved) {
        if (dns_gethostbyname(name, &ipaddr, NULL, NULL) == ERR_OK) {
            printf("Resolved IP: %s\n", ipaddr_ntoa(&ipaddr));
            ip_resolved = true;
        } else {
            printf("DNS resolution failed or in progress.\n");
        }
        vTaskDelay(2000);
    }
    printf("resolved IP: %s\n", ipaddr_ntoa(&ipaddr));

}

int MQTTClient::connect() {
    int ret = 0;
    while (!ret) {
       ret= init();
    }
    mqtt_dns_found(hostname, resolved_addr);
    tls_config = altcp_tls_create_config_client(cert, sizeof(cert)+1);
    if (tls_config == NULL) {
        printf("Failed to create TLS config.\n");
        return 0;
    }
    assert(tls_config);
    mbedtls_ssl_conf_authmode((mbedtls_ssl_config *)tls_config, MBEDTLS_SSL_VERIFY_REQUIRED);
    ci = {
            .client_id = "banglafin",
            .client_user = "smart_plug",
            .client_pass = "SmartPlug123!",
            .keep_alive = 60,
            .tls_config = tls_config,
            .server_name = hostname
    };
    mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, NULL);
    printf("resolved IP: %s\n", ipaddr_ntoa(&resolved_addr));
    mqtt_client_connect(client, &resolved_addr, 8883, mqtt_connection_cb, 0, &ci);
    return 1;
}
void MQTTClient::mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("MQTT connection accepted.\n");
    } else {
        printf("MQTT connection failed: %d\n", status);
    }
}

void MQTTClient::mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    printf("Incoming data: %.*s\n", len, (char *)data);
}

void MQTTClient::mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    printf("Incoming publish at topic: %s with total length %u\n", topic, tot_len);
}

void MQTTClient::subscribe( const char *topic) {
    mqtt_subscribe(client, topic, 0,mqtt_subscribe_cb, NULL);
}

void MQTTClient::mqtt_subscribe_cb(void *arg, err_t result) {
    if (result == ERR_OK) {
        printf("MQTT subscribe successful.\n");
    } else {
        printf("MQTT subscribe failed.\n");
    }
}

void MQTTClient::publish( const char *topic, const char *data) {
    mqtt_publish(client, topic, data, strlen(data), 0, 0, mqtt_publish_cb, NULL);
}

void MQTTClient::mqtt_publish_cb(void *arg, err_t err) {
    if (err == ERR_OK) {
        printf("MQTT publish successful.\n");
    } else {
        printf("MQTT publish failed.\n");
    }
}

void MQTTClient::disconnect() {
    mqtt_disconnect(client);
    mqtt_client_free(client);
    altcp_tls_free_config(tls_config);
}

bool MQTTClient::is_connected() {
    return mqtt_client_is_connected(client);
}