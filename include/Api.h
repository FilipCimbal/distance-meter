#ifndef D_API
#define D_API

#include "dtypes.h"


#include <stdlib.h>
#include "esp_log.h"
#include <exception>
#include <unordered_map>
#include <string>
#include "esp_log.h"
#include "ModbusDriver.h"


using namespace std;

class Api
{
private:
    ModbusDriver &modbusDriver;

public:

    Api(ModbusDriver &modbusDriver): modbusDriver(modbusDriver) {}
    ~Api() {}

    ResponseApi requestResolver(std::string endpoint, json args = nullptr);


};


#endif