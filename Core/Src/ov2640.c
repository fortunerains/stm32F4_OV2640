//影隐劫(ZGT)
#include <ov2640.h>
#include "tim.h"//引入定时器延时
#include "ov2640cfg.h"
#include "stdio.h"

/*------------------------SCCB底层函数(开始)------------------------*/
void SCCB_Delay_us(unsigned int us)
{
    uint16_t differ=0xffff-us-5;     
    HAL_TIM_Base_Start(&htim7);
    __HAL_TIM_SetCounter(&htim7,differ); 
    while(differ < 0xffff-5) 
    { 
        differ = __HAL_TIM_GetCounter(&htim7); 
    } 
    HAL_TIM_Base_Stop(&htim7);
}
/*******************************************************************************
* 函 数 名       :SCCB_Start()
* 函数功能       :SCCB发送起始信号
* 输    入       : 无
* 输    出       : 无
*******************************************************************************/
void SCCB_Start(void)
{
    SCCB_SDA_1;     //数据线高电平	 
    SCCB_SCL_1;	  //在时钟线高的时候数据线由高至低
    SCCB_Delay_us(50);  
    SCCB_SDA_0;
    SCCB_Delay_us(50); 
    SCCB_SCL_0;	    //数据线恢复低电平，单操作函数必要       
}

/*******************************************************************************
* 函 数 名       :SCCB_Stop()
* 函数功能       :SCCB发送停止信号
* 输    入       : 无
* 输    出       : 无
*******************************************************************************/
void SCCB_Stop(void)
{
    SCCB_SDA_0;
    SCCB_Delay_us(50);	 
    SCCB_SCL_1;	
    SCCB_Delay_us(50); 
    SCCB_SDA_1;	
    SCCB_Delay_us(50);
}  

/*******************************************************************************
* 函 数 名       :SCCB_No_Ack()
* 函数功能       :SCCB产生NA信号，即应答信号
* 输    入       : 无
* 输    出       : 无
*******************************************************************************/
void SCCB_No_Ack(void)
{
	SCCB_Delay_us(50);
	SCCB_SDA_1;	
	SCCB_SCL_1;	
	SCCB_Delay_us(50);
	SCCB_SCL_0;	
	SCCB_Delay_us(50);
	SCCB_SDA_0;	
	SCCB_Delay_us(50);
}
/*******************************************************************************
* 函 数 名       :SCCB_WR_Byte()
* 函数功能       :SCCB写一个字节数据
* 输    入       : dat：需要写入的数据
* 输    出       : 0成功，1失败
*******************************************************************************/
unsigned char SCCB_WR_Byte(unsigned char dat)
{
	unsigned char j,res;	 
	for(j=0;j<8;j++) //循环8次发送数据
	{
		if(dat&0x80)SCCB_SDA_1;	
		else SCCB_SDA_0;
		dat<<=1;
		SCCB_Delay_us(50);
		SCCB_SCL_1;	
		SCCB_Delay_us(50);
		SCCB_SCL_0;		   
	}
    SCCB_SDA_IN();		//设置SDA为输入  
	SCCB_Delay_us(50);
	SCCB_SCL_1;			//接收第九位,以判断是否发送成功
	SCCB_Delay_us(50);
	if(SCCB_SDA_READ())res=1;  //SDA=1发送失败，返回1
	else res=0;         //SDA=0发送成功，返回0
	SCCB_SCL_0;
    SCCB_SDA_OUT();		//设置SDA为输出     
	return res;  
}	
/*******************************************************************************
* 函 数 名       :SCCB_RD_Byte()
* 函数功能       :SCCB读取一个字节的信号
* 输    入       : 无
* 输    出       : 读到的数据
*******************************************************************************/
unsigned char SCCB_RD_Byte(void)
{
	unsigned char temp=0,j;    
    SCCB_SDA_IN();		//设置SDA为输入
	for(j=8;j>0;j--) 	//循环8次接收数据
	{		     	  
		SCCB_Delay_us(50);
		SCCB_SCL_1;
		temp=temp<<1;
		if(SCCB_SDA_READ())temp++;   
		SCCB_Delay_us(50);
		SCCB_SCL_0;
	}
    SCCB_SDA_OUT();		//设置SDA为输出 	
	return temp;
} 
/*------------------------SCCB底层函数(结束)------------------------*/

/*------------------------SCCB应用函数(开始)------------------------*/
/*******************************************************************************
* 函 数 名       :SCCB_WR_Reg()
* 函数功能       :SCCB写入一个字节的数据到寄存器
* 输    入       : reg：寄存器地址；     data：写入的数据
* 输    出       : 成功返回0，失败返回1
*******************************************************************************/
unsigned char SCCB_WR_Reg(unsigned char reg,unsigned char data)
{
	unsigned char res=0;
	SCCB_Start(); 					//启动SCCB传输
	if(SCCB_WR_Byte(SCCB_ID))res=1;	//写器件ID	  
	SCCB_Delay_us(100);
  	if(SCCB_WR_Byte(reg))res=1;		//写寄存器地址	  
	SCCB_Delay_us(100);
  	if(SCCB_WR_Byte(data))res=1; 	//写数据	 
  	SCCB_Stop();	  
  	return	res;
}
/*******************************************************************************
* 函 数 名       :SCCB_RD_Reg()
* 函数功能       :SCCB从寄存器读取一个字节的数据
* 输    入       : reg：寄存器地址
* 输    出       : 返回读取到的数据
*******************************************************************************/
unsigned char SCCB_RD_Reg(unsigned char reg)
{
	unsigned char val=0;
    //设置要读的寄存器
	SCCB_Start();           //启动SCCB传输
    SCCB_WR_Byte(SCCB_ID);  //确定该通信路劲需要联系的设备ID
  	SCCB_Delay_us(100);	  
	SCCB_WR_Byte(reg);//确定需要读的寄存器
    SCCB_Delay_us(100);		
    SCCB_Stop();			  
	SCCB_Delay_us(100);	
    //开始读寄存器内容
    SCCB_Start(); 
	SCCB_WR_Byte(SCCB_ID|0X01);	//发送读命令	
	SCCB_Delay_us(100);	 	
  	val=SCCB_RD_Byte();		 	//读取数据	
  	SCCB_No_Ack();
  	SCCB_Stop();
  	return val;
}
/*******************************************************************************
* 函 数 名       :SCCB_Init()
* 函数功能       :SCCB初始化
* 输    入       :无
* 输    出       :无
*******************************************************************************/
void SCCB_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOD_CLK_ENABLE();//启动GPIOD相关时钟，因为SCCB的引脚我们定义为PD6和PD7
    GPIO_InitStruct.Pin = DCMI_SCL_Pin|DCMI_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    HAL_GPIO_WritePin(DCMI_SCL_GPIO_Port,DCMI_SCL_Pin,GPIO_PIN_SET);
    HAL_GPIO_WritePin(DCMI_SDA_GPIO_Port,DCMI_SDA_Pin,GPIO_PIN_SET);
    SCCB_SDA_OUT();
}
/*------------------------SCCB应用函数(结束)------------------------*/


/*------------------------OV2640应用函数(开始)------------------------*/
/*******************************************************************************
* 函 数 名       :OV2640_Init()
* 函数功能       :初始化OV2640，默认初始化后输出1600*1200图片
* 输    入       : reg：寄存器地址
* 输    出       : 返回读取到的数据
*******************************************************************************/
uint8_t OV2640_Init(void)
{ 
	unsigned short int i=0;
    unsigned short int reg_ID=0;
//    unsigned int reg;
    //配置上电\复位引脚
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOG_CLK_ENABLE();
    GPIO_InitStruct.Pin = OV2640_PWDN_Pin|OV2640_RESET_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    //硬件复位
 	OV2640_PWDN_0;	//POWER ON
	HAL_Delay(10);
	OV2640_RST_0;	//复位OV2640
	HAL_Delay(10);
	OV2640_RST_1;	//结束复位  
    //开始通过SCCB配置OV2640
    SCCB_Init();    //初始化SCCB 的IO口
	SCCB_WR_Reg(OV2640_DSP_RA_DLMT, 0x01);	//操作sensor寄存器
 	SCCB_WR_Reg(OV2640_SENSOR_COM7, 0x80);	//软复位OV2640
	HAL_Delay(50);     

    reg_ID=SCCB_RD_Reg(OV2640_SENSOR_MIDH);	//读取厂家ID 高八位07FA2
	reg_ID<<=8;
	reg_ID|=SCCB_RD_Reg(OV2640_SENSOR_MIDL);	//读取厂家ID 低八位
	if(reg_ID!=OV2640_MID)
	{
		
		return 1;
	}
    printf("MID:%d\r\n",reg_ID);
	reg_ID=SCCB_RD_Reg(OV2640_SENSOR_PIDH);	//读取厂家ID 高八位
	reg_ID<<=8;
	reg_ID|=SCCB_RD_Reg(OV2640_SENSOR_PIDL);	//读取厂家ID 低八位
	if(reg_ID!=OV2640_PID)
	{
		
		return 2;
	} 
    printf("HID:%d\r\n",reg_ID);
 	//初始化 OV2640,采用SXGA分辨率(1600*1200)  
	for(i=0;i<sizeof(ov2640_svga_init_reg_tbl)/2;i++)
	{
	   	SCCB_WR_Reg(ov2640_svga_init_reg_tbl[i][0],ov2640_svga_init_reg_tbl[i][1]);
 	} 
  	return 0x00; 	//ok
}
/*******************************************************************************
* 函 数 名       :OV2640_JPEG_Mode()
* 函数功能       :OV2640切换JPEG模式
* 输    入       : 无
* 输    出       : 无
*******************************************************************************/
void OV2640_JPEG_Mode(void) 
{
	uint16_t i=0;
	//设置:YUV422格式
	for(i=0;i<(sizeof(ov2640_yuv422_reg_tbl)/2);i++)
	{
		SCCB_WR_Reg(ov2640_yuv422_reg_tbl[i][0],ov2640_yuv422_reg_tbl[i][1]); 
	} 
	
	//设置:输出JPEG数据
	for(i=0;i<(sizeof(ov2640_jpeg_reg_tbl)/2);i++)
	{
		SCCB_WR_Reg(ov2640_jpeg_reg_tbl[i][0],ov2640_jpeg_reg_tbl[i][1]);  
	}  
}
/*******************************************************************************
* 函 数 名       :OV2640_RGB565_Mode()
* 函数功能       :OV2640切换REB565模式
* 输    入       : 无
* 输    出       : 无
*******************************************************************************/
void OV2640_RGB565_Mode(void) 
{
	uint16_t i=0;
	//设置:RGB565输出
	for(i=0;i<(sizeof(ov2640_rgb565_reg_tbl)/2);i++)
	{
		SCCB_WR_Reg(ov2640_rgb565_reg_tbl[i][0],ov2640_rgb565_reg_tbl[i][1]); 
	} 
} 
//自动曝光设置参数表,支持5个等级
const static uint8_t OV2640_AUTOEXPOSURE_LEVEL[5][8]=
{
	{
		0xFF,0x01,
		0x24,0x20,
		0x25,0x18,
		0x26,0x60,
	},
	{
		0xFF,0x01,
		0x24,0x34,
		0x25,0x1c,
		0x26,0x00,
	},
	{
		0xFF,0x01,	
		0x24,0x3e,	
		0x25,0x38,
		0x26,0x81,
	},
	{
		0xFF,0x01,
		0x24,0x48,
		0x25,0x40,
		0x26,0x81,
	},
	{
		0xFF,0x01,	
		0x24,0x58,	
		0x25,0x50,	
		0x26,0x92,	
	},
}; 
/*******************************************************************************
* 函 数 名       :OV2640_Auto_Exposure(unsigned char level)
* 函数功能       :OV2640自动曝光等级设置
* 输    入       :曝光等级0~4
* 输    出       : 无
*******************************************************************************/
void OV2640_Auto_Exposure(unsigned char level)
{  
	unsigned char i;
	unsigned char *p=(unsigned char*)OV2640_AUTOEXPOSURE_LEVEL[level];
	for(i=0;i<4;i++)
	{ 
		SCCB_WR_Reg(p[i*2],p[i*2+1]); 
	} 
}
/*******************************************************************************
* 函 数 名       :OV2640_Light_Mode(unsigned char mode)
* 函数功能       :白平衡设置
* 输    入       :0-自动；1-自动；2-太阳；3-阴天；4-室内
* 输    出       : 无
*******************************************************************************/
void OV2640_Light_Mode(unsigned char mode)
{
	unsigned char regccval=0X5E;//Sunny 
	unsigned char regcdval=0X41;
	unsigned char regceval=0X54;
	switch(mode)
	{ 
		case 0://auto 
			SCCB_WR_Reg(0XFF,0X00);	 
			SCCB_WR_Reg(0XC7,0X10);//AWB ON 
			return;  	
		case 2://cloudy
			regccval=0X65;
			regcdval=0X41;
			regceval=0X4F;
			break;	
		case 3://office
			regccval=0X52;
			regcdval=0X41;
			regceval=0X66;
			break;	
		case 4://home
			regccval=0X42;
			regcdval=0X3F;
			regceval=0X71;
			break;	
	}
	SCCB_WR_Reg(0XFF,0X00);	 
	SCCB_WR_Reg(0XC7,0X40);	//AWB OFF 
	SCCB_WR_Reg(0XCC,regccval); 
	SCCB_WR_Reg(0XCD,regcdval); 
	SCCB_WR_Reg(0XCE,regceval);  
}
/*******************************************************************************
* 函 数 名       :OV2640_Color_Saturation(unsigned char sat)
* 函数功能       :色度设置
* 输    入       :0-(-2)；1-(-1)；2-(0)；3-(+1)；4-(+2)
* 输    出       : 无
*******************************************************************************/
void OV2640_Color_Saturation(unsigned char sat)
{ 
	unsigned char reg7dval=((sat+2)<<4)|0X08;
	SCCB_WR_Reg(0XFF,0X00);		
	SCCB_WR_Reg(0X7C,0X00);		
	SCCB_WR_Reg(0X7D,0X02);				
	SCCB_WR_Reg(0X7C,0X03);			
	SCCB_WR_Reg(0X7D,reg7dval);			
	SCCB_WR_Reg(0X7D,reg7dval); 		
}
/*******************************************************************************
* 函 数 名       :OV2640_Brightness(unsigned char bright)
* 函数功能       :亮度设置
* 输    入       :0-(-2)；1-(-1)；2-(0)；3-(+1)；4-(+2)
* 输    出       : 无
*******************************************************************************/
void OV2640_Brightness(unsigned char bright)
{
  SCCB_WR_Reg(0xff, 0x00);
  SCCB_WR_Reg(0x7c, 0x00);
  SCCB_WR_Reg(0x7d, 0x04);
  SCCB_WR_Reg(0x7c, 0x09);
  SCCB_WR_Reg(0x7d, bright<<4); 
  SCCB_WR_Reg(0x7d, 0x00); 
}
/*******************************************************************************
* 函 数 名       :OV2640_Contrast(unsigned char contrast)
* 函数功能       :对比度设置
* 输    入       :0-(-2)；1-(-1)；2-(0)；3-(+1)；4-(+2)
* 输    出       : 无
*******************************************************************************/
void OV2640_Contrast(unsigned char contrast)
{
	unsigned char reg7d0val=0X20;//默认为普通模式
	unsigned char reg7d1val=0X20;
  	switch(contrast)
	{
		case 0://-2
			reg7d0val=0X18;	 	 
			reg7d1val=0X34;	 	 
			break;	
		case 1://-1
			reg7d0val=0X1C;	 	 
			reg7d1val=0X2A;	 	 
			break;	
		case 3://1
			reg7d0val=0X24;	 	 
			reg7d1val=0X16;	 	 
			break;	
		case 4://2
			reg7d0val=0X28;	 	 
			reg7d1val=0X0C;	 	 
			break;	
	}
	SCCB_WR_Reg(0xff,0x00);
	SCCB_WR_Reg(0x7c,0x00);
	SCCB_WR_Reg(0x7d,0x04);
	SCCB_WR_Reg(0x7c,0x07);
	SCCB_WR_Reg(0x7d,0x20);
	SCCB_WR_Reg(0x7d,reg7d0val);
	SCCB_WR_Reg(0x7d,reg7d1val);
	SCCB_WR_Reg(0x7d,0x06);
}
/*******************************************************************************
* 函 数 名       :OV2640_Special_Effects(unsigned char eft)
* 函数功能       :特效设置
* 输    入       :0普通模式；1负片；2黑白；3偏红色；4偏绿色；5偏蓝色；6复古
* 输    出       : 无
*******************************************************************************/   
void OV2640_Special_Effects(unsigned char eft)
{
	unsigned char reg7d0val=0X00;//默认为普通模式
	unsigned char reg7d1val=0X80;
	unsigned char reg7d2val=0X80; 
	switch(eft)
	{
		case 1://负片
			reg7d0val=0X40; 
			break;	
		case 2://黑白
			reg7d0val=0X18; 
			break;	 
		case 3://偏红色
			reg7d0val=0X18; 
			reg7d1val=0X40;
			reg7d2val=0XC0; 
			break;	
		case 4://偏绿色
			reg7d0val=0X18; 
			reg7d1val=0X40;
			reg7d2val=0X40; 
			break;	
		case 5://偏蓝色
			reg7d0val=0X18; 
			reg7d1val=0XA0;
			reg7d2val=0X40; 
			break;	
		case 6://复古
			reg7d0val=0X18; 
			reg7d1val=0X40;
			reg7d2val=0XA6; 
			break;	 
	}
	SCCB_WR_Reg(0xff,0x00);
	SCCB_WR_Reg(0x7c,0x00);
	SCCB_WR_Reg(0x7d,reg7d0val);
	SCCB_WR_Reg(0x7c,0x05);
	SCCB_WR_Reg(0x7d,reg7d1val);
	SCCB_WR_Reg(0x7d,reg7d2val); 
}
/*******************************************************************************
* 函 数 名       :OV2640_Color_Bar(unsigned char sw)
* 函数功能       :彩条测试
* 输    入       :0关闭；1开启；
* 输    出       : 无
*******************************************************************************/  
void OV2640_Color_Bar(unsigned char sw)
{
	unsigned char reg;
	SCCB_WR_Reg(0XFF,0X01);
	reg=SCCB_RD_Reg(0X12);
	reg&=~(1<<1);
	if(sw)reg|=1<<1; 
	SCCB_WR_Reg(0X12,reg);
}
/*******************************************************************************
* 函 数 名       :OV2640_Window_Set(unsigned int sx,unsigned int sy,unsigned int width,unsigned int height)
* 函数功能       :设置图像输出窗口
* 输    入       :sx,sy,起始地址；width,height:宽度(对应:horizontal)和高度(对应:vertical)
* 输    出       : 无
*******************************************************************************/ 
void OV2640_Window_Set(unsigned int sx,unsigned int sy,unsigned int width,unsigned int height)
{
	unsigned int endx;
	unsigned int endy;
	unsigned char temp; 
	endx=sx+width/2;	//V*2
 	endy=sy+height/2;
	
	SCCB_WR_Reg(0XFF,0X01);			
	temp=SCCB_RD_Reg(0X03);				//读取Vref之前的值
	temp&=0XF0;
	temp|=((endy&0X03)<<2)|(sy&0X03);
	SCCB_WR_Reg(0X03,temp);				//设置Vref的start和end的最低2位
	SCCB_WR_Reg(0X19,sy>>2);			//设置Vref的start高8位
	SCCB_WR_Reg(0X1A,endy>>2);			//设置Vref的end的高8位
	
	temp=SCCB_RD_Reg(0X32);				//读取Href之前的值
	temp&=0XC0;
	temp|=((endx&0X07)<<3)|(sx&0X07);
	SCCB_WR_Reg(0X32,temp);				//设置Href的start和end的最低3位
	SCCB_WR_Reg(0X17,sx>>3);			//设置Href的start高8位
	SCCB_WR_Reg(0X18,endx>>3);			//设置Href的end的高8位
}
/*******************************************************************************
* 函 数 名       :OV2640_OutSize_Set(uint16_t width,uint16_t height)
* 函数功能       :设置图像输出大小
* 输    入       :width,height:宽度(对应:horizontal)和高度(对应:vertical),width和height必须是4的倍数
* 输    出       : 0,设置成功；其它为失败
*******************************************************************************/ 
unsigned char OV2640_OutSize_Set(uint16_t width,uint16_t height)
{
	unsigned int outh;
	unsigned int outw;
	unsigned char temp; 
	if(width%4)return 1;
	if(height%4)return 2;
	outw=width/4;
	outh=height/4; 
	SCCB_WR_Reg(0XFF,0X00);	
	SCCB_WR_Reg(0XE0,0X04);			
	SCCB_WR_Reg(0X5A,outw&0XFF);		//设置OUTW的低八位
	SCCB_WR_Reg(0X5B,outh&0XFF);		//设置OUTH的低八位
	temp=(outw>>8)&0X03;
	temp|=(outh>>6)&0X04;
	SCCB_WR_Reg(0X5C,temp);				//设置OUTH/OUTW的高位 
	SCCB_WR_Reg(0XE0,0X00);	
	return 0;
}

//设置图像开窗大小
//由:OV2640_ImageSize_Set确定传感器输出分辨率从大小.
//该函数则在这个范围上面进行开窗,用于OV2640_OutSize_Set的输出
//注意:本函数的宽度和高度,必须大于等于OV2640_OutSize_Set函数的宽度和高度
//     OV2640_OutSize_Set设置的宽度和高度,根据本函数设置的宽度和高度,由DSP
//     自动计算缩放比例,输出给外部设备.
//width,height:宽度(对应:horizontal)和高度(对应:vertical),width和height必须是4的倍数
//返回值:0,设置成功
//    其他,设置失败
/*******************************************************************************
* 函 数 名       :OV2640_ImageWin_Set(unsigned int offx,unsigned int offy,unsigned int width,unsigned int height)
* 函数功能       :设置图像输出大小
* 输    入       :width,height:宽度(对应:horizontal)和高度(对应:vertical),width和height必须是4的倍数
* 输    出       : 0,设置成功；其它为失败
*******************************************************************************/ 
unsigned char OV2640_ImageWin_Set(unsigned int offx,unsigned int offy,unsigned int width,unsigned int height)
{
	unsigned int hsize;
	unsigned int vsize;
	unsigned char temp; 
	if(width%4)return 1;
	if(height%4)return 2;
	hsize=width/4;
	vsize=height/4;
	SCCB_WR_Reg(0XFF,0X00);	
	SCCB_WR_Reg(0XE0,0X04);					
	SCCB_WR_Reg(0X51,hsize&0XFF);		//设置H_SIZE的低八位
	SCCB_WR_Reg(0X52,vsize&0XFF);		//设置V_SIZE的低八位
	SCCB_WR_Reg(0X53,offx&0XFF);		//设置offx的低八位
	SCCB_WR_Reg(0X54,offy&0XFF);		//设置offy的低八位
	temp=(vsize>>1)&0X80;
	temp|=(offy>>4)&0X70;
	temp|=(hsize>>5)&0X08;
	temp|=(offx>>8)&0X07; 
	SCCB_WR_Reg(0X55,temp);				//设置H_SIZE/V_SIZE/OFFX,OFFY的高位
	SCCB_WR_Reg(0X57,(hsize>>2)&0X80);	//设置H_SIZE/V_SIZE/OFFX,OFFY的高位
	SCCB_WR_Reg(0XE0,0X00);	
	return 0;
} 
//该函数设置图像尺寸大小,也就是所选格式的输出分辨率
//UXGA:1600*1200,SVGA:800*600,CIF:352*288
//width,height:图像宽度和图像高度
//返回值:0,设置成功
//    其他,设置失败
/*******************************************************************************
* 函 数 名       :OV2640_ImageSize_Set(unsigned int width,unsigned int height)
* 函数功能       :该函数设置图像尺寸大小,也就是所选格式的输出分辨率
* 输    入       :width,height:图像宽度和图像高度，UXGA:1600*1200,SVGA:800*600,CIF:352*288
* 输    出       : 0,设置成功；其它为失败
*******************************************************************************/ 
unsigned char OV2640_ImageSize_Set(unsigned int width,unsigned int height)
{ 
	unsigned char temp; 
	SCCB_WR_Reg(0XFF,0X00);			
	SCCB_WR_Reg(0XE0,0X04);			
	SCCB_WR_Reg(0XC0,(width)>>3&0XFF);		//设置HSIZE的10:3位
	SCCB_WR_Reg(0XC1,(height)>>3&0XFF);		//设置VSIZE的10:3位
	temp=(width&0X07)<<3;
	temp|=height&0X07;
	temp|=(width>>4)&0X80; 
	SCCB_WR_Reg(0X8C,temp);	
	SCCB_WR_Reg(0XE0,0X00);				 
	return 0;
}
/*------------------------OV2640应用函数(结束)------------------------*/
//DCMI DMA配置
//mem0addr:存储器地址0  将要存储摄像头数据的内存地址(也可以是外设地址)
//mem1addr:存储器地址1  当只使用mem0addr的时候,该值必须为0
//memblen:存储器位宽,可以为:DMA_MDATAALIGN_BYTE/DMA_MDATAALIGN_HALFWORD/DMA_MDATAALIGN_WORD
//meminc:存储器增长方式,可以为:DMA_MINC_ENABLE/DMA_MINC_DISABLE

//DCMI_HandleTypeDef hdcmi;
//DMA_HandleTypeDef hdma_dcmi;

