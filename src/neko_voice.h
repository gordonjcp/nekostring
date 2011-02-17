/* nekostring - neko_voice.h */

/* nekostring polyphonic string ensemble
 *
 * Copyright (C) 2008 Gordon JC Pearce and others
 *
 * For further information on contributors and the history of nekostring,
 * please see the README
 *
*/

#ifndef _NEKO_VOICE_H
#define _NEKO_VOICE_H

#include <string.h>

#include <ladspa.h>
#include "dssi.h"

#include "neko_types.h"

/* maximum size of a rendering burst */
#define NEKO_NUGGET_SIZE      64

/* minBLEP constants */
/* minBLEP table oversampling factor (must be a power of two): */
#define MINBLEP_PHASES          64
/* MINBLEP_PHASES minus one: */
#define MINBLEP_PHASE_MASK      63
/* length in samples of (truncated) step discontinuity delta: */
#define STEP_DD_PULSE_LENGTH    72
/* length in samples of (truncated) slope discontinuity delta: */
#define SLOPE_DD_PULSE_LENGTH   71
/* the longer of the two above: */
#define LONGEST_DD_PULSE_LENGTH STEP_DD_PULSE_LENGTH
/* MINBLEP_BUFFER_LENGTH must be a power of two, equal to or greater
 * than NEKO_NUGGET_SIZE plus LONGEST_DD_PULSE_LENGTH: */
#define MINBLEP_BUFFER_LENGTH  256
/* MINBLEP_BUFFER_LENGTH minus one: */
#define MINBLEP_BUFFER_MASK    255
/* delay between start of DD pulse and the discontinuity, in samples: */
#define DD_SAMPLE_DELAY          4

struct _neko_patch_t
{
    char          name[31];

    float         bass;
    float cello;
    float         viola;
    float         violin;
    float speed;
    float         osc2_pulsewidth;
    float vibrato_chorus;
    float         mix;
    float         attack;
    float decay;
    float         bass_decay;
    float         tone;
    float         volume;
};

enum neko_voice_status
{
    NEKO_VOICE_OFF,       /* silent: is not processed by render loop */
    NEKO_VOICE_ON,        /* has not received a note off event */
    NEKO_VOICE_SUSTAINED, /* has received note off, but sustain controller is on */
    NEKO_VOICE_RELEASED   /* had note off, not sustained, in final decay phase of envelopes */
};

struct blosc
{
    int   last_waveform,    /* persistent */
          waveform,         /* comes from LADSPA port each cycle */
          state;            /* persistent */
    float pos,              /* persistent */
          pw;               /* comes from LADSPA port each cycle */
    float osc_audio[MINBLEP_BUFFER_LENGTH];
	float hp_last, hp_last_in;
	float lp_last;
	float lp_d1, lp_d2, lp_d3, lp_d4;
};

/*
 * neko_voice_t
 */
struct _neko_voice_t
{
    unsigned int  note_id;

    unsigned char status;
    unsigned char key;
    unsigned char velocity;
    unsigned char rvelocity;   /* the note-off velocity */

    /* translated controller values */
    float         pressure;    /* filter resonance multiplier, off = 1.0, full on = 0.0 */

    /* persistent voice state */
    float         prev_pitch,
                  target_pitch,
                  lfo_pos;
    struct blosc  osc_bass,
                  osc_cello,
					osc_viola,
				osc_violin;
    float         eg1,
                  eg2,
                  delay1,
                  delay2,
                  delay3,
                  delay4,
                  c5;
    unsigned char eg1_phase,
                  eg2_phase;
    int           osc_index;       /* shared index into voice->blosc.osc_audio, not voice->osc_audio! */
    float         osc_audio[NEKO_NUGGET_SIZE];
    float         vibrato_chorus[NEKO_NUGGET_SIZE];
    float         freqcut_buf[NEKO_NUGGET_SIZE];
    float         vca_buf[NEKO_NUGGET_SIZE];
};

#define _PLAYING(voice)    ((voice)->status != NEKO_VOICE_OFF)
#define _ON(voice)         ((voice)->status == NEKO_VOICE_ON)
#define _SUSTAINED(voice)  ((voice)->status == NEKO_VOICE_SUSTAINED)
#define _RELEASED(voice)   ((voice)->status == NEKO_VOICE_RELEASED)
#define _AVAILABLE(voice)  ((voice)->status == NEKO_VOICE_OFF)

extern float neko_pitch[128];

typedef struct { float value, delta; } float_value_delta;
extern float_value_delta step_dd_table[];

extern float slope_dd_table[];

/* neko_voice.c */
neko_voice_t *neko_voice_new(neko_synth_t *synth);
void            neko_voice_note_on(neko_synth_t *synth,
                                     neko_voice_t *voice,
                                     unsigned char key,
                                     unsigned char velocity);
void            neko_voice_remove_held_key(neko_synth_t *synth,
                                             unsigned char key);
void            neko_voice_note_off(neko_synth_t *synth,
                                      neko_voice_t *voice,
                                      unsigned char key,
                                      unsigned char rvelocity);
void            neko_voice_release_note(neko_synth_t *synth,
                                          neko_voice_t *voice);
void            neko_voice_set_ports(neko_synth_t *synth,
                                       neko_patch_t *patch);
void            neko_voice_update_pressure_mod(neko_synth_t *synth,
                                                 neko_voice_t *voice);

/* neko_voice_render.c */
void neko_init_tables(void);
void neko_voice_render(neko_synth_t *synth, neko_voice_t *voice,
                         LADSPA_Data *out, unsigned long sample_count,
                         int do_control_update);

/* inline functions */

/*
 * neko_voice_off
 * 
 * Purpose: Turns off a voice immediately, meaning that it is not processed
 * anymore by the render loop.
 */
static inline void
neko_voice_off(neko_voice_t* voice)
{
    voice->status = NEKO_VOICE_OFF;
    /* silence the oscillator buffers for the next use */
    if (voice->osc_index < MINBLEP_BUFFER_LENGTH - LONGEST_DD_PULSE_LENGTH) {
        int length = LONGEST_DD_PULSE_LENGTH * sizeof(float);
        memset(voice->osc_bass.osc_audio   + voice->osc_index, 0, length);
        memset(voice->osc_cello.osc_audio  + voice->osc_index, 0, length);
        memset(voice->osc_viola.osc_audio  + voice->osc_index, 0, length);
        memset(voice->osc_violin.osc_audio + voice->osc_index, 0, length);
    } else {
        int length = (MINBLEP_BUFFER_LENGTH - voice->osc_index) * sizeof(float);
        memset(voice->osc_bass.osc_audio   + voice->osc_index, 0, length);
        memset(voice->osc_cello.osc_audio  + voice->osc_index, 0, length);
        memset(voice->osc_viola.osc_audio  + voice->osc_index, 0, length);
        memset(voice->osc_violin.osc_audio + voice->osc_index, 0, length);
        length = (LONGEST_DD_PULSE_LENGTH * sizeof(float)) - length;
        memset(voice->osc_bass.osc_audio,   0, length);
        memset(voice->osc_cello.osc_audio,  0, length);
        memset(voice->osc_viola.osc_audio,  0, length);
        memset(voice->osc_violin.osc_audio, 0, length);
    }
    /* -FIX- decrement active voice count? */
}

/*
 * neko_voice_start_voice
 */
static inline void
neko_voice_start_voice(neko_voice_t *voice)
{
    voice->status = NEKO_VOICE_ON;
    /* -FIX- increment active voice count? */
}

#endif /* _NEKO_VOICE_H */
