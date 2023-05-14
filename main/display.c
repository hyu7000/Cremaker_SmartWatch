#include "display.h"

static const char *TAG = "display";

startEndPos textStartEndPos;

// extern colorList displayColorList;

char weather2[10] = "00/10";

esp_lcd_panel_handle_t panel_handle;

gpio_config_t bk_gpio_config = {
    .mode = GPIO_MODE_OUTPUT,
    // .pull_down_en = 1,
    .pin_bit_mask = 1ULL << LCD_PIN_NUM_BK_LIGHT
};

spi_bus_config_t buscfg = {
    .sclk_io_num = LCD_PIN_NUM_CLK,
    .mosi_io_num = LCD_PIN_NUM_MOSI,
    .miso_io_num = -1,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = LCD_HEIGHT * LCD_WIDTH * 2 + 8
};

esp_lcd_panel_io_handle_t io_handle = NULL;
esp_lcd_panel_io_spi_config_t io_config = {
    .dc_gpio_num = LCD_PIN_NUM_DC,
    .cs_gpio_num = LCD_PIN_NUM_CS,
    .pclk_hz = LCD_PIXEL_CLOCK_HZ,
    .lcd_cmd_bits = LCD_CMD_BITS,
    .lcd_param_bits = LCD_PARAM_BITS,
    .spi_mode = 0,
    .trans_queue_depth = 10,
};

esp_lcd_panel_dev_config_t panel_config = {
    .reset_gpio_num = LCD_PIN_NUM_RST,
    .rgb_endian = LCD_RGB_ENDIAN_RGB,
    .bits_per_pixel = 16,
};

uint8_t monthNum=0, dayNum=0, hourNum=0, minNum=0;

bool isCheckedMsg = true;

void display_turnOn_Off(bool turnOnOff)
{
  gpio_set_level(LCD_PIN_NUM_BK_LIGHT, turnOnOff);
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, turnOnOff)); 
}

void drawStatusScreen(void)
{
  isStatusScreen = true;

  clearALL();

  printTextAlignCenter(TOP_TEXT_Y_POS, STATUS_SCREEN_TEXT,convert_colordata(TITLE_NUMBER_COLOR));
  drawTimeNumber(TIME_NUMBER_Y_POS,hourNum,minNum,convert_colordata(TIME_NUMBER_COLOR));
  drawDayNumber(DAY_NUMBER_Y_POS,monthNum,dayNum,convert_colordata(DAY_NUMBER_COLOR));
  drawWeather(convert_colordata(WEATHER_COLOR), false);
}

void display_init_test()
{
  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));  
}

void display_init(bool xySwap)
{
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(gpio_set_level(LCD_PIN_NUM_BK_LIGHT, LCD_BK_LIGHT_OFF_LEVEL));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));  

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true)); 

    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, xySwap));

    ESP_ERROR_CHECK(gpio_set_level(LCD_PIN_NUM_BK_LIGHT, LCD_BK_LIGHT_ON_LEVEL));

    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle,true));

    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle,0,OFFSET_DISPLAY_H));

    clearALL();
}

uint16_t* setTextColor(unsigned int textNum, uint16_t color, tFont font)
{  
  uint16_t *displayText;

  uint8_t width = font.chars[textNum].image->width;
  uint8_t width_index = 0;

  uint8_t height = font.chars[textNum].image->height;
  int dataSize = width * height;
  int dataIndex = 0;
  displayText = (uint16_t *)malloc(sizeof(uint16_t)*dataSize);
  
  for(int i = 0; i < 10; i++)
  {
    if(width <= 8*i)
    {
      width = 8*i;
      break;
    }
  }

  int dataSize8 = width * height / 8;

  // ESP_LOGI(TAG, "dataSize = %d, dataSize8 = %d", dataSize, dataSize8);

  for (int i = 0; i < dataSize8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      if (font.chars[textNum].image->data[i] & (0x80 >> j))
      {
        displayText[dataIndex] = color;
      }
      else
      {
        displayText[dataIndex] = BACKGROUND_COLOR;
      }

      dataIndex++;
      width_index++;

      if (font.chars[textNum].image->width <= width_index)
      {
        // ESP_LOGI(TAG, "width_index = %d, dataIndex = %d, testNum = %d, i = %d, j = %d", width_index, dataIndex, testNum, i ,j);
        width_index = 0;        
        break;
      }
    }
  }

  // ESP_LOGI(TAG, "dataIndex = %d, testNum = %d", dataIndex, testNum);

  // for(int i = 0; i <dataSize;i++)
  // {
  //     ESP_LOGI(TAG, "displayText [%d] = %d", i, displayText[i]);
  //     if(!(i%20)) ESP_LOGI(TAG, "last");
  // }

  return displayText;
}

uint16_t convert_colordata(uint16_t color)
{
    uint16_t tempNum, convertedColorData;

    tempNum = color & 0b1111100000000000; //R

    // tempNum = tempNum >> 11;

    tempNum = tempNum >> 8;

    convertedColorData = tempNum;

    // tempNum = color & 0b0000011111000000; //G
    tempNum = color & 0b0000011100000000; 

    tempNum = tempNum << 5;

    convertedColorData = convertedColorData + tempNum;

    tempNum = color & 0b0000000011100000; 

    tempNum = tempNum >> 5;

    convertedColorData = convertedColorData + tempNum;

    // tempNum = color & 0b0000000000111111; //B
    tempNum = color & 0b0000000000011111; //B

    tempNum = tempNum << 8;

    convertedColorData = convertedColorData + tempNum;

    // ESP_LOGI(TAG, "convertedColorData = %d", convertedColorData);

    return convertedColorData;
}

bool isASCIIText(char text)
{
//   ESP_LOGI(TAG, "isASCIIText, text = %d", text);

  if(text < 127) return true;

  return false;
}

bool isInKoreanUnicode(unsigned int unicodeNum)
{
  if(unicodeNum <= 0xD7AF && unicodeNum >= 0xAC00) return true;

  return false;
}

bool isKoreanText(char *character, uint8_t startIndex)
{
//   ESP_LOGI(TAG, "isKoreanText");

  if(character[startIndex] & 0b11100000 && !(character[startIndex] & 0b00010000))
  {
    if(character[startIndex+1] & 0b10000000 && !(character[startIndex+1] & 0b01000000))
    {
       if(character[startIndex+2] & 0b10000000&& !(character[startIndex+2] & 0b01000000))
       {
        return true;
       }       
    }
  }  
  
  return false;
}

unsigned int convertUnicodeNumToCode(unsigned int unicode)
{
  unsigned int code;
  
  if(unicode >= COMPLETE_TYPE_TEXT_INDEX_IN_UNICODE)
  {
    code = unicode - CALI_VALUE_OF_CT_INDEX; //유니코드와 Font.h 파일간의 배열 인덱스 차이 계산
  }
  else
  {
    code = unicode - CALI_VALUE_OF_CV_INDEX; //유니코드와 Font.h 파일간의 배열 인덱스 차이 계산
  }

  return code;
}

void clearALL()
{
  clearRec(0,0,LCD_WIDTH,LCD_HEIGHT);
}

void clearRecTimteDayNum()
{
  clearRec(TIME_NUMBER_X_POS, TIME_NUMBER_SPACE_WIDTH, TIME_NUMBER_Y_POS, DAY_NUMBER_HEIGHT);
  
}

void clearRec(uint8_t xPos, uint8_t yPos, uint16_t width, uint16_t height)
{
  uint16_t *background;

  if(xPos + width > LCD_WIDTH) ESP_LOGI(TAG, "width over");

  if(yPos + height > LCD_HEIGHT) ESP_LOGI(TAG, "height over");

  background = (uint16_t *)malloc(sizeof(uint16_t) * width * height);

  for (int i = 0; i < width * height; i++)
  {
    background[i] = BACKGROUND_COLOR;
  }

  int result = esp_lcd_panel_draw_bitmap(panel_handle, xPos, yPos, xPos + width, yPos + height, background);

  if(result)
  {
    ESP_LOGI(TAG, "failed to clear display.");
  }
  else
  {
    ESP_LOGI(TAG, "successfully cleared display.");
  }

  free(background);
}

unsigned int convertKoreanToUnicodeNum(char *text, uint8_t startIndex)
{ 
  unsigned int num1 = 0, num2 = 0, num3 = 0;

  num1 = text[startIndex] - 0b11100000;
  num1 = num1 << 12;

  num2 = text[startIndex + 1] - 0b10000000;
  num2 = num2 << 6;

  num3 = text[startIndex + 2] - 0b10000000;

  return num1 + num2 + num3;
}

int calWidthOfText(char *text)
{
  uint8_t index = 0;
  uint8_t text_len = strlen(text);
  int xTotal = 0;

  while(index < text_len)
  {
    if(isASCIIText(text[index]))
    {
      xTotal += Font.chars[text[index]-GAP_UNICODE_FONTFILE].image->width + X_SPACE_TEXT_PIXEL;
      index++;
    }
    else if(isKoreanText(text, index))
    {
      xTotal += KOREAN_WIDTH + X_SPACE_TEXT_PIXEL;
      index += 3;
    }
  }
  
  return xTotal;
}

void calCenterPosOfText(char *text)
{
  int widthOfAllText = calWidthOfText(text);

  if(widthOfAllText < LCD_WIDTH)
  {
    textStartEndPos.start_X_Pos = (LCD_WIDTH - widthOfAllText)/2;
    textStartEndPos.numOfLine   = 0;
  }
  else if(widthOfAllText > LCD_WIDTH)
  {
    uint8_t lines = widthOfAllText / LCD_WIDTH;
     
    textStartEndPos.start_X_Pos = (LCD_WIDTH - (widthOfAllText - (LCD_WIDTH - X_START_POS_IN_ALIGN_CENTER * 2) * lines))/2;
    textStartEndPos.numOfLine   = lines;
  }
}

void drawText(bool isKorean, uint8_t align, uint16_t *x, uint16_t *y, char *text, uint16_t startIndex, uint8_t start_X_Pos, uint16_t end_X_Pos, uint16_t color)
{
  unsigned int unicodeNum, fontIndex;
  static uint8_t lines = 0;
  uint16_t *textToDisplay;

  if(isKorean)
  {
    unicodeNum = convertKoreanToUnicodeNum(text, startIndex);
    if(isInKoreanUnicode(unicodeNum))
      fontIndex = convertUnicodeNumToCode(unicodeNum);
    else
      fontIndex = 0;

    // ESP_LOGI(TAG, "fontIndex = %d, unicodeNum = %d", fontIndex, unicodeNum);
  }
  else
  {
    fontIndex = *text - GAP_UNICODE_FONTFILE;
    // ESP_LOGI(TAG, "fontIndex = %d", fontIndex);
  }

  if(*x + Font.chars[fontIndex].image->width > end_X_Pos)
  {
    lines++;
    
    if(align == CENTER_ALIGN)
    {
      if(lines < (textStartEndPos.numOfLine))
      {
        *x = X_START_POS_IN_ALIGN_CENTER;
      }
      else
      {
        *x = textStartEndPos.start_X_Pos;
      }      
    }
    else
    {
      *x = start_X_Pos;
    }

    *y += Font.chars[fontIndex].image->height + Y_SPACE_TEXT_PIXEL;    
  }

  textToDisplay = setTextColor(fontIndex ,color, Font);

  uint8_t result = esp_lcd_panel_draw_bitmap(panel_handle,*x,*y,*x + Font.chars[fontIndex].image->width,*y+Font.chars[fontIndex].image->height, textToDisplay);

  if(result)
  {
    ESP_LOGI(TAG, "failed to display color = %d", result);
  }
  else
  {
    ESP_LOGI(TAG, "Success! x = %d, y = %d, xEnd = %d, yEnd = %d, result = %d", *x, *y, *x + Font.chars[fontIndex].image->width, *y + Font.chars[fontIndex].image->height, result);
  }
 
  free(textToDisplay); 

  *x += Font.chars[fontIndex].image->width + X_SPACE_TEXT_PIXEL;  
}

void printText(uint16_t x, uint16_t y, char *text, uint8_t align, uint16_t color)
{
  uint16_t index = 0;
  uint16_t xIndex = x;
  uint16_t yIndex = y;

  uint8_t text_len = strlen(text);

//   ESP_LOGI(TAG, "text_len = %d", text_len);

  while(index < text_len)
  {
    // ESP_LOGI(TAG, "index = %d", index);

    if(isASCIIText(text[index]))
    {
      drawText(NON_KOREAN, align, &xIndex, &yIndex, &text[index], 0,     x, LCD_WIDTH, color);
      index++;
    }
    else if(isKoreanText(text, index))
    {      
      drawText(KOREAN,     align, &xIndex, &yIndex, text,         index, x, LCD_WIDTH, color);
      index += 3;
    }
    else
    {
      drawText(NON_KOREAN, align, &xIndex, &yIndex, "?",          0,     x, LCD_WIDTH, color);
      index++;
    }
  }
}

void printTextAlignCenter(uint16_t y, char *text, uint16_t color)
{
  uint8_t x;
  
  calCenterPosOfText(text);

  if(textStartEndPos.numOfLine == 0)
  {
    x = textStartEndPos.start_X_Pos;
  }
  else
  {
    x = X_START_POS_IN_ALIGN_CENTER;
  }

  printText(x, y, text, CENTER_ALIGN, color);
}

void drawTimeNumber(uint16_t y, uint8_t hour, uint8_t minute, uint16_t color) 
{
  uint16_t *timeToDisplay;

  uint8_t hour10   = hour/10;
  uint8_t hour1    = hour%10;
  uint8_t minute10 = minute/10;
  uint8_t minute1  = minute%10;

  timeToDisplay = setTextColor(hour10, color, TimeNumber);
  esp_lcd_panel_draw_bitmap(panel_handle, TIME_NUMBER_X_POS + TIME_NUMBER_WIDTH * 0, y, TIME_NUMBER_X_POS + TIME_NUMBER_WIDTH * 0 + TimeNumber.chars[hour10].image->width, y + TimeNumber.chars[hour10].image->height, timeToDisplay);  
  vTaskDelay(10 / portTICK_PERIOD_MS);
  free(timeToDisplay);  

  timeToDisplay = setTextColor(hour1, color, TimeNumber);
  esp_lcd_panel_draw_bitmap(panel_handle, TIME_NUMBER_X_POS + TIME_NUMBER_WIDTH * 1, y, TIME_NUMBER_X_POS + TIME_NUMBER_WIDTH * 1 + TimeNumber.chars[hour1 ].image->width, y + TimeNumber.chars[hour1 ].image->height, timeToDisplay);  
  vTaskDelay(10 / portTICK_PERIOD_MS);
  free(timeToDisplay);

  timeToDisplay = setTextColor(TIME_COLONS_INDEX, color, TimeNumber);
  esp_lcd_panel_draw_bitmap(panel_handle, TIME_NUMBER_X_POS + TIME_NUMBER_WIDTH * 2, y, TIME_NUMBER_X_POS + TIME_NUMBER_WIDTH * 2 + TimeNumber.chars[TIME_COLONS_INDEX].image->width, y + TimeNumber.chars[TIME_COLONS_INDEX].image->height, timeToDisplay);  
  vTaskDelay(10 / portTICK_PERIOD_MS);
  free(timeToDisplay);

  timeToDisplay = setTextColor(minute10, color, TimeNumber);
  esp_lcd_panel_draw_bitmap(panel_handle, TIME_NUMBER_X_POS + TIME_NUMBER_WIDTH * 2 + TIME_COLONS_WIDTH, y, TIME_NUMBER_X_POS + TIME_NUMBER_WIDTH * 2 + TIME_COLONS_WIDTH + TimeNumber.chars[minute10].image->width, y + TimeNumber.chars[minute10].image->height, timeToDisplay);  
  vTaskDelay(10 / portTICK_PERIOD_MS);
  free(timeToDisplay);

  timeToDisplay = setTextColor(minute1, color, TimeNumber);
  esp_lcd_panel_draw_bitmap(panel_handle, TIME_NUMBER_X_POS + TIME_NUMBER_WIDTH * 3 + TIME_COLONS_WIDTH, y, TIME_NUMBER_X_POS + TIME_NUMBER_WIDTH * 3 + TIME_COLONS_WIDTH + TimeNumber.chars[minute1 ].image->width, y + TimeNumber.chars[minute1 ].image->height, timeToDisplay);  
  vTaskDelay(10 / portTICK_PERIOD_MS);
  free(timeToDisplay);
}

void drawDayNumber(uint16_t y, uint8_t month, uint8_t day, uint16_t color)
{
  char text[10] = "00/00";
  sprintf(text,"%02u/%02u",month,day);

  printTextAlignCenter(y, text, color);
}

uint8_t changeCharToInt(char c)
{
  return c - '0';
}

void setDayTime(char *cmd)
{
  monthNum = changeCharToInt(cmd[7])*10  + changeCharToInt(cmd[8]);
  dayNum   = changeCharToInt(cmd[9])*10  + changeCharToInt(cmd[10]);
  hourNum  = changeCharToInt(cmd[11])*10 + changeCharToInt(cmd[12]);
  minNum   = changeCharToInt(cmd[13])*10 + changeCharToInt(cmd[14]);
}

void drawWeather(uint16_t color, bool isNeedClear)
{
  if(isNeedClear) clearRec(0, WEATHER_TEXT_Y_POS, LCD_WIDTH, TOTAL_FONT_HEIGHT);
  printTextAlignCenter(WEATHER_TEXT_Y_POS, weather, color);
}

void drawPasskey(uint16_t y, char * passkey, uint16_t color)
{
  isStatusScreen = false; 
  xTimerStop(xOneShotTimer, 0);

  clearALL();
  if(!isScreenOn) isScreenOn = true;
  display_turnOn_Off(isScreenOn);
  printTextAlignCenter(y, passkey, color);
}