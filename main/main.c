#include "main.h"

#define ESP_INTR_FLAG_DEFAULT 0

#define MOTOR_PIN 21

#define BATTERY_PIN ADC_CHANNEL_5

static const char *TAG = "main";

static int adc_raw[2][10], voltage[2][10];

QueueHandle_t gpio_evt_queue;
QueueHandle_t gpio_evt_queue_2;

TimerHandle_t xOneShotTimer, xAutoReloadTimer;
TimerHandle_t xMotorOffTimer;

TaskHandle_t xHandle_ScreenOff = NULL;

static bool example_adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle);
static void example_adc_calibration_deinit(adc_cali_handle_t handle);

static void screenOffTimer(TimerHandle_t xTimer);
static void timeUpdate(TimerHandle_t xTimer);

// adc1_config_channel_atten(ADC1_GPIO5_CHANNEL, ADC_ATTEN_DB_11);

// esp_adc_cal_characteristics_t adc1_chars;
// esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);

// gpio_config_t motor_gpio_config = {
//     .mode = GPIO_MODE_OUTPUT,
//     .pull_down_en = 1,
//     .pin_bit_mask = 1ULL << MOTOR_PIN
// };

gpio_config_t up_btn_gpio_config = {
    .intr_type = GPIO_INTR_POSEDGE,
    .mode = GPIO_MODE_INPUT,
    .pull_down_en = 1,
    .pin_bit_mask = 1ULL << UP_BTN
};

gpio_config_t down_btn_gpio_config = {
    .intr_type = GPIO_INTR_POSEDGE,
    .mode = GPIO_MODE_INPUT,
    .pull_down_en = 1,
    .pin_bit_mask = 1ULL << DOWN_BTN
};

bool isScreenOn = true;
uint32_t time_ISR = 0, time_ISR_2 = 0;

static void IRAM_ATTR gpio_isr_handler(void *args)
{
    int pinNumber = (uint32_t)args;
    xQueueSendFromISR(gpio_evt_queue, &pinNumber, NULL);
}

static void IRAM_ATTR gpio_isr_handler_2(void *args)
{
    int pinNumber = (uint32_t)args;
    xQueueSendFromISR(gpio_evt_queue_2, &pinNumber, NULL);
}

static void screenOffTimer(TimerHandle_t xTimer)
{
    isScreenOn = false;

    display_turnOn_Off(isScreenOn);

    printf("screen Off\n");
}

static void timeUpdate(TimerHandle_t xTimer)
{
    minNum++;

    if(minNum >= 60){
        minNum = 0;
        hourNum++;
    }

    if(hourNum >= 24){
        hourNum = 0;
    }

    printf("min ++\n");

    if(isStatusScreen) drawTimeNumber(TIME_NUMBER_Y_POS,hourNum,minNum,convert_colordata(TIME_NUMBER_COLOR));
}

void vib_motor(uint16_t time_ms)
{
    gpio_set_level(MOTOR_PIN, true);
    vTaskDelay(time_ms / portTICK_PERIOD_MS);
    gpio_set_level(MOTOR_PIN, false);
}

void lcd_On_Task(void *params)
{
    uint32_t io_num;    
    uint32_t time1, time2;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, -1)) {  
            time1 = xTaskGetTickCount();
            if(time1 - time_ISR >= 50){     
                isScreenOn = !isScreenOn;

                display_turnOn_Off(isScreenOn);                
                
                if(isScreenOn) xTimerStart(xOneShotTimer, 0);
                
                time2 = time_ISR = xTaskGetTickCount();    
                printf("time1 : %ld, time2 : %ld\n", time1, time2);
            }                        
        }
    }
}

void statusScreenOn_Task(void *params)
{
    uint32_t time3, time4;
    uint32_t io_num;    
    for(;;) {
        if(xQueueReceive(gpio_evt_queue_2, &io_num, -1)) {
            time3 = xTaskGetTickCount();    
            if(xTaskGetTickCount() - time_ISR_2 >= 50){     
                if(isScreenOn && !isStatusScreen){ 
                    // display_init_test();
                    drawStatusScreen();
                }

                time4 = time_ISR_2 = xTaskGetTickCount();

                printf("time3 : %ld, time4 : %ld\n", time3, time4);
            }            
        }
    }
}

void app_main(void)
{
    // ESP_ERROR_CHECK(gpio_config(&motor_gpio_config));

    ESP_ERROR_CHECK(gpio_config(&up_btn_gpio_config));
    ESP_ERROR_CHECK(gpio_config(&down_btn_gpio_config));

    //change gpio interrupt type for one pin
    gpio_set_intr_type(UP_BTN, GPIO_INTR_POSEDGE);
    gpio_set_intr_type(DOWN_BTN, GPIO_INTR_POSEDGE);

    gpio_evt_queue   = xQueueCreate(10, sizeof(uint32_t));
    gpio_evt_queue_2 = xQueueCreate(10, sizeof(uint32_t));

    //start gpio task
    xTaskCreate(lcd_On_Task, "lcd_On_Task", 2048, NULL, 5, NULL);
    xTaskCreate(statusScreenOn_Task, "statusScreenOn_Task", 4096, NULL, 5, NULL);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(UP_BTN, gpio_isr_handler, (void*) UP_BTN);
    gpio_isr_handler_add(DOWN_BTN, gpio_isr_handler_2, (void*) DOWN_BTN);    

    ble_init();
    display_init(NO_XY_SWAP);

    display_turnOn_Off(true);    

    drawStatusScreen();

    gpio_sleep_set_direction(LCD_PIN_NUM_BK_LIGHT, GPIO_MODE_DEF_OUTPUT);
    gpio_sleep_set_direction(LCD_PIN_NUM_MOSI, GPIO_MODE_DEF_OUTPUT);
    gpio_sleep_set_direction(MOTOR_PIN, GPIO_MODE_DEF_OUTPUT);
    
    xOneShotTimer    = xTimerCreate("OneShot",    pdMS_TO_TICKS(5000UL),  pdFALSE, 0, screenOffTimer);
    xAutoReloadTimer = xTimerCreate("AutoReload", pdMS_TO_TICKS(60000UL), pdTRUE,  0, timeUpdate);

    xTimerStart(xOneShotTimer,    0);
    xTimerStart(xAutoReloadTimer, 0);    

    // adc_oneshot_unit_handle_t adc1_handle;
    // adc_oneshot_unit_init_cfg_t init_config1 = {
    //     .unit_id = ADC_UNIT_1,
    // };

    // ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // adc_oneshot_chan_cfg_t config = {
    //     .bitwidth = ADC_BITWIDTH_DEFAULT,
    //     .atten = ADC_ATTEN_DB_11,
    // };

    // ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, BATTERY_PIN, &config));

    // adc_cali_handle_t adc1_cali_handle = NULL;
    // bool do_calibration1 = example_adc_calibration_init(ADC_UNIT_1, ADC_ATTEN_DB_11, &adc1_cali_handle);

    // ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, BATTERY_PIN, &adc_raw[0][0]));
    // ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, BATTERY_PIN, adc_raw[0][0]);
    // if (do_calibration1)
    // {
    //     ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw, &voltage));
    //     ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, BATTERY_PIN, voltage);
    // }
    vTaskDelay(pdMS_TO_TICKS(1000));

    // ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
    // if (do_calibration1) {
    //     example_adc_calibration_deinit(adc1_cali_handle);
    // }

    // gpio_sleep_set_direction(BATTERY_PIN, GPIO_MODE_DEF_INPUT);
}

static bool example_adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}


static void example_adc_calibration_deinit(adc_cali_handle_t handle)
{
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));
}