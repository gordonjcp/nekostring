/* nekostring - neko_ports.c */

/* nekostring polyphonic string ensemble
 *
 * (C) 2006 Gordon JC Pearce and others
 *
 * For further information on contributors and the history of nekostring,
 * please see the README
 *
*/

#include <ladspa.h>

#include "neko_ports.h"

struct neko_port_descriptor neko_port_description[NEKO_PORTS_COUNT] = {

#define PD_OUT     (LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO)
#define PD_IN      (LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL)
#define HD_MIN     (LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_MINIMUM)
#define HD_LOW     (LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_LOW)
#define HD_MID     (LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_MIDDLE)
#define HD_HI      (LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_HIGH)
#define HD_MAX     (LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_MAXIMUM)
#define HD_440     (LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_440)
#define HD_LOG     (LADSPA_HINT_LOGARITHMIC)
#define HD_DETENT  (LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_INTEGER | LADSPA_HINT_DEFAULT_MINIMUM)
#define HD_SWITCH  (LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_INTEGER | LADSPA_HINT_TOGGLED )
#define XPT_LIN    NEKO_PORT_TYPE_LINEAR
#define XPT_LOG    NEKO_PORT_TYPE_LOGARITHMIC
#define XPT_DETE   NEKO_PORT_TYPE_DETENT
#define XPT_ONOFF  NEKO_PORT_TYPE_ONOFF
#define XPT_VCF    NEKO_PORT_TYPE_VCF_MODE
    { PD_OUT, "Left",              0,               0.0f,     0.0f,   0,         0.,0.,0. },
    { PD_OUT, "Right",              0,               0.0f,     0.0f,   0,         0.,0.,0. },
    { PD_IN,  "Bass Level",          HD_MID, 0.0f,    1.0f,   XPT_LIN,   0.,1.,0. },
    { PD_IN,  "Cello Level",          HD_MID, 0.0f,    1.0f,   XPT_LIN,   0.,1.,0. },
    { PD_IN,  "Viola Level",          HD_MID, 0.0f,    1.0f,   XPT_LIN,   0.,1.,0. },
    { PD_IN,  "Violin Level",          HD_MID, 0.0f,    1.0f,   XPT_LIN,   0.,1.,0. },
    { PD_IN,  "Speed",          HD_MID, 0.5f,    2.0f,   XPT_LIN,  0.5,2.,0. },
    { PD_IN,  "Depth",          HD_MID, 0.0f,    1.0f,   XPT_LIN,   0.,1.,0. },
 { PD_IN,  "Vibrato/Chorus",          HD_MID, -1.0f,    1.0f,   XPT_LIN,   -1.,1.,0. },
 { PD_IN,  "Mix",          HD_MID, 0.0f,    1.0f,   XPT_LIN,   0.,1.,0. },
 { PD_IN,  "Tone",          HD_MID, 0.0f,    1.0f,   XPT_LIN,   0.,1.,0. },
{ PD_IN,  "Attack",          HD_MID, 0.0f,    1.0f,   XPT_LIN,   0.,1.,0. },
{ PD_IN,  "Decay",          HD_MID, 0.0f,    1.0f,   XPT_LIN,   0.,1.,0. },
{ PD_IN,  "Bass Decay",          HD_MID, 0.0f,    1.0f,   XPT_LIN,   0.,1.,0. },

    { PD_IN,  "Volume",              HD_LOW,          0.0f,     1.0f,   XPT_LIN,   0.,1.,0. },
    { PD_IN,  "Tuning",              HD_440,          415.3f,   466.2f, XPT_LIN,   415.3,466.2,0. }
#undef PD_OUT
#undef PD_IN
#undef HD_MIN
#undef HD_LOW
#undef HD_MAX
#undef HD_440
#undef HD_LOG
#undef HD_DETENT
#undef HD_SWITCH
#undef XPT_LIN
#undef XPT_LOG
#undef XPT_DETE
#undef XPT_ONOFF
#undef XPT_VCF
};
