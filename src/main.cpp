
#include <stdio.h>
#include <string>
#include <time.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "webserver.h"
#include "main.h"
#include <nlohmann/json.hpp>
#include "spiffs.h"
#include "ModbusDriver.h"
#include "SerialComm.h"
#include "ModbusMotor.h"
#include "SonarMan.h"

// git test

using json = nlohmann::basic_json<std::map, std::vector, std::string, bool, std::int64_t, std::uint64_t, float>;

#include <esp_http_server.h>

extern "C"
{
    void app_main(void);
}

static const char *TAG = "APP";

SystemError systemError = SystemError::NONE;


uint64_t uptimeStart = 0;
uint64_t uptime = 0;
uint64_t time_span = 1000;
SerialComm serialComm;
ModbusDriver modbusDriver(serialComm);
ModbusMotor modbusMotor(1, modbusDriver);
SonarMan sonarProbeA(33, 34, 35);


Api _api(modbusDriver, modbusMotor);
bool networkConnected = false;




void uptimeInit()
{
    uptimeStart = (uint64_t)time(nullptr);
}

void uptimeSync()
{
    uptime = (uint32_t)((uint64_t)time(nullptr) - uptimeStart);
}


void main_task(void *pvParameters)
{
    int eventId = 0;
    while (1)
    {
        vTaskDelay(1000 / portTICK_RATE_MS);
        //modbusDriver.getPhaseData(phaseData, uptime);
        eventId++;
        // websocket_send_all_event("read", eventId, json(phaseData));
        uptimeSync();
    }
}

void app_main()
{
    bool lastNetworkConnected = !networkConnected;
    uint8_t networkDisconnected = 0;
    spiffs_init();

    ESP_ERROR_CHECK(nvs_flash_init());
    serialComm.start();
    initialise_wifi(NULL);
    wifi_apply();
    uptimeInit();
    //wifi_apply(deviceConfig);
    sonarProbeA.init();



    xTaskCreate(&main_task, "main_task", 16384, NULL, 5, NULL);

    while (1)
    {
        vTaskDelay(20 / portTICK_RATE_MS);
        if (sonarProbeA.measure())
        {
            ESP_LOGI(TAG, "MeasureA: %s", json(sonarProbeA.getLastMeasurement()).dump().c_str());
        }
        else
        {
            ESP_LOGI(TAG, "MeasureA failed");
        }
        if (lastNetworkConnected != networkConnected)
        {
            lastNetworkConnected = networkConnected;
        }
        if (networkConnected)
        {
            networkDisconnected = 0;
        }
        else
        {
            networkDisconnected++;
        }

        if (networkDisconnected == 60)
        {
                //wifi_apply(deviceConfig);
        }
    }

    // curl -H "Content-Type: text/xml" --data-binary @firmware.bin http://192.168.123.14/update
}
