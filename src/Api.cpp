#include "Api.h"
#include "esp_log.h"
#include "main.h"
#include "esp_system.h"

static const char *TAG = "API";

ResponseApi Api::requestResolver(std::string endpoint, json args)
{
    ESP_LOGI(TAG, "endpoint: %s data: %s", endpoint.c_str(), args.dump().c_str());
    ResponseApi response;
    try
    {
        if (endpoint == "api/read")
        {
            response.args = json(modbusDriver.getHoldings(args.at("address"), args.at("start"), args.at("count")));
            // response.message = "Reads " + string(args.at("count")) + " registers from " + string(args.at("start"));
        }
        if (endpoint == "api/write")
        {
            response.args = json(modbusDriver.setHoldings(args.at("address"), args.at("start"), args.at("data")));
            // response.message = "Reads " + string(args.at("count")) + " registers from " + string(args.at("start"));
        }
        if (endpoint == "api/flow")
        {
            modbusMotor.flowRequest(args.at("data"));
        }
        if (endpoint == "api/speed")
        {
            modbusMotor.speedRequest(args.at("data"));
        }
        if (endpoint == "api/measure")
        {
            try
            {
                uint16_t flow = modbusMotor.flowGet();
                float speed = (float)flow / 40.6944f;
                SonicMeasurement measurement = sonarMan.getLastMeasurement();
                response.args = json{{"flow", flow}, {"echo", measurement.delta}, {"speed", speed}};
            }
            catch (const std::exception &e)
            {
                response.args = json{{"flow", 0}, {"echo", nullptr}, {"speed", 0}};
                response.code = ResponseError::ERROR;
            }
        }
        if (endpoint == "api/init")
        {
            modbusMotor.init();
        }
        if (endpoint == "api/reboot")
        {
            // esp_restart();
        }
    }
    catch (const std::exception &e)
    {
        ESP_LOGI(TAG, "API ERROR: %s ", e.what());
    }

    return response;
}