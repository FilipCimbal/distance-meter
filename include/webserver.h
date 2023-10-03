#include <stdio.h>
#include <string>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <esp_http_server.h>
#include <nlohmann/json.hpp>

using json = nlohmann::basic_json<std::map, std::vector, std::string, bool, std::int64_t, std::uint64_t, float>;


void websocket_send_all_event(std::string event, uint32_t id, json payload);
esp_err_t hello_get_handler(httpd_req_t *req);
esp_err_t update_post_handler(httpd_req_t *req);
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err);
esp_err_t ctrl_put_handler(httpd_req_t *req);
httpd_handle_t start_webserver(void);
void stop_webserver(httpd_handle_t server);
void initialise_wifi(void *arg);
void wifi_sta_init();
void wifi_ap_init();
void wifi_apply();
