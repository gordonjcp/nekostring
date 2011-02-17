/* nekostring - gui_friendly_patches.c */

/* nekostring polyphonic string ensemble
 *
 * (C) 2006 Gordon JC Pearce and others
 *
 * For further information on contributors and the history of nekostring,
 * please see the README
 *
*/

#include "neko_voice.h"

#define FRIENDLY_PATCH_COUNT  1

int friendly_patch_count = FRIENDLY_PATCH_COUNT;

neko_patch_t friendly_patches[FRIENDLY_PATCH_COUNT] = {
    {
        "nekostrings",
        0.999, // bass
        .75, // cello
		1.0, // viola
		0.8, // violin
		1.0, // speed
		1.0, // depth
		0,	// vib/cho
		1.0, // mix
		0.75,	// attack
		0.5, // decay
		0.0001, // bass decay
		0.75, // tone
		.7 // volume
    }
 
};
