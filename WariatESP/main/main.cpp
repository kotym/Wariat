
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "Wariat.hpp"
#include "ESPWariat.hpp"
void InitUart();

static constexpr gpio_num_t kBuiltInLed = GPIO_NUM_2;


extern "C" void app_main(void)
{
    printf("start0\n");
    InitUart();

    gpio_reset_pin(kBuiltInLed);
    gpio_set_direction(kBuiltInLed, GPIO_MODE_OUTPUT);
    
    WariatESP wariat;
    bool ledState = false;
    while(true)
    {
        WariatCommon::Payload::BlinkToggle blink;
        printf("loop\n");
        wariat.SendData(blink);

        ledState = !ledState;
        gpio_set_level(kBuiltInLed, ledState);
        
        vTaskDelay(pdMS_TO_TICKS(300));
        
        

    }
}
