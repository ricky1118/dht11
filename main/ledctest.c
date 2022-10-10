/********* timertest *******
 * Example ledctest软件定时器 .
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "sdkconfig.h"
#include "driver/timer.h"
#include "driver/ledc.h"
#include "esp_timer.h"
static const char *TAG = "ledctest";

/**
 * 摘要
 * 使用PWM控制器来输出PWM波（占空比）调节LED灯亮度，
 * 2路PWM输出的管脚呼吸闪烁，从俺到亮，之后又渐变暗，不断循环交替
 * fade 渐变/淡入/淡出/褪色
 ************/
//相关参数宏定义
#define LEDC_TOTAL_NUM  2//控制组总数量


#define LEDC_HS_TIMER LEDC_TIMER_0//使用定时器0
#define LEDC_HS_MODE   LEDC_LOW_SPEED_MODE //定时器模式,只有低速模式没有高速模式
#define LEDC_HS_CH0_GPIO  GPIO_NUM_17 //LED0 接到17端口
#define LEDC_HS_CH0_CHANNEL LEDC_CHANNEL_0  // 将17定义到CHANNEL0端口上， 
#define LEDC_HS_CH1_GPIO  GPIO_NUM_18 //LED0 接到18端口
#define LEDC_HS_CH1_CHANNEL LEDC_CHANNEL_1  // 将18定义到CHANNEL1端口上， 

#define LEDC_TEST_DUTY  8000      
#define LEDC_TEST_FADE_TIME 3000
 ledc_channel_config_t ledc_channel[LEDC_TOTAL_NUM];
//LEDC 初始化
static void ledc_init(){
         //定义定时器结构体，并完成初始化赋值
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT,//频率占空比的分辨率，即占空比的最小值这里是13位分辨率2^13  1/8192
        .freq_hz = 5000,//设置PWM频率
        .speed_mode = LEDC_HS_MODE,//设置定时器为低速模式，S3没有高速模式
        .timer_num = LEDC_HS_TIMER//悬着使用定时器0

    };
    ledc_timer_config(&ledc_timer);

    //配置所有通道结构体并赋值，
    ledc_channel_config_t ledc_channel[LEDC_TOTAL_NUM] = {
        {
            .channel = LEDC_HS_CH0_CHANNEL,
            .duty = 0,
            .gpio_num =LEDC_HS_CH0_GPIO,
            .speed_mode = LEDC_HS_MODE,
            .timer_sel = LEDC_HS_TIMER
        },
        {
            .channel = LEDC_HS_CH1_CHANNEL,
            .duty = 0,
            .gpio_num = LEDC_HS_CH1_GPIO,
            .speed_mode = LEDC_HS_MODE,
            .timer_sel = LEDC_HS_TIMER
        },

    };
    //遍历并使能       用预先准备好的配置设置led控制器( Set LED Controller with previously prepared configuration)
    int ch;
    for(ch =0;ch<LEDC_TOTAL_NUM;ch++){

        ledc_channel_config(&ledc_channel[ch]);
    }
}

/***********************************************/
void app_main(void)
{
   ledc_init();
   
   ledc_fade_func_install(0);//初始化渐变服务安装，即安装LEDC渐变功能
   int ch;
   uint flag = 0;
   while (1)
   {
    ESP_LOGI(TAG,"1.逐渐变大的周期目标 = %d\n",LEDC_TEST_DUTY);
    for(ch = 0; ch<LEDC_TOTAL_NUM;ch++){
        //设置渐变功能和时间  
        ledc_set_fade_with_time(ledc_channel[ch].speed_mode,ledc_channel[ch].channel,LEDC_TEST_DUTY,LEDC_TEST_FADE_TIME);
        //渐变开始
        ledc_fade_start(ledc_channel[ch].speed_mode,ledc_channel[ch].channel,LEDC_FADE_NO_WAIT);
    }
    vTaskDelay(LEDC_TEST_FADE_TIME / portTICK_PERIOD_MS);

      ESP_LOGI(TAG,"1.逐渐变小的周期目标 = %d\n",flag);
    for(ch = 0; ch<LEDC_TOTAL_NUM;ch++){
        //设置渐变功能和时间  
        ledc_set_fade_with_time(ledc_channel[ch].speed_mode,ledc_channel[ch].channel,0,LEDC_TEST_FADE_TIME);
        //渐变开始
        ledc_fade_start(ledc_channel[ch].speed_mode,ledc_channel[ch].channel,LEDC_FADE_NO_WAIT);
    }
    vTaskDelay(LEDC_TEST_FADE_TIME / portTICK_PERIOD_MS);

   }
   
   
}
