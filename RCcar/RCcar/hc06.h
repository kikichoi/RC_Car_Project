#ifndef HC06_H_
#define HC06_H_

char response[100];
char AT_Buff ;

void hc06_init()
{
	DDRE = 0XFF; //  출력 설정 (KEY 핀)
	PORTE &= ~(1 << PE4); // KEY 핀을 LOW 설정 (AT 모드 비활성화)
}

void hc06_enter_at_mode()
{
	PORTE |= (1 << PE4);  // KEY 핀을 high 설정 (AT 모드 활성화)
}

void hc06_exit_at_mode()
{
	PORTE &= ~(1 << PE4); // KEY 핀을 LOW 설정 (AT 모드 활성화)
}

void hc06_send_AT(const char *cmd, char *response, uint8_t max_length)
{//hc06 AT Command TX
	uart0_transmit_string(cmd);
	//uart0_transmit('\r');
	//uart0_transmit('\n');
	
	uint8_t i = 0 ;
	while(i < max_length - 1)
	{//HC06 모듈 응답 값 받기
		if (UCSR0A & (1 << RXC0))
		{
			response[i] = uart0_receive();
			if (response[i] == '\r' || response[i] == '\n')
			{
				break;
			}
			i++;
		}
	}
	//response[i] = '0' ; //문자열 끝에 null 추가
}

void hc06_command()
{
	hc06_enter_at_mode(); // EN HIGH
	
	//AT+BIND=0020,04,BDD4A0 //마스터에서 슬레이브 주소를 페어링
	hc06_send_AT("AT+BIND?\r\n", response, sizeof(response));
	uart1_transmit_string(response); //HC05 모듈 응답 터미널로 띄우기
	
	hc06_exit_at_mode(); // EN LOW
}

#endif /* HC06_H_ */