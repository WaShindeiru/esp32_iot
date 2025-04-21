#include "dht11.h"
#include "driver/gpio.h"
#include <stdio.h>
#include <string.h>
#include <rom/ets_sys.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DHT11_PIN 5
#define ZERO_TIMESTAMP 32

int wait_for_state(dht11_t dht11,int state,int timeout)
{
    gpio_set_direction(dht11.dht11_pin, GPIO_MODE_INPUT);
    int count = 0;
    
    while(gpio_get_level(dht11.dht11_pin) != state)
    {
        if(count >= timeout) return -1;
        count += 2;
        esp_rom_delay_us(2);
        
    }

    return  count;
}

void hold_low(dht11_t dht11,int hold_time_us)
{
    gpio_set_direction(dht11.dht11_pin,GPIO_MODE_OUTPUT);
    gpio_set_level(dht11.dht11_pin,0);
    esp_rom_delay_us(hold_time_us);
    gpio_set_level(dht11.dht11_pin,1);
}

void hold_low_ms(dht11_t dht11,int hold_time_ms)
{
    gpio_set_direction(dht11.dht11_pin,GPIO_MODE_OUTPUT);
    gpio_set_level(dht11.dht11_pin,0);
    vTaskDelay(pdMS_TO_TICKS(hold_time_ms));
    gpio_set_level(dht11.dht11_pin,1);
}

void set_input(void) {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << DHT11_PIN),
        .pull_down_en = 0,
        .pull_up_en = 1
    };

    gpio_config(&io_conf);
}

void set_output(void) {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << DHT11_PIN),
        .pull_down_en = 0,
        .pull_up_en = 1
    };

    gpio_config(&io_conf);
}

int dht11_read(dht11_t *dht11,int connection_timeout)
{
    int waited = 0;
    int one_duration = 0;
    int zero_duration = 0;
    int timeout_counter = 0;

    uint8_t received_data[5] =
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00
    };

    while(timeout_counter < connection_timeout)
    {
        timeout_counter++;
        
        hold_low_ms(*dht11, 18);
        
        vTaskSuspendAll();

        waited = wait_for_state(*dht11,0,40);

        if(waited == -1)
        {
            xTaskResumeAll();

            ESP_LOGE("DHT11:","Failed at phase 1");
            vTaskDelay(pdMS_TO_TICKS( 20 ));
            continue;
        } 


        waited = wait_for_state(*dht11,1,90);
        if(waited == -1)
        {
            xTaskResumeAll();

            ESP_LOGE("DHT11:","Failed at phase 2");
            vTaskDelay(pdMS_TO_TICKS( 20 ));
            continue;
        } 
        
        waited = wait_for_state(*dht11,0,90);
        if(waited == -1)
        {
            xTaskResumeAll();
             
            ESP_LOGE("DHT11:","Failed at phase 3");
            vTaskDelay(pdMS_TO_TICKS( 20 ));
            continue;
        }

        xTaskResumeAll();
        break;
    }
    
    if(timeout_counter == connection_timeout) return -1;

    vTaskSuspendAll();
    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            zero_duration = wait_for_state(*dht11,1,58);
            one_duration = wait_for_state(*dht11,0,74);
            received_data[i] |= (one_duration > ZERO_TIMESTAMP) << (7 - j);
        }
    }
    xTaskResumeAll();
    
    int crc = received_data[0]+received_data[1]+received_data[2]+received_data[3];
    crc = crc & 0xff;

    if(crc == received_data[4]) {
        dht11->humidity = received_data[0] + received_data[1] / 10.0;
        dht11->temperature = received_data[2] + received_data[3] / 10.0;
      return 0;
    }
    else {
        ESP_LOGE("DHT11:", "Wrong checksum");
        return -1;
    }
    return 0;
}