

#ifndef ADC_H_
#define ADC_H_

void ADC_Init(void)
{
	ADMUX = 0x00;
	ADCSRA = 0x00;
	ADMUX |= (1<<REFS0);
	ADCSRA |= (1<<ADEN) | (1<<ADFR) | (3<<ADPS0) ;   //ADC사용 허가, 프리러닝, 인터럽트 허용, 8분주
	sei(); // 전역 인터럽트 허가.
}

unsigned int Read_ADC_Data(unsigned char adc_input)
{
	unsigned int adc_Data = 0; // ADC_Data 변수 = 0
	
	ADMUX &= ~(0x1F);		// ADMUX 값으로 0으로 클리어
	ADMUX |= (adc_input & 0x07); // adc-input 채널로 초기화
	
	ADCSRA |= (1<<ADSC);	// AD 변환 시작
	while(!(ADCSRA & (1<<ADIF))); // AD 변환 종료 대기
	adc_Data = ADCL;	// ADC 변환 값 읽기. 하위 비트 먼저 읽고 상위 비트를 읽는다.
	adc_Data |= ADCH<<8;
	
	return adc_Data;	// ADC 변환값 반환
}

#endif /* ADC_H_ */