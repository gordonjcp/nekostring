/* nekostring - neko_voice_render.c */

/* nekostring polyphonic string ensemble
 *
 * Copyright (C) 2008 Gordon JC Pearce and others
 *
 * For further information on contributors and the history of nekostring,
 * please see the README
 *
*/

#define _BSD_SOURCE    1
#define _SVID_SOURCE   1
#define _ISOC99_SOURCE 1

#include <math.h>

#include <ladspa.h>

#include "neko.h"
#include "neko_synth.h"
#include "neko_voice.h"

#define M_2PI_F (2.0f * (float)M_PI)
#define M_PI_F (float)M_PI

#define VCF_FREQ_MAX  (0.825f)    /* original filters only stable to this frequency */

static int   tables_initialized = 0;

float        neko_pitch[128];


//static float sine_wave[4 + WAVE_POINTS + 1], triangle_wave[4 + WAVE_POINTS + 1];

#define pitch_ref_note 69

#define volume_to_amplitude_scale 128

static float volume_to_amplitude_table[4 + volume_to_amplitude_scale + 2];

static float velocity_to_attenuation[128];

static float qdB_to_amplitude_table[4 + 256 + 0];

void
neko_init_tables(void)
{
    int i, qn, tqn;
    float pexp;
    float volume, volume_exponent;
    float ol, amp;

    if (tables_initialized)
        return;

    /* oscillator waveforms */
    for (i = 0; i <= WAVE_POINTS; ++i) {
        sine_wave[i] = sinf(M_2PI_F * (float)i / (float)WAVE_POINTS); //* 0.5f;
    }
    //sine_wave[-1 + 4] = sine_wave[WAVE_POINTS - 1 + 4];  /* guard points both ends */

    qn = WAVE_POINTS / 4;
    tqn = 3 * WAVE_POINTS / 4;

    for (i = 0; i <= WAVE_POINTS; ++i) {
        if (i < qn)
            triangle_wave[i + 4] = (float)i / (float)qn;
        else if (i < tqn)
            triangle_wave[i + 4] = 1.0f - 2.0f * (float)(i - qn) / (float)(tqn - qn);
        else
            triangle_wave[i + 4] = (float)(i - tqn) / (float)(WAVE_POINTS - tqn) - 1.0f;
    }
    triangle_wave[-1 + 4] = triangle_wave[WAVE_POINTS - 1 + 4];

    /* MIDI note to pitch */
    for (i = 0; i < 128; ++i) {
        pexp = (float)(i - pitch_ref_note) / 12.0f;
        neko_pitch[i] = powf(2.0f, pexp);//;
    }

    /* volume to amplitude
     *
     * This generates a curve which is:
     *  volume_to_amplitude_table[128 + 4] = 0.25 * 3.16...   ~=  -2dB
     *  volume_to_amplitude_table[64 + 4]  = 0.25 * 1.0       ~= -12dB
     *  volume_to_amplitude_table[32 + 4]  = 0.25 * 0.316...  ~= -22dB
     *  volume_to_amplitude_table[16 + 4]  = 0.25 * 0.1       ~= -32dB
     *   etc.
     */
    volume_exponent = 1.0f / (2.0f * log10f(2.0f));
    for (i = 0; i <= volume_to_amplitude_scale; i++) {
        volume = (float)i / (float)volume_to_amplitude_scale;
        volume_to_amplitude_table[i + 4] = powf(2.0f * volume, volume_exponent) / 4.0f;
    }
    volume_to_amplitude_table[ -1 + 4] = 0.0f;
    volume_to_amplitude_table[129 + 4] = volume_to_amplitude_table[128 + 4];

    /* velocity to attenuation
     *
     * Creates the velocity to attenuation lookup table, for converting
     * velocities [1, 127] to full-velocity-sensitivity attenuation in
     * quarter decibels.  Modeled after my TX-7's velocity response.*/
    velocity_to_attenuation[0] = 253.9999f;
    for (i = 1; i < 127; i++) {
        if (i >= 10) {
            ol = (powf(((float)i / 127.0f), 0.32f) - 1.0f) * 100.0f;
            amp = powf(2.0f, ol / 8.0f);
        } else {
            ol = (powf(((float)10 / 127.0f), 0.32f) - 1.0f) * 100.0f;
            amp = powf(2.0f, ol / 8.0f) * (float)i / 10.0f;
        }
        velocity_to_attenuation[i] = log10f(amp) * -80.0f;
    }
    velocity_to_attenuation[127] = 0.0f;

    /* quarter-decibel attenuation to amplitude */
    qdB_to_amplitude_table[-1 + 4] = 1.0f;
    for (i = 0; i <= 255; i++) {
        qdB_to_amplitude_table[i + 4] = powf(10.0f, (float)i / -80.0f);
    }

    tables_initialized = 1;
}

static inline float
volume(float level)
{
    unsigned char segment;
    float fract;

    level *= (float)volume_to_amplitude_scale;
    segment = lrintf(level - 0.5f);
    fract = level - (float)segment;

    return volume_to_amplitude_table[segment + 4] + fract *
               (volume_to_amplitude_table[segment + 5] -
                volume_to_amplitude_table[segment + 4]);
}

static inline float
qdB_to_amplitude(float qdB)
{
    int i = lrintf(qdB - 0.5f);
    float f = qdB - (float)i;
    return qdB_to_amplitude_table[i + 4] + f *
           (qdB_to_amplitude_table[i + 5] -
            qdB_to_amplitude_table[i + 4]);
}


static inline void
blosc_place_step_dd(float *buffer, int index, float phase, float w, float scale)
{
    float r;
    int i;

    r = MINBLEP_PHASES * phase / w;
    i = lrintf(r - 0.5f);
    r -= (float)i;
    i &= MINBLEP_PHASE_MASK;  /* port changes can cause i to be out-of-range */
    /* This would be better than the above, but more expensive:
     *  while (i < 0) {
     *    i += MINBLEP_PHASES;
     *    index++;
     *  }
     */

    while (i < MINBLEP_PHASES * STEP_DD_PULSE_LENGTH) {
        buffer[index & MINBLEP_BUFFER_MASK] += scale * (step_dd_table[i].value + r * step_dd_table[i].delta);
        i += MINBLEP_PHASES;
        index++;
    }
}


static inline void
blosc_place_slope_dd(float *buffer, int index, float phase, float w, float slope_delta)
{
    float r;
    int i;

    r = MINBLEP_PHASES * phase / w;
    i = lrintf(r - 0.5f);
    r -= (float)i;
    i &= MINBLEP_PHASE_MASK;  /* port changes can cause i to be out-of-range */

    slope_delta *= w;

    while (i < MINBLEP_PHASES * SLOPE_DD_PULSE_LENGTH) {
        buffer[index & MINBLEP_BUFFER_MASK] += slope_delta * (slope_dd_table[i] + r * (slope_dd_table[i + 1] - slope_dd_table[i]));
        i += MINBLEP_PHASES;
        index++;
    }
}


void vco(unsigned long sample_count, neko_voice_t *voice, struct blosc *osc, int index0,
	float highpass, float lowpass, float gain, float w) {

  gain *= 0.5f;
  int index = index0;
  unsigned long sample;
  float pos = osc->pos;
  int   state = osc->state;
  float pw = 0.5f;  /* width of saw portion of waveform, from 0 to 1 */
  float bias, slope;
  float out;
  float hp_last, hp_last_in, lp_last;
  hp_last=osc->hp_last;
  hp_last_in=osc->hp_last_in;
  lp_last = osc->lp_last;
  
  float lp1 = osc->lp_d1, lp2=osc->lp_d2, hp;
  float lp3 = osc->lp_d3, lp4=osc->lp_d4;


  // clamp offscale filter values
  if (highpass < 0.0001) highpass = 0.0001;
  if (highpass > .995) highpass = .995;
  if (lowpass < 0.0001) lowpass = 0.0001;
  if (lowpass > .995) lowpass = .995;
  if (gain < 0.0001) gain = 0.0001;
  if (pw < w) pw = w;  /* w is sample phase width */
  else if (pw > 1.0f - w) pw = 1.0f - w;
  bias = -0.5f * pw;  /* to correct for DC offset */
  slope = 1.0f / pw;

  //freqcut = cutoff[sample] * 2.0f;


  for (sample=0; sample < sample_count; sample++) {

	pos += w;

	if (state == 0) {  /* first half of waveform : descending saw */
            out = 1.0f - pos * slope + bias;
            if (pos >= pw) {
                out = bias;
		blosc_place_slope_dd(osc->osc_audio, index, pos - pw, w, slope * gain);
                state = 1;
            }
            if (pos >= 1.0f) {
                pos -= 1.0f;
                out = 1.0f - pos * slope + bias;
		blosc_place_step_dd(osc->osc_audio, index, pos, w, gain);
		blosc_place_slope_dd(osc->osc_audio, index, pos, w, -slope * gain);
                state = 0;
            }
	} else {  /* second half of waveform: chopped off portion */
            out = bias;
            if (pos >= 1.0f) {
                pos -= 1.0f;
                out = 1.0f - pos * slope + bias;
		blosc_place_step_dd(osc->osc_audio, index, pos, w, gain);
		blosc_place_slope_dd(osc->osc_audio, index, pos, w, -slope * gain);
                state = 0;
            }
            if (!state && pos >= pw) {
                out = bias;
		blosc_place_slope_dd(osc->osc_audio, index, pos - pw, w, slope * gain);
                state = 1;
            }
        }

	osc->osc_audio[(index + DD_SAMPLE_DELAY) & MINBLEP_BUFFER_MASK] += gain * out;
	index++;
  }

	// qres goes from 2 (fully down) to 0.05 (fully up)

	index = index0;
	for (sample=0; sample < sample_count; sample++) {
		out = osc->osc_audio[index & MINBLEP_BUFFER_MASK];
		osc->osc_audio[index & MINBLEP_BUFFER_MASK] = 0.0f; /* silence for next time around */
		index++;
	// 12dB/oct filter code
	/*
		lp2 = lp2 + lowpass * lp1;
		hp = out - lp2 - 1.17f * lp1;
		lp1 = lowpass * hp + lp1;
	
		lp4 = lp4 + highpass * lp3;
		hp = lp2 - lp4 - 1.7f * lp3;
		lp3 = highpass * hp + lp3;
	
		out=hp;
	*/
	// 6dB/oct filter code (more like the Solina filters)
		out=(lowpass*out)+((1-lowpass)*lp_last);
		lp_last=out;
		hp_last = highpass * hp_last + out - hp_last_in;
		hp_last_in = out;
		out=hp_last;
		voice->osc_audio[sample] += out;
	}

		  	osc->hp_last = hp_last;
			osc->hp_last_in = hp_last_in;
		  	osc->lp_last = lp_last;
			
			osc->lp_d1=lp1;
			osc->lp_d2=lp2;
			osc->lp_d3=lp3;
			osc->lp_d4=lp4;	
	
        osc->pos=pos;
	osc->state=state;
}


/*
 * neko_voice_render
 *
 * generate the actual sound data for this voice
 */
void
neko_voice_render(neko_synth_t *synth, neko_voice_t *voice,
                    LADSPA_Data *out, unsigned long sample_count,
                    int do_control_update)
{
    unsigned long sample;

    /* state variables saved in voice */

    float         lfo_pos    = voice->lfo_pos,
                  eg1        = voice->eg1,
                  eg2        = voice->eg2;
    unsigned char eg1_phase  = voice->eg1_phase,
                  eg2_phase  = voice->eg2_phase;
    int           osc_index  = voice->osc_index;

    /* temporary variables used in calculating voice */

    float fund_pitch;
    float deltat = synth->deltat;

    /* set up synthesis variables from patch */
    float         omega1;
	float kf;

    float         eg1_amp = 1; // qdB_to_amplitude(velocity_to_attenuation[voice->velocity]);
    float         eg1_rate_level[3], eg1_one_rate[3];

    float         eg2_amp = 1;// qdB_to_amplitude(velocity_to_attenuation[voice->velocity]);
    float         eg2_rate_level[3], eg2_one_rate[3];

    float         vol_out = volume(*(synth->volume) * synth->cc_volume);
	
	float attack = 0.001 * exp(-2* (1-*(synth->attack))*log(10));
	float decay = 0.001 * exp(-2* (1-*(synth->decay))*log(10));
	float bass_decay = 0.001 * exp(-2* (1-*(synth->bass_decay))*log(10));



    fund_pitch =  voice->target_pitch;
    fund_pitch *= synth->pitch_bend * *(synth->tuning);

    omega1 = fund_pitch;

    eg1_rate_level[0] = attack * eg1_amp;  /* eg1_attack_time * 1.0f * eg1_amp */
    eg1_one_rate[0] = 1.0f - attack;
    eg1_rate_level[1] = decay * 1 *eg1_amp;
    eg1_one_rate[1] = 1.0f - decay;
    eg1_rate_level[2] = 0.0f;                                 /* eg1_release_time * 0.0f * eg1_amp */
    eg1_one_rate[2] = 1.0f - decay;
    eg2_rate_level[0] = 0;//*(synth->eg2_attack_time) * eg2_amp;
    eg2_one_rate[0] = 1.0f;// - *(synth->eg2_attack_time);
    eg2_rate_level[1] = 0; //*(synth->bass_decay) * *(synth->eg2_sustain_level) * */eg2_amp;
    eg2_one_rate[1] = 1.0f - bass_decay;
    eg2_rate_level[2] = 0.0f;
    eg2_one_rate[2] = 1.0f - bass_decay;

    eg1_amp *= 0.99f;  /* XSynth's original eg phase 1 to 2 transition check was:  */
    eg2_amp *= 0.99f;  /*    if (!eg1_phase && eg1 > 0.99f) eg1_phase = 1;         */


    /* --- LFO, EG1, and EG2 section */

    for (sample = 0; sample < sample_count; sample++) {

        eg1 = eg1_rate_level[eg1_phase] + eg1_one_rate[eg1_phase] * eg1;
        eg2 = eg2_rate_level[eg2_phase] + eg2_one_rate[eg2_phase] * eg2;
        voice->vca_buf[sample] = eg1 * vol_out;

        if (!eg1_phase && eg1 > eg1_amp) eg1_phase = 1;  /* flip from attack to decay */
        if (!eg2_phase && eg2 > eg2_amp) eg2_phase = 1;  /* flip from attack to decay */
    }

    /* --- VCO 1 section */


	kf = omega1/(float)(synth->sample_rate)*10.0f;

	//printf("%f %f\n",omega1, kf);
	
	// generate cello
	vco(sample_count, voice, &voice->osc_cello, osc_index, 0.97, 0.3-(kf/2), *(synth->cello), deltat * omega1/2);
	// generate viola
	vco(sample_count, voice, &voice->osc_viola, osc_index, 0.9-kf, 0.3+kf, *(synth->viola)-(.25-(kf/2)), deltat * omega1);
	// generate violin
	
	vco(sample_count, voice, &voice->osc_violin, osc_index, 0.8, 0.35+kf, *(synth->violin)/*-(.25-(kf/2))*/, deltat * omega1*2);

	// apply the VCA, copying the buffer as we go
	for (sample = 0; sample < sample_count; sample++) {
		out[sample] += voice->osc_audio[sample] * voice->vca_buf[sample];
		voice->osc_audio[sample] = 0.0f;  /* silence voice audio buffer for the next use */
	}

    osc_index = (osc_index + sample_count) & MINBLEP_BUFFER_MASK;

    if (do_control_update) {
        /* do those things should be done only once per control-calculation
         * interval ("nugget"), such as voice check-for-dead, pitch envelope
         * calculations, volume envelope phase transition checks, etc. */

        /* check if we've decayed to nothing, turn off voice if so */
        if (eg1_phase == 2 &&
            voice->vca_buf[sample_count - 1] < 6.26e-6f) {
            /* sound has completed its release phase (>96dB below volume '5' max) */

            XDB_MESSAGE(XDB_NOTE, " neko_voice_render check for dead: killing note id %d\n", voice->note_id);
            neko_voice_off(voice);
            return; /* we're dead now, so return */
        }

        /* already saved prev_pitch above */
    }

    /* save things for next time around */

    voice->lfo_pos    = lfo_pos;
    voice->eg1        = eg1;
    voice->eg1_phase  = eg1_phase;
    voice->eg2        = eg2;
    voice->eg2_phase  = eg2_phase;
    voice->osc_index  = osc_index;
}
