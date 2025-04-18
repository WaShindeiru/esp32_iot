#include <stdio.h>
#include <inttypes.h>
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "event_handlers.h"
#include "http_rest_json_client.h"
#include "cJSON.h"

#include "driver/gpio.h"
#include "sdkconfig.h"

#include "helpers.h"


#define DHT11_PIN 5
#define ZERO_TIMESTAMP 32
 
#define URL "http://192.168.0.145:8080/sensorData"

#define WIFI_SSID "ssid"
#define WIFI_PASSWORD "password"
 
static const char *TAG = "main";
 
typedef struct
{
    int dht11_pin;
    float temperature;
    float humidity;
} dht11_t;
 
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
        
        waited = wait_for_state(*dht11,0,40);

        if(waited == -1)
        {
            ESP_LOGE("DHT11:","Failed at phase 1");
            vTaskDelay(pdMS_TO_TICKS( 20 ));
            continue;
        } 


        waited = wait_for_state(*dht11,1,90);
        if(waited == -1)
        {
            ESP_LOGE("DHT11:","Failed at phase 2");
            vTaskDelay(pdMS_TO_TICKS( 20 ));
            continue;
        } 
        
        waited = wait_for_state(*dht11,0,90);
        if(waited == -1)
        {
            ESP_LOGE("DHT11:","Failed at phase 3");
            vTaskDelay(pdMS_TO_TICKS( 20 ));
            continue;
        } 
        break;
        
    }
    
    if(timeout_counter == connection_timeout) return -1;

    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            zero_duration = wait_for_state(*dht11,1,58);
            one_duration = wait_for_state(*dht11,0,74);
            received_data[i] |= (one_duration > ZERO_TIMESTAMP) << (7 - j);
        }
    }
    
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

void app_main(void)
{
  esp_err_t ret = ESP_OK;
  char *response_body;

  ESP_LOGI(TAG, "Starting app_main...");

  wifi_event_group = xEventGroupCreate();

  ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }

  ESP_ERROR_CHECK(ret);

  ESP_ERROR_CHECK(esp_event_loop_create_default());

  ESP_ERROR_CHECK(esp_netif_init());
  (void)esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));

  wifi_config_t wifi_config = {
      .sta = {
          .ssid = WIFI_SSID,
          .password = WIFI_PASSWORD,
          .threshold.authmode = WIFI_AUTH_WPA2_PSK,
          .pmf_cfg = {
              .capable = true,
              .required = false},
      },
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "Waiting for WiFi connection...");
  (void)xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT | WIFI_GOT_IP_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

  ESP_LOGI(TAG, "WiFi connected");

  ESP_LOGI(TAG, "Starting Main Loop...");

  char json_string[256];
  char buffer[50];

  set_output();

  dht11_t data = {
      .dht11_pin = DHT11_PIN,
      .humidity = 0,
      .temperature = 0
  };

  while (1)
  {
    dht11_read(&data, 1000);
    ESP_LOGI(TAG, "Humidity: %.2f, Temperature: %.2f", data.humidity, data.temperature);

    strcpy(json_string, "{");
    snprintf(buffer, sizeof(buffer), "\"humidity\": %f, ", data.humidity);
    strcat(json_string, buffer);
    snprintf(buffer, sizeof(buffer), "\"temperature\": %f ", data.temperature);
    strcat(json_string, buffer);
    strcat(json_string, "}");

    cJSON *json_temp = cJSON_Parse(json_string);
    ESP_LOGI(TAG, "json: %s", json_string);

    http_rest_recv_json_t response_buffer = {0};

    ESP_LOGI(TAG, "Fetching Data to URL: %s", URL);
    ret = http_rest_client_post_json_mine(URL, json_temp, &response_buffer);
    int status_code = response_buffer.status_code;

    if (ret != ESP_OK)
    {
      ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(ret));
      http_rest_client_cleanup_json(&response_buffer);
    }
    else
    {

      if (status_code != 201)
      {
        ESP_LOGE(TAG, "HTTP POST request failed with status code: %d", status_code);
        http_rest_client_cleanup_json(&response_buffer);
      }
      else
      {
        char *jsonString = cJSON_Print(response_buffer.json);
        ESP_LOGI(TAG, "Response: %s", jsonString);

        free(jsonString);
        http_rest_client_cleanup_json(&response_buffer);
      }
    }
    ESP_LOGI(TAG, "Looping in 5 minutes...");
    vTaskDelay(60000 * 5 / portTICK_PERIOD_MS);
  }
}