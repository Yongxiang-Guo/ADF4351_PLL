#include "ADF4351.h"
#include "delay.h"
#include "lcd.h"

//////////////////////////////////
//连线说明：
//CLK----------PB15
//DATA---------PB14
//LE-----------PB13
//CE-----------PB12
void ADF_Output_GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
}
 
void ADF_Input_GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP; 
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING ; 
	GPIO_Init(GPIOB, &GPIO_InitStruct);	
}

void delay (int length)
{
	while (length >0)
    	length--;
}

void WriteToADF4351(u8 count, u8 *buf)
{
	u8 ValueToWrite = 0;
	u8 i = 0;
	u8 j = 0;
	
//ADF_Output_GPIOInit();
	
	ADF4351_CE = 1;
	delay_us(1);
	ADF4351_CLK = 0;
	ADF4351_LE = 0;
	delay_us(1);
	
	for(i = count; i>0; i--)
	{
		ValueToWrite = *(buf+i-1);
		for(j=0; j<8; j++)
		{
			if(0x80 == (ValueToWrite & 0x80))		//首位为1
			{
				ADF4351_OUTPUT_DATA = 1;
			}
			else
			{
				ADF4351_OUTPUT_DATA = 0;
			}
			delay_us(1);
			ADF4351_CLK = 1;
			delay_us(1);
			ValueToWrite <<= 1;
			ADF4351_CLK = 0;	
		}
	}
	ADF4351_OUTPUT_DATA = 0;
	delay_us(1);
	ADF4351_LE = 1;
	delay_us(1);
	ADF4351_LE = 0;
}


void ReadToADF4351(u8 count, u8 *buf)
{
	u8 i = 0;
	u8 j = 0;
	u8 iTemp = 0;
	u8 RotateData = 0;
	
	ADF_Input_GPIOInit();
	ADF4351_CE = 1;
	delay_us(1);
	ADF4351_CLK = 0;
	ADF4351_LE = 0;
	delay_us(1);
	
	for(i = count; i>0; i--)
	{
		for(j = 0; j<8; j++)
		{
			RotateData <<=1;
			delay_us(1);
			iTemp = ADF4351_INPUT_DATA;
			ADF4351_CLK = 1;
			if(0x01 == (iTemp&0x01))
			{
				RotateData |= 1;
			}
			delay_us(1);
			ADF4351_CLK = 0;
		}
		*(buf+i-1) = RotateData;
	}
	delay_us(1);
	ADF4351_LE = 1;
	delay_us(1);
	ADF4351_LE = 0;
}


void ADF4351Init(void)
{
	u8 buf[4] = {0,0,0,0};
	
	ADF_Output_GPIOInit();
	
	buf[3] = 0x00;				
	buf[2] = 0x58;
	buf[1] = 0x00;				//write communication register 0x00580005 to control the progress 
 	buf[0] = 0x05;				//to write Register 5 to set digital lock detector
	WriteToADF4351(4,buf);		

	//////////////////////////////////////控制VCO输出分频系数——32
	buf[3] = 0x00;
	buf[2] = 0xDC;				//(DB23=1)The signal is taken from the VCO directly;(DB22-20:4H)the RF divider is 4;(DB19-12:50H)BAND SELECT CLOCK DIVIDER VALUE is 200
	buf[1] = 0x80;				//(DB11=0)VCO powerd up;(DB10=0)MUTE DISABLED;
 	buf[0] = 0x3C;				//(DB5=1)RF output is enabled;(DB4-3=3H)Output power level is 5dBm
	WriteToADF4351(4,buf);		
  //////////////////////////////////////
	
	buf[3] = 0x00;
	buf[2] = 0x00;				//(DB16-15:00)CLOCK DIVIDER OFF
	buf[1] = 0x04;				//(DB14-3:96H)clock divider value is 150.
 	buf[0] = 0xB3;
	WriteToADF4351(4,buf);	

  //////////////////////////////////////控制R分频器值——4
	buf[3] = 0x00;
	buf[2] = 0x01;				//(DB6=1)set PD polarity is positive;(DB7=1)LDP is 6nS;
	buf[1] = 0x0E;				//(DB8=0)enable fractional-N digital lock detect;
 	buf[0] = 0x42;				//(DB12-9:7H)set Icp 2.50 mA;
	WriteToADF4351(4,buf);		//(DB23-14:1H)R counter is 25
  //////////////////////////////////////
	
	//////////////////////////////////////控制MOD值——125
	buf[3] = 0x00;          
	buf[2] = 0x00;          //(DB14-3:6H)MOD counter is 5;
	buf[1] = 0x83;			    //(DB26-15:6H)PHASE word is 1,neither the phase resync 
 	buf[0] = 0xe9;			    //nor the spurious optimization functions are being used
	WriteToADF4351(4,buf);  //(DB27=0)prescaler value is 4/5											 
  //////////////////////////////////////
	
	//////////////////////////////////////控制INT和FRAC值——115、25	对应频率90MHz
	buf[3] = 0x00;
	buf[2] = 0x39;
	buf[1] = 0x80;
 	buf[0] = 0xc8;				 //(DB14-3:0H)FRAC value is 0;
	WriteToADF4351(4,buf); //(DB30-15:140H)INT value is 100;	
	//////////////////////////////////////
}

//输出给定频率信号
//输入参数：频率值————90MHz~110MHz，单位100KHz
void ADF4351WriteFreq(u16 freq)
{
	u32 R0 = 0;					//寄存器R0控制值
	u16 INT = 0;				//N Count INT
	u16 FRAC = 0;				//N Count FRAC
	u8 buf[4] = {0,0,0,0};
	
	////////////////////////固定值
	u16 MOD = 125;			//MOD
	u16 RFIN = 100;			//RFIN 100MHz
	u16 R_DIV = 4;			//R Count div
	u16 CLK_DIV = 32;		//FOUT div
	////////////////////////
	
	//求解对应R0控制字
	INT = freq * CLK_DIV / (RFIN / R_DIV) / 10;
	FRAC = ((freq * CLK_DIV) % ((RFIN / R_DIV) * 10)) * MOD / ((RFIN / R_DIV) * 10); 
	R0 = (INT << 15) + (FRAC << 3);
	//LCD_ShowxNum(100,400,INT,10,24,0);
	//LCD_ShowxNum(100,450,FRAC,10,24,0);		//已检验INT、FRAC计算正确
	
	/////////////////////////写寄存器:必须按照R5~R0重新配置所有寄存器
	ADF_Output_GPIOInit();
	
	buf[3] = 0x00;				
	buf[2] = 0x58;
	buf[1] = 0x00;				//write communication register 0x00580005 to control the progress 
 	buf[0] = 0x05;				//to write Register 5 to set digital lock detector
	WriteToADF4351(4,buf);		

	//////////////////////////////////////控制VCO输出分频系数——32
	buf[3] = 0x00;
	buf[2] = 0xDC;				//(DB23=1)The signal is taken from the VCO directly;(DB22-20:4H)the RF divider is 4;(DB19-12:50H)BAND SELECT CLOCK DIVIDER VALUE is 200
	buf[1] = 0x80;				//(DB11=0)VCO powerd up;(DB10=0)MUTE DISABLED;
 	buf[0] = 0x3C;				//(DB5=1)RF output is enabled;(DB4-3=3H)Output power level is 5dBm
	WriteToADF4351(4,buf);		
  //////////////////////////////////////
	
	buf[3] = 0x00;
	buf[2] = 0x00;				//(DB16-15:00)CLOCK DIVIDER OFF
	buf[1] = 0x04;				//(DB14-3:96H)clock divider value is 150.
 	buf[0] = 0xB3;
	WriteToADF4351(4,buf);	

  //////////////////////////////////////控制R分频器值——4
	buf[3] = 0x00;
	buf[2] = 0x01;				//(DB6=1)set PD polarity is positive;(DB7=1)LDP is 6nS;
	buf[1] = 0x0E;				//(DB8=0)enable fractional-N digital lock detect;
 	buf[0] = 0x42;				//(DB12-9:7H)set Icp 2.50 mA;
	WriteToADF4351(4,buf);		//(DB23-14:1H)R counter is 25
  //////////////////////////////////////
	
	//////////////////////////////////////控制MOD值——125
	buf[3] = 0x00;          
	buf[2] = 0x00;          //(DB14-3:6H)MOD counter is 5;
	buf[1] = 0x83;			    //(DB26-15:6H)PHASE word is 1,neither the phase resync 
 	buf[0] = 0xe9;			    //nor the spurious optimization functions are being used
	WriteToADF4351(4,buf);  //(DB27=0)prescaler value is 4/5											 
  //////////////////////////////////////
	
	//////////////////////////////////////写入R0寄存器
	buf[3] = R0 >> 24;
	buf[2] = (R0 & 0x00ff0000) >> 16;
	buf[1] = (R0 & 0x0000ff00) >> 8;
 	buf[0] = R0 & 0x000000ff ;				 
	WriteToADF4351(4,buf); 
	//////////////////////////////////////
	
	//LCD_ShowxNum(200,100,freq/10,3,24,0);
	//LCD_ShowString(236,100,16,24,24,".");
	//LCD_ShowxNum(248,100,freq%10,1,24,0);
	//LCD_ShowString(280,100,56,24,24,"MHz");
}

