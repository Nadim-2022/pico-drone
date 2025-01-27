#include "SharedResources.h"

SharedResources::SharedResources() {
    co2 = 0;
    tem = 0;
    RH = 0;
    fanSpeed = 0;
    co2SP = 0;
    xbuttonQueue = xQueueCreate(10, sizeof(EventType));
    eventGroup = xEventGroupCreate();
    fanControllerEvent = xEventGroupCreate();
    pressure = 0;
    // mutex = xSemaphoreCreateMutex();
    co2_mutex = xSemaphoreCreateMutex();
    tem_mutex = xSemaphoreCreateMutex();
    RH_mutex = xSemaphoreCreateMutex();
    pressure_mutex = xSemaphoreCreateMutex();
    fanSpeed_mutex = xSemaphoreCreateMutex();
    co2SP_mutex = xSemaphoreCreateMutex();
    ssid_mutex = xSemaphoreCreateMutex();
    password_mutex = xSemaphoreCreateMutex();


}

int SharedResources::getCo2() {
    xSemaphoreTake(co2_mutex, portMAX_DELAY);
    int temp = co2;
    xSemaphoreGive(co2_mutex);
    return temp;
}

void SharedResources::setCo2(int co2) {
    xSemaphoreTake(co2_mutex, portMAX_DELAY);
    SharedResources::co2 = co2;
    xSemaphoreGive(co2_mutex);
}

int SharedResources::getTem() {
    xSemaphoreTake(tem_mutex, portMAX_DELAY);
    int temp = tem;
    xSemaphoreGive(tem_mutex);
    return temp;
}

void  SharedResources::setTem(int tem) {
    xSemaphoreTake(tem_mutex, portMAX_DELAY);
    SharedResources::tem = tem;
    xSemaphoreGive(tem_mutex);
}

int SharedResources::getRH() {
    xSemaphoreTake(RH_mutex, portMAX_DELAY);
    int temp = RH;
    xSemaphoreGive(RH_mutex);
    return temp;
}

void SharedResources::setRH(int RH) {
    xSemaphoreTake(RH_mutex, portMAX_DELAY);
    SharedResources::RH = RH;
    xSemaphoreGive(RH_mutex);
}

int SharedResources::getFanSpeed() {
    xSemaphoreTake(fanSpeed_mutex, portMAX_DELAY);
    int temp = fanSpeed;
    xSemaphoreGive(fanSpeed_mutex);
    return temp;
}

void SharedResources::setFanSpeed(int fanSpeed) {
    xSemaphoreTake(fanSpeed_mutex, portMAX_DELAY);
    SharedResources::fanSpeed = fanSpeed;
    xSemaphoreGive(fanSpeed_mutex);
}

int SharedResources::getCo2SP() {
    xSemaphoreTake(co2SP_mutex, portMAX_DELAY);
    int temp = co2SP;
    xSemaphoreGive(co2SP_mutex);
    return temp;
}

void SharedResources::setCo2SP(int co2SP) {
    xSemaphoreTake(co2SP_mutex, portMAX_DELAY);
    SharedResources::co2SP = co2SP;
    xSemaphoreGive(co2SP_mutex);
}

int SharedResources::getPressure() {
    xSemaphoreTake(pressure_mutex, portMAX_DELAY);
    int temp = pressure;
    xSemaphoreGive(pressure_mutex);
    return temp;
}

void SharedResources::setPressure(int pressure) {
    xSemaphoreTake(pressure_mutex, portMAX_DELAY);
    SharedResources::pressure = pressure;
    xSemaphoreGive(pressure_mutex);
}



void SharedResources::setSSID(char *ssid) {
    xSemaphoreTake( ssid_mutex, portMAX_DELAY);
    ssid2 = ssid;
    xSemaphoreGive(ssid_mutex);
}

char *SharedResources::getSSID() {
    xSemaphoreTake(ssid_mutex, portMAX_DELAY);
    char *temp = ssid2;
    xSemaphoreGive(ssid_mutex);
    return temp;
}

void SharedResources::setPassword(char *password) {

    xSemaphoreTake(password_mutex, portMAX_DELAY);
    password2 = password;
    xSemaphoreGive(password_mutex);

}

char *SharedResources::getPassword() {

    xSemaphoreTake(password_mutex, portMAX_DELAY);
    char *temp = password2;
    xSemaphoreGive(password_mutex);
    return temp;

}
