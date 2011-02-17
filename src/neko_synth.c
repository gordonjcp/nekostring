/* nekostring - neko_synth.c */

/* nekostring polyphonic string ensemble
 *
 * (C) 2006 Gordon JC Pearce and others
 *
 * For further information on contributors and the history of nekostring,
 * please see the README
 *
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include <ladspa.h>

#include "neko.h"
#include "neko_synth.h"
#include "neko_voice.h"
#include "gui_data.h"

/*
 * neko_synth_all_voices_off
 *
 * stop processing all notes immediately
 */
void
neko_synth_all_voices_off(neko_synth_t *synth)
{
    int i;
    neko_voice_t *voice;

    for (i = 0; i < synth->voices; i++) {
        voice = synth->voice[i];
        if (_PLAYING(voice)) {
            neko_voice_off(voice);
        }
    }
    for (i = 0; i < 8; i++) synth->held_keys[i] = -1;
}

/*
 * neko_synth_note_off
 *
 * handle a note off message
 */
void
neko_synth_note_off(neko_synth_t *synth, unsigned char key, unsigned char rvelocity)
{
    int i, count = 0;
    neko_voice_t *voice;

    for (i = 0; i < synth->voices; i++) {
        voice = synth->voice[i];
        if (synth->monophonic ? (_PLAYING(voice)) :
                                (_ON(voice) && (voice->key == key))) {
            XDB_MESSAGE(XDB_NOTE, " neko_synth_note_off: key %d rvel %d voice %d note id %d\n", key, rvelocity, i, voice->note_id);
            neko_voice_note_off(synth, voice, key, rvelocity);
            count++;
        }
    }

    if (!count)
        neko_voice_remove_held_key(synth, key);
}

/*
 * neko_synth_all_notes_off
 *
 * put all notes into the released state
 */
void
neko_synth_all_notes_off(neko_synth_t* synth)
{
    int i;
    neko_voice_t *voice;

    /* reset the sustain controller */
    synth->cc[MIDI_CTL_SUSTAIN] = 0;
    for (i = 0; i < synth->voices; i++) {
        voice = synth->voice[i];
        if (_ON(voice) || _SUSTAINED(voice)) {
            neko_voice_release_note(synth, voice);
        }
    }
}

/*
 * neko_synth_free_voice_by_kill
 *
 * selects a voice for killing. the selection algorithm is a refinement
 * of the algorithm previously in fluid_synth_alloc_voice.
 */
static neko_voice_t*
neko_synth_free_voice_by_kill(neko_synth_t *synth)
{
    int i;
    int best_prio = 10001;
    int this_voice_prio;
    neko_voice_t *voice;
    int best_voice_index = -1;

    for (i = 0; i < synth->voices; i++) {
        voice = synth->voice[i];
    
        /* safeguard against an available voice. */
        if (_AVAILABLE(voice))
            return voice;
    
        /* Determine, how 'important' a voice is.
         * Start with an arbitrary number */
        this_voice_prio = 10000;

        if (_RELEASED(voice)) {
            /* This voice is in the release phase. Consider it much less
             * important than a voice which is still held. */
            this_voice_prio -= 2000;
        } else if (_SUSTAINED(voice)) {
            /* The sustain pedal is held down, and this voice is still "on"
             * because of this even though it has received a note off.
             * Consider it less important than voices which have not yet
             * received a note off. This decision is somewhat subjective, but
             * usually the sustain pedal is used to play 'more-voices-than-
             * fingers', and if so, it won't hurt as much to kill one of those
             * voices. */
             this_voice_prio -= 1000;
        };

        /* We are not enthusiastic about releasing voices, which have just been
         * started.  Otherwise hitting a chord may result in killing notes
         * belonging to that very same chord.  So subtract the age of the voice
         * from the priority - an older voice is just a little bit less
         * important than a younger voice. */
        this_voice_prio -= (synth->note_id - voice->note_id);
    
        /* -FIX- not yet implemented:
         * /= take a rough estimate of loudness into account. Louder voices are more important. =/
         * if (voice->volenv_section != FLUID_VOICE_ENVATTACK){
         *     this_voice_prio += voice->volenv_val*1000.;
         * };
         */

        /* check if this voice has less priority than the previous candidate. */
        if (this_voice_prio < best_prio)
            best_voice_index = i,
            best_prio = this_voice_prio;
    }

    if (best_voice_index < 0)
        return NULL;

    voice = synth->voice[best_voice_index];
    XDB_MESSAGE(XDB_NOTE, " neko_synth_free_voice_by_kill: no available voices, killing voice %d note id %d\n", best_voice_index, voice->note_id);
    neko_voice_off(voice);
    return voice;
}

/*
 * neko_synth_alloc_voice
 */
static neko_voice_t *
neko_synth_alloc_voice(neko_synth_t* synth, unsigned char key)
{
    int i;
    neko_voice_t* voice;

    /* If there is another voice on the same key, advance it
     * to the release phase to keep our CPU usage low. */
    for (i = 0; i < synth->voices; i++) {
        voice = synth->voice[i];
        if (voice->key == key && (_ON(voice) || _SUSTAINED(voice))) {
            neko_voice_release_note(synth, voice);
        }
    }

    /* check if there's an available voice */
    voice = NULL;
    for (i = 0; i < synth->voices; i++) {
        if (_AVAILABLE(synth->voice[i])) {
            voice = synth->voice[i];
            break;
        }
    }

    /* No success yet? Then stop a running voice. */
    if (voice == NULL) {
        voice = neko_synth_free_voice_by_kill(synth);
    }

    if (voice == NULL) {
        XDB_MESSAGE(XDB_NOTE, " neko_synth_alloc_voice: failed to allocate a voice (key=%d)\n", key);
        return NULL;
    }

    XDB_MESSAGE(XDB_NOTE, " neko_synth_alloc_voice: key %d voice %p\n", key, voice);
    return voice;
}

/*
 * neko_synth_note_on
 */
void
neko_synth_note_on(neko_synth_t *synth, unsigned char key, unsigned char velocity)
{
    neko_voice_t* voice;

    if (synth->monophonic) {

        voice = synth->voice[0];
        if (_PLAYING(synth->voice[0])) {
            XDB_MESSAGE(XDB_NOTE, " neko_synth_note_on: retriggering mono voice on new key %d\n", key);
        }

    } else { /* polyphonic mode */

        voice = neko_synth_alloc_voice(synth, key);
        if (voice == NULL)
            return;

    }

    voice->note_id  = synth->note_id++;

    neko_voice_note_on(synth, voice, key, velocity);
}

/*
 * neko_synth_key_pressure
 */
void
neko_synth_key_pressure(neko_synth_t *synth, unsigned char key, unsigned char pressure)
{
    int i;
    neko_voice_t* voice;

    /* save it for future voices */
    synth->key_pressure[key] = pressure;
    
    /* check if any playing voices need updating */
    for (i = 0; i < synth->voices; i++) {
        voice = synth->voice[i];
        if (_PLAYING(voice) && voice->key == key) {
            neko_voice_update_pressure_mod(synth, voice);
        }
    }
}

/*
 * neko_synth_damp_voices
 *
 * advance all sustained voices to the release phase (note that this does not
 * clear the sustain controller.)
 */
void
neko_synth_damp_voices(neko_synth_t* synth)
{
    int i;
    neko_voice_t* voice;

    for (i = 0; i < synth->voices; i++) {
        voice = synth->voice[i];
        if (_SUSTAINED(voice)) {
            neko_voice_release_note(synth, voice);
        }
    }
}

/*
 * neko_synth_update_wheel_mod
 */
void
neko_synth_update_wheel_mod(neko_synth_t* synth)
{
    synth->mod_wheel = 1.0f - (float)(synth->cc[MIDI_CTL_MSB_MODWHEEL] * 128 +
                                      synth->cc[MIDI_CTL_LSB_MODWHEEL]) / 16256.0f;
    if (synth->mod_wheel < 0.0f)
        synth->mod_wheel = 0.0f;
    /* don't need to check if any playing voices need updating, because it's global */
}

/*
 * neko_synth_update_volume
 */
void
neko_synth_update_volume(neko_synth_t* synth)
{
    synth->cc_volume = (float)(synth->cc[MIDI_CTL_MSB_MAIN_VOLUME] * 128 +
                               synth->cc[MIDI_CTL_LSB_MAIN_VOLUME]) / 16256.0f;
    if (synth->cc_volume > 1.0f)
        synth->cc_volume = 1.0f;
    /* don't need to check if any playing voices need updating, because it's global */
}

/*
 * neko_synth_control_change
 */
void
neko_synth_control_change(neko_synth_t *synth, unsigned int param, signed int value)
{
    synth->cc[param] = value;

    switch (param) {

      case MIDI_CTL_MSB_MODWHEEL:
      case MIDI_CTL_LSB_MODWHEEL:
        neko_synth_update_wheel_mod(synth);
        break;

      case MIDI_CTL_MSB_MAIN_VOLUME:
      case MIDI_CTL_LSB_MAIN_VOLUME:
        neko_synth_update_volume(synth);
        break;

      case MIDI_CTL_SUSTAIN:
        XDB_MESSAGE(XDB_NOTE, " neko_synth_control_change: got sustain control of %d\n", value);
        if (value < 64)
            neko_synth_damp_voices(synth);
        break;

      case MIDI_CTL_ALL_SOUNDS_OFF:
        neko_synth_all_voices_off(synth);
        break;

      case MIDI_CTL_RESET_CONTROLLERS:
        neko_synth_init_controls(synth);
        break;

      case MIDI_CTL_ALL_NOTES_OFF:
        neko_synth_all_notes_off(synth);
        break;

      /* what others should we respond to? */

      /* these we ignore (let the host handle):
       *  BANK_SELECT_MSB
       *  BANK_SELECT_LSB
       *  DATA_ENTRY_MSB
       *  NRPN_MSB
       *  NRPN_LSB
       *  RPN_MSB
       *  RPN_LSB
       * -FIX- no! we need RPN (0, 0) Pitch Bend Sensitivity!
       */
    }
}

/*
 * neko_synth_channel_pressure
 */
void
neko_synth_channel_pressure(neko_synth_t *synth, signed int pressure)
{
    int i;
    neko_voice_t* voice;

    /* save it for future voices */
    synth->channel_pressure = pressure;

    /* check if any playing voices need updating */
    for (i = 0; i < synth->voices; i++) {
        voice = synth->voice[i];
        if (_PLAYING(voice)) {
            neko_voice_update_pressure_mod(synth, voice);
        }
    }
}

/*
 * neko_synth_pitch_bend
 */
void
neko_synth_pitch_bend(neko_synth_t *synth, signed int value)
{
    synth->pitch_wheel = value; /* ALSA pitch bend is already -8192 - 8191 */
    synth->pitch_bend = exp((float)(value * synth->pitch_wheel_sensitivity) /
                            (float)(8192 * 12) * M_LN2);
    /* don't need to check if any playing voices need updating, because it's global */
}

/*
 * neko_synth_init_controls
 */
void
neko_synth_init_controls(neko_synth_t *synth)
{
    int i;
    neko_voice_t* voice;

    synth->current_program = -1;

    /* if sustain was on, we need to damp any sustained voices */
    if (NEKO_SYNTH_SUSTAINED(synth)) {
        synth->cc[MIDI_CTL_SUSTAIN] = 0;
        neko_synth_damp_voices(synth);
    }

    for (i = 0; i < 128; i++) {
        synth->key_pressure[i] = 0;
        synth->cc[i] = 0;
    }
    synth->channel_pressure = 0;
    synth->pitch_wheel_sensitivity = 2;  /* two semi-tones */
    synth->pitch_wheel = 0;
    synth->cc[7] = 127;                  /* full volume */

    neko_synth_update_wheel_mod(synth);
    neko_synth_update_volume(synth);
    neko_synth_pitch_bend(synth, 0);

    /* check if any playing voices need updating */
    for (i = 0; i < synth->voices; i++) {
        voice = synth->voice[i];
        if (_PLAYING(voice)) {
            neko_voice_update_pressure_mod(synth, voice);
        }
    }
}

/*
 * neko_synth_select_program
 */
void
neko_synth_select_program(neko_synth_t *synth, unsigned long bank,
                            unsigned long program)
{
    if (bank || program >= 128) return;
    synth->current_program = program;
    neko_voice_set_ports(synth, &synth->patches[program]);
}

/*
 * neko_data_friendly_patches
 *
 * give the new user a default set of good patches to get started with
 */
void
neko_data_friendly_patches(neko_synth_t *synth)
{
    int i;

    pthread_mutex_lock(&synth->patches_mutex);

    memcpy(synth->patches, friendly_patches, friendly_patch_count * sizeof(neko_patch_t));

    for (i = friendly_patch_count; i < 128; i++) {
        memcpy(&synth->patches[i], &neko_init_voice, sizeof(neko_patch_t));
    }

    pthread_mutex_unlock(&synth->patches_mutex);
}

/*
 * neko_synth_set_program_descriptor
 */
int
neko_synth_set_program_descriptor(neko_synth_t *synth,
                                    DSSI_Program_Descriptor *pd,
                                    unsigned long bank, unsigned long program)
{
    if (bank || program >= 128) {
        return 0;
    }
    pd->Bank = bank;
    pd->Program = program;
    pd->Name = synth->patches[program].name;
    return 1;

}

/*
 * neko_synth_handle_patches
 */
char *
neko_synth_handle_patches(neko_synth_t *synth, const char *key,
                            const char *value)
{
    int section;

    XDB_MESSAGE(XDB_DATA, " neko_synth_handle_patches: received new '%s'\n", key);

    section = key[7] - '0';
    if (section < 0 || section > 3)
        return dssi_configure_message("patch configuration failed: invalid section '%c'", key[7]);

    pthread_mutex_lock(&synth->patches_mutex);

    if (!neko_data_decode_patches(value, &synth->patches[section * 32])) {
        pthread_mutex_unlock(&synth->patches_mutex);
        return dssi_configure_message("patch configuration failed: corrupt data");
    }

    pthread_mutex_unlock(&synth->patches_mutex);

    return NULL; /* success */
}

/*
 * neko_synth_handle_monophonic
 */
char *
neko_synth_handle_monophonic(neko_synth_t *synth, const char *value)
{
    int mode = -1;

    if (!strcmp(value, "on")) mode = NEKO_MONO_MODE_ON;
    else if (!strcmp(value, "once")) mode = NEKO_MONO_MODE_ONCE;
    else if (!strcmp(value, "both")) mode = NEKO_MONO_MODE_BOTH;
    else if (!strcmp(value, "off"))  mode = NEKO_MONO_MODE_OFF;

    if (mode == -1) {
        return dssi_configure_message("error: monophonic value not recognized");
    }

    if (mode == NEKO_MONO_MODE_OFF) {  /* polyphonic mode */

        synth->monophonic = 0;
        synth->voices = synth->polyphony;

    } else {  /* one of the monophonic modes */

        dssp_voicelist_mutex_lock(synth);

        if (!synth->monophonic) neko_synth_all_voices_off(synth);
        synth->monophonic = mode;
        synth->voices = 1;

        dssp_voicelist_mutex_unlock(synth);
    }

    return NULL;
}

/*
 * neko_synth_handle_polyphony
 */
char *
neko_synth_handle_polyphony(neko_synth_t *synth, const char *value)
{
    int polyphony = atoi(value);
    int i;
    neko_voice_t *voice;

    if (polyphony < 1 || polyphony > NEKO_MAX_POLYPHONY) {
        return dssi_configure_message("error: polyphony value out of range");
    }
    /* set the new limit */
    synth->polyphony = polyphony;

    if (!synth->monophonic) {
        synth->voices = polyphony;

        /* turn off any voices above the new limit */

        dssp_voicelist_mutex_lock(synth);

        for (i = polyphony; i < NEKO_MAX_POLYPHONY; i++) {
            voice = synth->voice[i];
            if (_PLAYING(voice)) {
                neko_voice_off(voice);
            }
        }

        dssp_voicelist_mutex_unlock(synth);
    }

    return NULL;
}

/*
 * neko_synth_handle_glide
 */
char *
neko_synth_handle_glide(neko_synth_t *synth, const char *value)
{
    int mode = -1;

    if (!strcmp(value, "legato"))        mode = NEKO_GLIDE_MODE_LEGATO;
    else if (!strcmp(value, "initial"))  mode = NEKO_GLIDE_MODE_INITIAL;
    else if (!strcmp(value, "always"))   mode = NEKO_GLIDE_MODE_ALWAYS;
    else if (!strcmp(value, "leftover")) mode = NEKO_GLIDE_MODE_LEFTOVER;
    else if (!strcmp(value, "off"))      mode = NEKO_GLIDE_MODE_OFF;

    if (mode == -1) {
        return dssi_configure_message("error: glide value not recognized");
    }

    synth->glide = mode;

    return NULL;
}

/*
 * neko_synth_handle_bendrange
 */
char *
neko_synth_handle_bendrange(neko_synth_t *synth, const char *value)
{
    int range = atoi(value);

    if (range < 0 || range > 12) {
        return dssi_configure_message("error: bendrange value out of range");
    }
    synth->pitch_wheel_sensitivity = range;
    neko_synth_pitch_bend(synth, synth->pitch_wheel);  /* recalculate current pitch_bend */

    return NULL;
}

/*
 * neko_synth_render_voices
 */
void
neko_synth_render_voices(neko_synth_t *synth, LADSPA_Data *out, unsigned long sample_count,
                        int do_control_update)
{
    unsigned long i;
    neko_voice_t* voice;

    /* clear the buffer */
    for (i = 0; i < sample_count; i++)
        out[i] = 0.0f;

#if defined(NEKO_DEBUG) && (NEKO_DEBUG & XDB_AUDIO)
out[0] += 0.10f; /* add a 'buzz' to output so there's something audible even when quiescent */
#endif /* defined(NEKO_DEBUG) && (NEKO_DEBUG & XDB_AUDIO) */

    /* render each active voice */
    for (i = 0; i < synth->voices; i++) {
        voice = synth->voice[i];
    
        if (_PLAYING(voice)) {
            neko_voice_render(synth, voice, out, sample_count, do_control_update);
        }
    }
}

void run_bog(neko_synth_t *synth, unsigned long sample_count)
{
  unsigned int i;
  float p;

  float f = (float)sample_count * *(synth->tuning) * (synth->deltat);
  
  for(i = 0; i<12; i++) {
	  p= synth->bog_pos[i];
	  p += (f * neko_pitch[24+i]);
	  if (p>1.0f) p -= 1.0f;
	  synth->bog_pos[i]=p;
  }
}
