#include "parseCMD.h"

uint8_t indexCmd;

bool isStatusScreen = true;

static const char *TAG = "parseCMD";

char msgTitle[40];
char msgContent[150];
char subText[100];

char weather[20] = "02/01";
char tempChar[10];

static float cmd_value(char *receiveCMD)
{
  return (strtod(&receiveCMD[indexCmd], NULL));
}

bool seenCMD(char *receiveCMD, char *cmdToCheck)
{
  uint8_t rollBackIndex = indexCmd;
  uint8_t cmdToCheck_len = strlen(cmdToCheck);

  for(uint8_t i=0; i<cmdToCheck_len; i++)
  {   
    if(receiveCMD[indexCmd] == ' ') 
      indexCmd++;

    if(receiveCMD[indexCmd] != cmdToCheck[i])
    {
      indexCmd = rollBackIndex;
      return false;
    }

    indexCmd++;
  }

  return true;
}

void parseCMD(char *cmd, uint16_t color)
{
  // ESP_LOGI(TAG, "parseCMD started");  

  if(seenCMD(cmd, "MS"))
  {    
    if(seenCMD(cmd, "T:"))
    {
      memset(msgTitle,   0, sizeof(msgTitle));
      strncpy(msgTitle, cmd + 5, strlen(cmd)-5);      
    }

    if(seenCMD(cmd, "C:"))
    {
      memset(msgContent, 0, sizeof(msgContent));
      strncpy(msgContent, cmd + 5, strlen(cmd)-5);   
    }

    if(seenCMD(cmd, "S:"))
    {
      memset(subText,    0, sizeof(subText));
      strncpy(subText, cmd + 5, strlen(cmd)-5);   
    }

    if(seenCMD(cmd, "O"))
    {
      // vib_motor(100);

      isStatusScreen = false;
      isCheckedMsg   = false;         

      if(!isScreenOn)
      {
        isScreenOn = !isScreenOn;
        // display_turnOn_Off(isScreenOn);
        xTimerStart(xOneShotTimer, 0);
      }      

      clearALL();
      printTextAlignCenter(NOTI_MSG_TITLE_Y_POS, msgTitle, color);
      printTextAlignCenter(NOTI_MSG_CONTENT_Y_POS, msgContent, color);      
    }    
  }
  else if(seenCMD(cmd,"YD")) // YD 명령어는 "YD yyyyMMddHHmm" 형태로 전달되어 옴
  {
    setDayTime(cmd);
    if(isCheckedMsg && isScreenOn)
    {
      clearRecTimteDayNum();
      drawDayNumber(DAY_NUMBER_Y_POS, monthNum, dayNum, color);
      drawTimeNumber(TIME_NUMBER_Y_POS, hourNum, minNum, color);
    }    
  }
  else if(seenCMD(cmd,"WD"))
  {    
    if(seenCMD(cmd,"PTY"))
    {       
      switch((int)cmd_value(cmd))
      {
        case 0:
          sprintf(weather, "없음");
          break;
        case 1:
          sprintf(weather, "비");
          break;
        case 2:
          sprintf(weather, "비/눈");
          break;
        case 3:
          sprintf(weather, "눈");
          break;
        case 5:
          sprintf(weather, "빗방울");
          break;
        case 6:
          sprintf(weather, "빗방울/눈날림");
          break;
        case 7:
          sprintf(weather, "눈날림");
          break;
      }
    }
    
    if(seenCMD(cmd,"T1H"))
    {
      sprintf(tempChar, "/ %3.1f도", cmd_value(cmd));
      strcat(weather, tempChar);
    }

    if(isCheckedMsg && isScreenOn)
    {
      drawWeather(color, true);
    }
  }
  indexCmd = 0;
  // ESP_LOGI(TAG, "parseCMD finished");
}