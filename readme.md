<!--
 * @Description: 
 * @Author: zpw
 * @LastEditors: zpw
 * @Date: 2019-04-19 19:20:39
 * @LastEditTime: 2019-04-19 20:30:07
 -->
# 程序参数管理
对需要保存到Flash的程序参数进行统一管理，提供读取、写入、还原默认值功能，含数据校验。
资源占用：
依赖：      FAL

## 使用示例
```C
#define LED0_PIN GET_PIN(D, 13)
#define LED1_PIN GET_PIN(D, 12)
//添加LED Device
led_add_device(LED0_PIN);
led_add_device(LED1_PIN);

//LED编号按照添加的顺序从1开始计数
led_on(1);
led_on(2);

//关闭led
led_off(1);
led_off(2);

//翻转一次LED
led_tog(1);
led_tog(2);

//1号LED, 亮500ms, 灭500ms
led_blink(1,500, 500);
//1号LED, 亮1500ms, 灭200ms
led_blink(1,1500, 200);

//2号LED, 单次，亮1500后熄灭.进入关闭状态
led_blink(2,1500, 0);
```
状态可以运行中随时设置
## LED状态的更新
---
需要将led状态更新的处理丢到任意一个线程里面
```C
while (1)
{
    led_process(10);  //传入tick等于线程执行的cycle
    rt_thread_mdelay(10);
}
```
