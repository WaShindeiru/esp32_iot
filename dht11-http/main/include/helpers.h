#include <esp_err.h>
#include "http_rest_types.h"
#include "http_rest_client.h"

esp_err_t http_rest_client_post_mine(char *url, char *body_data, http_rest_recv_buffer_t *http_rest_recv_buffer);
esp_err_t http_rest_client_post_json_mine(char *url, cJSON *body_json, http_rest_recv_json_t *http_rest_recv_json);
