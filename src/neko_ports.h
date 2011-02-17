/* nekostring - neko_ports.h */

/* nekostring polyphonic string ensemble
 *
 * (C) 2006 Gordon JC Pearce and others
 *
 * For further information on contributors and the history of nekostring,
 * please see the README
 *
*/

#ifndef _NEKO_PORTS_H
#define _NEKO_PORTS_H

#include <ladspa.h>

#define NEKO_PORT_LEFT 	             0
#define NEKO_PORT_RIGHT              1
#define NEKO_PORT_BASS_LEVEL          2
#define NEKO_PORT_CELLO_LEVEL       3
#define NEKO_PORT_VIOLA_LEVEL     4
#define NEKO_PORT_VIOLIN_LEVEL         5
#define NEKO_PORT_SPEED       6
#define NEKO_PORT_DEPTH     7
#define NEKO_PORT_VIBRATO_CHORUS            8
#define NEKO_PORT_MIX         9
#define NEKO_PORT_ATTACK       10
#define NEKO_PORT_DECAY       11
#define NEKO_PORT_BASS_DECAY       12
#define NEKO_PORT_TONE       13
#define NEKO_PORT_VOLUME    14
#define NEKO_PORT_TUNING             15

#define NEKO_PORTS_COUNT  16

#define NEKO_PORT_TYPE_LINEAR       0
#define NEKO_PORT_TYPE_LOGARITHMIC  1
#define NEKO_PORT_TYPE_DETENT       2
#define NEKO_PORT_TYPE_ONOFF        3
#define NEKO_PORT_TYPE_VCF_MODE     4

struct neko_port_descriptor {

    LADSPA_PortDescriptor          port_descriptor;
    char *                         name;
    LADSPA_PortRangeHintDescriptor hint_descriptor;
    LADSPA_Data                    lower_bound;
    LADSPA_Data                    upper_bound;
    int                            type;
    float                          a, b, c;  /* scaling parameters for continuous controls */

};

extern struct neko_port_descriptor neko_port_description[];

#endif /* _NEKO_PORTS_H */
