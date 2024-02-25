#include "SonarMan.h"
static const char *TAG = "ECHO";

unsigned long IRAM_ATTR micros()
{
    return (unsigned long) (esp_timer_get_time());
}
void IRAM_ATTR delayMicroseconds(uint64_t us)
{
    uint64_t m = micros();
    if(us){
        uint64_t e = (m + us);
        if(m > e){ //overflow
            while(micros() > e){
            }
        }
        while(micros() < e){
        }
    }
}

static void IRAM_ATTR gpio_isr_a_handler(void* arg)
{
    SonarMan* source = (SonarMan*) arg;
    if (gpio_get_level(source->probeAPin))
    {
        source->refATime = esp_timer_get_time();
    }
    else
    {
        source->finishATime = esp_timer_get_time();
    }
}

static void IRAM_ATTR gpio_isr_b_handler(void* arg)
{
    SonarMan* source = (SonarMan*) arg;
    if (gpio_get_level(source->probeBPin))
    {
        source->refBTime = esp_timer_get_time();
    }
    else
    {
        source->finishBTime = esp_timer_get_time();
        //ESP_LOGI(TAG, "set the A fin time %lli", source->finishATime);
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
    gpio_isr_handler_add(probeAPin, gpio_isr_a_handler, (void*) this);
    gpio_isr_handler_add(probeBPin, gpio_isr_b_handler, (void*) this);

    return true;

}

bool SonarMan::measure(int64_t timeout)
{
    finishATime = 0;
    finishBTime = 0;
    refATime = esp_timer_get_time();
    refBTime = refATime;
    
    
    gpio_set_level(drivePin, 1);
    delayMicroseconds(11);
    gpio_set_level(drivePin, 0);

    while ((finishATime == 0) || (finishBTime == 0))
    {
        if ((esp_timer_get_time() - refATime) > timeout)
        {      
            vTaskDelay(100 / portTICK_PERIOD_MS);
            return false;
        }
    }
    return true;
}

SonicMeasurement SonarMan::getLastMeasurement()
{
    SonicMeasurement retval;
    retval.probeA = finishATime - refATime;
    retval.probeB = finishBTime - refBTime;
    retval.delta = retval.probeA - retval.probeB;

    return retval;
}