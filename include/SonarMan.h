#ifndef SONAR_MAN_
#define SONAR_MAN_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::basic_json<std::map, std::vector, std::string, bool, std::int64_t, std::uint64_t, float>;


struct SonicMeasurement 
{
    int64_t probeA = 0;
    int64_t probeB = 0;
    int64_t delta = 0;
};

inline void to_json(json &j, const SonicMeasurement &tVar)
{
    j = json{
        {"probeA", tVar.probeA},
        {"probeB", tVar.probeB},
        {"delta", tVar.delta},
    };
}

inline void from_json(const json &j, SonicMeasurement &tVar)
{
    if (j.contains("probeA"))
    {
        j.at("probeA").get_to(tVar.probeA);
    }
    if (j.contains("probeB"))
    {
        j.at("probeB").get_to(tVar.probeB);
    }
    if (j.contains("delta"))
    {
        j.at("delta").get_to(tVar.delta);
    }
}

class SonarMan
{

public:

    gpio_num_t drivePin; 
    gpio_num_t probeAPin;
    gpio_num_t probeBPin;
    int64_t refTime = 0;

    int64_t refATime = 0;
    int64_t finishATime = 0;

    int64_t refBTime = 0;
    int64_t finishBTime = 0;   

    SonicMeasurement measurement; 

public:
    SonarMan(gpio_num_t _drivePin, gpio_num_t _probeAPin, gpio_num_t _probeBPin): drivePin(_drivePin), probeAPin(_probeAPin), probeBPin(_probeBPin) {}
    SonarMan(uint32_t _drivePin, uint32_t _probeAPin, uint32_t _probeBPin): drivePin((gpio_num_t)_drivePin), probeAPin((gpio_num_t)_probeAPin), probeBPin((gpio_num_t)_probeBPin) {}
    ~SonarMan() {}

    bool init();
    bool measure(int64_t timeout = 1000000);
    bool measure(uint8_t attempts, int64_t periodMs,  int64_t timeout = 1000000);
    SonicMeasurement getLastMeasurement();

    
};


#endif

