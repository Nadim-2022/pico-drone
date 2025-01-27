#include "src/irqs.h"

irqs::irqs( uint8_t PinA) { //constructor
    setupPin(PinA);
}
QueueHandle_t irqs::eventQueue = xQueueCreate(10, sizeof(irqs::EventType));

void irqs::setupPin(uint8_t pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
    gpio_set_irq_enabled_with_callback(pin, GPIO_IRQ_EDGE_RISE, true,
                                       callback);
}

void irqs::callback(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    EventType event;
    static uint32_t lastButtonPressTime = 0;
    if (gpio == ROT_SW_PIN ) {
        uint32_t currentTime = to_ms_since_boot(get_absolute_time());
        if ((currentTime - lastButtonPressTime) > BUTTON_DEBOUNCE_DELAY_MS) {
            event = BUTTON_PRESS_EVENT;
            xQueueSendFromISR(eventQueue, &event, &xHigherPriorityTaskWoken);
            lastButtonPressTime = currentTime;
        }
    }
    if(gpio == BUTTON_0){
        uint32_t currentTime = to_ms_since_boot(get_absolute_time());
        if ((currentTime - lastButtonPressTime) > BUTTON_DEBOUNCE_DELAY_MS) {
            event = BUTTON_PRESS_EVENT_0;
            xQueueSendFromISR(eventQueue, &event, &xHigherPriorityTaskWoken);
            lastButtonPressTime = currentTime;
        }
    }
    if(gpio == BUTTON_1){
        uint32_t currentTime = to_ms_since_boot(get_absolute_time());
        if ((currentTime - lastButtonPressTime) > BUTTON_DEBOUNCE_DELAY_MS) {
            event = BUTTON_PRESS_EVENT_1;
            xQueueSendFromISR(eventQueue, &event, &xHigherPriorityTaskWoken);
            lastButtonPressTime = currentTime;
        }
    }
    if(gpio == BUTTON_2){
        uint32_t currentTime = to_ms_since_boot(get_absolute_time());
        if ((currentTime - lastButtonPressTime) > BUTTON_DEBOUNCE_DELAY_MS) {
            event = BUTTON_PRESS_EVENT_2;
            xQueueSendFromISR(eventQueue, &event, &xHigherPriorityTaskWoken);
            lastButtonPressTime = currentTime;
        }
    }

    if (gpio == ROT_A_PIN || gpio == ROT_B_PIN) {
        bool rotA = gpio_get(ROT_A_PIN);
        bool rotB = gpio_get(ROT_B_PIN);

        if (rotA != rotB) {
            event = ROTARY_ENCODER_CW_EVENT;
        } else {
            event = ROTARY_ENCODER_CCW_EVENT;
        }

        xQueueSendFromISR(eventQueue, &event, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
