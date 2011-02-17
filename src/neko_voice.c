/* nekostring - neko_voice.c */

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

#include <stdlib.h>
#include <math.h>

#include "neko_types.h"
#include "neko.h"
#include "neko_synth.h"
#include "neko_voice.h"

/*
 * neko_voice_new
 */
neko_voice_t *
neko_voice_new(neko_synth_t *synth)
{
    neko_voice_t *voice;

    voice = (neko_voice_t *)calloc(sizeof(neko_voice_t), 1);
    if (voice) {
        voice->status = NEKO_VOICE_OFF;
    }
    return voice;
}

/*
 * neko_voice_note_on
 */
void
neko_voice_note_on(neko_synth_t *synth, neko_voice_t *voice,
                     unsigned char key, unsigned char velocity)
{
    int i;
	float oct_mult,osc_pos;

    if (!synth->monophonic || !(_ON(voice) || _SUSTAINED(voice))) {

        /* brand-new voice, or monophonic voice in release phase; set
         * everything up */
        XDB_MESSAGE(XDB_NOTE, " neko_voice_note_on in polyphonic/new section: key %d, mono %d, old status %d\n", key, synth->monophonic, voice->status);

        voice->target_pitch = neko_pitch[key];
        switch(synth->glide) {
          case NEKO_GLIDE_MODE_LEGATO:
            if (synth->held_keys[0] >= 0) {
                voice->prev_pitch = neko_pitch[synth->held_keys[0]];
            } else {
                voice->prev_pitch = voice->target_pitch;
            }
            break;

          case NEKO_GLIDE_MODE_INITIAL:
            if (synth->held_keys[0] >= 0) {
                voice->prev_pitch = voice->target_pitch;
            } else {
                voice->prev_pitch = synth->last_noteon_pitch;
            }
            break;

          case NEKO_GLIDE_MODE_ALWAYS:
            if (synth->held_keys[0] >= 0) {
                voice->prev_pitch = neko_pitch[synth->held_keys[0]];
            } else {
                voice->prev_pitch = synth->last_noteon_pitch;
            }
            break;

          case NEKO_GLIDE_MODE_LEFTOVER:
            /* leave voice->prev_pitch at whatever it was */
            break;

          default:
          case NEKO_GLIDE_MODE_OFF:
            voice->prev_pitch = voice->target_pitch;
            break;
        }
        if (!_PLAYING(voice)) {
			voice->lfo_pos = 0.0f;
            voice->eg1 = 0.0f;
            voice->eg2 = 0.0f;
            voice->delay1 = 0.0f;
            voice->delay2 = 0.0f;
            voice->delay3 = 0.0f;
            voice->delay4 = 0.0f;
            voice->c5     = 0.0f;
            voice->osc_index = 0;

			oct_mult=pow(2,(key/12)-2);
			
			voice->osc_bass.pos = 0;//synth->voice[i]->osc_bass.pos*oct_mult;
			voice->osc_bass.state = 0;
			osc_pos=synth->bog_pos[key%12] *oct_mult /2;
			osc_pos=osc_pos-(float)trunc(osc_pos);
			voice->osc_cello.pos = osc_pos;//synth->voice[i]->osc_cello.pos*oct_mult;
			voice->osc_cello.state = (osc_pos > 0.5f) ? 1 : 0;
			osc_pos=synth->bog_pos[key%12] *oct_mult;
			osc_pos=osc_pos-(float)trunc(osc_pos);
			voice->osc_viola.pos = osc_pos;
			voice->osc_viola.state = (osc_pos > 0.5f) ? 1 : 0;
			osc_pos=synth->bog_pos[key%12] *oct_mult*2;
			osc_pos=osc_pos-(float)trunc(osc_pos);
			voice->osc_violin.pos = osc_pos;//synth->voice[i]->osc_violin.pos*oct_mult*2;
			voice->osc_violin.state = (osc_pos > 0.5f) ? 1 : 0;

		
		}
        voice->eg1_phase = 0;
        voice->eg2_phase = 0;
        neko_voice_update_pressure_mod(synth, voice);

    } else {

        /* synth is monophonic, and we're modifying a playing voice */
        XDB_MESSAGE(XDB_NOTE, " neko_voice_note_on in monophonic section: old key %d => new key %d\n", synth->held_keys[0], key);

        /* set new pitch */
        voice->target_pitch = neko_pitch[key];//
        if (synth->glide == NEKO_GLIDE_MODE_INITIAL ||
            synth->glide == NEKO_GLIDE_MODE_OFF)
            voice->prev_pitch = voice->target_pitch;

        /* if in 'on' or 'both' modes, and key has changed, then re-trigger EGs */
        if ((synth->monophonic == NEKO_MONO_MODE_ON ||
             synth->monophonic == NEKO_MONO_MODE_BOTH) &&
            (synth->held_keys[0] < 0 || synth->held_keys[0] != key)) {
            voice->eg1_phase = 0;
            voice->eg2_phase = 0;
        }

        /* all other variables stay what they are */

    }
	
	// gjcp - moved these from the front of the function
	// might screw with our octave detection
	
	voice->key      = key;
    voice->velocity = velocity;
    synth->last_noteon_pitch = voice->target_pitch;

    /* add new key to the list of held keys */

    /* check if new key is already in the list; if so, move it to the
     * top of the list, otherwise shift the other keys down and add it
     * to the top of the list. */
    for (i = 0; i < 7; i++) {
        if (synth->held_keys[i] == key)
            break;
    }
    for (; i > 0; i--) {
        synth->held_keys[i] = synth->held_keys[i - 1];
    }
    synth->held_keys[0] = key;

    if (!_PLAYING(voice)) {

        neko_voice_start_voice(voice);

    } else if (!_ON(voice)) {  /* must be NEKO_VOICE_SUSTAINED or NEKO_VOICE_RELEASED */

        voice->status = NEKO_VOICE_ON;

    }
}

/*
 * neko_voice_set_release_phase
 */
static inline void
neko_voice_set_release_phase(neko_voice_t *voice)
{
    voice->eg1_phase = 2;
    voice->eg2_phase = 2;
}

/*
 * neko_voice_remove_held_key
 */
inline void
neko_voice_remove_held_key(neko_synth_t *synth, unsigned char key)
{
    int i;

    /* check if this key is in list of held keys; if so, remove it and
     * shift the other keys up */
    for (i = 7; i >= 0; i--) {
        if (synth->held_keys[i] == key)
            break;
    }
    if (i >= 0) {
        for (; i < 7; i++) {
            synth->held_keys[i] = synth->held_keys[i + 1];
        }
        synth->held_keys[7] = -1;
    }
}

/*
 * neko_voice_note_off
 */
void
neko_voice_note_off(neko_synth_t *synth, neko_voice_t *voice,
                      unsigned char key, unsigned char rvelocity)
{
    unsigned char previous_top_key;

    XDB_MESSAGE(XDB_NOTE, " neko_set_note_off: called for voice %p, key %d\n", voice, key);

    /* save release velocity */
    voice->rvelocity = rvelocity;

    previous_top_key = synth->held_keys[0];

    /* remove this key from list of held keys */
    neko_voice_remove_held_key(synth, key);

    if (synth->monophonic) {  /* monophonic mode */

        if (synth->held_keys[0] >= 0) {

            /* still some keys held */

            if (synth->held_keys[0] != previous_top_key) {

                /* most-recently-played key has changed */
                voice->key = synth->held_keys[0];
                XDB_MESSAGE(XDB_NOTE, " note-off in monophonic section: changing pitch to %d\n", voice->key);
                voice->target_pitch = neko_pitch[voice->key];
                if (synth->glide == NEKO_GLIDE_MODE_INITIAL ||
                    synth->glide == NEKO_GLIDE_MODE_OFF)
                    voice->prev_pitch = voice->target_pitch;

                /* if mono mode is 'both', re-trigger EGs */
                if (synth->monophonic == NEKO_MONO_MODE_BOTH && !_RELEASED(voice)) {
                    voice->eg1_phase = 0;
                    voice->eg2_phase = 0;
                }

            }

        } else {  /* no keys still held */

            if (NEKO_SYNTH_SUSTAINED(synth)) {

                /* no more keys in list, but we're sustained */
                XDB_MESSAGE(XDB_NOTE, " note-off in monophonic section: sustained with no held keys\n");
                if (!_RELEASED(voice))
                    voice->status = NEKO_VOICE_SUSTAINED;

            } else {  /* not sustained */

                /* no more keys in list, so turn off note */
                XDB_MESSAGE(XDB_NOTE, " note-off in monophonic section: turning off voice %p\n", voice);
                neko_voice_set_release_phase(voice);
                voice->status = NEKO_VOICE_RELEASED;

            }
        }

    } else {  /* polyphonic mode */

        if (NEKO_SYNTH_SUSTAINED(synth)) {

            if (!_RELEASED(voice))
                voice->status = NEKO_VOICE_SUSTAINED;

        } else {  /* not sustained */

            neko_voice_set_release_phase(voice);
            voice->status = NEKO_VOICE_RELEASED;

        }
    }
}

/*
 * neko_voice_release_note
 */
void
neko_voice_release_note(neko_synth_t *synth, neko_voice_t *voice)
{
    XDB_MESSAGE(XDB_NOTE, " neko_voice_release_note: turning off voice %p\n", voice);
    if (_ON(voice)) {
        /* dummy up a release velocity */
        voice->rvelocity = 64;
    }
    neko_voice_set_release_phase(voice);
    voice->status = NEKO_VOICE_RELEASED;
}

/*
 * neko_voice_set_ports
 */
void
neko_voice_set_ports(neko_synth_t *synth, neko_patch_t *patch)
{
    *(synth->bass)        = patch->bass;
    *(synth->cello)     = patch->cello;
    *(synth->viola)   = patch->viola;
    *(synth->violin)        = patch->violin;
    *(synth->speed)     = patch->speed;
    *(synth->vibrato_chorus)          = patch->vibrato_chorus;
    *(synth->mix)       = patch->mix;
    *(synth->attack)     = patch->attack;
    *(synth->decay)      = patch->decay;
    *(synth->bass_decay)      = patch->bass_decay;
    *(synth->tone)      = patch->tone;
    *(synth->volume)            = patch->volume;
}

/*
 * neko_voice_update_pressure_mod
 */
void
neko_voice_update_pressure_mod(neko_synth_t *synth, neko_voice_t *voice)
{
    unsigned char kp = synth->key_pressure[voice->key];
    unsigned char cp = synth->channel_pressure;
    float p;

    /* add the channel and key pressures together in a way that 'feels' good */
    if (kp > cp) {
        p = (float)kp / 127.0f;
        p += (1.0f - p) * ((float)cp / 127.0f);
    } else {
        p = (float)cp / 127.0f;
        p += (1.0f - p) * ((float)kp / 127.0f);
    }
    /* set the pressure modifier so it ranges between 1.0 (no pressure, no
     * resonance boost) and 0.25 (full pressure, resonance boost 75% of the way
     * to filter oscillation) */
    voice->pressure = 1.0f - (p * 0.75f);
}
