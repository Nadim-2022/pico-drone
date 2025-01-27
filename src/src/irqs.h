//
// Created by iamna on 05/10/2024.
//

#ifndef RP2040_FREERTOS_IRQ_IRQS_H
#define RP2040_FREERTOS_IRQ_IRQS_H
#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "hardware/gpio.h"

#define ROT_SW_PIN 12
#define ROT_A_PIN 10
#define ROT_B_PIN 11
#define BUTTON_0 9
#define BUTTON_1 8
#define BUTTON_2 7
#define BUTTON_DEBOUNCE_DELAY_MS 200 // replace with your actual delay time in milliseconds

class irqs {
public:
    typedef enum {
        BUTTON_PRESS_EVENT,
        ROTARY_ENCODER_CW_EVENT,
        ROTARY_ENCODER_CCW_EVENT,
        BUTTON_PRESS_EVENT_0,
        BUTTON_PRESS_EVENT_1,
        BUTTON_PRESS_EVENT_2,
    } EventType;
    EventType event;
    static QueueHandle_t eventQueue;
    explicit irqs( uint8_t PinA);
    void setupPin(uint8_t pin);
    static void callback(uint gpio, uint32_t events);
};


#endif //RP2040_FREERTOS_IRQ_IRQS_H
