/*********ledctest *******
 *LEDC PWM控制器的使用测试
 1、GPIO 17 18接LED灯，通过PWM调节灯光亮度渐变实现呼吸灯效果 
 
***********************/
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
 * 2路PWM输出的管脚呼吸闪烁，从暗到亮，之后又渐变暗，不断循环交替
 * fade 渐变/淡入/淡出/褪色
 * 出现未解决的问题，通道结构体赋值时有问题未解决，具体在程序中有说明
 ************/
//相关参数宏定义
#define LEDC_TOTAL_NUM  2//控制组总数量


#define LEDC_HS_TIMER LEDC_TIMER_0//使用定时器0
#define LEDC_HS_MODE   LEDC_LOW_SPEED_MODE //定时器模式,只有低速模式没有高速模式
#define LEDC_HS_CH0_GPIO  GPIO_NUM_17 //LED0 接到17端口
#define LEDC_HS_CH0_CHANNEL LEDC_CHANNEL_0  // 将17定义到CHANNEL0端口上， 
#define LEDC_HS_CH1_GPIO  GPIO_NUM_18//LED0 接到18端口
#define LEDC_HS_CH1_CHANNEL LEDC_CHANNEL_1  // 将18定义到CHANNEL1端口上， 

#define LEDC_TEST_DUTY  8000     //目标占空比
#define LEDC_TEST_FADE_TIME 3000 //渐变时间
 ledc_channel_config_t ledc_channel[LEDC_TOTAL_NUM];//通道结构体数组
//LEDC 初始化
  static void ledc_init(){
   ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT,//频率占空比的分辨率，即占空比的最小值这里是13位分辨率2^13  1/8192
        .freq_hz = 5000,//设置PWM频率
        .speed_mode = LEDC_HS_MODE,//设置定时器为低速模式，S3没有高速模式
        .timer_num = LEDC_HS_TIMER//悬着使用定时器0

    };
    ledc_timer_config(&ledc_timer);

    //配置所有通道结构体并赋值，这里出现问题？
       ledc_channel[0].channel = LEDC_HS_CH0_CHANNEL;
        ledc_channel[0].duty = 0;
        ledc_channel[0].gpio_num =LEDC_HS_CH0_GPIO;
        ledc_channel[0].speed_mode = LEDC_HS_MODE;
        ledc_channel[0].timer_sel = LEDC_HS_TIMER;
      
        ledc_channel[1].channel = LEDC_HS_CH1_CHANNEL;
        ledc_channel[1].duty = 0;
        ledc_channel[1].gpio_num =LEDC_HS_CH1_GPIO;
        ledc_channel[1].speed_mode = LEDC_HS_MODE;
        ledc_channel[1].timer_sel = LEDC_HS_TIMER;
  /********************************************* */
    //配置所有通道结构体并赋值，这里出现问题？只能完成一个通道的赋值，不能同时完成两个通道赋值，也就是17  18只有一个引脚上的灯完成渐变。原因还未找到。
    //如果把这一段直接加入到app_main中，赋值又能成功，原因未明
    /*ledc_channel_config_t ledc_channel[LEDC_TOTAL_NUM]] = {{
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

    };*/
    /********************************************* */
    //遍历并使能       用预先准备好的配置设置led控制器( Set LED Controller with previously prepared configuration)
    int ch0;
    for(ch0 = 0;ch0<LEDC_TOTAL_NUM;ch0++){

        ledc_channel_config(&ledc_channel[ch0]);
    }
 
}

/***********************************************/
void app_main(void)
{
   ledc_init();
/*********************/
 //直接放在这里能完成通道设置，如果这段直接放在ledc_init（）只有一个通道能渐变
  /* ledc_channel_config_t ledc_channel[LEDC_TOTAL_NUM] = {
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

    };*/
    /*********************/

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
