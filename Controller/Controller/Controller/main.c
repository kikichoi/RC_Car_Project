
#define F_CPU 14745600

#include <avr/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"
#include "hc06.h"
#include "timer.h"
#include "adc.h"
#include "lcd.h"
#include "IO.h"

#define Start 0xaa
#define End 0xaf

unsigned int ADC_Data = 0;
int adc_X, adc_Y = 0; // 조이스틱 X축, Y축 값
char Message[40];

unsigned char pump = 0;

int timer_count ;

ISR(TIMER1_COMPA_vect)
{// 1ms interrupt 
	timer_count++;
	
	if (timer_count % 1 == 0)
	{
		Joystick_Read();
		SW_read();
		//uart1_transmit_string(Message); // 조이스틱 값 컴퓨터로 출력
	}
	else{}	
	
	if (timer_count % 2 == 0)
	{//2ms
		TX0_Packet();
	}
	else{}
		
	if (timer_count >= 100)
	{
		timer_count = 0;
	}
	else{}
}

ISR(USART1_RX_vect)
{
	char RX1_data = UDR1;
	//uart1_transmit(RX1_data);
	uart0_transmit(RX1_data);
}

ISR(USART0_RX_vect)
{// 블루투스 값 수신
	char RX0_data;
	RX0_data = UDR0;	
	uart1_transmit(RX0_data);
}

ISR(ADC_vect)
{
	ADC_Data = ADCW; // ADC 값 읽기.
}

void TX0_Packet()
{
	unsigned char tx0_buff[11] = {0,};
	unsigned char idx = 0;	
	
	tx0_buff[idx++] = Start; // 시작 비트
	
	//전진, 후진 정보 , int형태 char로 변환
	tx0_buff[idx++] = (adc_Y >> 24) & 0xFF; // 가장 높은 바이트
	tx0_buff[idx++] = (adc_Y >> 16) & 0xFF; // 두 번째 바이트
	tx0_buff[idx++] = (adc_Y >> 8) & 0xFF;  // 세 번째 바이트
	tx0_buff[idx++] = adc_Y & 0xFF;         // 가장 낮은 바이트
	
	//좌회전, 우회전 정보
	tx0_buff[idx++] = (adc_X >> 24) & 0xFF; // 가장 높은 바이트
	tx0_buff[idx++] = (adc_X >> 16) & 0xFF; // 두 번째 바이트
	tx0_buff[idx++] = (adc_X >> 8) & 0xFF;  // 세 번째 바이트
	tx0_buff[idx++] = adc_X & 0xFF;         // 가장 낮은 바이트
	
	tx0_buff[idx++] = pump; // 발사버튼 정보
	tx0_buff[idx++] = End; // 종료 비트	
	
	// 0x00값 패킷 마지막이 아닌 중간에 껴있는 경우 종료됨을 방지하고자 길이만큼 전송
	uart0_transmit_string_with_length(tx0_buff,11);
	uart1_transmit_string_with_length(tx0_buff,11);
}

void Joystick_Read()
{	
	adc_X = Read_ADC_Data(0); // 조이스틱의 X축 값 읽음.
	adc_Y = Read_ADC_Data(1); // 조이스틱의 Y축 값 읽음.	
	if (adc_X <= 0)
	{
		adc_X = 1;
	}
	
	if (adc_Y <= 0)
	{
		adc_Y = 1;
	}
	
	LCD_print();
}

void LCD_print()
{
	// 조이스틱 X값을 확인하기 위해 LCD에 출력.
	sprintf(Message,"X: %04d ", adc_X);
	LCD_Pos(0,0);
	LCD_Str(Message);
	
	// 조이스틱 Y값을 확인하기 위해 LCD에 출력.
	sprintf(Message,"Y: %04d ", adc_Y);
	LCD_Pos(1,0);
	LCD_Str(Message);
}

void SW_init()
{
	DDRD = 0xef;
}

void SW_read()
{
	if (PIND == 0b11101111)
	{
		pump = 0xae;		
	}
	else pump = 0;
}

int main(void)
{
	DDRB = 0xff;
	
	//UART
	uart1_init(115200);
	uart0_init(115200);
	InterruptInit0_Rx();
	InterruptInit1_Rx();
	//uart1_transmit_string("hi choi \r\n"); // test
	//uart0_init(9600); // AT모드 설정시 HC05 진입 38400
	
	//Timer
	ISR_CTC_Init();
	
	//BT
	hc06_init();
	//hc06_command();		
	
	ADC_Init();
	LCD_Init();		
	SW_init();
	
	while (1)
	{	
	}
}
