/* nekostring - neko_synth.h */

/* nekostring polyphonic string ensemble
 *
 * (C) 2006 Gordon JC Pearce and others
 *
 * For further information on contributors and the history of nekostring,
 * please see the README
 *
*/

#ifndef _NEKO_SYNTH_H
#define _NEKO_SYNTH_H

#include <pthread.h>

#include <ladspa.h>
#include "dssi.h"

#include "neko.h"
#include "neko_types.h"

#define NEKO_MONO_MODE_OFF  0
#define NEKO_MONO_MODE_ON   1
#define NEKO_MONO_MODE_ONCE 2
#define NEKO_MONO_MODE_BOTH 3

#define NEKO_GLIDE_MODE_LEGATO   0
#define NEKO_GLIDE_MODE_INITIAL  1
#define NEKO_GLIDE_MODE_ALWAYS   2
#define NEKO_GLIDE_MODE_LEFTOVER 3
#define NEKO_GLIDE_MODE_OFF      4

#define WAVE_POINTS 1024          /* must be a power of two */

//float sine_wave[], triangle_wave[];

extern float sine_wave[4 + WAVE_POINTS + 1], triangle_wave[4 + WAVE_POINTS + 1];

/*
 * neko_synth_t
 */
struct _neko_synth_t {
    /* output */
    LADSPA_Data    *left;
	LADSPA_Data		*right;
    unsigned long   sample_rate;
    float           deltat;            /* 1 / sample_rate */
    unsigned long   nugget_remains;

    /* bottom octave generator */

    float			bog_pos[12];
    /* voice tracking and data */
    unsigned int    note_id;           /* incremented for every new note, used for voice-stealing prioritization */
    int             polyphony;         /* requested polyphony, must be <= NEKO_MAX_POLYPHONY */
    int             voices;            /* current polyphony, either requested polyphony above or 1 while in monophonic mode */
    int             monophonic;        /* true if operating in monophonic mode */
    int             glide;             /* current glide mode */
    float           last_noteon_pitch; /* glide start pitch for non-legato modes */
    signed char     held_keys[8];      /* for monophonic key tracking, an array of note-ons, most recently received first */
    
    pthread_mutex_t voicelist_mutex;
    int             voicelist_mutex_grab_failed;

    neko_voice_t *voice[NEKO_MAX_POLYPHONY];

    pthread_mutex_t patches_mutex;
    neko_patch_t *patches;
    int             pending_program_change;
    int             current_program;

    /* current non-LADSPA-port-mapped controller values */
    unsigned char   key_pressure[128];
    unsigned char   cc[128];                  /* controller values */
    unsigned char   channel_pressure;
    unsigned char   pitch_wheel_sensitivity;  /* in semitones */
    int             pitch_wheel;              /* range is -8192 - 8191 */

	// gjcp - nasty hackery for ensemble (should be malloced)
	unsigned long buffer_pos;
	float ens_buffer[1280];
	float ens_x1, ens_y1;
	float ens_lp1, ens_lp2;

    /* translated controller values */
    float           mod_wheel;                /* filter cutoff multiplier, off = 1.0, full on = 0.0 */
    float           pitch_bend;               /* frequency multiplier, product of wheel setting and sensitivity, center = 1.0 */
    float           cc_volume;                /* volume multiplier, 0.0 to 1.0 */

    /* LADSPA ports / Neko patch parameters */
    LADSPA_Data    *bass;
    LADSPA_Data    *cello;
    LADSPA_Data    *viola;
    LADSPA_Data    *violin;
    LADSPA_Data    *speed;
    LADSPA_Data    *depth;
    LADSPA_Data    *vibrato_chorus;
    LADSPA_Data    *mix;
    LADSPA_Data    *attack;
    LADSPA_Data    *decay;
    LADSPA_Data    *bass_decay;
    LADSPA_Data    *tone;
    LADSPA_Data    *volume;
    LADSPA_Data    *tuning;
};

void  neko_synth_all_voices_off(neko_synth_t *synth);
void  neko_synth_note_off(neko_synth_t *synth, unsigned char key,
                            unsigned char rvelocity);
void  neko_synth_all_notes_off(neko_synth_t *synth);
void  neko_synth_note_on(neko_synth_t *synth, unsigned char key,
                           unsigned char velocity);
void  neko_synth_key_pressure(neko_synth_t *synth, unsigned char key,
                                unsigned char pressure);
void  neko_synth_damp_voices(neko_synth_t *synth);
void  neko_synth_update_wheel_mod(neko_synth_t *synth);
void  neko_synth_control_change(neko_synth_t *synth, unsigned int param,
                                  signed int value);
void  neko_synth_channel_pressure(neko_synth_t *synth, signed int pressure);
void  neko_synth_pitch_bend(neko_synth_t *synth, signed int value);
void  neko_synth_init_controls(neko_synth_t *synth);
void  neko_synth_select_program(neko_synth_t *synth, unsigned long bank,
                                  unsigned long program);
void  neko_data_friendly_patches(neko_synth_t *synth);
int   neko_synth_set_program_descriptor(neko_synth_t *synth,
                                          DSSI_Program_Descriptor *pd,
                                          unsigned long bank,
                                          unsigned long program);
char *neko_synth_handle_patches(neko_synth_t *synth, const char *key,
                                  const char *value);
char *neko_synth_handle_polyphony(neko_synth_t *synth, const char *value);
char *neko_synth_handle_monophonic(neko_synth_t *synth, const char *value);
char *neko_synth_handle_glide(neko_synth_t *synth, const char *value);
char *neko_synth_handle_bendrange(neko_synth_t *synth, const char *value);
void  neko_synth_render_voices(neko_synth_t *synth, LADSPA_Data *out,
                                 unsigned long sample_count,
                                 int do_control_update);

/* in neko-dssi.c: */
int   dssp_voicelist_mutex_lock(neko_synth_t *synth);
int   dssp_voicelist_mutex_unlock(neko_synth_t *synth);
char *dssi_configure_message(const char *fmt, ...);

void run_bog(neko_synth_t *synth, unsigned long sample_count);

/* these come right out of alsa/asoundef.h */
#define MIDI_CTL_MSB_MODWHEEL           0x01    /**< Modulation */
#define MIDI_CTL_MSB_PORTAMENTO_TIME    0x05    /**< Portamento time */
#define MIDI_CTL_MSB_MAIN_VOLUME        0x07    /**< Main volume */
#define MIDI_CTL_MSB_BALANCE            0x08    /**< Balance */
#define MIDI_CTL_LSB_MODWHEEL           0x21    /**< Modulation */
#define MIDI_CTL_LSB_PORTAMENTO_TIME    0x25    /**< Portamento time */
#define MIDI_CTL_LSB_MAIN_VOLUME        0x27    /**< Main volume */
#define MIDI_CTL_LSB_BALANCE            0x28    /**< Balance */
#define MIDI_CTL_SUSTAIN                0x40    /**< Sustain pedal */
#define MIDI_CTL_ALL_SOUNDS_OFF         0x78    /**< All sounds off */
#define MIDI_CTL_RESET_CONTROLLERS      0x79    /**< Reset Controllers */
#define MIDI_CTL_ALL_NOTES_OFF          0x7b    /**< All notes off */

#define NEKO_SYNTH_SUSTAINED(_s)  ((_s)->cc[MIDI_CTL_SUSTAIN] >= 64)

#endif /* _NEKO_SYNTH_H */
