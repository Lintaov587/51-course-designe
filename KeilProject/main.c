#include <REGX52.H>
#define uint8_t unsigned char

/* 按键的三种状态 */
#define REST  0x00
#define INCN  0x01
#define NONE  0x02

/* 数码管的段码   */
uint8_t seg[] = {0x18, 0x3f, 0x92, 0x13, 0x35,
				 0x51, 0x50, 0x3b, 0x10, 0x11};

/* 声明使用的引脚 */
sfr     seg7  = 0xA0;
sbit    seg1  = 0x87;
sbit    seg2  = 0x86;
sbit    k1    = 0x82;
sbit    LED   = 0x85;
sbit    k2    = 0x84;

/* 声明使用的函数 */
void delay(unsigned int time);  
     /* 延时函数  */

void segDisplay(uint8_t *disNum); 
    /* 数码管显示 */

void ledDisplay(int period, uint8_t disNum); 
    /* PWM 控制led的前半个周期 */

void ledDisplay2(int period, uint8_t disNum);  
    /* PWM 控制led的后半个周期 */

void numInc(uint8_t *disNum);	
 	/*disNum 高四位表示数码管高位，
   低四位表示数码管低位，
这个函数让显示数值正确加1  */

uint8_t low4B(uint8_t num);                      
   /*取num低四位*/

uint8_t high4B(uint8_t num);            
   /*取num高四位*/

uint8_t readKey(void);              
   /*读取按键*/

void main()
{
    uint8_t initDisplayNum = 0x00;    
	   //初始显示数值
    LED = 1;                       
	   //初始化LED
    while (1)
    {
	    segDisplay(&initDisplayNum);  
		  //主循环函数
    }
}
void ledDisplay(int period, uint8_t disNum)
{
    static int brightNess = 0;             
	  //初始亮度
    int i;                              
	  //循环辅助变量
    
    if(disNum > brightNess) brightNess++;
    else if (brightNess == 0) brightNess = 0;
    else brightNess--;                
	  //复位时LED亮度慢慢降低而不是突然熄灭

    for(i = 0; i < period; i++)
    {
        if (i > brightNess)
        {
            LED = 1;
        }
        else
        {
            LED = 0;
        }
    }
    LED = 1;                     
	   //前半PWM周期
}

void ledDisplay2(int period, uint8_t disNum)   
//后半PWM周期，同上函数
{
    static int brightNess = 0;
    int i;

    if(disNum > brightNess) brightNess++;
    else if (disNum == 0) brightNess = 0;
    else brightNess--;

    for(i = 200; i < 200+period; i++)
    {
        if (i > brightNess)
        {
            LED = 1;
        }
        else
        {
            LED = 0;
        }
    }
    LED = 1;    

}							

uint8_t readKey(void)           //按键检测函数
{
    static uint8_t k1_press_time = 0;
    static uint8_t k2_press_time = 0;
    static uint8_t k1_last = 0;
    static uint8_t k2_last = 0;
    static bit toggle1 = 0;
    static bit toggle2 = 0;
    uint8_t key1;
    uint8_t key2;                       
	       //定义辅助变量

    key1 = ~k1;
    key2 = ~k2;

    k1_press_time = k1_press_time * key1 + key1;
    k2_press_time = k2_press_time * key2 + key2;    
//按下时间计数，按下时间等于kx_press_time * 控制周期


    if(k1_last == 0 && key1 == 1)  
    {
        toggle1 = 1;
    }                              
	   //按键1跳变检测



    if(k2_last == 0 && key2 == 1)
    {
        toggle2 = 1;
    }                           
	       //按键2跳变检测
    

    k1_last = key1;
    k2_last = key2;


    if(k1_press_time > 5 && toggle1)
    {
        k1_press_time = 0;
        toggle1 = 0;
        return INCN;           
		        //按键1状态判定
    }
    else if(k2_press_time > 5 && toggle2)
    {
        k2_press_time = 0;
        toggle2 = 0;
        return REST;
    }                            
	      //按键2状态判定
    else
    {
        return NONE;         
		       //无状态判定
    }
}

void delay(unsigned int time)        //延时
{
    int i,j;
    for(i = 0; i < time; i++)
    {
        for(j = 0; j < i; j++)
        ;
    }
}

uint8_t low4B(uint8_t num)           //
{
    return num & 0x0f;
}

uint8_t high4B(uint8_t num)                   //
{
    return (num & 0xf0) >> 4;
}

void segDisplay(uint8_t *disNum)
{
    uint8_t keyStatus = NONE;
    seg1 = 0;
    seg2 = 1;
    seg7 = seg[low4B(*disNum)];   
	         //根据disNum 低四位控制数码管显示低位
	//delay(5);
    ledDisplay(150, *disNum);        
	       //根据disNum 控制LED 亮度 前半周期
    seg7 = 0xff;                 
	         //清空显示，消除鬼影
    seg1 = 1;
    seg2 = 0;
    seg7 = seg[high4B(*disNum)];      
	     //根据disNum 高四位控制数码管显示高位
	//delay(5); 
    ledDisplay2(150, *disNum);         
	     //根据disNum 控制LED 亮度 后半周期
    seg7 = 0xff;                     
	     //清空显示，消除鬼影
    keyStatus = readKey();           
	      //读取按键
    if(keyStatus == INCN)
    {
        numInc(disNum);             
		    //按键触发加一事件
    }
    else if(keyStatus == REST)
    {
        *disNum = 0x00;            
		     //显示归零
    }
    else
    {
        ;
    }                                    
    
}
void numInc(uint8_t *disNum)
{
	if(low4B(*disNum) != 0x09)
	{

		(*disNum)++;
	}	                   
	      //如果不是该进位，就加一
	else if((*disNum) == 0x99)
	{
		*disNum = 0x99;
	}                           
	 //如果计满，不变
	else
	{
		(*disNum) = (((high4B(*disNum)+1)<<4) & 0xf0);
	}                          
	   //否则进位，即高四位加一，低四位清零。
}
