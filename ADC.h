#ifndef _ADC_H_
#define _ADC_H_

/***** PUBLIC FUNCTIONS *****/
#define BUF_SIZE 190


/* initialize ADC input channels */
void ADC_Init(void);

/* public variables */
volatile long audio_buffer_sum;
volatile unsigned char flags;  // individual bit flags: (flags,0)=buf_full, (flags,1)=last_beat_flag, (flags,2)=energy_hist


#endif
