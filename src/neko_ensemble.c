/* nekostring - neko_ensemble.c */

/* nekostring polyphonic string ensemble
 *
 * (C) 2006 Gordon JC Pearce and others
 *
 * For further information on contributors and the history of nekostring,
 * please see the README
 *
 * The ensemble effect started life as Giant Flange from Steve Harris's plugins.
*/

#include <stdio.h>
#include <math.h>
#include "neko_synth.h"

#define BUF_MASK 1023
void
ensemble (float *left, float *right, unsigned long sample_count,
	 neko_synth_t * synth)
{
  unsigned long sample;
  unsigned long sample_rate = synth->sample_rate;
  unsigned long buffer_pos = synth->buffer_pos;

  float delay_1, delay_2, delay_3;

//  float xomega = 6.2831852f * (0.5 / (float)sample_rate);
//  float yomega = 6.2831852f * (5.75 / (float)sample_rate);

  float xomega = 1024.0f * (*(synth->speed)*0.5 / (float)sample_rate);
  float yomega = 1024.0f * (*(synth->speed)*5.75 / (float)sample_rate);

  float dr =  0.5* 0.002 * (float)sample_rate * 0.5f;	// FIXME - identify this bit

  float x1 = synth->ens_x1;
  float y1 = synth->ens_y1;
  float lp1 = synth->ens_lp1;
  float lp2 = synth->ens_lp2;
  

  float depth1=3;
  float depth2=*(synth->depth)*2.5;
  float mix=1;//0.5+0.5* *(synth->mix);
  float s1, s2;			// samples from the buffer

  float s_mix_1; //, s_mix_2, s_mix_3;			// mix for antialiasing
  float so1,so2,so3;
  float d1,d2,hp,lowpass,q;

  //float mult = WAVE_POINTS/6.28;

  lowpass=0.15f+(*(synth->tone)*0.6);

  for (sample = 0; sample < sample_count; sample++)
    {
      // Write input into delay line
      // we'll just use left as the input
	lp2 = lp2 + lowpass * lp1;
	hp =  left[sample] - lp2 -1.7 * lp1;
	lp1 = lowpass * hp + lp1;
    synth->ens_buffer[buffer_pos] = lp2;

	  // we have two LFOs, a fast one for the vibrato and a slow one for the chorus
	  // we need to create three delays, each modulated by the LFOs but offset by 120 degrees

	  //d1=200+20*sin(x1)+1.0f; d2=sin(y1)+1.0f;
	  d1 = 200 + 20*sine_wave[(int)(x1)]+1.0f; d2 = 0.5*sine_wave[(int)(y1)] + 1.0f;
	  delay_1 = 1100 + (d1*depth1+d2*depth2 * dr);

	  //d1=200+20*sin(x1+2.1)+1.0f; d2=0.5*sin(y1+2.1)+1.0f;
	  d1 = 200 + 20*sine_wave[(int)(x1+341) & BUF_MASK]+1.0f; d2 = 0.5*sine_wave[(int)(y1+341) & BUF_MASK] + 1.0f;
	  delay_2 = 1000 + (d1*depth1+d2*depth2 * dr);

	  //d1=200+20*sin(x1+4.2)+1.0f; d2=0.5*sin(y1+4.2)+1.0f;
	  d1 = 200 + 20*sine_wave[(int)(x1+682) & BUF_MASK]+1.0f; d2 = 0;//.5*sine_wave[(int)(y1+682) & BUF_MASK] + 1.0f;
	  delay_3 = 1200 + (d1*depth1+d2*depth2 * dr);

// get two samples from the buffer
// antialias them
// first sample

	  s1 = synth->ens_buffer[(buffer_pos - (int) delay_1) & BUF_MASK];
    s2 = synth->ens_buffer[((buffer_pos - (int) delay_1) - 1) & BUF_MASK];
    s_mix_1 = delay_1 - (int) delay_1;
	so1=(1-s_mix_1)*s1+s_mix_1*s2;

	s1 = synth->ens_buffer[(buffer_pos - (int) delay_2) & BUF_MASK];
    s2 = synth->ens_buffer[((buffer_pos - (int) delay_2) - 1) & BUF_MASK];
    s_mix_1 = delay_2 - (int) delay_2;
	so2=(1-s_mix_1)*s1+s_mix_1*s2;

	s1 = synth->ens_buffer[(buffer_pos - (int) delay_3) & BUF_MASK];
    s2 = synth->ens_buffer[((buffer_pos - (int) delay_3) - 1) & BUF_MASK];
    s_mix_1 = delay_3 - (int) delay_3;
	so3=(1-s_mix_1)*s1+s_mix_1*s2;

/*
	  s1 = synth->ens_buffer[(buffer_pos - (int) delay_1) % WAVE_POINTS];
	  s2 = synth->ens_buffer[((buffer_pos - (int) delay_1) - 1) % WAVE_POINTS];
	  s_mix_1 = delay_1 - (int) delay_1;
	  so1=(1-s_mix_1)*s1+s_mix_1*s2;

	  s1 = synth->ens_buffer[(buffer_pos - (int) delay_2) % WAVE_POINTS];
	  s2 = synth->ens_buffer[((buffer_pos - (int) delay_2) - 1) % WAVE_POINTS];
	  s_mix_1 = delay_2 - (int) delay_2;
	  so2=(1-s_mix_1)*s1+s_mix_1*s2;

	  s1 = synth->ens_buffer[(buffer_pos - (int) delay_3) % WAVE_POINTS];
	  s2 = synth->ens_buffer[((buffer_pos - (int) delay_3) - 1) % WAVE_POINTS];
	  s_mix_1 = delay_3 - (int) delay_3;
	  so3=(1-s_mix_1)*s1+s_mix_1*s2;
*/
// mix is the wet/dry proportion
	right[sample] =	(left[sample] * (1 - mix)) + ((so1+so2)/2 * mix);
	left[sample] =	(left[sample] * (1 - mix)) + ((so2+so3)/2 * mix);

      buffer_pos = (buffer_pos + 1) & (BUF_MASK);


      // Run LFOs
	  	x1+=xomega;
	y1+=yomega;
	if (x1>1024) x1-=1024;
	if (y1>1024) y1-=1024;


}	
  synth->ens_x1 = x1;
  synth->ens_y1 = y1;
  synth->ens_lp1 = lp1;
  synth->ens_lp2 = lp2;
  synth->buffer_pos = buffer_pos;

  
}
