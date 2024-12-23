
#ifndef TIMER_H_
#define TIMER_H_

#define Right 1
#define Left 0

void ISR_CTC_Init()
{// interrupt every 1ms
	
	// CTC 모드로 설정 (Clear Timer on Compare Match)
	TCCR1B |= (1 << WGM12);
	
	// 비교일치 OCR1A 설정
	// 14745600 클럭 사용시 (14745600) / (1000 * 64) - 1 = 229
	OCR1A = 229;
	
	// 분주율 64
	TCCR1B |= (1 << CS11) | (1 << CS10);
	
	//output compare match A
	TIMSK |= (1 << OCIE1A);
	
	// 전역 인터럽트 허가
	sei();
}

// 3번 Timer / Counter 설정
void Init_PWM3_Motor()
{
	DDRE = (1 << PE3)|(1 << PE4)|(1 << PE5) ;
	// fast PWM 10bit
	TCCR3A |= (1 << WGM31) | (1 << WGM30);
	TCCR3B |= (1 << WGM32);
	
	// 비반전 모드  TOP : 0xFF  비교일치값 : OCR3B 레지스터
	// OCnB , OCnC 출력설정
	TCCR3A |= (1 << COM3A1) | (1 << COM3B1) | (1 << COM3C1);	
	
	TCCR3B |= (1 << CS32) | (0 << CS30); // 분주율 256
	// 256 / 14745600 * 1024(10bit) = 17ms	
	
	// duty 설정
	OCR3A = 0;	
	OCR3B = 0;
	OCR3C = 0;	
	sei();
}

void PWM3_duty_cycle( int R_value,  int L_value)
{// duty 값 조정 함수
	
	//오른쪽 바퀴 제어
	OCR3B = R_value;	
	
	//왼쪽 바퀴 제어
	OCR3C = L_value;	
}




#endif /* TIMER_H_ */