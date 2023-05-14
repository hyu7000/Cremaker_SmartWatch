#pragma once

#include "display.h"
#include "esp_timer.h"
#include "main.h"
#include "esp_log.h"

extern char weather[20];

void parseCMD(char *cmd, uint16_t color);