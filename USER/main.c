#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "RN8209G_SPI.h"	
#include "usmart.h"

const u8 TEXT_Buffer[]={"WarShipSTM32 SPI TEST"};
//extern u8 address;
//u8 BUF_len[8];
//u32 BUF_ReceiveData[32];//读出寄存器的值，从0到31开始储存，
//u32 *data=BUF_ReceiveData;
//u8*len=BUF_len;
//extern u32  RMSIAreg ;
//extern u32  PowerPAreg ;
//extern u32  RMSUreg  ;
//extern u32  KiA;
//extern u32  KiB  ;
//extern u32  Ku		;
//extern u32  Kp   ;
//extern u32  TempU      ;
//extern u32  TempIA     ;
//extern u32  TempPowerPA;


u32  data_r[4];
u32  *p= &data_r[0];
u8 mode_key=0;

__align(4) u8 dtbuf[50]; 

u32 SPL_IA_reg ;//读adc值
u32 SPL_U_reg ;
//u8*len;
 int main(void)
 {	  
  	delay_init();	    	 //延时函数初始化	  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
  	uart_init(9600);	 	//串口初始化为115200
	  usmart_dev.init(SystemCoreClock/1000000);	//初始化USMART		
	 
//	LED_Init();		  		//初始化与LED连接的硬件接口
  	LCD_Init();			   	//初始化LCD 	
	 
	KEY_Init();				//按键初始化
	POINT_COLOR=RED;	
	LCD_ShowString(5,40,130,24,24,"Urms(24H) :"); 
	LCD_ShowString(5,70,130,24,24,"Iarms(22H):"); 
	LCD_ShowString(5,100,130,24,24,"PowA(26H) :"); 
	
	LCD_ShowString(20,150,60,24,24,"U :"); 
	LCD_ShowString(20,180,60,24,24,"IA:"); 
	LCD_ShowString(20,210,60,24,24,"PA:"); 
	
	LCD_ShowString(220,150,20,24,24,"V"); 
	LCD_ShowString(220,180,20,24,24,"A"); 
	LCD_ShowString(220,210,20,24,24,"W");
	 
//	LCD_ShowString(20,240,60,24,24,"Ku:"); //先不用，先显示ADC的值
//	LCD_ShowString(20,265,60,24,24,"Ki:"); //先不用，先显示ADC的值
	LCD_ShowString(20,295,60,24,24,"Kp:"); 
	 
	 
	RN8209G_SPI_config( );
	RN8209_WriteData(0xEA,0XFA);//软件复位
	delay_ms(50);
	RN8209_WriteData(0xEA,0XE5);//写使能
	delay_ms(50);
	RN8209_Parameter_Adjust();//校表第一步，参数设置
//		delay_ms(10);
//		
//		RN8209_ActivePower_Adjust();//校表第2.1步——————有功校正
//		delay_ms(10);
//    
//		RN8209_ReactivePower_Adjust();//校表第2.2步——————无功校正
//		delay_ms(10);
//		
//		RN8209_Rms_Adjust();//校表第3步——————有效值校正
		delay_ms(50);
//	 
	while(1)
	{
//		KEY_Scan(0);
//		if((GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0)&&(mode_key==0))//按下之后，计算K值
//		{ 
//			POINT_COLOR=BLUE;
//			LCD_ShowString(50,10,130,24,24,"  Count_Kx         "); 		
//			delay_ms(50);			
//			RN8209_Count_Kx();                                                 		                                                        
//			sprintf((char *)dtbuf,"%f       ",Ku);                             
//		  LCD_ShowString(100,240,120,24,24,dtbuf);                            
//			                                                                   
//			sprintf((char *)dtbuf,"%f       ",KiA);
//			LCD_ShowString(60,265,200,24,24,dtbuf);
//			
//			
//			sprintf((char *)dtbuf,"%f       ",Kp);
//			LCD_ShowString(100,295,120,24,24,dtbuf); 
//			delay_ms(1000);	
//			mode_key=1;
//		}
//		
//		if((GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0)&&(mode_key==1))//再次按键之后用K计算电压电流功率
//		{
//			POINT_COLOR=BLUE;
//			LCD_ShowString(50,10,130,24,24,"  PowerEnergyCount         "); 
//			mode_key=0;
////			RN8209_PowerEnergyCount();

//			sprintf((char *)dtbuf,"%x       ",TempU); 
//			LCD_ShowString(100,150,120,24,24,dtbuf); 
//			
//			sprintf((char *)dtbuf,"%x      ",TempIA); 
//			LCD_ShowString(100,180,120,24,24,dtbuf); 
//			
//			sprintf((char *)dtbuf,"%x       ",TempPowerPA); 
//			LCD_ShowString(100,210,120,24,24,dtbuf); 
//			delay_ms(1000);		
//		}
		POINT_COLOR=BLUE;
		LCD_ShowString(50,10,130,24,24,"present value"); //一刚开始，读取寄存器的值
		
		RN8209_ReadData(0x7F);//ID
		
//		RN8209_KZ( ); //首先是测量空载
//		RMSIAreg=RN8209_ReadData(IARMS);//电流A通道有效值
//	  PowerPAreg=RN8209_ReadData(PowerPA);//有功功率A通道
//	  RMSUreg=RN8209_ReadData(URMS);//电压通道有效值
		

	  RMSUreg=RN8209_ReadData(URMS);//电压通道有效值			
		sprintf((char *)dtbuf,"%x       ",RMSUreg);//把数字转换成字符串，格式要匹配
		LCD_ShowString(140,40,130,24,24,dtbuf); 
		
		RMSIAreg=RN8209_ReadData(IARMS);//电流A通道有效值
		sprintf((char *)dtbuf,"%x       ",RMSIAreg);
		LCD_ShowString(140,70,130,24,24,dtbuf); 
		
		PowerPAreg=RN8209_ReadData(PowerPA);//有功功率A通道
		sprintf((char *)dtbuf,"%x       ",PowerPAreg);
		LCD_ShowString(140,100,130,24,24,dtbuf); 
		
		
		RN8209_PowerEnergyCount();
		sprintf((char *)dtbuf,"%x     ",TempIA); 
		LCD_ShowString(100,180,120,24,24,dtbuf); 
//		delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);
//		delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);		


		LCD_ShowString(20,240,60,24,24,"U_adc"); //先不用，先显示ADC的值
  	SPL_U_reg=RN8209_ReadData(SPL_U);
		sprintf((char *)dtbuf,"%x       ",SPL_U_reg);
		LCD_ShowString(140,240,60,24,24,dtbuf);

		LCD_ShowString(20,265,60,24,24,"IA_adc:"); //先显示ADC的值
		SPL_IA_reg=RN8209_ReadData(SPL_IA);//读电流通道A的ADC
		sprintf((char *)dtbuf,"%x       ",SPL_IA_reg);
		LCD_ShowString(140,265,60,24,24,dtbuf);
		delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);
		delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);delay_ms(50);

		

		
//SPL_IA_reg		
		
//		RN8209_ReadData(0x00,&data);//读系统寄存器
//		delay_ms(10);
//		RN8209_ReadData(0x02,&data);//脉冲频率
//		delay_ms(10);
//		RN8209_ReadData(0x0E,&data);//读电流通道A的有效值校正补偿
////		delay_ms(10);
////		RN8209_ReadData(0x20,&data);//有功脉冲计数
//      delay_ms(10);
//		RN8209_ReadData(0x22,&data);//电流A通道有效值
//		delay_ms(10);        ·····
//		RN8209_ReadData(0x24);//电压通道有效值
//		delay_ms(5);
//		delay_ms(10);
//		RN8209_ReadData(0x25,&data);//读电压频率寄存器
//		delay_ms(10);
//		RN8209_ReadData(0x26,&data);//有功功率A通道
//		delay_ms(10);
//		RN8209_ReadData(0x29,&data);//读有功能量
//		delay_ms(10);
//		RN8209_ReadData(0x30,&data);//读电流通道A的ADC
//		delay_ms(10);
//		RN8209_ReadData(0x32,&data);//读电压通道的ADC
//		delay_ms(10);
//		RN8209_ReadData(0x2b,&data);
//		 delay_ms(10);
//		RN8209_ReadData(0x2c,&data);
//		delay_ms(10);
//		RN8209_ReadData(0x35,&data);//读电压频率寄存器2

		
//		RN8209_readIA_RMS( ); 


	


//	  data_r[0]=0;//下次来读的时候，就先清零
//		data_r[1]=0;
//		data_r[2]=0;
//		data_r[3]=0;
////		RN8209_ReadData(0x7F,data_r);//ID
////		RN8209_ReadData(0x22,data_r);//电流A通道有效值
////		delay_ms(200);
////		printf("   %x \r\n",*data_r); //显示ID		
//		data_r[0]=0;//下次来读的时候，就先清零
//		data_r[1]=0;
//		data_r[2]=0;
//		data_r[3]=0;
		
		

	}
}
 








