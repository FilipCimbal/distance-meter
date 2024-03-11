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
#include "ModbusMotor.h"
#include "SonarMan.h"


using namespace std;

class Api
{
private:
    ModbusDriver &modbusDriver;
    ModbusMotor &modbusMotor;
    SonarMan &sonarMan;
    

public:

    Api(ModbusDriver &modbusDriver, ModbusMotor &modbusMotor, SonarMan &sonarMan): modbusDriver(modbusDriver), modbusMotor(modbusMotor), sonarMan(sonarMan) {}
    ~Api() {}

    ResponseApi requestResolver(std::string endpoint, json args = nullptr);


};


#endif