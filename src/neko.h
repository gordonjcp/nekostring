/* nekostring - neko.h */

/* nekostring polyphonic string ensemble
 *
 * (C) 2006 Gordon JC Pearce and others
 *
 * For further information on contributors and the history of nekostring,
 * please see the README
 *
*/

#ifndef _NEKO_H
#define _NEKO_H

/* ==== debugging ==== */

/* NEKO_DEBUG bits */
#define XDB_DSSI   1   /* DSSI interface */
#define XDB_AUDIO  2   /* audio output */
#define XDB_NOTE   4   /* note on/off, voice allocation */
#define XDB_DATA   8   /* plugin patchbank handling */
#define GDB_MAIN  16   /* GUI main program flow */
#define GDB_OSC   32   /* GUI OSC handling */
#define GDB_IO    64   /* GUI patch file input/output */
#define GDB_GUI  128   /* GUI GUI callbacks, updating, etc. */

/* If you want debug information, define NEKO_DEBUG to the XDB_* bits you're
 * interested in getting debug information about, bitwise-ORed together.
 * Otherwise, leave it undefined. */
// #define NEKO_DEBUG (1+8+16+32+64)

#define NEKO_DEBUG (GDB_MAIN)
#ifdef NEKO_DEBUG

#include <stdio.h>
#define NEKO_DEBUG_INIT(x)
#define XDB_MESSAGE(type, fmt...) { if (NEKO_DEBUG & type) fprintf(stderr, "neko-dssi.so" fmt); }
#define GDB_MESSAGE(type, fmt...) { if (NEKO_DEBUG & type) fprintf(stderr, "Neko_gtk" fmt); }
// -FIX-:
// #include "message_buffer.h"
// #define NEKO_DEBUG_INIT(x)  mb_init(x)
// #define XDB_MESSAGE(type, fmt...) { \-
//     if (NEKO_DEBUG & type) { \-
//         char _m[256]; \-
//         snprintf(_m, 255, fmt); \-
//         add_message(_m); \-
//     } \-
// }

#else  /* !NEKO_DEBUG */

#define XDB_MESSAGE(type, fmt...)
#define GDB_MESSAGE(type, fmt...)
#define NEKO_DEBUG_INIT(x)

#endif  /* NEKO_DEBUG */

/* ==== end of debugging ==== */

#define NEKO_MAX_POLYPHONY     64
#define NEKO_DEFAULT_POLYPHONY  8

#endif /* _NEKO_H */
