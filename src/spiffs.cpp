/*
 * spiffs.c
 *
 *  Created on: 13. 4. 2021
 *      Author: cavojsky
 */

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_spiffs.h"
#include <stdlib.h>
#include "esp_log.h"

static const char* SPIFFS_PATH = "";

esp_err_t ret;

static const char *TAG = "SPIFFS";

void spiffs_init() {

	esp_vfs_spiffs_conf_t conf = { .base_path = SPIFFS_PATH, .partition_label =
	NULL, .max_files = 5, .format_if_mount_failed = 0 };

	// Use settings defined above to initialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is an all-in-one convenience function.
	ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG,"Failed to mount or format filesystem");
		} else if (ret == ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG,"Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(TAG,"Failed to initialize SPIFFS %s",
					esp_err_to_name(ret));
		}
		return;
	} else {
		ESP_LOGI(TAG,"SPIFFS filesystem is mounted successfully");
	}

}