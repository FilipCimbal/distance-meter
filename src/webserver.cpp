#include "webserver.h"
#include "esp_log.h"
#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "main.h"

#include "lwip/err.h"
#include "lwip/sys.h"


static const char *TAG = "APP";

httpd_handle_t server = NULL;
static EventGroupHandle_t s_wifi_event_group;

vector<string> split(const string &s, char delim)
{
    vector<string> result;
    stringstream ss(s);
    string item;

    while (getline(ss, item, delim))
    {
        result.push_back(item);
    }
    return result;
}

void cors(httpd_req_t *request)
{
    httpd_resp_set_hdr(request, "Connection", "close");
    httpd_resp_set_hdr(request, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(request, "Access-Control-Allow-Headers", "*");
    httpd_resp_set_hdr(request, "Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT, DELETE");
}

bool getIpFromString(ip4_addr_t *retval, std::string &ip)
{
    return (ip4addr_aton(ip.c_str(), retval) > 0);
}

void websocket_send_all_event(std::string event, uint32_t id, json payload)
{

    json packet = {
        {"type", "event"},
        {"event", event},
        {"args", payload}};

    std::string resp = packet.dump();
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)resp.c_str();
    ws_pkt.len = resp.length();
    ws_pkt.fragmented = 0;
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    for (int i = 0; i < CONFIG_LWIP_MAX_SOCKETS; ++i)
    {
        struct sockaddr_in6 addr;
        socklen_t addr_size = sizeof(addr);
        int sock = LWIP_SOCKET_OFFSET + i;
        int res = getpeername(sock, (struct sockaddr *)&addr, &addr_size);
        if (res == 0)
        {
            ESP_LOGI(TAG, "sock: %d -- addr: %x, port: %d", sock, addr.sin6_addr.un.u32_addr[3], addr.sin6_port);
            httpd_ws_send_frame_async(server, sock, &ws_pkt);
        }
    }
}

/* This handler allows the custom error handling functionality to be
 * tested from client side. For that, when a PUT request 0 is sent to
 * URI /ctrl, the /hello and /echo URIs are unregistered and following
 * custom error handler http_404_error_handler() is registered.
 * Afterwards, when /hello or /echo is requested, this custom error
 * handler is invoked which, after sending an error message to client,
 * either closes the underlying socket (when requested URI is /echo)
 * or keeps it open (when requested URI is /hello). This allows the
 * client to infer if the custom error handler is functioning as expected
 * by observing the socket state.
 */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if (strcmp("/hello", req->uri) == 0)
    {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    }
    else if (strcmp("/update", req->uri) == 0)
    {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/update URI is not available");
        /* Return ESP_FAIL to close underlying socket */
        return ESP_FAIL;
    }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

esp_err_t post_handler(httpd_req_t *req)
{
    char buf[1024];
    int ret;

    ESP_LOGI(TAG, "POST REQUEST from  - %s", req->uri);

    if (req->content_len > 1024)
    {
        ESP_LOGE(TAG, "Content is exceeded - %i", req->content_len);
    }
    cors(req);
    ret = httpd_req_recv(req, buf, req->content_len);
    buf[req->content_len] = 0;
    if (ret)
    {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
        {
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Bad POST content");
            return ESP_FAIL;
        }
        httpd_resp_set_type(req, "application/json");
        // httpd_resp_set_hdr(req, "Content-Type", "application/json");
        json args = json();
        try
        {
            args = json::parse(buf);
        }
        catch (const std::exception &e)
        {
            ESP_LOGE(TAG, "ARGS POST ERROR %s", e.what());
        }

        httpd_resp_send(req, json(_api.requestResolver(string(req->uri).substr(1), args)).dump().c_str(), HTTPD_RESP_USE_STRLEN);
    }
    else
    {
        httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    }
    return ESP_OK;
}

esp_err_t get_handler(httpd_req_t *req)
{
    cors(req);
    httpd_resp_set_type(req, "application/json");
    // httpd_resp_set_hdr(req, "Content-Type", "application/json");
    httpd_resp_send(req, json(_api.requestResolver(string(req->uri).substr(1))).dump().c_str(), HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t getUri = {
    .uri = "/*",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL};

httpd_uri_t postUri = {
    .uri = "/*",
    .method = HTTP_POST,
    .handler = post_handler,
    .user_ctx = NULL};

httpd_uri_t optionUri = {
    .uri = "/*",
    .method = HTTP_OPTIONS,
    .handler = post_handler,
    .user_ctx = NULL};


httpd_handle_t start_webserver(void)
{
    server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    config.stack_size = 4096 * 6;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &getUri);
        httpd_register_uri_handler(server, &postUri);
        httpd_register_uri_handler(server, &optionUri);

        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void stop_webserver()
{
    // Stop the httpd server
    httpd_stop(server);
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{

    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
        ESP_ERROR_CHECK(esp_wifi_connect());
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
        // ESP_LOGI(TAG, "Got IP: '%s'",ip4addr_ntoa((ip4_addr_t *)event->event_info.got_ip.ip_info.ip));
        ESP_LOGI(TAG, "my new IP: " IPSTR, IP2STR(&event->event_info.got_ip.ip_info.ip));

        /* Start the web server */
        if (server == NULL)
        {
            server = start_webserver();
        }
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
        ESP_ERROR_CHECK(esp_wifi_connect());

        /* Stop the web server */
        if (server)
        {
            stop_webserver();
            server = NULL;
        }
        break;
    default:
        break;
    }
    return ESP_OK;
}

void wifi_ap_init()
{

    s_wifi_event_group = xEventGroupCreate();
    ESP_LOGI(TAG, "AP WiFi initi");

    tcpip_adapter_init();
    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {};
    wifi_config.ap = {};
    wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.ap.max_connection = 3;

    strcpy((char *)wifi_config.ap.ssid, "DistanceMeter");

    strcpy((char *)wifi_config.ap.password, "atreaatrea");
    tcpip_adapter_ip_info_t ipAddressInfo;
    memset(&ipAddressInfo, 0, sizeof(ipAddressInfo));

    ip4addr_aton("10.0.0.1", &ipAddressInfo.ip);
    ip4addr_aton("255.255.255.0", &ipAddressInfo.netmask);
    ipAddressInfo.gw = ipAddressInfo.ip;
    ESP_LOGI(TAG, "Setting AP WiFi configuration SSID %s...", wifi_config.ap.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
    ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &ipAddressInfo));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));
    ESP_ERROR_CHECK(esp_wifi_start());

    if (server == NULL)
    {
        server = start_webserver();
    }
}

void wifi_sta_init(const char *ssid, const char *pass)
{

    s_wifi_event_group = xEventGroupCreate();
    ESP_LOGI(TAG, "AP WiFi initi");
    tcpip_adapter_init();
    esp_wifi_disconnect();
    esp_wifi_stop();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {};
    wifi_config.sta = {};
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    strcpy((char *)wifi_config.sta.ssid, ssid);
    strcpy((char *)wifi_config.sta.password, pass);

    ESP_LOGI(TAG, "Setting STA WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config((wifi_interface_t)ESP_IF_WIFI_STA, &wifi_config));
    tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_STA);
    ESP_ERROR_CHECK(esp_wifi_start());
}

void initialise_wifi(void *arg)
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, server));
}

void wifi_apply()
{
    if (true)
    {
        wifi_ap_init();
    }
    else
    {
        wifi_sta_init("ssid", "strong_password");
    }
}