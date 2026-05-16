
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "Wariat.hpp"
#include "ESPWariat.hpp"
void InitUart();

static constexpr gpio_num_t kBuiltInLed = GPIO_NUM_2;

WariatESP wariat;

void WariatUpdate(void* data)
{
    while(true)
    {
        wariat.Update();
        vTaskDelay(pdMS_TO_TICKS(16));
    }
}

extern "C" void app_main(void)
{
    printf("start0\n");
    InitUart();

    gpio_reset_pin(kBuiltInLed);
    gpio_set_direction(kBuiltInLed, GPIO_MODE_OUTPUT);

    xTaskCreate(WariatUpdate, "WariatUpdate", 4080, nullptr, 1, nullptr);

    bool ledState = false;
    while(true)
    {
        //WariatCommon::Payload::BlinkToggle blink;
        printf("  .  ");
        //wariat.SendData(blink);

        ledState = !ledState;
        gpio_set_level(kBuiltInLed, ledState);
        
        vTaskDelay(pdMS_TO_TICKS(5000));
        
        

    }
}
