/*********DHT11 *******
 *DHT11实验，不能读到正确的参数
 1、传感器的输出接口不能按设定样得到相应的信号。
 
***********************/
//#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

//#include "waiter.h"
 #include "dht11.h"
static const char *TAG = "DHT11_test";


/****方案一采用调用现成库的方式读取成功***
 * 在项目文件夹下建立components文件夹，在建导入dht11.c,并建include文件夹并导入dth11.h文件夹
 * dth11.h头文件主要是定义 重要的结构体，和对外接口函数
 * dth11.c具体函数实现
 * 主程序esp_dht11.c ,调用.h中定义的函数实现对传感器读取
************************************************************/
void app_main()
{
    DHT11_init(GPIO_NUM_8);

    while(1) {
        ESP_LOGI(TAG,"Temperature is %d \n", DHT11_read().temperature);
        ESP_LOGI(TAG,"Humidity is %d\n", DHT11_read().humidity);
        ESP_LOGI(TAG,"Status code is %d\n", DHT11_read().status);
        vTaskDelay(3000/portTICK_PERIOD_MS);
    }
}



/*  方案二：读取不成功
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"*/


/*#define DHT11_PIN   GPIO_NUM_8//定义DHT11的引脚
 
#define uchar unsigned char
#define uint8 unsigned char
#define uint16 unsigned short
 
//温湿度定义
uchar ucharFLAG,uchartemp;
float Humi,Temp;
uchar ucharT_data_H,ucharT_data_L,ucharRH_data_H,ucharRH_data_L,ucharcheckdata;
uchar ucharT_data_H_temp,ucharT_data_L_temp,ucharRH_data_H_temp,ucharRH_data_L_temp,ucharcheckdata_temp;
uchar ucharcomdata;
 
static void InputInitial(void)//设置端口为输入
{
  gpio_pad_select_gpio(DHT11_PIN);
  gpio_set_direction(DHT11_PIN, GPIO_MODE_INPUT);
}
 
static void OutputHigh(void)//输出1
{
  gpio_pad_select_gpio(DHT11_PIN);
  gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(DHT11_PIN, 1);
}
 
static void OutputLow(void)//输出0
{
  gpio_pad_select_gpio(DHT11_PIN);
  gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(DHT11_PIN, 0);
}
 
static uint8 getData()//读取状态
{
	return gpio_get_level(DHT11_PIN);
}
 
//读取一个字节数据
static void COM(void)    // 温湿写入
{
    uchar i;
    for(i=0;i<8;i++)
    {
        ucharFLAG=2;
        //等待IO口变低，变低后，通过延时去判断是0还是1
        while((getData()==0)&&ucharFLAG++) ets_delay_us(10);
        ets_delay_us(35);//延时35us
        uchartemp=0;
 
        //如果这个位是1，35us后，还是1，否则为0
        if(getData()==1)
          uchartemp=1;
        ucharFLAG=2;
 
        //等待IO口变高，变高后，表示可以读取下一位
        while((getData()==1)&&ucharFLAG++)
          ets_delay_us(10);
        if(ucharFLAG==1)
          break;
        ucharcomdata<<=1;
        ucharcomdata|=uchartemp;
    }
}
 
void Delay_ms(uint16 ms)
{
	int i=0;
	for(i=0; i<ms; i++){
		ets_delay_us(1000);
	}
}
 
void DHT11(void)   //温湿传感启动
{
    OutputLow();
    Delay_ms(19);  //>18MS
    OutputHigh();
    InputInitial(); //输入
    ets_delay_us(30);
    if(!getData())//表示传感器拉低总线
    {
        ucharFLAG=2;
        //等待总线被传感器拉高
        while((!getData())&&ucharFLAG++)
          ets_delay_us(10);
        //等待总线被传感器拉低
        while((getData())&&ucharFLAG++)
          ets_delay_us(10);
        COM();//读取第1字节，
        ucharRH_data_H_temp=ucharcomdata;
        COM();//读取第2字节，
        ucharRH_data_L_temp=ucharcomdata;
        COM();//读取第3字节，
        ucharT_data_H_temp=ucharcomdata;
        COM();//读取第4字节，
        ucharT_data_L_temp=ucharcomdata;
        COM();//读取第5字节，
        ucharcheckdata_temp=ucharcomdata;
        OutputHigh();
        //判断校验和是否一致
        uchartemp=(ucharT_data_H_temp+ucharT_data_L_temp+ucharRH_data_H_temp+ucharRH_data_L_temp);
        if(uchartemp==ucharcheckdata_temp)
        {
            //校验和一致，
            ucharRH_data_H=ucharRH_data_H_temp;
            ucharRH_data_L=ucharRH_data_L_temp;
            ucharT_data_H=ucharT_data_H_temp;
            ucharT_data_L=ucharT_data_L_temp;
            ucharcheckdata=ucharcheckdata_temp;
            //保存温度和湿度
            Humi=ucharRH_data_H;
            Humi=((uint16)Humi<<8|ucharRH_data_L)/10;
 
            Temp=ucharT_data_H;
            Temp=((uint16)Temp<<8|ucharT_data_L)/10;
        }
        else
        {
          Humi=100;
          Temp=100;
        }
    }
    else //没用成功读取，返回0
    {
    	Humi=0,
    	Temp=0;
    }
 
    OutputHigh(); //输出
}
 
 
void app_main()
{
    char dht11_buff[50]={0};
 
    while(1)
    {
      DHT11(); //读取温湿度
      ESP_LOGI(TAG,"Temp=%.2f--Humi=%.2f%%RH \r\n", Temp,Humi);
      vTaskDelay(300);  //延时300毫秒
    }
}*/

//****  方案三读取不成功*************************************************************
/*#define DHT11_PIN     GPIO_NUM_21//(21)   //可通过宏定义，修改引脚

#define DHT11_CLR     gpio_set_level(DHT11_PIN, 0) 
#define DHT11_SET     gpio_set_level(DHT11_PIN, 1) 
#define DHT11_IN      gpio_set_direction(DHT11_PIN, GPIO_MODE_INPUT)
#define DHT11_OUT     gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT)

uint8_t DHT11Data[4]={0};
uint8_t Temp, Humi;

//us延时函数，误差不能太大
void DelayUs(  uint32_t nCount)  
{
    ets_delay_us(nCount);
}  

void DHT11_Start(void)
{ 
  DHT11_OUT;      //设置端口方向
  DHT11_CLR;      //拉低端口  
  DelayUs(19*1000);   
//   vTaskDelay(19 * portTICK_RATE_MS); //持续最低18ms;

  DHT11_SET;      //释放总线
  DelayUs(30);    //总线由上拉电阻拉高，主机延时30uS;
  DHT11_IN;       //设置端口方向

  while(!gpio_get_level(DHT11_PIN))  //DHT11等待80us低电平响应信号结束
  {
     ESP_LOGI(TAG,"引脚电平1=%d",gpio_get_level(DHT11_PIN));   
    }
  while(gpio_get_level(DHT11_PIN))//DHT11   将总线拉高80us
  {
     ESP_LOGI(TAG,"引脚电平2=%d",gpio_get_level(DHT11_PIN));   
    }
}

uint8_t DHT11_ReadValue(void)
{ 
  uint8_t i,sbuf=0;
  for(i=8;i>0;i--)
  {
    sbuf<<=1; 
    while(!gpio_get_level(DHT11_PIN))
    {
     ESP_LOGI(TAG,"引脚电平3=%d",gpio_get_level(DHT11_PIN));   
    }
    DelayUs(30);                        // 延时 30us 后检测数据线是否还是高电平 
    if(gpio_get_level(DHT11_PIN))
    {
      sbuf|=1;  
    }
    else
    {
      sbuf|=0;
    }
    while(gpio_get_level(DHT11_PIN)){
        
     ESP_LOGI(TAG,"引脚电平4=%d",gpio_get_level(DHT11_PIN));   
    }
    
  }
  return sbuf;   
}

uint8_t DHT11_ReadTemHum(uint8_t *buf)
{
  uint8_t check;

  buf[0]=DHT11_ReadValue();
  buf[1]=DHT11_ReadValue();
  buf[2]=DHT11_ReadValue();
  buf[3]=DHT11_ReadValue();
    
  check =DHT11_ReadValue();

  if(check == buf[0]+buf[1]+buf[2]+buf[3])
    return 1;
  else
    return 0;
} 

void app_main(void)
{
    ESP_LOGI(TAG,"ESP32 DHT11 TEST:%s,%s!\r\n",__DATE__,__TIME__);
    gpio_pad_select_gpio(DHT11_PIN);
    while(1) {
        DHT11_Start();
        if(DHT11_ReadTemHum(DHT11Data))
        {
            Temp=DHT11Data[2];
            Humi=DHT11Data[0];      
            printf("Temp=%d, Humi=%d\r\n",Temp,Humi);
        }
        else
        {
            printf("DHT11 Error!\r\n");
        }
      
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}*/

/**********************************************/

