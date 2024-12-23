
#define F_CPU 14745600

#include <avr/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "hc06.h"
#include "timer.h"
#include "SRF02.h"

#define Start 0xaa
#define End 0xaf

//timer
int timer_count ;

//uart0 
unsigned char RX0_buff[11] = {0,};
unsigned char RX0_idx = 0;
int RX0_flag = 0;

//motor
int motor_control;
int counter = 0;
int pump_flag = 0;
int servo_count = 0;

//SRF02
unsigned char SRF02_buff[100] ={0,0};
float SRF02_range;

int test = 0;
int pump_count = 0;

ISR(TIMER1_COMPA_vect)
{//1ms
	timer_count++;
	
	if (timer_count % 1 == 0)
	{//1ms		
	}
	else{}
	
	if (timer_count % 10 == 0)
	{//10ms
		if(SRF02_range < 30.0)
		{
			Buzzer(2024);
		}					
	}
	else{}
	
	if (timer_count % 100 == 0)
	{//100ms		
		//SRF02
		Start_SRF02_Conv(0xe4, microSec);
		SRF02_range = Get_SRF02_Range(0xe4);
		SRF02_range = SRF02_range/10000*340/2 ;
		//sprintf(SRF02_buff,"%.1f cm",SRF02_range);
		//uart1_transmit_string(SRF02_buff);	
	}
	else{}
	
	if (timer_count >= 500)
	{		
		Servo_Motor();
		timer_count = 0;		
	}
	else{}
}

ISR(USART1_RX_vect)
{	
	char RX1_data = UDR1;	
	uart1_transmit(RX1_data);
	uart0_transmit(RX1_data);
}

ISR(USART0_RX_vect)
{// 블루투스 값 수신	
	unsigned char rx0_data;
	rx0_data = UDR0;	
	
	if(rx0_data == Start) 
	{
		RX0_flag = 1;
	}
	
	if(RX0_flag == 1)
	{
		RX0_buff[RX0_idx++] = rx0_data;
		
		if (RX0_buff[9] == 0xae)
		{
			pump_flag = 1;			
		}
	}
	
	if(rx0_data == End) 
	{//패킷 끝나는 지점
		RX0_idx = 0;
		RX0_flag = 0;
		// char 형태를 int로 변환			
		int num_y = 0;
		num_y |= (RX0_buff[1] << 24); // 가장 높은 바이트
		num_y |= (RX0_buff[2] << 16); // 두 번째 바이트
		num_y |= (RX0_buff[3] << 8);  // 세 번째 바이트
		num_y |= RX0_buff[4];         // 가장 낮은 바이트		
		
		int num_x = 0;
		num_x |= (RX0_buff[5] << 24); // 가장 높은 바이트
		num_x |= (RX0_buff[6] << 16); // 두 번째 바이트
		num_x |= (RX0_buff[7] << 8);  // 세 번째 바이트
		num_x |= RX0_buff[8];         // 가장 낮은 바이트			
		
		char Message[20];
		sprintf(Message,"Y: %04d X: %04d Z: %04d SRF02: %.1fcm", num_y,num_x,pump_flag , SRF02_range);
		uart1_transmit_string(Message);	
		Motor_Control(num_x , num_y);	
	}	
}


void Servo_Motor()
{	
	//0.5s마다 함수 실행
	if(pump_flag == 1)
	{
		Pump_Start();
		servo_count = 1;
	}
	else if(servo_count == 1)
	{
		OCR3A = 88;
		servo_count++;
	}
	else if(servo_count == 2)
	{
		OCR3A = 27;
		servo_count++;
	}
	else
	{
		Pump_End();
		servo_count = 0;
	}

	pump_flag = 0;
}

void Motor_Control(int X, int Y)
{
	int speed_R = 0;
	int speed_L = 0;
	
	//제자리 회전
	if ((X > 600 && X < 1024) && (400 < Y && Y < 600))
	{//좌회전
		MOTOR_Left_turn();		
		speed_R = 400;
		speed_L = 400;		
	}
	else if ((X > 0 && X < 400) && (400 < Y && Y < 600))	
	{//우회전
		MOTOR_Right_turn();
		speed_R = 400;
		speed_L = 400;
	}
	
	//직진 후진
	if ((480 < X && X < 520) && (0 < Y && Y < 400))
	{//직진
		MOTOR_Forward();
		if(100 < Y && Y < 380)
		{//조이스틱 최대로 올리기 전
			speed_R = 400;
			speed_L = 400;
		}
		else
		{//조이스틱 최대로 올림
			speed_R = 700;
			speed_L = 700;
		}			
	}
	else if ((480 < X && X < 520) && (600 < Y && Y < 1024))
	{//후진
		MOTOR_Backward();
		if(600 < Y && Y < 880)
		{//조이스틱 최대로 올리기 전
			speed_R = 400;
			speed_L = 400;
		}
		else
		{//조이스틱 최대로 올림
			speed_R = 700;
			speed_L = 700;
		}
	}
	
	//직진 중 회전
	if ((520 < X && X < 1024) && (0 < Y && Y < 400))
	{//직진 중 좌회전
		MOTOR_Forward();	
		speed_R = 700;
		speed_L = 350;
	}
	else if ((0 < X && X < 480) && (0 < Y && Y < 400))
	{//직진 중 우회전
		MOTOR_Forward();		
		if ((460 < X && X < 470))
		{//좌회전 예외처리
			speed_R = 700;
			speed_L = 350;
		}
		else 				
		{
			speed_R = 350;
			speed_L = 700;
		}
	}
	
	//후진 중 회전
	if ((520 < X && X < 1024) && (600 < Y && Y < 1024))
	{//후진 중 좌회전
		MOTOR_Backward();
		speed_R = 700;
		speed_L = 350;
	}
	else if ((0 < X && X < 480) && (600 < Y && Y < 1024))
	{//후진 중 우회전
		MOTOR_Backward();		
		speed_R = 350;
		speed_L = 700;
		
	}	
	
	PWM3_duty_cycle(speed_R, speed_L);
}

void MOTOR_Forward() //정방향
{
	PORTC &= (1 << PC0)|(0 << PC1)|(1 << PC6)|(0 << PC7) ;
	PORTC |= (1 << PC0)|(0 << PC1)|(1 << PC6)|(0 << PC7) ;
}

void MOTOR_Backward() //역방향
{
	PORTC &= (0 << PC0)|(1 << PC1)|(0 << PC6)|(1 << PC7) ;
	PORTC |= (0 << PC0)|(1 << PC1)|(0 << PC6)|(1 << PC7) ;
}

void MOTOR_Left_turn()
{
	PORTC &= (1 << PC0)|(0 << PC1)|(0 << PC6)|(1 << PC7) ;     
	PORTC |= (1 << PC0)|(0 << PC1)|(0 << PC6)|(1 << PC7) ;
}
void MOTOR_Right_turn()
{
	PORTC &= (0 << PC0)|(1 << PC1)|(1 << PC6)|(0 << PC7) ;
	PORTC |= (0 << PC0)|(1 << PC1)|(1 << PC6)|(0 << PC7) ;    
}

void Motor_Init()
{
	DDRC = (1 << PC0)|(1 << PC1)|(1 << PC6)|(1 << PC7) ;
}

void Pump_Init()
{
	DDRB = 0xff;
}

void Pump_End()
{
	PORTB |= (1<<PB7); //PB7 사용
}

void Pump_Start()
{
	PORTB &= ~(1<<PB7); //PB7 사용
}

void Buzzer(int period)
{
	int i;
	int duty ;
	duty = period >> 1; //나누기 2
	
	// 1출력
	PORTG = PORTG | (1<<4); 
	
	// period 절반 시간 지남
	for(i=0; i<duty; i++) _delay_us(1);
	
	// 0 출력
	PORTG = PORTG & ~(1<<4);
	
	// period 절반 시간 지남
	for(i=0; i<duty; i++) _delay_us(1);
}

int main(void)
{	
	DDRG = 0xFF;
	
	//UART
	uart1_init(115200);
	uart0_init(115200);
	InterruptInit0_Rx();
	InterruptInit1_Rx();
	//uart0_init(9600); // AT모드 설정시 HC05 진입 38400
	
	//Timer
	ISR_CTC_Init();
	Init_PWM3_Motor();
	
	//BT
	//hc06_init();
	//hc06_command();
	
	Motor_Init();
	TWI_Init();//SRF02	
	Pump_Init();
	
	//test
	//uart1_transmit_string("hi choi \r\n");

    while (1) 
    {		
    }
}

