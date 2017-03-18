#include "RN8209G_SPI.h"
#include "delay.h" 
#include "math.h"
#include "usart.h"
//////////////////////////////////////////////////////////////////////////////////	 
// 1 .高低电平的持续时间为1毫秒，频率为500hz,62.5字节每秒
// 1 .写地址和读命令是间隔3毫秒，手册说是大于半个时钟周期就行
// 1 .
// 1 .SCSN  ------B12
//    SCLK--------B13
// 1 .SDO --------B14
// 1 .SDI --------B15

//////////////////////////////////////////////////////////////////////////////////
u8 Vu=0;   //额定电压输入时，电压通道的电压=引脚电压*放大倍数
u8 Vi=0;	
u8 Un=220;		//额定输入的电压   ，校表时修改
float Ib=8;		//额定输入的电流   ，校表时修改
unsigned int EC=3200;
u32 RMSIAreg=0;
u32 RMSIBreg=0;
u32 RMSUreg=0;//有效值寄存器，暂时放在外边
u32 PowerPAreg=0;

u32 RMSIAreg_adj=0;
u32 RMSIBreg_adj=0;
u32 RMSUreg_adj=0;//校表的时候，有效值寄存器，空载，暂时放在外边
u32 PowerAreg_adj=0;

u32 TempU=0;
u32 TempIA=0;
u32 TempPowerPA=0;

float err_PowerA=0;
float err_Phase_A=0;
float err_reactive_A=0;
u8 angle_reactive_A=0;
float KiA=0;
float KiB=0;
float Ku=0;
float Kp=0;

extern u32 data_r[4];

Adjust_Parameter_TypeDef Adjust_Parameter_Structure;
 
//以下是SPI模块的初始化代码，配置成主机模式		  
//SPI口初始化
//这里针是对SPI2的初始化
void RN8209G_SPI_config(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );//PORTB时钟使能  	
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 |GPIO_Pin_13 | GPIO_Pin_15 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;  //PB13/14/15复用推挽输出 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOE2,3,4

	GPIO_SetBits(GPIOB,GPIO_Pin_12|GPIO_Pin_15);  //PB13/14/15上拉
	GPIO_ResetBits(GPIOB,GPIO_Pin_13);

}   




//***********************************************************************
//*///// 函数名称: u8 RN8209_ReadData(u8 add,u8 *data,u8 *len) 
//*///// 函数功能: 读取8209寄存器数据 
//*///// 输入参数: add-8209寄存器地址 *data-读取后放置的缓冲区 *len-数据长度 
//*///// 输出参数: 0-读取失败 1-读取成功 
//**************************2016.10.13审核*********************************************/
//u32 RN8209_ReadData(u8 address,u32 *data)
u32 RN8209_ReadData(u8 address)
{
	u8 i;
	u32 data=0;
//	u8 return_s=1;
	u8 address_s=address;	
//	Adjust_Parameter_TypeDef    Adjust_Parameter_Structure;
//	u8 data8;	
//	data=0;//下次来读的时候，就先清零
	SCLK_L;	
	SCSN_L;
	delay_ms(1);	 
//	SCLK_H;    
	for(i=0;i<8;i++)/*发送操作码和地址*/  
   {
	   SCLK_H;//主机在高电平写命令字节
     if((address_s&0x80)==0x80)  
	    {	
        SDI_H  
	    }
      else
	    {	
        SDI_L;
	    }
	   delay_ms(1);//然后等1毫秒，等着从机读取。
     SCLK_L;//从设备下降沿接收数据，这是读取数据的正中央  
	   delay_ms(1);
     address_s<<= 1; //-------这里已经改变了变量的值，下面又要调用，就不行---------------------------
   } 
	 SDI_L; //读取完之后，数据保持低电平 
   delay_ms(1);//间隔时间t1应该大于半个时钟周期。1+1	 
	 
/*------------------下面是产生接收的时钟脉冲------------------*/
	switch(address)
	{
		case 0x07:	 
		case 0x08:	
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43://-------------------------------------------寄存器长度为1-------------------------//
		for(i=0;i<8;i++)
		 {
			 SCLK_H;				//从机在高电平将数据输出
		   delay_ms(1); 	//等待从机放好数据
		   SCLK_L;				//一个下降沿后，数据已经准备好了
		   if((GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14)))//问题：为什么直接用SDO 不行呢?
		    {
					data<<=1;
		    	data|=0x00000001;					
		    }
		   else
		    {
					data<<=1;
		    	data&=0xFFFFFFFE;				
		    }	
				delay_ms(1);
		 }
		delay_ms(1);
		SCSN_H;
		delay_ms(2);
		return(data);
		break;
	
		case 0x00:
		case 0x01:
		case 0x02:	
		case 0x03:	
		case 0x04:	
		case 0x05:
		case 0x06:
		case 0x09:	//老版本			
		case 0x0A:	
		case 0x0B:	
		case 0x0C:	//老版本
		case 0x0D:	//老版本
		case 0x0E:	
		case 0x0F:	
		case 0x10:	
		case 0x11:	//新版添加
		case 0x12:	//新版添加
		case 0x13:	//新版添加
		case 0x14:	//新版添加
		case 0x15:	//新版添加
		case 0x16:  //新版添加
		case 0x17:	//新版添加
		case 0x20: 
		case 0x21:
		case 0x25:	
		case 0x45://----------------------------------------寄存器长度为2-------------------------------//
		for(i=0;i<16;i++)
		 {
			 SCLK_H;				//从机在高电平将数据输出
		   delay_ms(1); 	//等待从机放好数据
		   SCLK_L;				//一个下降沿后，数据已经准备好了
		   if((GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14)))//问题：为什么直接用SDO 不行呢?
		    {
					data<<=1;
		    	data|=0x00000001;					
		    }
		   else
		    {
					data<<=1;
		    	data&=0xFFFFFFFE;				
		    }	
				delay_ms(1);
		 }
		delay_ms(1);
		SCSN_H;
		delay_ms(2);
		return(data);
		break;
		
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x29:	
		case 0x2A:	
		case 0x2B:
		case 0x2C:	
		case 0x2D:
		case 0x30:
		case 0x31:	
		case 0x32:  //新版添加	
		case 0x35:
    case 0x7f://-----------------------------------------长度为3个字-------------------------------//
		for(i=0;i<24;i++)
		 {
			 SCLK_H;				//从机在高电平将数据输出
		   delay_ms(1); 	//等待从机放好数据
		   SCLK_L;				//一个下降沿后，数据已经准备好了
		   if((GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14)))//问题：为什么直接用SDO 不行呢?
		    {
					data<<=1;
		    	data|=0x00000001;					
		    }
		   else
		    {
					data<<=1;
		    	data&=0xFFFFFFFE;				
		    }	
				delay_ms(1);
		 }
		delay_ms(1);
		SCSN_H;
		delay_ms(2);
		break;

		case 0x26 :	
		case 0x27 :	 
		case 0x28 :	 //老版本
		case 0x44 ://---------------------------------------长度为4个字节--------------------------//
		for(i=0;i<32;i++)
		 {
			 SCLK_H;				//从机在高电平将数据输出
		   delay_ms(1); 	//等待从机放好数据
		   SCLK_L;				//一个下降沿后，数据已经准备好了
		   if((GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14)))//问题：为什么直接用SDO 不行呢?
		    {
					data<<=1;
		    	data|=0x00000001;					
		    }
		   else
		    {
					data<<=1;
		    	data&=0xFFFFFFFE;				
		    }	
				delay_ms(1);
		 }
		delay_ms(1);
		SCSN_H;
		delay_ms(2);
		return(data);	
		break;
		 
		default :
						return(data);	

	}
	 delay_ms(2);
	 SCSN_H;
	 return(data);	

}


/***********************************************************************
//函数名称: void RN8209_WriteData(u8 *ptr)  
//函数功能: 写入数据到8209寄存器中 
//输入参数: *ptr-指向要写入的数据缓冲区  
//输出参数: 无  
//地址的8位，最高位0——读；1--写
***********************************************************************/
void RN8209_WriteData(u8 address,u32 order)    
{
	u8 i;
	u32 res;
//	u8 return_s=1;
	u8 address_s=address;
	if(address_s!=0xEA)//地址的8位，最高位0——读；1--写
	 {
	 	 address_s|=0x80;
	 }
	 SCLK_L;
	 SCSN_L; 
	 delay_ms(1);	
	 for(i=0;i<8;i++)/*发送操作码和地址*/  
    {
		  SCLK_H;//主机在高电平写命令字节
      if((address_s&0x80)==0x80)  
		   {
         SDI_H  
		   }
      else
		   {
         SDI_L;
		   }
		   delay_ms(1);
       SCLK_L;/*从设备下降沿接收数据*/  
		   delay_ms(1);
       address_s<<= 1; //-------这里已经改变了变量的值，下面又要调用，就不行---------------------------	
    } 
    delay_ms(1);//--下面是产生接收的时钟脉冲---//
	switch(address)
	{
		case 0xEA:  //-------------------------------------特殊命令寄存器--------------------------------//
			
		case 0x07:	 
		case 0x08:	
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43://-------------------------------------------寄存器长度为1---------------------------//
		for(i=0;i<8;i++)/*发送操作码和地址*/  //命令的字节不同，循环次数不同
		 {
		 	 SCLK_H;//主机在高电平写命令字节
		 	 if((order&0x80)==0x80)  //命令的字节不同，相与的位数不同
		 	  {
		 	  	SDI_H  
		 	  }
		 	 else
		 	  {
		 	  	SDI_L;
		 	  }
		 	 delay_ms(1);
		 	 SCLK_L;/*从设备下降沿接收数据*/  
		 	 delay_ms(1);
		 	 order<<= 1;
		 } 
		 delay_ms(1);
//		 res=RN8209_ReadData(address);//回调函数
		 printf("   %x \r\n",res); //显示ID
		break;
		
		case 0x00:
		case 0x01:
		case 0x02:	
		case 0x03:	
		case 0x04:	
		case 0x05:
		case 0x06:
		case 0x09:	//老版本			
		case 0x0A:	
		case 0x0B:	
		case 0x0C:	//老版本
		case 0x0D:	//老版本
		case 0x0E:	
		case 0x0F:	
		case 0x10:	
		case 0x11:	//新版添加
		case 0x12:	//新版添加
		case 0x13:	//新版添加
		case 0x14:	//新版添加
		case 0x15:	//新版添加
		case 0x16:  //新版添加
		case 0x17:	//新版添加
		case 0x20: 
		case 0x21:
		case 0x25:	
		case 0x45://------------------------------------------寄存器长度为2---------------------------//
		for(i=0;i<16;i++)/*发送操作码和地址*/  
		 {
		 	 SCLK_H;//主机在高电平写命令字节
		 	 if((order&0x8000)==0x8000)//命令的字节不同，相与的位数不同  
		 	  {
		 	  	SDI_H  
		 	  }
		 	 else
		 	  {
		 	  	SDI_L;
		 	  }
		 	 delay_ms(1);
		 	 SCLK_L;/*从设备下降沿接收数据*/  
		 	 delay_ms(1);
		 	 order<<= 1;
		 } 
		 delay_ms(1);
//		 res=RN8209_ReadData(address);//回调函数
		 printf("   %x \r\n",res); //显示ID
		 break;
			
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x29:	
		case 0x2A:	
		case 0x2B:
		case 0x2C:	
		case 0x2D:
		case 0x30:
		case 0x31:	
		case 0x32:  //新版添加	
		case 0x35:
    case 0x7f://------------------------------------------长度为3个字-----------------------------//
		for(i=0;i<24;i++)/*发送操作码和地址*/  
		 {
		 	 SCLK_H;//主机在高电平写命令字节
		 	 if((order&0x800000)==0x800000)  
		 	  {
		 	  	SDI_H  
		 	  }
		 	 else
		 	  {
		 	  	SDI_L;
		 	  }
		 	 delay_ms(1);
		 	 SCLK_L;/*从设备下降沿接收数据*/  
		 	 delay_ms(1);
		 	 order<<= 1;
		 } 
		 delay_ms(1);
//		 res=RN8209_ReadData(address);//回调函数
		 printf("   %x \r\n",res); //显示ID
		break;	
		
		case 0x26 :	 
		case 0x27 :	 
		case 0x28 :	 //老版本
		case 0x44 ://----------------------------------长度为4个字节-------------------------//
		for(i=0;i<32;i++)/*发送操作码和地址*/  
		 {
		 	 SCLK_H;//主机在高电平写命令字节
		 	 if((order&0x80000000)==0x80000000)  
		 	  {
		 	  	SDI_H  
		 	  }
		 	 else
		 	  {
		 	  	SDI_L;
		 	  }
		 	 delay_ms(1);
		 	 SCLK_L;/*从设备下降沿接收数据*/  
		 	 delay_ms(1);
		 	 order<<= 1;
		 } 
		 delay_ms(1);
//		 res=RN8209_ReadData(address);//回调函数
		 printf("   %x \r\n",res); //显示ID
		break;	
		default :
		break;
	}
	delay_ms(2);
	SCSN_H;
	delay_ms(2);
	
}

/*****************************************************************************
//函数名称:RN8209_Parameter_Adjust(void)  
//函数功能:校表第一步——————参数设置。
//输入参数:
//输出参数:
******************************************************************************/
void RN8209_Parameter_Adjust(void)
{
//	float temp_HFConst;
	
	Adjust_Parameter_TypeDef Adjust_Parameter_Structure;	
	RN8209_WriteData(0xEA,0XE5);//写使能
	
//	Adjust_Parameter_Structure.AdjustSYSCON=0x000f;//（000f，关闭电流B，电压增益为4倍，电流A为16倍）
//	Adjust_Parameter_Structure.AdjustSYSCON=0x0000;//（00H，关闭电流B，所有增益为1倍）
	Adjust_Parameter_Structure.AdjustSYSCON=0x000C;//（000c，关闭电流B，电压增益为4倍，电流A为1倍）
	Adjust_Parameter_Structure.AdjustEMUCON=0x1403;//（1403，电能读后清零，只累加正向，打开全部高通滤波器）
//	Adjust_Parameter_Structure.AdjustHFConst=0X0B02;//这个是与PFCNT寄存器做比较，5A
	Adjust_Parameter_Structure.AdjustHFConst=0X06E1;//这个是与PFCNT寄存器做比较,8A
//	Adjust_Parameter_Structure.AdjustHFConst=(int)(16.1079*Vu*Vi*(10^11)/(EC*Un*Ib));
	Adjust_Parameter_Structure.AdjustPStart=0x0060;//这是默认的
	Adjust_Parameter_Structure.AdjustEMUCON2=0x0080;//功率及有效值更新速度为13.982HZ
	
	RN8209_WriteData(SYSCON,Adjust_Parameter_Structure.AdjustSYSCON);//B通道ADCON设置，ADC增益选择
	RN8209_WriteData(EMUCON,Adjust_Parameter_Structure.AdjustEMUCON);//能量累加模式设置	
	RN8209_WriteData(HFConst,Adjust_Parameter_Structure.AdjustHFConst);
	RN8209_WriteData(PStart,Adjust_Parameter_Structure.AdjustPStart);//启动电流设置成默认的
	RN8209_WriteData(EMUCON2,Adjust_Parameter_Structure.AdjustEMUCON2);
}
/*****************************************************************************
//函数名称:RN8209_ActivePower_Adjust(void)  
//函数功能:校表第2.1步——————有功校正
//输入参数:
//输出参数:
******************************************************************************/
void RN8209_ActivePower_Adjust(void)
{
//	float Pgain;
	
	
	RN8209_WriteData(0xEA,0XE5);//写使能
	
	if((-err_PowerA/(1+err_PowerA))>=0)                  //A通道功率增益校正
	{
		Adjust_Parameter_Structure.AdjustGPQA=(int)((-err_PowerA/(1+err_PowerA))*(2^15));
		RN8209_WriteData(GPQA,Adjust_Parameter_Structure.AdjustGPQA);
	}
	else
	{
		Adjust_Parameter_Structure.AdjustGPQA=(int)((2^16)+(-err_PowerA/(1+err_PowerA))*(2^15));
		RN8209_WriteData(GPQA,Adjust_Parameter_Structure.AdjustGPQA);
	}
	
	if((asin(-err_Phase_A/1.732))>=0)										 //A通道相位校正
	{
		Adjust_Parameter_Structure.AdjustPhsA=(int)(((asin(-err_Phase_A/1.732))/0.02));
		RN8209_WriteData(PhsA,Adjust_Parameter_Structure.AdjustPhsA);
	}
	else
	{
		Adjust_Parameter_Structure.AdjustPhsA=(int)(((asin(-err_Phase_A/1.732))/0.02)+(2^8));
		RN8209_WriteData(PhsA,Adjust_Parameter_Structure.AdjustPhsA);                        
	}
	
	
}
/*****************************************************************************
//函数名称:RN8209_ActivePower_Adjust(void)  
//函数功能:校表第2.2步——————无功校正
//输入参数:
//输出参数:
******************************************************************************/
void RN8209_ReactivePower_Adjust(void)
{
	if((err_reactive_A*(tan(angle_reactive_A)))>=0) 
	{
		Adjust_Parameter_Structure.AdjustQPhsCal=(int)((err_reactive_A*(tan(angle_reactive_A)))*(2^15));
		RN8209_WriteData(QPhsCal,Adjust_Parameter_Structure.AdjustQPhsCal);
	}		
	else 
	{
		Adjust_Parameter_Structure.AdjustQPhsCal=(int)((err_reactive_A*(tan(angle_reactive_A)))*(2^15)+(2^16));
		RN8209_WriteData(QPhsCal,Adjust_Parameter_Structure.AdjustQPhsCal);
	}
}

/*****************************************************************************
//函数名称:RN8209_Rms_Adjust(void)  
//函数功能:校表第3步——————有效值校正
//输入参数:
//输出参数:
******************************************************************************/
void RN8209_Rms_Adjust(void)
{
	u32 temp_IARMS=0;
	u32 temp_IARMS_t=0;
//	u32 temp_IARMS_back=0;
//	u32 temp_IARMS_2=0;
//	
//	u32 temp_IARMS_3=0;
//	
//	u32 temp_IARMS_4=0;
	u8 i=0;
	u32 Iave=0;
	u16 temp_IARMSOS=0;

	for(i=0;i<10;i++)
	{

//		temp_IARMS+=RN8209_ReadData(IARMS,&temp_IARMS_back);//也就是A通道电流有效值寄存器0X22
		temp_IARMS_t=RN8209_ReadData(IARMS);//也就是A通道电流有效值寄存器0X22
		temp_IARMS+=temp_IARMS_t;
		delay_ms(5);
//		temp_IARMS_1=RN8209_ReadData(IARMS,&temp_IARMS);
//		delay_ms(5);
//		temp_IARMS_2=RN8209_ReadData(IARMS,&temp_IARMS);
//		delay_ms(5);
//		temp_IARMS_3=RN8209_ReadData(IARMS,&temp_IARMS);
//		delay_ms(5);
//		temp_IARMS_4=RN8209_ReadData(IARMS,&temp_IARMS);

	}
	temp_IARMS=(temp_IARMS/10);
	Iave=(temp_IARMS*temp_IARMS);//变成了加2，有点奇怪哟
	Iave=~Iave;
	temp_IARMSOS=(u16 )(Iave>>8);
//	temp_IARMSOS=0xD80D ;`
	Adjust_Parameter_Structure.AdjustIARMSOS=temp_IARMSOS;
	RN8209_WriteData(IARMSOS,Adjust_Parameter_Structure.AdjustIARMSOS);
}

//*****************************************************************************
//函数名称: void RN8209_Count_Kx(void )  
//函数功能: 从计量芯片中读取数据,并计算粗转换系数
//输入参数: 无
//输出参数: 无
//******************************************************************************/	
void RN8209_Count_Kx(void )  
{  
	RMSIAreg=RN8209_ReadData(IARMS);
	RMSIBreg=RN8209_ReadData(IBRMS);
	RMSUreg=RN8209_ReadData(URMS);
	if(RMSIAreg>0x800000)
	{
		RMSIAreg=0;
	}
	if(RMSIBreg>0x800000)
	{
		RMSIBreg=0;
	}
	if(RMSUreg>0x800000)//24位有符号数，最高位为1时做0处理
	{
		RMSUreg=0;
	}
//	KiA=(float)(((float)Ib)/((float)(RMSIAreg_adj-RMSIAreg)));
	KiA=(float)(0.00007233796);
	KiB=(float)(Ib/(RMSIBreg_adj-RMSIBreg));
	Ku=(float)(Un/(RMSUreg_adj-RMSUreg));
	Kp =(float)((3.22155*(10^12))/((2^32)*(Adjust_Parameter_Structure.AdjustHFConst)*EC));//算死
}

//*****************************************************************************
//函数名称: void RN8209_KZ(void)  
//函数功能: 空载时的寄存器的值
//输入参数: 无
//输出参数: 无
//******************************************************************************/	
void RN8209_KZ(void)  
{
  RMSIAreg_adj=RN8209_ReadData(IARMS);
	RMSIBreg_adj=RN8209_ReadData(IBRMS);
	RMSUreg_adj=RN8209_ReadData(URMS);//读取校准时，空载的值
	PowerAreg_adj=RN8209_ReadData(PowerPA);
}

//*****************************************************************************
//函数名称: void RN8209_PowerEnergyCount(void )  
//函数功能: 从计量芯片读取数据（电压、电流、功率、脉冲数），及电压、电流、频率、功率因素计算
//输入参数: 无
//输出参数: 无
//******************************************************************************/	
void RN8209_PowerEnergyCount(void)
{
	RMSIAreg=RN8209_ReadData(IARMS);
	RMSIBreg=RN8209_ReadData(IBRMS);
	RMSUreg=RN8209_ReadData(URMS);
	PowerPAreg=RN8209_ReadData(PowerPA);
	
	RMSIAreg_adj=0x6c000;
	KiA=(float)(0.00007233796);
//	if(RMSUreg_adj<RMSUreg)//怎么是电压U啦？
//	{
//		TempIA=RMSUreg_adj;//测试用，之前是0
//	}
//	else
//	{
//		TempIA=(float)(KiA*(RMSIAreg_adj-RMSIAreg));//原
//	}
	TempIA=(float)(KiA*(RMSIAreg));
	TempU=(float)(Ku*(RMSUreg_adj-RMSUreg));
	
	TempPowerPA=(float)(Kp*(RMSIBreg_adj-RMSIBreg));
}

//*****************************************************************************
//函数名称: void RN8209_DC_Config(void )  
//函数功能: 配置直流参数
//输入参数: 无
//输出参数: 无
//******************************************************************************/	
void RN8209_DC_Config(void )
{
	Adjust_Parameter_TypeDef Adjust_Parameter_Structure;
	
	Adjust_Parameter_Structure.AdjustSYSCON=0x0000;//SYSCON 中 BIT[1:0]、 BIT[3:2]、 BIT[5:4]写入 0，配置三路 ADC 为 1 倍增益；
	Adjust_Parameter_Structure.AdjustEMUCON=0x57E3;//EMUCON 中将 IA/IB/U 三路的 ADC 的高通使能关闭， BIT[14]、 BIT[6:5]配置为 1；
	
	RN8209_WriteData(SYSCON,Adjust_Parameter_Structure.AdjustSYSCON);
	RN8209_WriteData(EMUCON,Adjust_Parameter_Structure.AdjustEMUCON);
}  
//*****************************************************************************
//函数名称: void RN8209_DC_Adjust(void )  
//函数功能: 直流校正
//输入参数: 无
//输出参数: 无
//******************************************************************************/	
void RN8209_DC_Adjust(void )
{
	u32 temp1_IARMS_DC;
	u32 temp1_URMS_DC;//这两个参数本来是3个字节，但是只有32位的
	u32 temp2_IARMS_DC;
	u32 temp2_URMS_DC;
	u32 temp3_IARMS_DC;
	u32 temp3_URMS_DC;
	u8 i;

	u16 temp_DCIAH;
	u16 temp_DCUH;
	u16 temp_DCL_Ia;
	u16 temp_DCL_U;
	u16 temp_DCL;
	
	u16 temp_DC_IARMSOS;

	for(i=0;i<10;i++)
	{
//			temp1_IARMS_DC+=RN8209_ReadData(IARMS,&temp1_IARMS_DC);
		temp1_IARMS_DC+=RN8209_ReadData(IARMS);
			delay_ms(5);
//			temp1_URMS_DC+=RN8209_ReadData(URMS,&temp1_URMS_DC);
		temp1_URMS_DC+=RN8209_ReadData(URMS);
			delay_ms(5);
	}
	temp1_IARMS_DC=(temp1_IARMS_DC/10);
	temp1_URMS_DC=(temp1_URMS_DC/10);//输入接地，读 IA、 IB、 U 三路的有效值 10 次，计算有效值的平均值 IARMS、 IBRMS、URMS
	
	temp_DCIAH=(u16)(temp1_IARMS_DC>>8);//将 IARMS1 有效值的BIT[23:8]写入 DCIAH 寄存器，ok
	temp_DCL_Ia=(((u16)((temp1_IARMS_DC)&0x000000f0))>>4);////BIT[7:4]写入 DCL 寄存器的BIT[3:0
	

	temp_DCUH=(u16 )(temp1_URMS_DC>>8);//将 URMS1 有效值的 BIT[23:8]写入 DCUH 寄存器， ok
	temp_DCL_U=(((u16)((temp1_URMS_DC)&0x000000f0))<<4);//BIT[7:4]写入 DCL 寄存器的BIT[11:8]
	
	temp_DCL=temp_DCL_U+temp_DCL_Ia;//
	
	Adjust_Parameter_Structure.AdjustDCIAH=temp_DCIAH;
	RN8209_WriteData(DCIAH,Adjust_Parameter_Structure.AdjustDCIAH);//ok
	Adjust_Parameter_Structure.AdjustDCUH=temp_DCUH;
	RN8209_WriteData(DCUH,Adjust_Parameter_Structure.AdjustDCUH);
	Adjust_Parameter_Structure.AdjustDCL=temp_DCL;
	RN8209_WriteData(DCL,Adjust_Parameter_Structure.AdjustDCL);
	
	delay_ms(1000);
	delay_ms(1000);//等待 2S 后读 IA、IB、U 三路的有效值 10 次，计算有效值的平均值 IARMS2、IBRMS2、
	for(i=0;i<10;i++)
	{
//			temp2_IARMS_DC+=RN8209_ReadData(IARMS,&temp2_IARMS_DC);
		temp2_IARMS_DC+=RN8209_ReadData(IARMS);
			delay_ms(5);
//			temp2_URMS_DC+=RN8209_ReadData(URMS,&temp2_URMS_DC);
		 temp2_URMS_DC+=RN8209_ReadData(URMS);
			delay_ms(5);
	}
	temp2_IARMS_DC=(temp2_IARMS_DC/10);
	temp2_URMS_DC=(temp2_URMS_DC/10);
	if((temp2_IARMS_DC>temp1_IARMS_DC)||(temp2_URMS_DC>temp1_URMS_DC))//若有效值相对于未校正前变小，则校正完成，若值变大为原来的约为 2 倍，则需继续进行下一步操作；
	{
		temp3_IARMS_DC=~temp1_IARMS_DC;//将 IARMS1取反得到 IARMS3
		temp3_URMS_DC=~temp1_URMS_DC;
		
		temp_DCIAH=(u16)(temp3_IARMS_DC>>8);//将 IARMS3 有效值的BIT[23:8]写入 DCIAH 寄存器，
	  temp_DCL_Ia=(((u16)((temp3_IARMS_DC)&0x000000f0))>>4);////BIT[7:4]写入 DCL 寄存器的BIT[3:0
	  
	  temp_DCUH=(u16 )(temp3_URMS_DC>>8);//将 URMS3 有效值的 BIT[23:8]写入 DCUH 寄存器，
	  temp_DCL_U=(((u16)((temp3_URMS_DC)&0x000000f0))<<4);//BIT[7:4]写入 DCL 寄存器的BIT[11:8]
	  
	  temp_DCL=temp_DCL_U+temp_DCL_Ia;
	  Adjust_Parameter_Structure.AdjustDCIAH=temp_DCIAH;
	  RN8209_WriteData(DCIAH,Adjust_Parameter_Structure.AdjustDCIAH);
	  Adjust_Parameter_Structure.AdjustDCUH=temp_DCUH;
	  RN8209_WriteData(DCUH,Adjust_Parameter_Structure.AdjustDCUH);
	  Adjust_Parameter_Structure.AdjustDCL=temp_DCL;
	  RN8209_WriteData(DCL,Adjust_Parameter_Structure.AdjustDCL);
	}
		delay_ms(1000);
		delay_ms(1000);//稳定 2s 中,读 IA、IB电流的有效值 10次,求各自平均值进行有效值 offset 校正
		for(i=0;i<10;i++)
		{
//				temp2_IARMS_DC+=RN8209_ReadData(IARMS,&temp2_IARMS_DC);
			  temp2_IARMS_DC+=RN8209_ReadData(IARMS);
				delay_ms(5);	
		}
		temp2_IARMS_DC=(temp2_IARMS_DC/10);
		temp_DC_IARMSOS=(u16)(~((temp2_IARMS_DC^2)/(2^8)));
		Adjust_Parameter_Structure.AdjustIARMSOS=temp_DC_IARMSOS;
		RN8209_WriteData(IARMSOS,Adjust_Parameter_Structure.AdjustIARMSOS);
}
//*****************************************************************************
//函数名称: void RN8209_DC_Count_Kx(void ) 
//函数功能: DC电压、电流、功率转换系数确定
//输入参数: 无
//输出参数: 无
//******************************************************************************/	
//void RN8209_DC_Count_Kx(void ) 
//{
//	
//	
//}

//*****************************************************************************
//函数名称: void RN8209_DC_Count_Kx(void ) 
//函数功能: DC电压、电流、功率转换系数确定
//输入参数: 无
//输出参数: 无
//******************************************************************************/	
u32 RN8209_readIA_RMS(void ) 
{
	*data_r=RN8209_ReadData(0x22);//电流A通道有效值
	return *data_r;
}
