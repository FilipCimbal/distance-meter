#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"
#include "SerialComm.h"

static const char *TAG = "SERIAL_COMM";

void SerialComm::dataReceiver(uint8_t *data, int len)
{
	ESP_LOG_BUFFER_HEXDUMP(TAG, data, len, ESP_LOG_INFO);
	if (!responseFinish)
	{
		memset(recvBuffer, 0, sizeof(recvBuffer));
		recvBufferLen = len;
		memcpy(recvBuffer, data, len);
		responseFinish = true;
	}
}

void serial_listener(void *pvParameters)
{
	SerialComm *params = (SerialComm *)pvParameters;
	uint8_t *recvData = (uint8_t *)malloc(BUF_SIZE);
	uart_config_t uart_config = {.baud_rate = 19200, .data_bits = UART_DATA_8_BITS, .parity = UART_PARITY_EVEN, .stop_bits = UART_STOP_BITS_1, .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
	uart_param_config(SERIAL_CONN_PORT, &uart_config);

	uart_set_pin(SERIAL_CONN_PORT, SERIAL_CONN_TXD, SERIAL_CONN_RXD, SERIAL_CONN_RTS, SERIAL_CONN_CTS);

	uart_driver_install(SERIAL_CONN_PORT, BUF_SIZE * 2, 0, 0, NULL,
						ESP_INTR_FLAG_SHARED);

	while (1)
	{
		memset(recvData, 0, BUF_SIZE);
		int len = uart_read_bytes(SERIAL_CONN_PORT, recvData, BUF_SIZE,
								  30 / portTICK_RATE_MS);

		if (len > 0)
		{
			//ESP_LOG_BUFFER_HEX_LEVEL("RAW_DATA", (const char *)recvData, len, ESP_LOG_INFO);

			params->dataReceiver(recvData, len);
		}
	}
}

bool SerialComm::start()
{
	gpio_set_pull_mode(SERIAL_CONN_485, GPIO_PULLDOWN_ONLY);
	gpio_set_intr_type(SERIAL_CONN_485, GPIO_INTR_DISABLE);
	gpio_set_direction(SERIAL_CONN_485, GPIO_MODE_OUTPUT);
	gpio_set_level(SERIAL_CONN_485, 0);
	xTaskCreate(&serial_listener, "serial_listener", 16384, (void *)this, 1, NULL);
	return true;
}

size_t SerialComm::sendPacket(uint8_t *data, size_t len)
{
	int rlen = 0;
	responseFinish = false;
	pending = false;
	gpio_set_level(SERIAL_CONN_485, 1);
	vTaskDelay(1 / portTICK_RATE_MS);
	ESP_LOG_BUFFER_HEX_LEVEL("SEND", (const char *)data, len, ESP_LOG_DEBUG);
	rlen = uart_write_bytes(SERIAL_CONN_PORT, (const char *)data, len);
	uart_wait_tx_done(SERIAL_CONN_PORT, 100);
	vTaskDelay(1 / portTICK_RATE_MS);
	gpio_set_level(SERIAL_CONN_485, 0);
	return rlen;
}

uint16_t SerialComm::blockingRequest(uint8_t *data, size_t len, uint8_t *response)
{
	g_num_mutex.lock();
	timeoutTime = (uint64_t)time(NULL) + 1;
	pending = false;
	sendPacket(data, len);
	while (timeoutTime > (uint64_t)time(NULL))
	{
		if (responseFinish)
		{
			memcpy(response, recvBuffer, recvBufferLen);
			responseFinish = false;
			g_num_mutex.unlock();
			return recvBufferLen;
		}
	}
	responseFinish = false;
	g_num_mutex.unlock();
	return 0;
}
