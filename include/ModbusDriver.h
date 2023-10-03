#ifndef MODBUS_DRIVER
#define MODBUS_DRIVER

#include "dtypes.h"

#include <stdlib.h>
#include "esp_log.h"
#include <exception>
#include <unordered_map>
#include <string>
#include "esp_log.h"
#include "SerialComm.h"

using namespace std;

struct MBResponse
{
    uint8_t addr = 0;
    unordered_map<uint32_t, uint32_t> holding;
    unordered_map<uint32_t, uint32_t> input;
    unordered_map<uint32_t, bool> coil;
    unordered_map<uint32_t, bool> discrete;
};

struct MBRegister
{
    uint32_t index;
    uint32_t value;

    MBRegister(uint32_t index, uint32_t value) : index(index), value(value) {}
};

enum class MBFunction
{
    READ_DO = 0x01,
    READ_DI = 0x02,
    READ_AO = 0x03,
    READ_AI = 0x04,
    WRITE_DO = 0x05,
    WRITE_AO = 0x06,
    WRITES_DO = 0x0F,
    WRITES_AO = 0x10    
};

class ModbusDriver
{
private:
    SerialComm &serialComm;

public:
    ModbusDriver(SerialComm &serialComm) : serialComm(serialComm) {}
    ~ModbusDriver() {}

    uint16_t crcCalc(const uint8_t *data, int len);
    void crcAdd(uint8_t *data, int len);
    bool crcCheck(uint8_t *data, int len);
    bool setHolding(uint8_t addr, uint16_t register, uint16_t data);
    bool setHoldings(uint8_t addr, uint16_t start, vector<uint16_t> data);
    unordered_map<uint32_t, uint32_t> getHoldings(uint8_t addr, uint16_t start, uint8_t count);
    unordered_map<uint32_t, uint32_t> getInputs(uint8_t addr, uint16_t start, uint8_t count);
    bool changeAddress(uint8_t from, uint8_t to);
};

#endif