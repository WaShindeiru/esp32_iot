#include "helpers.h"

static const char *TAG = "http_rest_json_client";

static char *certificate;

esp_err_t http_rest_client_post_mine(char *url, char *body_data, http_rest_recv_buffer_t *http_rest_recv_buffer)
{
  esp_err_t ret = ESP_OK;

  esp_http_client_handle_t client;

  memset(http_rest_recv_buffer, 0, sizeof(http_rest_recv_buffer_t));

  ESP_LOGD(TAG, "Initializing client");

  esp_http_client_config_t config = {
      .url = url,
      .method = HTTP_METHOD_POST,
      .event_handler = http_event_handler,
#ifdef CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
      .crt_bundle_attach = esp_crt_bundle_attach,
#endif
      .user_agent = CONFIG_HTTP_REST_CLIENT_USER_AGENT,
      .user_data = http_rest_recv_buffer,
  };

  if (certificate != NULL)
  {
    config.cert_pem = certificate;
  }

  client = esp_http_client_init(&config);

  esp_http_client_set_header(client, "Content-Type", "application/json");
  esp_http_client_set_header(client, "Authorization", "Bearer 43O7DCZJHPS66NVVRKXRJ3SCDY");

  esp_http_client_set_post_field(client, body_data, strlen(body_data));

  ret = esp_http_client_perform(client);

  ESP_LOGD(TAG, "Get request complete");

  if (ESP_OK != ret)
  {
    ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(ret));
    esp_http_client_cleanup(client);
    return ret;
  }

  int status_code = esp_http_client_get_status_code(client);

  http_rest_recv_buffer->status_code = status_code;

  ESP_LOGD(TAG, "Cleaning up client before returning");
  esp_http_client_cleanup(client);

  return ret;
}

esp_err_t http_rest_client_post_json_mine(char *url, cJSON *body_json, http_rest_recv_json_t *http_rest_recv_json)
{
  esp_err_t ret = ESP_OK;

  char *body_data = cJSON_Print(body_json);

  http_rest_recv_buffer_t http_rest_recv_buffer;

  ret = http_rest_client_post_mine(url, body_data, &http_rest_recv_buffer);

  free(body_data);

  if (ESP_OK != ret)
  {
    ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(ret));
    return ret;
  }

  if (http_rest_recv_buffer.status_code >= 300)
  {
    ESP_LOGE(TAG, "HTTP POST request failed with status code %d", http_rest_recv_buffer.status_code);
    return ESP_FAIL;
  }

  ESP_LOGD(TAG, "Parsing JSON");

  cJSON *json = cJSON_Parse((char *)http_rest_recv_buffer.buffer);

  if (json == NULL)
  {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL)
    {
      ESP_LOGE(TAG, "Error before: %s", error_ptr);
    }
    ret = ESP_FAIL;
    goto cleanup;
  }

  http_rest_recv_json->json = json;
  http_rest_recv_json->status_code = http_rest_recv_buffer.status_code;

  ESP_LOGD(TAG, "JSON parsed");

  goto cleanup;

cleanup:
  http_rest_client_cleanup(&http_rest_recv_buffer);
  return ret;
}