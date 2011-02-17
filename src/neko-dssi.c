/* nekostring - neko-dssi.c */

/* nekostring polyphonic string ensemble
 *
 * Copyright (C) 2008 Gordon JC Pearce and others
 *
 * For further information on contributors and the history of nekostring,
 * please see the README
 *
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>

#include <ladspa.h>
#include "dssi.h"

#include "neko_types.h"
#include "neko.h"
#include "neko_ports.h"
#include "neko_synth.h"
#include "neko_voice.h"

static LADSPA_Descriptor *neko_LADSPA_descriptor = NULL;
static DSSI_Descriptor   *neko_DSSI_descriptor = NULL;

static void neko_cleanup(LADSPA_Handle instance);
static void neko_run_synth(LADSPA_Handle instance, unsigned long sample_count,
                             snd_seq_event_t *events, unsigned long event_count);
							 
void
ensemble (float *left, float *right, unsigned long sample_count,
	 neko_synth_t * plugin_data);
/* ---- mutual exclusion ---- */

static inline int
dssp_voicelist_mutex_trylock(neko_synth_t *synth)
{
    int rc;

    /* Attempt the mutex lock */
    rc = pthread_mutex_trylock(&synth->voicelist_mutex);
    if (rc) {
        synth->voicelist_mutex_grab_failed = 1;
        return rc;
    }
    /* Clean up if a previous mutex grab failed */
    if (synth->voicelist_mutex_grab_failed) {
        neko_synth_all_voices_off(synth);
        synth->voicelist_mutex_grab_failed = 0;
    }
    return 0;
}

inline int
dssp_voicelist_mutex_lock(neko_synth_t *synth)
{
    return pthread_mutex_lock(&synth->voicelist_mutex);
}

inline int
dssp_voicelist_mutex_unlock(neko_synth_t *synth)
{
    return pthread_mutex_unlock(&synth->voicelist_mutex);
}

/* ---- LADSPA interface ---- */

/*
 * neko_instantiate
 *
 * implements LADSPA (*instantiate)()
 */
static LADSPA_Handle
neko_instantiate(const LADSPA_Descriptor *descriptor, unsigned long sample_rate)
{
    neko_synth_t *synth = (neko_synth_t *)calloc(1, sizeof(neko_synth_t));
    int i;

    if (!synth) return NULL;
    for (i = 0; i < NEKO_MAX_POLYPHONY; i++) {
        synth->voice[i] = neko_voice_new(synth);
        if (!synth->voice[i]) {
            // XDB_MESSAGE(-1, " neko_instantiate: out of memory!\n");
            neko_cleanup(synth);
            return NULL;
        }
    }
    if (!(synth->patches = (neko_patch_t *)malloc(128 * sizeof(neko_patch_t)))) {
        XDB_MESSAGE(-1, " neko_instantiate: out of memory!\n");
        neko_cleanup(synth);
        return NULL;
    }

    /* do any per-instance one-time initialization here */
    synth->sample_rate = sample_rate;
    synth->deltat = 1.0f / (float)synth->sample_rate;
    synth->polyphony = NEKO_DEFAULT_POLYPHONY;
    synth->voices = NEKO_DEFAULT_POLYPHONY;
    synth->monophonic = 0;
    synth->glide = 0;
    synth->last_noteon_pitch = 0.0f;
    pthread_mutex_init(&synth->voicelist_mutex, NULL);
    synth->voicelist_mutex_grab_failed = 0;
    pthread_mutex_init(&synth->patches_mutex, NULL);
    synth->pending_program_change = -1;
    synth->current_program = -1;
    neko_data_friendly_patches(synth);
    neko_synth_init_controls(synth);

    return (LADSPA_Handle)synth;
}

/*
 * neko_connect_port
 *
 * implements LADSPA (*connect_port)()
 */
static void
neko_connect_port(LADSPA_Handle instance, unsigned long port, LADSPA_Data *data)
{
    neko_synth_t *synth = (neko_synth_t *)instance;

    switch (port) {
      case NEKO_PORT_LEFT:			synth->left			= data;  break;
	  case NEKO_PORT_RIGHT:			synth->right            = data;  break;
      case NEKO_PORT_BASS_LEVEL:	synth->bass        = data;  break;
      case NEKO_PORT_CELLO_LEVEL:	synth->cello     = data;  break;
      case NEKO_PORT_VIOLA_LEVEL:	synth->viola   = data;  break;
      case NEKO_PORT_VIOLIN_LEVEL:	synth->violin        = data;  break;
      case NEKO_PORT_SPEED:			synth->speed     = data;  break;
      case NEKO_PORT_DEPTH:			synth->depth   = data;  break;
      case NEKO_PORT_VIBRATO_CHORUS:	synth->vibrato_chorus         = data;  break;
      case NEKO_PORT_MIX:			synth->mix       = data;  break;
      case NEKO_PORT_ATTACK:		synth->attack     = data;  break;
      case NEKO_PORT_DECAY:			synth->decay      = data;  break;
      case NEKO_PORT_BASS_DECAY:	synth->bass_decay      = data;  break;
      case NEKO_PORT_TONE:			synth->tone      = data;  break;
      case NEKO_PORT_VOLUME:		synth->volume            = data;  break;
      case NEKO_PORT_TUNING:		synth->tuning            = data;  break;

      default:
        break;
    }
}

/*
 * neko_activate
 *
 * implements LADSPA (*activate)()
 */
static void
neko_activate(LADSPA_Handle instance)
{
    neko_synth_t *synth = (neko_synth_t *)instance;

    synth->nugget_remains = 0;
    synth->note_id = 0;
    neko_synth_all_voices_off(synth);
	printf("called activate\n");
}

/*
 * neko_ladspa_run_wrapper
 *
 * implements LADSPA (*run)() by calling neko_run_synth() with no events
 */
static void
neko_ladspa_run_wrapper(LADSPA_Handle instance, unsigned long sample_count)
{
    neko_run_synth(instance, sample_count, NULL, 0);
}

// optional:
//  void (*run_adding)(LADSPA_Handle Instance,
//                     unsigned long SampleCount);
//  void (*set_run_adding_gain)(LADSPA_Handle Instance,
//                              LADSPA_Data   Gain);

/*
 * neko_deactivate
 *
 * implements LADSPA (*deactivate)()
 */
void
neko_deactivate(LADSPA_Handle instance)
{
    neko_synth_t *synth = (neko_synth_t *)instance;

    neko_synth_all_voices_off(synth);  /* stop all sounds immediately */
}

/*
 * neko_cleanup
 *
 * implements LADSPA (*cleanup)()
 */
static void
neko_cleanup(LADSPA_Handle instance)
{
    neko_synth_t *synth = (neko_synth_t *)instance;
    int i;

    for (i = 0; i < NEKO_MAX_POLYPHONY; i++)
        if (synth->voice[i]) free(synth->voice[i]);
    if (synth->patches) free(synth->patches);
    free(synth);
}

/* ---- DSSI interface ---- */

/*
 * dssi_configure_message
 */
char *
dssi_configure_message(const char *fmt, ...)
{
    va_list args;
    char buffer[256];

    va_start(args, fmt);
    vsnprintf(buffer, 256, fmt, args);
    va_end(args);
    return strdup(buffer);
}

/*
 * neko_configure
 *
 * implements DSSI (*configure)()
 */
char *
neko_configure(LADSPA_Handle instance, const char *key, const char *value)
{
    XDB_MESSAGE(XDB_DSSI, " neko_configure called with '%s' and '%s'\n", key, value);

    if (strlen(key) == 8 && !strncmp(key, "patches", 7)) {

        return neko_synth_handle_patches((neko_synth_t *)instance, key, value);

    } else if (!strcmp(key, "polyphony")) {

        return neko_synth_handle_polyphony((neko_synth_t *)instance, value);

    } else if (!strcmp(key, "monophonic")) {

        return neko_synth_handle_monophonic((neko_synth_t *)instance, value);

    } else if (!strcmp(key, "glide")) {

        return neko_synth_handle_glide((neko_synth_t *)instance, value);

    } else if (!strcmp(key, "bendrange")) {

        return neko_synth_handle_bendrange((neko_synth_t *)instance, value);

    } else if (!strcmp(key, DSSI_PROJECT_DIRECTORY_KEY)) {

        return NULL; /* plugin has no use for project directory key, ignore it */

    } else if (!strcmp(key, "load")) {

        return dssi_configure_message("warning: host sent obsolete 'load' key with filename '%s'", value);

    }
    return strdup("error: unrecognized configure key");
}

/*
 * neko_get_program
 *
 * implements DSSI (*get_program)()
 */
const DSSI_Program_Descriptor *
neko_get_program(LADSPA_Handle instance, unsigned long index)
{
    neko_synth_t *synth = (neko_synth_t *)instance;
    static DSSI_Program_Descriptor pd;

    XDB_MESSAGE(XDB_DSSI, " neko_get_program called with %lu\n", index);

    if (index < 128) {
        neko_synth_set_program_descriptor(synth, &pd, 0, index);
        return &pd;
    }
    return NULL;
}

/*
 * neko_select_program
 *
 * implements DSSI (*select_program)()
 */
void
neko_select_program(LADSPA_Handle handle, unsigned long bank,
                      unsigned long program)
{
    neko_synth_t *synth = (neko_synth_t *)handle;

    XDB_MESSAGE(XDB_DSSI, " neko_select_program called with %lu and %lu\n", bank, program);

    /* ignore invalid program requests */
    if (bank || program >= 128)
        return;
    
    /* Attempt the patch mutex, return if lock fails. */
    if (pthread_mutex_trylock(&synth->patches_mutex)) {
        synth->pending_program_change = program;
        return;
    }

    neko_synth_select_program(synth, bank, program);

    pthread_mutex_unlock(&synth->patches_mutex);
}

/*
 * dssp_handle_pending_program_change
 */
static inline void
dssp_handle_pending_program_change(neko_synth_t *synth)
{
    /* Attempt the patch mutex, return if lock fails. */
    if (pthread_mutex_trylock(&synth->patches_mutex))
        return;

    neko_synth_select_program(synth, 0, synth->pending_program_change);
    synth->pending_program_change = -1;

    pthread_mutex_unlock(&synth->patches_mutex);
}

/*
 * neko_get_midi_controller
 *
 * implements DSSI (*get_midi_controller_for_port)()
 */
int
neko_get_midi_controller(LADSPA_Handle instance, unsigned long port)
{
    XDB_MESSAGE(XDB_DSSI, " neko_get_midi_controller called for port %lu\n", port);
    switch (port) {
      default:
        break;
    }

    return DSSI_NONE;
}

/*
 * neko_handle_event
 */
static inline void
neko_handle_event(neko_synth_t *synth, snd_seq_event_t *event)
{
    XDB_MESSAGE(XDB_DSSI, " neko_handle_event called with event type %d\n", event->type);

    switch (event->type) {
      case SND_SEQ_EVENT_NOTEOFF:
        neko_synth_note_off(synth, event->data.note.note, event->data.note.velocity);
        break;
      case SND_SEQ_EVENT_NOTEON:
        if (event->data.note.velocity > 0)
           neko_synth_note_on(synth, event->data.note.note, event->data.note.velocity);
        else
           neko_synth_note_off(synth, event->data.note.note, 64); /* shouldn't happen, but... */
        break;
      case SND_SEQ_EVENT_KEYPRESS:
        neko_synth_key_pressure(synth, event->data.note.note, event->data.note.velocity);
        break;
      case SND_SEQ_EVENT_CONTROLLER:
        neko_synth_control_change(synth, event->data.control.param, event->data.control.value);
        break;
      case SND_SEQ_EVENT_CHANPRESS:
        neko_synth_channel_pressure(synth, event->data.control.value);
        break;
      case SND_SEQ_EVENT_PITCHBEND:
        neko_synth_pitch_bend(synth, event->data.control.value);
        break;
      /* SND_SEQ_EVENT_PGMCHANGE - shouldn't happen */
      /* SND_SEQ_EVENT_SYSEX - shouldn't happen */
      /* SND_SEQ_EVENT_CONTROL14? */
      /* SND_SEQ_EVENT_NONREGPARAM? */
      /* SND_SEQ_EVENT_REGPARAM? */
      default:
        break;
    }
}

/*
 * neko_run_synth
 *
 * implements DSSI (*run_synth)()
 */
static void
neko_run_synth(LADSPA_Handle instance, unsigned long sample_count,
                 snd_seq_event_t *events, unsigned long event_count)
{
    neko_synth_t *synth = (neko_synth_t *)instance;
    unsigned long samples_done = 0;
    unsigned long event_index = 0;
    unsigned long burst_size;

    /* attempt the mutex, return only silence if lock fails. */
    if (dssp_voicelist_mutex_trylock(synth)) {
        memset(synth->left, 0, sizeof(LADSPA_Data) * sample_count);
		memset(synth->right, 0, sizeof(LADSPA_Data) * sample_count);
        return;
    }

    if (synth->pending_program_change > -1)
        dssp_handle_pending_program_change(synth);

    while (samples_done < sample_count) {
        if (!synth->nugget_remains)
            synth->nugget_remains = NEKO_NUGGET_SIZE;

        /* process any ready events */
	while (event_index < event_count
	       && samples_done == events[event_index].time.tick) {
            neko_handle_event(synth, &events[event_index]);
            event_index++;
        }

        /* calculate the sample count (burst_size) for the next
         * neko_voice_render() call to be the smallest of:
         * - control calculation quantization size (NEKO_NUGGET_SIZE, in
         *     samples)
         * - the number of samples remaining in an already-begun nugget
         *     (synth->nugget_remains)
         * - the number of samples until the next event is ready
         * - the number of samples left in this run
         */
        burst_size = NEKO_NUGGET_SIZE;
        if (synth->nugget_remains < burst_size) {
            /* we're still in the middle of a nugget, so reduce the burst size
             * to end when the nugget ends */
            burst_size = synth->nugget_remains;
        }
        if (event_index < event_count
            && events[event_index].time.tick - samples_done < burst_size) {
            /* reduce burst size to end when next event is ready */
            burst_size = events[event_index].time.tick - samples_done;
        }
        if (sample_count - samples_done < burst_size) {
            /* reduce burst size to end at end of this run */
            burst_size = sample_count - samples_done;
        }

		run_bog(synth, burst_size);
        /* render the burst */
        neko_synth_render_voices(synth, synth->left + samples_done, burst_size,
                                (burst_size == synth->nugget_remains));
        samples_done += burst_size;
        synth->nugget_remains -= burst_size;
    }
	// gjcp - this looks like where "global" effects should be!
	ensemble (synth->left, synth->right, sample_count, synth);


	
#if defined(NEKO_DEBUG) && (NEKO_DEBUG & XDB_AUDIO)
*synth->left += 0.10f; /* add a 'buzz' to output so there's something audible even when quiescent */
#endif /* defined(NEKO_DEBUG) && (NEKO_DEBUG & XDB_AUDIO) */

    dssp_voicelist_mutex_unlock(synth);
}

// optional:
//    void (*run_synth_adding)(LADSPA_Handle    Instance,
//                             unsigned long    SampleCount,
//                             snd_seq_event_t *Events,
//                             unsigned long    EventCount);

/* ---- export ---- */

const LADSPA_Descriptor *ladspa_descriptor(unsigned long index)
{
    switch (index) {
    case 0:
        return neko_LADSPA_descriptor;
    default:
        return NULL;
    }
}

const DSSI_Descriptor *dssi_descriptor(unsigned long index)
{
    switch (index) {
    case 0:
        return neko_DSSI_descriptor;
    default:
        return NULL;
    }
}

void _init()
{
    int i;
    char **port_names;
    LADSPA_PortDescriptor *port_descriptors;
    LADSPA_PortRangeHint *port_range_hints;

    NEKO_DEBUG_INIT("neko-dssi.so");

    neko_init_tables();

    neko_LADSPA_descriptor =
        (LADSPA_Descriptor *) malloc(sizeof(LADSPA_Descriptor));
    if (neko_LADSPA_descriptor) {
        neko_LADSPA_descriptor->UniqueID = 2944;
        neko_LADSPA_descriptor->Label = "nekostring";
        neko_LADSPA_descriptor->Properties = 0;
        neko_LADSPA_descriptor->Name = "Neko DSSI plugin";
        neko_LADSPA_descriptor->Maker = "Gordon JC Pearce";
        neko_LADSPA_descriptor->Copyright = "GNU General Public License version 2 or later";
        neko_LADSPA_descriptor->PortCount = NEKO_PORTS_COUNT;

        port_descriptors = (LADSPA_PortDescriptor *)
                                calloc(neko_LADSPA_descriptor->PortCount, sizeof
                                                (LADSPA_PortDescriptor));
        neko_LADSPA_descriptor->PortDescriptors =
            (const LADSPA_PortDescriptor *) port_descriptors;

        port_range_hints = (LADSPA_PortRangeHint *)
                                calloc(neko_LADSPA_descriptor->PortCount, sizeof
                                                (LADSPA_PortRangeHint));
        neko_LADSPA_descriptor->PortRangeHints =
            (const LADSPA_PortRangeHint *) port_range_hints;

        port_names = (char **) calloc(neko_LADSPA_descriptor->PortCount, sizeof(char *));
        neko_LADSPA_descriptor->PortNames = (const char **) port_names;

        for (i = 0; i < NEKO_PORTS_COUNT; i++) {
            port_descriptors[i] = neko_port_description[i].port_descriptor;
            port_names[i]       = neko_port_description[i].name;
            port_range_hints[i].HintDescriptor = neko_port_description[i].hint_descriptor;
            port_range_hints[i].LowerBound     = neko_port_description[i].lower_bound;
            port_range_hints[i].UpperBound     = neko_port_description[i].upper_bound;
        }

        neko_LADSPA_descriptor->instantiate = neko_instantiate;
        neko_LADSPA_descriptor->connect_port = neko_connect_port;
        neko_LADSPA_descriptor->activate = neko_activate;
        neko_LADSPA_descriptor->run = neko_ladspa_run_wrapper;
        neko_LADSPA_descriptor->run_adding = NULL;
        neko_LADSPA_descriptor->set_run_adding_gain = NULL;
        neko_LADSPA_descriptor->deactivate = neko_deactivate;
        neko_LADSPA_descriptor->cleanup = neko_cleanup;
    }

    neko_DSSI_descriptor = (DSSI_Descriptor *) malloc(sizeof(DSSI_Descriptor));
    if (neko_DSSI_descriptor) {
        neko_DSSI_descriptor->DSSI_API_Version = 1;
        neko_DSSI_descriptor->LADSPA_Plugin = neko_LADSPA_descriptor;
        neko_DSSI_descriptor->configure = neko_configure;
        neko_DSSI_descriptor->get_program = neko_get_program;
        neko_DSSI_descriptor->select_program = neko_select_program;
        neko_DSSI_descriptor->get_midi_controller_for_port = neko_get_midi_controller;
        neko_DSSI_descriptor->run_synth = neko_run_synth;
        neko_DSSI_descriptor->run_synth_adding = NULL;
        neko_DSSI_descriptor->run_multiple_synths = NULL;
        neko_DSSI_descriptor->run_multiple_synths_adding = NULL;
    }
}

void _fini()
{
    if (neko_LADSPA_descriptor) {
        free((LADSPA_PortDescriptor *) neko_LADSPA_descriptor->PortDescriptors);
        free((char **) neko_LADSPA_descriptor->PortNames);
        free((LADSPA_PortRangeHint *) neko_LADSPA_descriptor->PortRangeHints);
        free(neko_LADSPA_descriptor);
    }
    if (neko_DSSI_descriptor) {
        free(neko_DSSI_descriptor);
    }
}
