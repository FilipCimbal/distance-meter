#include "ModbusMotor.h"

bool ModbusMotor::init()
{
    _modbusDriver.setHoldings(_address, 53250, std::vector<uint16_t>{25968, 19744, 17731});
    _modbusDriver.setHoldings(_address, 53596, std::vector<uint16_t>{1, 0, 200});

    //_modbusDriver.setHoldings(_address, 53529, std::vector<uint16_t>{65000}); // max RPM
    _modbusDriver.setHoldings(_address, 53510, std::vector<uint16_t>{1}); // ridime na flow
    _modbusDriver.setHoldings(_address, 53575, std::vector<uint16_t>{5}); // ridime na flow
    _modbusDriver.setHoldings(_address, 53248, std::vector<uint16_t>{2}); // nevim

    return true;
}



void ModbusMotor::flowRequest(uint16_t flow){
    _flow = flow;
    _modbusDriver.setHoldings(_address, 53249, std::vector<uint16_t>{flow});
}

uint16_t ModbusMotor::flowGet(){    
    unordered_map<uint32_t, uint32_t> result = _modbusDriver.getHoldings(_address, 53299, 1);
    ESP_LOGI("RES", "RESULT: %s", json(result).dump().c_str());
    return 1;//(uint16_t)result.at(53299);
}
void ModbusMotor::heartBeat(){

}
