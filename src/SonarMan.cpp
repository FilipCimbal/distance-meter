#include "SonarMan.h"
#include <vector>
#include <numeric>
#include <iostream>

static const char *TAG = "ECHO";

int64_t average(std::vector<int64_t> const &v)
{
    if (v.empty())
    {
        return 0;
    }
    return accumulate(v.begin(), v.end(), 0.0) / v.size();
}

unsigned long IRAM_ATTR micros()
{
    return (unsigned long)(esp_timer_get_time());
}
void IRAM_ATTR delayMicroseconds(uint64_t us)
{
    uint64_t m = micros();
    if (us)
    {
        uint64_t e = (m + us);
        if (m > e)
        { // overflow
            while (micros() > e)
            {
            }
        }
        while (micros() < e)
        {
        }
    }
}

static void IRAM_ATTR gpio_isr_a_handler(void *arg)
{
    SonarMan *source = (SonarMan *)arg;
    if ((source->refATime == 0) && (gpio_get_level(source->probeAPin)))
    {
        source->refATime = esp_timer_get_time();
    }
    if ((source->refATime > 0) && (!gpio_get_level(source->probeAPin)))
    {
        source->finishATime = esp_timer_get_time();
    }
}

static void IRAM_ATTR gpio_isr_b_handler(void *arg)
{
    SonarMan *source = (SonarMan *)arg;
    if ((source->refBTime == 0) && (gpio_get_level(source->probeBPin)))
    {
        source->refBTime = esp_timer_get_time();
    }
    if ((source->refBTime > 0) && (!gpio_get_level(source->probeBPin)))
    {
        source->finishBTime = esp_timer_get_time();
    }
}

bool SonarMan::init()
{
    gpio_set_pull_mode(drivePin, GPIO_PULLDOWN_ONLY);
    gpio_set_intr_type(drivePin, GPIO_INTR_DISABLE);
    gpio_set_direction(drivePin, GPIO_MODE_OUTPUT);
    gpio_set_level(drivePin, 0);

    gpio_set_pull_mode(probeAPin, GPIO_PULLDOWN_ONLY);
    gpio_set_intr_type(probeAPin, GPIO_INTR_ANYEDGE);
    gpio_set_direction(probeAPin, GPIO_MODE_INPUT);

    gpio_set_pull_mode(probeBPin, GPIO_PULLDOWN_ONLY);
    gpio_set_intr_type(probeBPin, GPIO_INTR_ANYEDGE);
    gpio_set_direction(probeBPin, GPIO_MODE_INPUT);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(probeAPin, gpio_isr_a_handler, (void *)this);
    gpio_isr_handler_add(probeBPin, gpio_isr_b_handler, (void *)this);

    return true;
}

bool SonarMan::measure(uint8_t attempts, int64_t periodMs, int64_t timeout)
{
    std::vector<int64_t> avgs;
    for (uint8_t i = 0; i < attempts; i++)
    {
        if (measure(timeout))
        {
            if (measurement.probeA < 50)
                continue;
            if (measurement.probeB < 50)
                continue;
            if (measurement.delta > 4500)
                continue;
            avgs.push_back(measurement.delta);
        }
        vTaskDelay(periodMs / portTICK_PERIOD_MS);
    }
    int64_t avg = average(avgs);
    int64_t avgCnt = 0;
    int64_t deltaSum = 0;

    //ESP_LOGI(TAG, "Measurement att %i - AVG %lli", attempts, avg);

    for (auto &val : avgs)
    {
        if (val < avg)
        {
            deltaSum += val;
            avgCnt++;
        }
    }

    if (avgCnt == 0)
        return false;
    measurement.delta = (deltaSum / avgCnt);

    return true;
}

bool SonarMan::measure(int64_t timeout)
{
    finishATime = 0;
    finishBTime = 0;
    refTime = esp_timer_get_time();
    refBTime = 0;
    refATime = 0;

    gpio_set_level(drivePin, 1);
    delayMicroseconds(11);
    gpio_set_level(drivePin, 0);

    while ((finishATime == 0) || (finishBTime == 0))
    {
        if ((esp_timer_get_time() - refTime) > timeout)
        {            
            getLastMeasurement();
            return false;
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    getLastMeasurement();
    return true;
}

SonicMeasurement SonarMan::getLastMeasurement()
{
    //ESP_LOGI(TAG, "Measurement last %s", json(measurement).dump().c_str());
    measurement.probeA = finishATime - refATime;
    measurement.probeB = finishBTime - refBTime;
    measurement.delta = measurement.probeA - measurement.probeB;

    return measurement;
}