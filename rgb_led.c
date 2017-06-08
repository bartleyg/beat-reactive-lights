#include "helper.h"
#include "rgb_led.h"

void RGB_LED_Init(void) {

  /* Timer 0: 2 channels: OCR0A, OCR0B */
  set(DDRB,PB0);  // OC0A is PB0 output
  set(DDRB,PB1);  // OC0B is PB1 output

  set(TCCR0B,CS02); // timer prescalar /256 (~122 Hz in fast PWM, ~61 Hz in phase correct PWM)
  clear(TCCR0B,CS01);
  clear(TCCR0B,CS00);
  
  clear(TCCR0B,WGM02);  // UP to 0xFF, phase correct PWM mode
  clear(TCCR0A,WGM01);
  set(TCCR0A,WGM00);

// (standard high driving common cathode)
  set(TCCR0A,COM0A1); // clear at OCR0A, set at 0xFF
  clear(TCCR0A,COM0A0);

  set(TCCR0A,COM0B1); // clear at OCR0B, set at 0xFF
  clear(TCCR0A,COM0B0);
/*
  set(TCCR0A,COM0A1); // set at OCR0A, clear at 0xFF (inverting)
  set(TCCR0A,COM0A0);

  set(TCCR0A,COM0B1); // set at OCR0B, clear at 0xFF (inverting)
  set(TCCR0A,COM0B0);
*/

  /* Timer 1: channel OCR1B */
  set(DDRB,PB4);  // OC1B is PB4 output

  set(TCCR1,CS13); // timer prescalar /512 (~61 Hz)
  clear(TCCR1,CS12);
  set(TCCR1,CS11);
  clear(TCCR1,CS10);

  set(GTCCR,PWM1B);  // UP to OCR1C, PWM mode

// (standard high driving common cathode)
  set(GTCCR,COM1B1); // clear at 0CR1B, set at 0x00
  clear(GTCCR,COM1B0);
/*
  set(GTCCR,COM1B1); // set at 0CR1B, clear at 0x00 (inverting)
  set(GTCCR,COM1B0);
*/
  OCR1C = LED_MAX;  // set Timer 1 top(frequency) to 0xFF

}
