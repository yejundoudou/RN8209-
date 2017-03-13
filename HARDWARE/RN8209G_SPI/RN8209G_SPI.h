#ifndef __SPI_H
#define __SPI_H
#include "sys.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
////设置片选信号为高
//);//设置片选信号为低	
//
////设置时钟信号为高
//);//设置时钟信号为低	
//
///设置从输出信号为高
//;//设置从输出信号为低	
//
///设置从输入信号为高	
//设置从输入信号为低	
////////////////////////////////////////////////////////////////////////////////
  				  	    													
void RN8209G_SPI_config(void);
//u32 RN8209_ReadData(u8 address,u32 *data_r);
u32 RN8209_ReadData(u8 address);
void RN8209_WriteData(u8 address,u32 order);    
void RN8209_Parameter_Adjust(void);
void RN8209_ActivePower_Adjust(void);
void RN8209_ReactivePower_Adjust(void);
void RN8209_Rms_Adjust(void);
void RN8209_Count_Kx(void );
void RN8209_DC_Config(void );
void RN8209_DC_Adjust(void );
u32 RN8209_readIA_RMS(void );
void RN8209_KZ(void);


#define SCSN_H GPIO_SetBits(GPIOB, GPIO_Pin_12);//设置片选信号为高
#define SCSN_L GPIO_ResetBits(GPIOB, GPIO_Pin_12);//设置片选信号为低	

#define SCLK_H GPIO_SetBits(GPIOB, GPIO_Pin_13);//设置时钟信号为高
#define SCLK_L GPIO_ResetBits(GPIOB, GPIO_Pin_13);//设置时钟信号为低	

#define SDO  GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14);//设置从输出信号为高
	
#define SDI_H GPIO_SetBits(GPIOB, GPIO_Pin_15);//设置从输入信号为高
#define SDI_L GPIO_ResetBits(GPIOB, GPIO_Pin_15);//设置从输入信号为低	

//----------------------start RN8209  地址定义---------------------------------------------------//
//----------------------已经审核，具有特殊命令2016.10.12-----------------------------------------//
#define					SYSCON        0x00 
#define        	EMUCON 				0x01
#define        	HFConst     	0x02 
#define        	PStart      	0x03 
#define        	DStart      	0x04 
#define					GPQA        	0x05 
#define        	GPQB        	0x06 
#define        	PhsA        	0x07 
#define        	PhsB        	0x08
#define					QPhsCal				0x09    
#define					APOSA 				0x0a
#define        	APOSB 				0x0b
#define        	RPOSA 				0x0c
#define        	RPOSB 				0x0d
#define        	IARMSOS     	0x0e
#define        	IBRMSOS     	0x0f
#define        	IBGain      	0x10
#define					D2FPL       	0x11
#define        	D2FPH       	0x12
#define        	DCIAH       	0x13
#define        	DCIBH       	0x14
#define         DCUH					0x15   
#define         DCL   				0x16 
#define         EMUCON2				0x17

#define					PFCnt    			0x20
#define        	DFcnt    			0x21
#define        	IARMS       	0x22
#define        	IBRMS       	0x23
#define        	URMS        	0x24
#define					UFreq       	0x25
#define        	PowerPA     	0x26
#define        	PowerPB     	0x27
#define        	PowerQ     	  0x28
#define         EnergyP  			0x29
#define         EnergyP2 			0x2a
#define         EnergyD  			0x2b
#define         EnergyD2    	0x2c
#define         EMUStatus   	0x2d
#define         SPL_IA      	0x30
#define         SPL_IB      	0x31
#define         SPL_U       	0x32

#define         IE  					0x40
#define         IF  					0x41
#define         RIF  			 		0x42

#define         SysStatus  		0x43
#define         RData      		0x44
#define         WData      		0x45
#define         DeviceID   		0x7f

#define         WriteEnAdd   	0xea
#define         WriteEnData   0xe5//写使能

#define         WriteDisAdd   0xea
#define         WriteDisData  0xdc

#define         SelectIaAdd   0xea
#define         SelectIaData  0x5a//选择电流通道A

#define         SelectIbAdd   0xea
#define         SelectIbData  0xa5

#define         SoftRestAdd   0xea
#define         SoftRestData  0xfa//软件复位，等同硬件
//----------------------end RN8209  地址定义----------------------------------------------//

//-------------------------start 校表参数文件---------------------------------------------//
typedef struct 
{		
	u16     AdjustSYSCON;
	u16			AdjustEMUCON;
	u16			AdjustHFConst;
	u16			AdjustPStart;	
	u16			AdjustQStart;  			//10
	u16			AdjustGPQA;    	
	u16			AdjustGPQB;    	
	u16			AdjustIAGain;  	
	u16			AdjustUGain;   	
	u16			AdjustIBGain;  	
	u16			AdjustPhsA;	   	
	u16			AdjustPhsB;    	
	u16			AdjustQPhsCal; 			//22
	u16			AdjustAPOSA;   	
	u16			AdjustAPOSB;	 	
	u16			AdjustRPOSA;   	
	u16			AdjustRPOSB;   	
	u16			AdjustIARMSOS; 			//32
	u16			AdjustIBRMSOS;			//34
	u16			AdjustDCIAH;
	u16			AdjustDCIBH;
	u16			AdjustDCUH;
	u16			AdjustDCL;
	u16			AdjustEMUCON2; 	
	float		AdjustKUrms;								//电压系数
	float		AdjustKIArms;								// A通道电流系数
	float		AdjustKIBrms;								// B通道电流系数
	float		AdjustKPrms;								// 功率系数
//	u16		Adjust	RealUI[2];						// 
//	u32		Adjust	RealPw;								// ?????,????????? 0.2%
	u32			AdjustChkSum;             	
	u16			AdjustRTCDota0;							// RTC校正寄存器
	u8			AdjustTemperAdj[2];					// 高低温补偿值
//	u8		Adjust	RTCAdj[4];						// RTC
//	u8		Adjust	CurAdj;								// ???????
	u8 			AdjustOfsetAdjAcVolt[2]; 		//根据电压调整offset的值
//	u16			CorrectionTemper;  		//
}Adjust_Parameter_TypeDef;		//58 Byte
//-------------------------end 校表参数文件---------------------------------------------//

//----------------------------------------start测试参数定义----------------------------------------//
extern u32 RMSIAreg;
extern u32 RMSIBreg;
extern u32 RMSUreg;//有效值寄存器，暂时放在外边
extern u32 PowerPAreg;
extern u32 RMSIAreg_adj;
extern u32 RMSIBreg_adj;
extern u32 RMSUreg_adj;//校表的时候，有效值寄存器，空载，暂时放在外边
extern u32 PowerAreg_adj;
extern u32 TempU;
extern u32 TempIA;
extern u32 TempPowerPA;
extern float err_PowerA;
extern float err_Phase_A;
extern float err_reactive_A;
extern u8 angle_reactive_A;
extern float KiA;
extern float KiB;
extern float Ku;
extern float Kp;
//----------------------------------------end测试参数定义----------------------------------------//

#endif

