#ifndef _SERIAL_COMM
#define _SERIAL_COMM

#include <exception>
#include <unordered_map>
#include <string>
#include "esp_system.h"
#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include <mutex>
#include <vector>

#define delay(ms) (vTaskDelay(ms / portTICK_RATE_MS))

#define SERIAL_CONN_PORT    (UART_NUM_1)
#define SERIAL_CONN_TXD     (GPIO_NUM_19)
#define SERIAL_CONN_RXD     (GPIO_NUM_18)
#define SERIAL_CONN_RTS     (UART_PIN_NO_CHANGE)
#define SERIAL_CONN_CTS     (UART_PIN_NO_CHANGE)

#define BUF_SIZE (512)

using namespace std;



class SerialComm
{

private:

    uint8_t recvBuffer[BUF_SIZE];
    uint16_t recvBufferLen = 0;
    uint16_t recvBufferIndex = 0;
    
    bool pending = false;
    bool responseFinish = false;

    uint64_t timeoutTime = 0;

    std::mutex g_num_mutex;

public:
    SerialComm() {}
    ~SerialComm() {}

    bool start();
    void dataReceiver(uint8_t *data, int len);
    size_t sendPacket(uint8_t *data, size_t len);
    uint16_t blockingRequest(uint8_t *data, size_t len, uint8_t *response);
    
};

#endif