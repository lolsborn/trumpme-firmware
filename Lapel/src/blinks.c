/*
 * blinks.c
 *
 * Created: 10/1/2018 10:02:37 PM
 *  Author: steven
 */ 

#include "atmel_start_pins.h"
#include "blinks.h"
#include "clock_config.h"
#include "util/delay.h"


void blinks(bool *interrupt) {
	int x = rand() % (3 + 1 - 0) + 0;
	switch(x) {
		case 0:
		rblink(interrupt);
		break;
		case 1:
		chase(interrupt);
		break;
		case 2:
		pairs(interrupt);
		break;
		case 3:
		outin(interrupt);
		break;
	}
}

void reset_leds(void) {
	LED1_set_level(false);
	LED2_set_level(false);
	LED3_set_level(false);
	LED4_set_level(false);
}

void rblink(bool *interrupt) {

	int b = 0;
	for(int i=0; i<24; i++) {
		// Exit early for next sound
		if(*interrupt) {
			reset_leds();
			return;
		}
		b = rand() % (3 + 1 - 0) + 0;
		LED1_set_level(false);
		LED2_set_level(false);
		LED3_set_level(false);
		LED4_set_level(false);
		switch(b) {
			case 0:
			LED1_set_level(true);
			break;
			case 1:
			LED2_set_level(true);
			break;
			case 2:
			LED3_set_level(true);
			break;
			case 3:
			LED4_set_level(true);
			break;
		}
		_delay_ms(50);
	}
	reset_leds();	
}

void outin(bool *interrupt) {
	
	for(int i=0; i<8; i++) {
		// Exit early for next sound
		if(*interrupt) {
			reset_leds();
			return;
		}
		_delay_ms(150);
		LED1_set_level(true);
		LED2_set_level(false);
		LED3_set_level(false);
		LED4_set_level(true);
		_delay_ms(150);
		LED1_set_level(false);
		LED2_set_level(true);
		LED3_set_level(true);
		LED4_set_level(false);
	}
	reset_leds();
}

void pairs(bool *interrupt) {
	for(int i=0; i<6; i++) {
		if(*interrupt) {
			reset_leds();
			return;
		}
		_delay_ms(200);
		LED1_set_level(true);
		LED2_set_level(false);
		LED3_set_level(true);
		LED4_set_level(false);
		_delay_ms(200);
		LED1_set_level(false);
		LED2_set_level(true);
		LED3_set_level(false);
		LED4_set_level(true);
	}
	reset_leds();
}

void chase(bool *interrupt) {
	
	for(int i=0; i<5; i++) {
		if(*interrupt) {
			reset_leds();
			return;
		}

		_delay_ms(50);
		LED1_set_level(true);
		LED2_set_level(false);
		LED3_set_level(false);
		LED4_set_level(false);
		_delay_ms(50);
		LED1_set_level(false);
		LED2_set_level(true);
		LED3_set_level(false);
		LED4_set_level(false);
		_delay_ms(50);

		LED1_set_level(false);
		LED2_set_level(false);
		LED3_set_level(true);
		LED4_set_level(false);
		_delay_ms(50);

		LED1_set_level(false);
		LED2_set_level(false);
		LED3_set_level(false);
		LED4_set_level(true);
		_delay_ms(50);

		LED1_set_level(false);
		LED2_set_level(false);
		LED3_set_level(true);
		LED4_set_level(false);
		_delay_ms(50);

		LED1_set_level(false);
		LED2_set_level(true);
		LED3_set_level(false);
		LED4_set_level(false);
		_delay_ms(50);

		LED1_set_level(true);
		LED2_set_level(false);
		LED3_set_level(false);
		LED4_set_level(false);
	}
	reset_leds();
}