#pragma once

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "time.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "soc/adc_channel.h"
#include "hal/adc_types.h"

// #include "decode_image.h"
#include "display.h"
#include "ble.h"

#define UP_BTN 13
#define DOWN_BTN 14

extern TimerHandle_t xAutoReloadTimer, xOneShotTimer;

void vib_motor(uint16_t time_ms);