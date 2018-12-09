/*
 * blinks.h
 *
 * Created: 10/1/2018 10:03:04 PM
 *  Author: steven
 */ 


#ifndef BLINKS_H_
#define BLINKS_H_


void blinks(bool *interrupt);
void reset_leds(void);
void pairs(bool *interrupt);
void chase(bool *interrupt);
void rblink(bool *interrupt);
void outin(bool *interrupt);


#endif /* BLINKS_H_ */