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
        if (endpoint == "read")
        {
            response.args = json(modbusDriver.getHoldings(args.at("address"), args.at("start"), args.at("count")));
            // response.message = "Reads " + string(args.at("count")) + " registers from " + string(args.at("start"));
        }
        if (endpoint == "write")
        {
            response.args = json(modbusDriver.setHoldings(args.at("address"), args.at("start"), args.at("data")));
            // response.message = "Reads " + string(args.at("count")) + " registers from " + string(args.at("start"));
        }
        if (endpoint == "flow")
        {
            modbusMotor.flowRequest(args.at("data"));
        }
        if (endpoint == "speed")
        {
            modbusMotor.speedRequest(args.at("data"));
        }
        if (endpoint == "measure")
        {
            uint16_t flow = modbusMotor.flowGet();
            float speed = (float)flow / 40.6944f;
            if (sonarMan.measure(50, 100, 100000))
            {
                SonicMeasurement measurement = sonarMan.getLastMeasurement();
                response.args = json{{"flow", flow}, {"echo", measurement.delta}, {"speed", speed}};
            }
            else
            {
                response.args = json{{"flow", flow}, {"echo", nullptr}, {"speed", speed}};
                response.code = ResponseError::ERROR;
            }
            
        }
        if (endpoint == "init")
        {
            modbusMotor.init();
        }
        if (endpoint == "reboot")
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