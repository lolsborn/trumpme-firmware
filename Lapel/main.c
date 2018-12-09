#include <atmel_start.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/sleep.h>
//#include "usart_basic.h"
#include "dataflash.h"
#include "port.h"
#include "blinks.h"

// Set sample freq to 12k to match wav file sample rate
#define SAMPLE_FREQ (uint16_t)12000 /* Hz */
#define BUFFER_SIZE 32

/* Simple driver for the Sampling Timer, initialization done by the START init driver */
#define SAMPLE_TIMER TCA0.SINGLE
#define SAMPLE_TIMER_START() SAMPLE_TIMER.CTRLA |= TCA_SINGLE_ENABLE_bm
#define SAMPLE_TIMER_STOP() SAMPLE_TIMER.CTRLA &= ~(TCA_SINGLE_ENABLE_bm)
#define SAMPLE_TIMER_ENABLE_IRQ() SAMPLE_TIMER.INTCTRL = TCA_SINGLE_OVF_bm
#define SAMPLE_TIMER_DISABLE_IRQ() SAMPLE_TIMER.INTCTRL = 0
#define SAMPLE_TIMER_CLEAR_CNT() SAMPLE_TIMER.CNT = 0
#define SAMPLE_TIMER_SET_PER(P) SAMPLE_TIMER.PER = P

/* Event forwarding from Sampling Timer to ADC */
#define ENABLE_EVENT_FORWARDING() EVSYS.ASYNCUSER1 = EVSYS_ASYNCUSER1_SYNCCH0_gc
#define DISABLE_EVENT_FORWARDING() EVSYS.ASYNCUSER1 = 0

void     sample_timer_init(void);
void     playback(uint8_t clipid);
void     read_header(void);

uint8_t  buffer0[BUFFER_SIZE], buffer1[BUFFER_SIZE];
uint8_t *buffers[]     = {buffer0, buffer1};
bool active_buffer = 0, alt_buffer = 1, buffer_end;
uint8_t  buffer_index = 0;
bool start_playback, playback_finished = 0;

typedef struct {
	uint32_t file_start; 
	uint16_t file_size;			
} soundByte_t;

uint8_t file_count = 14;
uint8_t header_size = 0;
soundByte_t sounds[14];

// Read meta-data from flash memory about sound file sizes
void read_header(void) {
	//printf("read header\n");
	file_count = dataflash_read_byte(0);
	header_size = (2 * file_count) + 1;
	//printf("file_count %d\n", file_count);
	//printf("header size %d\n", header_size);
	uint8_t fsb[2];
	uint8_t x = 0;
	uint32_t size_total = 0;
	for(int i=1; i<=file_count*2; i = i + 2) {
		fsb[0] = dataflash_read_byte(i);
		fsb[1] = dataflash_read_byte(i+1);
		sounds[x].file_size = fsb[0] << 8 | fsb[1];
		sounds[x].file_start = size_total + header_size + 1;
		//printf("start: %lu size: %d\n", sounds[x].file_start, sounds[x].file_size);
		size_total += sounds[x].file_size;
		x++;
	}
	//printf("\ntotal size: %lu\n", size_total);
}

// Part of PLAYBACK routine - Load new data into DAC & rotate buffers
ISR(TCA0_OVF_vect) {
	
	/* Load next data into DAC */
	DAC_0_set_output(*(buffers[active_buffer] + buffer_index++));

	/* If end of buffer reached */
	if (buffer_index == BUFFER_SIZE) {
	//	puts("Buffer index reached, swapping");
		/* Reset buffer index */
		buffer_index = 0;
		/* Change active buffer */
		active_buffer ^= 1;
		alt_buffer ^= 1;
		/* Buffer full flag */
		buffer_end = 1;
	}

	/* Clear interrupt flag */
	SAMPLE_TIMER.INTFLAGS = 0xFF;
}

void playback(uint8_t clipid) {
	buffer_index = 0;
	buffer_end   = 0;
	uint16_t remaining = sounds[clipid].file_size;

	/* Start read process from dataflash */
	dataflash_read_multiple_start(sounds[clipid].file_start + 44); // Skip headers 44 bytes

	/* Read first data into buffers */
	dataflash_read_multiple_continue(buffers[active_buffer], BUFFER_SIZE);
	dataflash_read_multiple_continue(buffers[alt_buffer], BUFFER_SIZE);


	// Eyes. LED6 is tied to amplifier enable pin so this must be on for playback
	LED5_set_level(true);
	LED6_set_level(true);

	/* Enable DAC */
	DAC_0_enable();

	/* Enable sample timer overflow ISR and start timer */
	SAMPLE_TIMER_ENABLE_IRQ();
	SAMPLE_TIMER_CLEAR_CNT();
	SAMPLE_TIMER_START();

	bool donzored = false;
	do {
		if (buffer_end) {
//			puts("buffer end");
			/* Fill alternate buffer with new data */
			dataflash_read_multiple_continue(buffers[alt_buffer], BUFFER_SIZE);
			if(remaining<=BUFFER_SIZE) {
				donzored = true;
			}
			remaining -= BUFFER_SIZE;
			buffer_end = 0;
		}
		//printf("remaining: %u\n", remaining);
	} while (!donzored);

	/* Stop timer and disable sample timer overflow ISR */
	SAMPLE_TIMER_STOP();
	SAMPLE_TIMER_DISABLE_IRQ();

	/* Disable DAC */
	DAC_0_disable();

	/* Finalize read from dataflash */
	dataflash_read_multiple_stop();
	
	// Eyes off, this also disables amplifier

	LED5_set_level(false);
	LED6_set_level(false);

}

void sample_timer_init(void) {
	SAMPLE_TIMER_SET_PER(F_CPU / (3 * SAMPLE_FREQ));
	/* Stop event forwarding to ADC */
	DISABLE_EVENT_FORWARDING();
}

// Playback interrupt button pressed
#if HW_VERSION > 3
ISR(PORTB_PORT_vect) {
#else
ISR(PORTA_PORT_vect) {
#endif
	start_playback = 1;
	sleep_disable();
	
#if HW_VERSION > 3
	PORTB.INTFLAGS |= PORT_INT2_bm;
#else
	PORTA.INTFLAGS |= PORT_INT7_bm;
#endif
}

int main(void) {
	//return;
	atmel_start_init();
	sample_timer_init();
	
	dataflash_init();
	
	LED1_set_dir(PORT_DIR_OUT);
	LED3_set_dir(PORT_DIR_OUT);
	LED5_set_pull_mode(PORT_PULL_OFF);
	LED5_set_dir(PORT_DIR_OUT);
	LED6_set_dir(PORT_DIR_OUT);

	LED1_set_level(false);
	LED3_set_level(false);
	LED5_set_level(false);
	LED6_set_level(false);

	// Debugging UART	
	//PB3_set_dir(PORT_DIR_IN);
	//PB3_set_pull_mode(PORT_PULL_OFF);
	//PB2_set_dir(PORT_DIR_OUT);
	//PB2_set_level(false);
	//USART_init();

	LED2_set_dir(PORT_DIR_OUT);
	LED4_set_dir(PORT_DIR_OUT);
	LED2_set_level(false);
	LED4_set_level(false);
	
    read_header();

#if HW_VERSION > 3
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	PORTB_set_pin_dir(2, PORT_DIR_IN);
	PORTB_pin_set_isc(2, PORT_ISC_FALLING_gc);
	PORTB_set_pin_pull_mode(2, PORT_PULL_UP);
#else
	set_sleep_mode(SLEEP_MODE_IDLE);
	PORTA_pin_set_isc(7, PORT_ISC_FALLING_gc);
	PORTA_set_pin_pull_mode(7, PORT_PULL_UP);
#endif
	
	DAC_0_disable();

	int x = 0;
	while (1) {
		cli();
		if(start_playback && !playback_finished == 0) {
			start_playback = 0;
			playback_finished = 0;
			sei();
			playback(x++);
			if(x > file_count)
				x = 0;
			start_playback = 0;
			blinks(&start_playback);
			playback_finished = 1;
			continue;
		}
		playback_finished = 1;
		
		sei();
		sleep_mode();
	}

}
