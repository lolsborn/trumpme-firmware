#include <stdio.h>
#include <string.h>
#include <atmel_start.h>
#include <command_reader.h>
#include <usart_basic.h>
#include <atomic.h>
#include <avr/wdt.h>
#include <rstctrl.h>

#include "dataflash.h"
#define BUFFER_SIZE 40
#define TABLE_START_ADD 0
#define TABLE_END_ADD 511999 // 512k - 1 (starts at 0)


#define RX_BUFF_LEN 32
static char rx[RX_BUFF_LEN + 1];

uint8_t read_command(void)
{
	uint8_t i = 0;

	// If USART Basic driver is in IRQ-mode, enable global interrupts.
	//ENABLE_INTERRUPTS();

	// Test driver functions, assumes that the USART RX and
	// TX pins have been loopbacked, or that USART hardware
	// is configured in loopback mode

	fputs("\n> ", stdout);

	// Initialize rx buffer so strncmp() check will work
	memset(rx, 0, sizeof(rx));
	USART_xon();
	while(i < RX_BUFF_LEN) {
		uint8_t c = USART_read(); // Blocks until character is available
		switch(c) {
			case '\n':
				goto nextcmd;
				break;
			case '\r':
				break;
			case '\b':
				if(i>0)
					i--;
				break;
			default:
				rx[i] = c;
				i++;
				break;
		}
		putchar(c);
	}

	
nextcmd:


    //////////////////////////////////////////////////////////////////////////
    // System
    //////////////////////////////////////////////////////////////////////////
	// RESET
	if (strncmp("reset", (char *)rx, strlen("reset")) == 0 && strlen((char *)rx) == 5) {
		puts("\nok");
		RSTCTRL_reset();
	// HELP
	} else if(strncmp("help", (char *)rx, strlen("help")) == 0 && strlen((char*)rx) == 4) {
		puts("\nhelp                   this dialog");
		puts("reset                    reset mcu");
		puts("flash info               decode flash header info");
		puts("flash read               dumps contents of flash");
		puts("flash write <N>          reads N bytes from stdin and write them to flash");
		puts("flash erase              erase flash contents");
		puts("tone <n>                 play square wave of N hz");
		puts("tone stop                stop tone");
		puts("play <n>                 play the Nth clip");
		puts("led<1-6> toggle          toggle led light");
		puts("led<1-6> on              turn led on");
		puts("led<1-6> off             turn led off");


    //////////////////////////////////////////////////////////////////////////
    // Manage Storage
    //////////////////////////////////////////////////////////////////////////
	// Display Flash header info
	} else if(strncmp("flash info", (char *)rx, strlen("flash info")) == 0) {
		uint8_t filecnt = dataflash_read_byte(0);
		char filecntstr[4];
		itoa(filecnt, filecntstr, 10);
		
		uint32_t device_id;
		device_id = dataflash_jedec_id_read();
		char hex_id[33];
		memset(hex_id, 0, sizeof(hex_id));
		ultoa(device_id, hex_id, 16);
		fputs("Flash ", stdout);
		puts(hex_id);

		fputs("\n\nFiles: ", stdout);
		fputs(filecntstr, stdout);
		fputs("\nSizes:\n", stdout);
		
		uint8_t fsb[2];
		uint16_t fsize = 0;
		char fsizestr[6];
		for(int i=1; i<=filecnt*2; i = i + 2) {
			fsb[0] = dataflash_read_byte(i);
			fsb[1] = dataflash_read_byte(i+1);
			fsize = fsb[0] << 8 | fsb[1];
			itoa(fsize, fsizestr, 10);
			fputs("    ", stdout);
			fputs(fsizestr, stdout);
			fputs("\n", stdout);
		}
		
		puts("ok");
		
	// ERASE
	} else if(strncmp("flash erase", (char *)rx, strlen("flash erase")) == 0) {
		dataflash_erase_chip();
		puts("\nok");

	// READ
	} else if(strncmp("flash read", (char *)rx, strlen("flash read")) == 0) {
		uint32_t read_add = 0;
		uint8_t read_data_buffer[BUFFER_SIZE];
		dataflash_read_multiple_start(read_add);
		//char hexstr[8];
		do {
			/* Read data into buffer */
			dataflash_read_multiple_continue(read_data_buffer, BUFFER_SIZE);
			read_add += BUFFER_SIZE;
			for(int i=0; i<BUFFER_SIZE; i++) {
				//memset(hexstr, 0, sizeof(hexstr));
				//utoa(read_data_buffer[i], hexstr, 16);
				//fputs(hexstr, stdout);
				printf("%02X", read_data_buffer[i]);
			}
			puts("");
		} while (read_add <= TABLE_END_ADD); /* Read up till stored data */

		/* Finalize read from dataflash */
		dataflash_read_multiple_stop();

	// WRITE
	} else if(strncmp("flash write", (char *)rx, strlen("flash write")) == 0) {
		char * data_len_str = rx + 11;
		uint32_t data_len = strtoul(data_len_str, NULL, 10);
		puts("");
		//printf("%u", data_len);
		// TODO: check upper bound, can't write more bytes than we can store
		if(data_len == 0) {
			puts("Invalid data length");
			return 2;
		}

		char bytestr[3];
		bytestr[2] = 0;
		//dataflash_erase_sector_4k(0);
		USART_xoff();
		dataflash_erase_chip();
		char nextch;
		uint8_t chrcnt = 0;
		//dataflash_program_multiple_start(0);
		for(uint32_t p = 0; p<data_len; p++) {
			USART_xon();
			chrcnt = 0;
			while(chrcnt < 2) {
				nextch = USART_read();
				if(nextch == '\n')
					continue;
				bytestr[chrcnt] = nextch;
				chrcnt++;
			}
			USART_xoff();
			uint8_t val = strtoul(bytestr, NULL, 16);
			dataflash_program_byte(p, val);
			//dataflash_program_multiple_continue(&val, 1);
		}
		USART_xon();
		//dataflash_program_multiple_stop();
		puts("ok");

     //////////////////////////////////////////////////////////////////////////
	// LEDS
	//////////////////////////////////////////////////////////////////////////
	} else if(strncmp("led1 toggle", (char *)rx, strlen("led1 toggle")) == 0) {
		LED1_toggle_level();
		puts("\nok");
	} else if(strncmp("led2 toggle", (char *)rx, strlen("led1 toggle")) == 0) {
		LED2_toggle_level();
		puts("\nok");
	} else if(strncmp("led3 toggle", (char *)rx, strlen("led1 toggle")) == 0) {
		LED3_toggle_level();
		puts("\nok");
	} else if(strncmp("led4 toggle", (char *)rx, strlen("led1 toggle")) == 0) {
		LED4_toggle_level();
		puts("\nok");
	} else if(strncmp("led5 toggle", (char *)rx, strlen("led1 toggle")) == 0) {
		LED5_toggle_level();
		puts("\nok");
	} else if(strncmp("led6 toggle", (char *)rx, strlen("led1 toggle")) == 0) {
		LED6_toggle_level();
		puts("\nok");


	} else if(strncmp("led off", (char *)rx, strlen("led off")) == 0) {
		//LED_set_level(false);
		puts("\nok");
	} else if(strncmp("led on", (char *)rx, strlen("led on")) == 0) {
		//LED_set_level(true);
		puts("\nok");


	} else {
		fputs("\nInvalid command: ", stdout);
		puts(rx);
		return 0; // Error: Mismatch
	}

	return 1;
}
