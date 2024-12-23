/*
 * IO.h
 *
 * Created: 2024-12-03 오전 1:22:58
 *  Author: user
 */ 


#ifndef IO_H_
#define IO_H_

void Interrupt7_init()
{
	EIMSK = (1<<INT7); //외부 인터럽트 INT1 선택
	EICRA = (1<<ISC11); //하강 (1<<ISC11)|(1<<ISC10); //=상승 , 
	sei(); //전체 인터럽트 허가 
}


#endif /* IO_H_ */