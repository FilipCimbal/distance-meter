#ifndef _MODBUS_MOTOR
#define _MODBUS_MOTOR

#include <exception>
#include <unordered_map>
#include <string>
#include "esp_system.h"
#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <vector>
#include "ModbusDriver.h"


using namespace std;



class ModbusMotor
{

private:
    uint16_t _flow;
    uint8_t _address;
    ModbusDriver &_modbusDriver;


public:
    ModbusMotor(uint8_t address, ModbusDriver &modbusDriver): _address(address), _modbusDriver(modbusDriver) {}
    ~ModbusMotor() {}

    bool init();
    void flowRequest(float flow);
    void speedRequest(float speed);
    uint16_t flowGet();
    void heartBeat();
    
};

#endif