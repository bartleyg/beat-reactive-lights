#ifndef _RGB_LED_H_
#define _RGB_LED_H_

// tcnt# = pulse-width-in-ms * 65535 tcnts / period(20ms for 50Hz, 16.4 ms for 61Hz)
#define LED_MAX 0xFF

#define LEDR OCR0A  //PB0
#define LEDB OCR1B  //PB4
#define LEDG OCR0B  //PB1


// initialize RGB LED timers
void RGB_LED_Init(void);


#endif
