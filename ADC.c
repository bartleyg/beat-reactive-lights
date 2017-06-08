#include "helper.h"
#include "ADC.h"
#include <stdlib.h>// for abs

static unsigned char i_buf; // index in audio buffer to store ADC value

void ADC_Init(void) {
  // set voltage reference to VCC
  clear(ADMUX,REFS1);
  clear(ADMUX,REFS0);

  // set ADC clock (/64 = 128 KHz)
  set(ADCSRA,ADPS2);
  set(ADCSRA,ADPS1);
  clear(ADCSRA,ADPS0);

  // disable digital input on ADC3/PB3 pins
  set(DIDR0,ADC3D); // ADC3/PB3

  // set up interrupts
  set(ADCSRA,ADIE);
  
  // ADC3/PB3 channel selected
  clear(ADMUX,MUX3);
  clear(ADMUX,MUX2);
  set(ADMUX,MUX1);
  set(ADMUX,MUX0);
  
  // buffer index and flag
  i_buf = 0;
  flags = 0;

  // DEBUG output toggle freq
  //set(DDRB,PB0);

  // enable ADC subsystem
  set(ADCSRA,ADEN);
}

/* runs when each ADC conversion completes */
ISR(ADC_vect) {
  if(i_buf < BUF_SIZE-1) {  // add difference from 0-level to buffer sum
    audio_buffer_sum += abs(ADC-512);
  }
  else if(i_buf == BUF_SIZE-1) {  // add last sample and signal main that buffer is full
    audio_buffer_sum += abs(ADC-512);
    set(flags,0);
  }
  else {  // index is BUF_SIZE (aka 0) when here so reset running sum
    audio_buffer_sum = abs(ADC-512);
    i_buf = 0;
  }

  i_buf++;

  //toggle(PORTB,PB0);  // debug freq is 2x what oscope shows

  // begin next ADC conversion
  set(ADCSRA,ADSC);
}
