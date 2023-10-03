#include "Api.h"
#include "esp_log.h"
#include "main.h"
#include "esp_system.h"

static const char *TAG = "API";

ResponseApi Api::requestResolver(std::string endpoint, json args)
{
    ResponseApi response;
    try
    {
        if (endpoint == "read")
        {
            //response.args = json(phaseData);
        }
        if (endpoint == "reboot")
        {
            //esp_restart();
        }
    }
    catch (const std::exception &e)
    {
        ESP_LOGI(TAG, "API ERROR: %s ",e.what());
    }

    return response;
}