#pragma once

#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_heap_caps.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "parseCMD.h"

#include "Font.h"
#include "main.h"

#define LCD_HOST SPI2_HOST

#define LCD_PIXEL_CLOCK_HZ      (10 * 1000 * 1000)
#define LCD_BK_LIGHT_ON_LEVEL   1
#define LCD_BK_LIGHT_OFF_LEVEL  !LCD_BK_LIGHT_ON_LEVEL
#define LCD_PIN_NUM_MOSI        11  /*!< for 1-line SPI, this also refered as MOSI */
#define LCD_PIN_NUM_CLK         12
#define LCD_PIN_NUM_CS          10
#define LCD_PIN_NUM_DC          9
#define LCD_PIN_NUM_RST         6
#define LCD_PIN_NUM_BK_LIGHT    7

#define LCD_WIDTH               240
#define LCD_HEIGHT              280

#define LCD_CMD_BITS            8
#define LCD_PARAM_BITS          8

#define TOTAL_FONT_HEIGHT                     20

#define X_START_POS_IN_ALIGN_CENTER           10

#define GAP_UNICODE_FONTFILE                  32
#define X_SPACE_TEXT_PIXEL                    1
#define Y_SPACE_TEXT_PIXEL                    5
#define CONSONANTS_VOWELS_INDEX_IN_FONT       95
#define COMPLETE_TYPE_TEXT_INDEX_IN_FONT      189

#define CONSONANTS_VOWELS_INDEX_IN_UNICODE    12593
#define COMPLETE_TYPE_TEXT_INDEX_IN_UNICODE   44032

#define CALI_VALUE_OF_CV_INDEX                (CONSONANTS_VOWELS_INDEX_IN_UNICODE  - CONSONANTS_VOWELS_INDEX_IN_FONT)
#define CALI_VALUE_OF_CT_INDEX                (COMPLETE_TYPE_TEXT_INDEX_IN_UNICODE - COMPLETE_TYPE_TEXT_INDEX_IN_FONT)

#define DEFAULT_ALIGN                         0
#define CENTER_ALIGN                          1

#define KOREAN_WIDTH                          20
#define KOREAN_HEIGHT                         TOTAL_FONT_HEIGHT
#define KOREAN                                true    
#define NON_KOREAN                            false

#define BACKGROUND_COLOR                      0x0000

#define XY_SWAP    true
#define NO_XY_SWAP false

#define STATUS_SCREEN_TEXT "Love"

#define TOP_TEXT_Y_POS 245

#define TIME_NUMBER_WIDTH                     50
#define TIME_NUMBER_HEIGHT                    60
#define TIME_COLONS_WIDTH                     10

#define TIME_COLONS_INDEX                     10

#define TIME_NUMBER_SPACE_WIDTH               ((TIME_NUMBER_WIDTH)*4 + (TIME_COLONS_WIDTH))
#define TIME_NUMBER_X_POS                     (LCD_WIDTH - TIME_NUMBER_SPACE_WIDTH)/2 
#define TIME_NUMBER_Y_POS                     145

#define DAY_NUMBER_HEIGHT                     TOTAL_FONT_HEIGHT
#define DAY_NUMBER_Y_POS                      60

#define OFFSET_DISPLAY_H                      5

#define WEATHER_TEXT_Y_POS                    DAY_NUMBER_Y_POS + DAY_NUMBER_HEIGHT + 5

#define NOTI_MSG_TITLE_Y_POS                  60
#define NOTI_MSG_CONTENT_Y_POS                120

#define TITLE_NUMBER_COLOR                    0xF800
#define TIME_NUMBER_COLOR                     0x1FE4
#define DAY_NUMBER_COLOR                      0x1FE4
#define WEATHER_COLOR                         0x1FE4
#define PARSE_CMD_COLOR                       0x1FE4  

#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define ORANGE  0xFC00

typedef struct 
{
  uint16_t start_X_Pos;
  uint16_t end_X_Pos;
  uint8_t numOfLine;
} startEndPos;

// typedef struct 
// {
//   uint16_t timeColor;
//   uint16_t titleColor;
//   uint16_t dayColor;
//   uint16_t weatherColor;
// } colorList;

extern bool isStatusScreen;
extern bool isCheckedMsg;
extern bool isScreenOn;

extern uint8_t monthNum;
extern uint8_t dayNum;
extern uint8_t hourNum;
extern uint8_t minNum;

void display_init_test();
void display_init(bool xySwap);
void display_gpio_init(void);

void display_turnOn_Off(bool turnOnOff);
void drawStatusScreen(void);

uint16_t* setTextColor(unsigned int textNum, uint16_t color, tFont font);

uint16_t convert_colordata(uint16_t color);

void drawText(bool isKorean, uint8_t align, uint16_t *x, uint16_t *y, char *text, uint16_t startIndex, uint8_t start_X_Pos, uint16_t end_X_Pos, uint16_t color);
void printText(uint16_t x, uint16_t y, char *text, uint8_t align, uint16_t color);
void printTextAlignCenter(uint16_t y, char *text, uint16_t color);

void drawTimeNumber(uint16_t y, uint8_t hour, uint8_t minute, uint16_t color);
void drawDayNumber(uint16_t y, uint8_t month, uint8_t day, uint16_t color);
void setDayTime(char *cmd);
void drawWeather(uint16_t color, bool isNeedClear);
void drawPasskey(uint16_t y, char * passkey, uint16_t color);

void clearALL();
void clearRecTimteDayNum();
void clearRec(uint8_t xPos, uint8_t yPos, uint16_t width, uint16_t height);