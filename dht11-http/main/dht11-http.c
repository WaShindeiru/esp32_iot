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

#include "sdkconfig.h"
#include "dht11.h"

#include "helpers.h"
#include "freertos/queue.h"


#define DHT11_PIN 5
#define ZERO_TIMESTAMP 32
 
// #define URL "http://192.168.0.145:8080/sensorData"

#define SERVER_IP "192.168.0.145"
#define WIFI_SSID "ssid"
#define WIFI_PASSWORD "password"

#define URL "http://" SERVER_IP ":8080/sensorData"
 
static const char *TAG = "main";

typedef struct
{
    float temperature;
    float humidity;
} sensor_data;

QueueHandle_t sensorDataQueue;

static void vDhtTask(void *pvParameters) {
    dht11_t data = {
        .dht11_pin = DHT11_PIN,
        .humidity = 0,
        .temperature = 0
    };

    set_output();

    sensor_data data_temp;

    int return_value;
    BaseType_t status;
    
    for (;;) {
        return_value = dht11_read(&data, 1000);
        while (return_value == -1) {
            vTaskDelay(pdMS_TO_TICKS( 1000 ));
            return_value = dht11_read(&data, 1000);
        }

        data_temp.temperature = data.temperature;
        data_temp.humidity = data.humidity;

        ESP_LOGI(TAG, "Humidity: %.2f, Temperature: %.2f", data.humidity, data.temperature);

        ESP_LOGI(TAG, "Waiting to send");
        status = xQueueSendToBack( sensorDataQueue, &data_temp, portMAX_DELAY );

        if (status != pdPASS) {
            ESP_LOGI(TAG, "Failed to send to the sensorDataQueue");
        } else {
            ESP_LOGI(TAG, "Sent to the sensorDataQueue");
        }

        ESP_LOGI(TAG, "Looping in 30 seconds...");
        vTaskDelay(pdMS_TO_TICKS( 30000 ));
    }

};


static void vHttpTask(void *pvParameters) {
    char json_string[256];
    char buffer[50];

    sensor_data data_temp;
    BaseType_t status;
    esp_err_t ret;

    for (;;) {
        ESP_LOGI(TAG, "Waiting to receive");
        status = xQueueReceive(sensorDataQueue, &data_temp, portMAX_DELAY);

        if (status != pdPASS) {
            ESP_LOGI(TAG, "Failed to receive from sensorDataQueue");
        } else {
            ESP_LOGI(TAG, "Received from sensorDataQueue");
        }

        strcpy(json_string, "{");
        snprintf(buffer, sizeof(buffer), "\"humidity\": %f, ", data_temp.humidity);
        strcat(json_string, buffer);
        snprintf(buffer, sizeof(buffer), "\"temperature\": %f ", data_temp.temperature);
        strcat(json_string, buffer);
        strcat(json_string, "}");
    
        cJSON *json_temp = cJSON_Parse(json_string);
        ESP_LOGI(TAG, "json: %s", json_string);
    
        http_rest_recv_json_t response_buffer = {0};
    
        ESP_LOGI(TAG, "Sending Data to URL: %s", URL);
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
    }

};

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

  ESP_LOGI(TAG, "Starting Two tasks");

  sensorDataQueue = xQueueCreate(1, sizeof(sensor_data));

  xTaskCreate( vDhtTask, "Dht Task", 10000, NULL, 2, NULL );
  xTaskCreate( vHttpTask, "Http Task", 10000, NULL, 1, NULL );

}