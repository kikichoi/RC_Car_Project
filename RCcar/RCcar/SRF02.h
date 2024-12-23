/*
 * SRF02.h
 *
 * Created: 2024-12-02 오후 11:47:02
 *  Author: user
 */

#ifndef SRF02_H_
#define SRF02_H_


#define Inches 0x50         //인치 데이터를 받겠다.
#define Centimeters 0x51    //센치미터 데이터를 받겠다.
#define microSec 0x52       //us 시간 데이터를 받겠다.
#define Fake_Centimeters 0x57   //음파플 쏘지않고 받기만 하여 데이터 환산.
#define Fake_MicroSec 0x58      //음파플 쏘지않고 받기만 하여 데이터 환산.
#define BURST 0x5C          //음파를 쏘기만함.

//초음파 센서 주소 지정
#define USID1 0xE2
#define USID2 0xE4

//SRF02 레지스터
#define CommandReg 0
#define Unused 1
#define RangeHighByte 2
#define RangeLowByte 3

#define TWI_START 0x08
#define MT_REPEATED_START 0x10
#define MT_SLAW_ACK 0x18
#define MT_DATA_ACK 0x28
#define MT_SLAR_ACK 0x40
#define MT_DATA_NACK 0x58

unsigned char lcd_tx_buff[100] = {0,0};

void TWI_Init()
{
	TWBR = 10;
	//100k HZ I2C 클록 주파수
	TWSR = 0;
	TWCR = 0;
}

unsigned char TWI_Read(unsigned char addr, unsigned char regAddr)
{
	unsigned char Data;
	TWCR = ((1<<TWINT)|(1<<TWEN)|(1<<TWSTA));  //Start조건 전송
	while(((TWCR & (1 << TWINT)) == 0x00) || ((TWSR & 0xf8)!=TWI_START));

	TWDR = addr&(~0x01);                       //쓰기 위한 주소 전송
	TWCR = ((1<<TWINT)|(1<<TWEN));
	while(((TWCR & (1 << TWINT)) == 0x00) || ((TWSR & 0xf8)!=MT_SLAW_ACK));
	
	TWDR = regAddr;                            //Reg주소 전송
	TWCR = ((1<<TWINT)|(1<<TWEN));
	while(((TWCR & (1 << TWINT)) == 0x00) || ((TWSR & 0xf8)!=MT_DATA_ACK));
	
	TWCR = ((1<<TWINT)|(1<<TWEN)|(1<<TWSTA));  //Restart 전송
	while(((TWCR & (1 << TWINT)) == 0x00) || ((TWSR & 0xf8)!=MT_REPEATED_START));
	
	TWDR = addr|0x01;                          //읽기 위한 주소 전송
	TWCR = ((1<<TWINT)|(1<<TWEN));
	while(((TWCR & (1 << TWINT)) == 0x00) || ((TWSR & 0xf8)!=MT_SLAR_ACK));
	
	TWCR = ((1<<TWINT)|(1<<TWEN));
	while(((TWCR & (1 << TWINT)) == 0x00) || ((TWSR & 0xf8)!=MT_DATA_NACK));
	Data = TWDR;                        //Data읽기
	TWCR = ((1<<TWINT)|(1<<TWEN)|(1<<TWSTO));
	return Data;
}

void TWI_Write(unsigned char addr, unsigned char Data[],int NumberOfData)
{
	int i=0;
	TWCR = ((1<<TWINT)|(1<<TWEN)|(1<<TWSTA));
	while(((TWCR & (1 << TWINT)) == 0x00) || ((TWSR & 0xf8)!=TWI_START));
	
	TWDR = addr&(~0x01);
	TWCR = ((1<<TWINT)|(1<<TWEN));
	while(((TWCR & (1 << TWINT)) == 0x00) || ((TWSR & 0xf8)!=MT_SLAW_ACK));
	
	for(i=0;i<NumberOfData;i++)
	{
		TWDR = Data[i];
		TWCR = ((1<<TWINT)|(1<<TWEN));
		while(((TWCR & (1 << TWINT)) == 0x00) || ((TWSR & 0xf8)!=MT_DATA_ACK));
	}
	
	TWCR = ((1<<TWINT)|(1<<TWEN)|(1<<TWSTO));
}

void Change_SRF02_Adress(unsigned char nowAdress, unsigned char changeAdress)
{
	unsigned char sequense1[2] = {0x00,0xA0};
	unsigned char sequense2[2] = {0x00,0xAA};
	unsigned char sequense3[2] = {0x00,0xA5};
	unsigned char sequense4[2] = {0x00,};

	sequense4[1] = changeAdress;
	TWI_Write(nowAdress,sequense1,2);
	_delay_ms(1);
	TWI_Write(nowAdress,sequense2,2);
	_delay_ms(1);
	TWI_Write(nowAdress,sequense3,2);
	_delay_ms(1);
	TWI_Write(nowAdress,sequense4,2);
	_delay_ms(1);
}

void Start_SRF02_Conv(unsigned char Address, unsigned char mode)
{
	unsigned char ConvMode[2] = {0x00,};
	ConvMode[1] = mode;
	TWI_Write(Address,ConvMode,2);
}

unsigned int Get_SRF02_Range(unsigned char Address)
{
	unsigned int range;
	unsigned char High,Low;
	
	High = TWI_Read(Address, RangeHighByte);
	Low = TWI_Read(Address, RangeLowByte);
	range = (High<<8)+Low;
	return range;
}



#endif /* SRF02_H_ */