#include "helper.h"
#include "ADC.h"
#include "rgb_led.h"
#include <stdlib.h>// for abs/rand


#define NUM_NRG_SAMPLES 44
#define BUF_NRG_SIZE 66
#define NRG_THRESH_NUM 13   // avg*NUM/DEN + avg/DEN2 = beat percentage above avg for detect
#define NRG_THRESH_DEN 10   // = 1.35 threshold
#define NRG_THRESH_DEN2 20
#define NRG_AVG_THRESH 350  // threshold "silence" for 190 buf = 1.84 avg indiv sample
#define AUDIO_BUF_SUM_MAX 97090 // saturation point


// initialize clock
void Clock_Init(void) {
  CLKPR = (1<<CLKPCE);
  CLKPR = 0; // divide by 2^N, 0=8MHz
}

int main(void) {

  static char audio_energy_buf[BUF_NRG_SIZE];  // store sum of each 1024 audio buffer in array
  unsigned char i_energy = 0, i_store;  // index into audio_energy_buf[]
  int audio_buffer_sum_div = 0;
  long energy_run_avg = 0;
  unsigned char colorR, colorG, colorB;
  int randnum;
  int tmp_remove;  


  Clock_Init();
  ADC_Init();
  RGB_LED_Init();

  // start ADC conversion, ignoring first result
  set(ADCSRA,ADSC);
  while(check(ADCSRA,ADSC)) {}

  // seed random number generator with next ADC reading
  set(ADCSRA,ADSC);
  while(check(ADCSRA,ADSC)) {}
  srand(ADC);

  sei(); // enable global interrupts


  // loop forever necessary
  while(1) {

    if(check(flags,0)) {  // sum the buffer and store only if it's full of 1 sec audio sample
      
      if(!check(flags,2)) {//if(!energy_hist) {
        audio_buffer_sum_div = audio_buffer_sum/NUM_NRG_SAMPLES;  // pre-divide and add error bias
        energy_run_avg += audio_buffer_sum_div; // add new value
        
        /* pack intermediate avg value into bit array to subtract later */
        i_store = i_energy + i_energy/2;  // optimization =i_energy *66/44 = *3/2 = 1+1/2 = *(BUF_NRG_SIZE/NUM_NRG_SAMPLES)
        if(i_energy%2) {  // odd = starts in middle byte
          audio_energy_buf[i_store] = (char)((audio_energy_buf[i_store] & 0x0F) | (audio_buffer_sum_div << 4));
          audio_energy_buf[i_store+1] = (char)(audio_buffer_sum_div >> 4);
        }
        else {  // even = starts aligned with byte
          audio_energy_buf[i_store] = (char)audio_buffer_sum_div;
          audio_energy_buf[i_store+1] = (char)((audio_energy_buf[i_store+1] & 0xF0) | (audio_buffer_sum_div >> 8));
        }

      }

      // first run case to make sure enough sample history is obtained first
      if(!check(flags,2) && (i_energy == NUM_NRG_SAMPLES-1)) {//if(!energy_hist && (i_energy == BUF_NRG_SIZE-1)) {
        set(flags,2);//energy_hist = 1;
      }

      // compare new local energy to 1 sec energy history average to detect a beat
      if(check(flags,2)) {//if(energy_hist) {

        /* unpack old intermediate avg value to subtract from running average */
        i_store = i_energy + i_energy/2;  // optimization =i_energy *66/44 = *3/2 = 1+1/2 = *(BUF_NRG_SIZE/NUM_NRG_SAMPLES)
        if(i_energy%2) {  // odd = starts in middle byte
          tmp_remove = (int)audio_energy_buf[i_store] >> 4;
          tmp_remove = tmp_remove | ((int)audio_energy_buf[i_store+1] << 4);
        }
        else {  // even = starts aligned with byte
          tmp_remove = (int)audio_energy_buf[i_store];
          tmp_remove = tmp_remove | (((int)audio_energy_buf[i_store+1] & 0x0F) << 8);
        }

        energy_run_avg -= tmp_remove; // remove old value
        audio_buffer_sum_div = audio_buffer_sum/NUM_NRG_SAMPLES;  // pre-divide and add error bias
        energy_run_avg += audio_buffer_sum_div; // add new value

        /* pack intermediate avg value into bit array to subtract later */
        if(i_energy%2) {  // odd = starts in middle byte
          audio_energy_buf[i_store] = (audio_energy_buf[i_store] & 0x0F) | ((char)audio_buffer_sum_div << 4);
          audio_energy_buf[i_store+1] = (char)((int)audio_buffer_sum_div >> 4);
        }
        else {  // even = starts aligned with byte
          audio_energy_buf[i_store] = (char)audio_buffer_sum_div;
          audio_energy_buf[i_store+1] = (audio_energy_buf[i_store+1] & 0xF0) | (char)((int)audio_buffer_sum_div >> 8);
        }


        if(energy_run_avg > NRG_AVG_THRESH) { // loud enough to eliminate silent false positives

          // compare local energy to ratio above average for "beat" or "beat" if MAXed
          if((audio_buffer_sum > (energy_run_avg*NRG_THRESH_NUM/NRG_THRESH_DEN + energy_run_avg/NRG_THRESH_DEN2)) || audio_buffer_sum >= AUDIO_BUF_SUM_MAX) // > 1.35*avg
	    set(flags,3); // current_beat_flag=1
	  else
	    clear(flags,3); // current_beat_flag=0

	  // LED on if current beat or if current isn't beat but last was (fake +1 beat length to avoid beat strobing)
	  if(check(flags,3) || (!check(flags,3) && check(flags,1))) {

            // make new LED color if new beat (last sample was not beat AND last was not fake +1 beat)
            if(!check(flags,1) && !check(flags,4)) { //if(!last_beat_flag)
              cli();  // disable interrupts during rand() call to minimize stack/ram usage (prevents 15 extra bytes pushed on stack when ADC interrupts)
              randnum = rand(); // use random # more than once
              sei();  // re-enable ADC interrupt
              switch(randnum%7) {  // colors are specific to SuperBright RL5-RGB-DCA
                case 0: // white
                  colorR = LED_MAX/2;
                  colorG = LED_MAX/4;
                  colorB = LED_MAX/4;
                  break;
                case 1: // red
                  colorR = LED_MAX;
                  colorG = 0;
                  colorB = 0;
                  break;
                case 2: // magenta
                  colorR = LED_MAX*2/3;
                  colorG = 0;
                  colorB = LED_MAX/3;
                  break;
                case 3: // blue
                  colorR = 0;
                  colorG = 0;
                  colorB = LED_MAX;
                  break;
                case 4: // cyan
                  colorR = 0;
                  colorG = LED_MAX/2;
                  colorB = LED_MAX/2;
                  break;
                case 5: // green
                  colorR = 0;
                  colorG = LED_MAX;
                  colorB = 0;
                  break;
                case 6: // yellow
                  colorR = LED_MAX*3/4;
                  colorG = LED_MAX/4;
                  colorB = 0;
                  break;
              }
              // LED on new color
              LEDR = colorR;
              LEDG = colorG;
              LEDB = colorB;
            }//end make new LED color

	    // set flag for next iteration (wether to change color or not) if this is fake beat
	    if(!check(flags,3) && check(flags,1)) // this isn't and last was
	      set(flags,4); // this itr was fake beat
	    else
	      clear(flags,4); // was not fake beat

	    if(check(flags,3)) // if current was beat
	      set(flags,1); //last_beat_flag = 1;
	    else
	      clear(flags,1); //last_beat_flag = 0;
            
          }// end if current beat or fake beat LED on
          else { // LED off
            LEDR = 0;
            LEDG = 0;
            LEDB = 0;
            clear(flags,1);//last_beat_flag = 0;
	    clear(flags,4); // was not fake beat
          }
          
        }// end if(energy_average > NRG_AVG_THRESH)
        else { // too quiet: LED off to prevent false positives
          LEDR = 0;
          LEDG = 0;
          LEDB = 0;
          clear(flags,1);//last_beat_flag = 0;
	  clear(flags,4); // was not fake beat
        }


      }// end if(energy_hist)
            
      
      /* must be done at end of each time buffer full */
      if(i_energy < NUM_NRG_SAMPLES-1)  // move index
        i_energy++;
      else  // we have 1 sec audio energy history
        i_energy = 0;

      clear(flags,0);//buf_full = 0; // reset flag to get next sample group sum
    
    }// end if(buf_full)


  }// end loop forever

}// end main
