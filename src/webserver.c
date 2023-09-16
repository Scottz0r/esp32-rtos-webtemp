#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_chip_info.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <esp_mac.h>
#include <esp_http_server.h>
#include <string.h>

#include "webserver.h"
#include "prj_config.h"
#include "temp_sensor.h"
#include "string_builder.h"
#include "hw_mcp9808.h"

#define LOG_TAG "wbs"

#define WA_TEMPER_BUFF_SIZE 16

static void wifi_init_softap();
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static httpd_handle_t start_webserver();
static esp_err_t home_get_handler(httpd_req_t *req);
static esp_err_t info_get_handler(httpd_req_t *req);
static size_t create_home_page(char* buffer, size_t buffer_size);
static size_t create_info_page(char* buffer, size_t buffer_size);
static void format_temperature(int32_t value, char* buffer);
static const char* chip_model_str(esp_chip_model_t model);

void wbs_init()
{
    // Wifi and HTTP initialization
    wifi_init_softap();
    start_webserver();
}

static void wifi_init_softap()
{
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &wifi_event_handler,
        NULL,
        NULL));

    wifi_config_t wifi_config = {};

    wifi_config.ap.channel = 1;
    wifi_config.ap.max_connection = 4;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    strcpy((char *)wifi_config.ap.ssid, WEBS_AP_SSID);
    strcpy((char *)wifi_config.ap.password, WEBS_AP_PWD);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(LOG_TAG, "wifi_init_softap finished.");   
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t*)event_data;
        ESP_LOGI(LOG_TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t*)event_data;
        ESP_LOGI(LOG_TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

static esp_err_t home_get_handler(httpd_req_t *req)
{
    // TODO: Debugging memory allocations.
    size_t dft_free_size = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    ESP_LOGI(LOG_TAG, "Heap free size: %u", dft_free_size);

    // TODO - Pooled static memory would be much better.
    const size_t buff_size = 2048;
    char* buffer = (char*)heap_caps_malloc(buff_size, MALLOC_CAP_DEFAULT);
    if (buffer == NULL)
    {
        ESP_LOGE(LOG_TAG, "Buffer malloc failed!");
        httpd_resp_set_status(req, "500");
        httpd_resp_send(req, "Internal error", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    size_t home_slen = create_home_page(buffer, buff_size);
    esp_err_t rc = httpd_resp_send(req, buffer, home_slen);

    // Must free memory!
    heap_caps_free(buffer);

    if (rc != ESP_OK)
    {
        ESP_LOGE(LOG_TAG, "Failed to send response!");
    }

    return rc;
}

static esp_err_t info_get_handler(httpd_req_t *req)
{
    // TODO - is this URI what I registered?
    ESP_LOGI(LOG_TAG, "URI: %s", req->uri);

    // TODO - Pooled static memory would be much better.
    const size_t buff_size = 2048;
    char* buffer = (char*)heap_caps_malloc(buff_size, MALLOC_CAP_DEFAULT);
    if (buffer == NULL)
    {
        ESP_LOGE(LOG_TAG, "Buffer malloc failed!");
        httpd_resp_set_status(req, "500");
        httpd_resp_send(req, "Internal error", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }

    size_t home_slen = create_info_page(buffer, buff_size);
    esp_err_t rc = httpd_resp_send(req, buffer, home_slen);

    // Must free memory!
    heap_caps_free(buffer);

    if (rc != ESP_OK)
    {
        ESP_LOGE(LOG_TAG, "Failed to send response!");
    }

    return rc;
}

const httpd_uri_t home =
{
    .uri = "/",
    .method = HTTP_GET,
    .handler = home_get_handler,
    .user_ctx = NULL
};

const httpd_uri_t info =
{
    .uri = "/info",
    .method = HTTP_GET,
    .handler = info_get_handler,
    .user_ctx = NULL
};

static httpd_handle_t start_webserver()
{
    httpd_handle_t server;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    ESP_LOGI(LOG_TAG, "Starting server on port: '%d'", config.server_port);

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &home);
        httpd_register_uri_handler(server, &info);
        return server;
    }

    ESP_LOGE(LOG_TAG, "Error starting server!");
    return (httpd_handle_t)NULL;
}

/**
 * Build the home page, which displays temperature readings.
*/
static size_t create_home_page(char* buffer, size_t buffer_size)
{
    // Get the last temperature read by the sensor.
    int32_t last_temp;
    uint8_t last_err;
    tps_get_last(&last_temp, &last_err);

    char temper_buff[WA_TEMPER_BUFF_SIZE];
    format_temperature(last_temp, temper_buff);

    strbld_t sb;
    strbld_init(&sb, buffer, buffer_size);

    strbld_append(&sb, "<html>");
    strbld_append(&sb, "<head>");
    strbld_append_html(&sb, "Scottz0r RTOS Web Temp", "title");
    strbld_append(&sb, "</head>");
    strbld_append(&sb, "<body>");

    strbld_append(&sb, "<p>Temperature: ");
    strbld_append(&sb, temper_buff);
    strbld_append(&sb, "</p>");

    int32_t hist_array[TPS_HIST_READ_SIZE];
    int hist_count = tps_get_hist_values(hist_array, TPS_HIST_READ_SIZE);

    strbld_append(&sb, "<h3>Most recent values</h3><ul>");

    for (int i = 0; i < hist_count; ++i)
    {
        format_temperature(hist_array[i], temper_buff);
        strbld_append_html(&sb, temper_buff, "li");
    }
    strbld_append(&sb, "</ul>");

    // Links
    strbld_append(&sb, "<p>[<a href=\"/info\">device info</a>]</p>");

    strbld_append(&sb, "</body></html>");

    size_t slen = 0;
    strbld_get(&sb, &slen);

    return slen;
}

/**
 * Format a temperature value. Buffer must be of size WA_TEMPER_BUFF_SIZE
*/
static void format_temperature(int32_t value, char* buffer)
{    
    if (value != TPS_NO_VALUE && value < 999999 && value > -99999)
    {
        int32_t whole = value / 100;
        int32_t frac = value % 100;
        sprintf(buffer, "%ld.%ld", whole, frac);
    }
    else
    {
        strcpy(buffer, "---");
    }
}

static size_t create_info_page(char* ibuffer, size_t buffer_size)
{
    strbld_t sb;
    strbld_init(&sb, ibuffer, buffer_size);

    hw_mcp9808_dinfo info;
    hw_mcp9808_read_device_info(&info);

    char fmt_buff[32];

    strbld_append(&sb, "<html>");
    strbld_append(&sb, "<head>");
    strbld_append_html(&sb, "Scottz0r RTOS Web Temp ~ Info", "title");
    strbld_append(&sb, "</head>");
    strbld_append(&sb, "<body>");

    strbld_append_html(&sb, "Device Info", "h1");

    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    const char* model_str = chip_model_str(chip_info.model);

    strbld_append(&sb, "<p>");
    strbld_append(&sb, "Chip Model: ");
    sprintf(fmt_buff, "%s", model_str);
    strbld_append(&sb, fmt_buff);
    strbld_append(&sb, "</p>");

    strbld_append(&sb, "<p>");
    strbld_append(&sb, "Chip Revision (M.XX): ");
    sprintf(fmt_buff, "%u", chip_info.revision);
    strbld_append(&sb, fmt_buff);
    strbld_append(&sb, "</p>");

    strbld_append(&sb, "<p>");
    strbld_append(&sb, "Cores: ");
    sprintf(fmt_buff, "%u", chip_info.cores);
    strbld_append(&sb, fmt_buff);
    strbld_append(&sb, "</p>");

    strbld_append_html(&sb, "MCP9808 Info", "h2");

    strbld_append(&sb, "<p>");
    strbld_append(&sb, "Device Id: ");
    sprintf(fmt_buff, "%u", info.device_id);
    strbld_append(&sb, fmt_buff);
    strbld_append(&sb, "</p>");

    strbld_append(&sb, "<p>");
    strbld_append(&sb, "Device Revision: ");
    sprintf(fmt_buff, "%u", info.device_revision);
    strbld_append(&sb, fmt_buff);
    strbld_append(&sb, "</p>");

    strbld_append(&sb, "<p>");
    strbld_append(&sb, "Manufacturer Id: ");
    sprintf(fmt_buff, "%u", info.manufacturer_id);
    strbld_append(&sb, fmt_buff);
    strbld_append(&sb, "</p>");

    // Links
    strbld_append(&sb, "<p>[<a href=\"/\">home</a>]</p>");

    strbld_append(&sb, "</body></html>");

    size_t slen = 0;
    strbld_get(&sb, &slen);

    return slen;
}

static const char* chip_model_str(esp_chip_model_t model)
{
    switch(model)
    {
    case CHIP_ESP32:
        return "ESP32";
    case CHIP_ESP32S2:
        return "ESP32S2";
    case CHIP_ESP32S3:
        return "ESP32S3";
    case CHIP_ESP32C3:
        return "ESP32C3";
    case CHIP_ESP32C2:
        return "ESP32C2";
    case CHIP_ESP32C6:
        return "ESP32C6";
    case CHIP_ESP32H2:
        return "ESP32H2";
    case CHIP_POSIX_LINUX:
        return "POSIX_LINUX";
    default:
        return "Unknown";
    }
}
