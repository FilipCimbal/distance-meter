
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

SystemError systemError = SystemError::NONE; // inicializace stavu pro REST odpovedi

SerialComm serialComm;                    // objekt zajistujici komunikaci pomoci UART
ModbusDriver modbusDriver(serialComm);    // zjednodusena implementace ModBus protokolu RTU
ModbusMotor modbusMotor(1, modbusDriver); // zakladni obsluha motoru pomoci komunikace ModBus
SonarMan sonarProbe(33, 34, 35);          // Objekt pro manipuaci se senzory. Podporuje jeden trigger a dva vstupy pro ECHO

Api _api(modbusDriver, modbusMotor, sonarProbe); // jednoduche REST API pro rizeni a snimani

void app_main()
{

    spiffs_init();

    ESP_ERROR_CHECK(nvs_flash_init());
    serialComm.start();
    initialise_wifi(NULL);
    wifi_apply();
    // wifi_apply(deviceConfig); TODO - podpora ulozene konfigurace na SPIFFS

    sonarProbe.init();
    modbusMotor.init();

    uint16_t flow = 0;
    while (1) // hlavni smycka aplikace. TODO - nahradit za FreeRTOS Task
    {
        vTaskDelay(1000 / portTICK_RATE_MS);
        if (sonarProbe.measure(2, 10, 1000))
        {
            ESP_LOGI(TAG, "Measure: %lli", sonarProbe.getLastMeasurement().delta);
        }
        else
        {
            ESP_LOGI(TAG, "Measure problem: %s", json(sonarProbe.getLastMeasurement()).dump().c_str());
        }

        try
        {
            flow = modbusMotor.flowGet();
            ESP_LOGI(TAG, "Flow read: %u", flow);
        }
        catch (const std::exception &e)
        {
            ESP_LOGI(TAG, "Flow read error %s", e.what());
        }
    }
}
