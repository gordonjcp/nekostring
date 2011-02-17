/* nekostring - neko_data.c */

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

#include "dssi.h"

#include "neko_voice.h"

neko_patch_t neko_init_voice = {
        "nekostrings",
        0.999, // bass
        0.1, // cello
		1.0, // viola
		0.5, // violin
		1.0, // speed
		1.0, // depth
		0,	// vib/cho
		1.0, // mix
		0.5,	// attack
		0.5, // decay
		0.5, // bass decay
		0.5, // tone
		.7 // volume
	
	
};

/*
static int
is_comment(char *buf)  // line is blank, whitespace, or first non-whitespace character is 
{
    int i = 0;

    while (buf[i]) {
        if (buf[i] == '#') return 1;
        if (buf[i] == '\n') return 1;
        if (buf[i] != ' ' && buf[i] != '\t') return 0;
        i++;
    }
    return 1;
}
*/
/*
static void
parse_name(const char *buf, char *name, int *inlen)
{
    int i = 0, o = 0;
    unsigned int t;

    while (buf[i] && o < 30) {
        if (buf[i] < 33 || buf[i] > 126) {
            break;
        } else if (buf[i] == '%') {
            if (buf[i + 1] && buf[i + 2] && sscanf(buf + i + 1, "%2x", &t) == 1) {
                name[o++] = (char)t;
                i += 3;
            } else {
                break;
            }
        } else {
            name[o++] = buf[i++];
        }
    }
    // trim trailing spaces 
    while (o && name[o - 1] == ' ') o--;
    name[o] = '\0';

    if (inlen) *inlen = i;
}
*/
int
neko_data_read_patch(FILE *file, neko_patch_t *patch)
{
/*    int format, i;
    char buf[256], buf2[90];
    neko_patch_t tmp;

    do {
        if (!fgets(buf, 256, file)) return 0;
    } while (is_comment(buf));

    if (sscanf(buf, " neko-dssi patch format %d begin", &format) != 1 ||
        format < 0 || format > 1)
        return 0;

    if (!fgets(buf, 256, file)) return 0;
    if (sscanf(buf, " name %90s", buf2) != 1) return 0;
    parse_name(buf2, tmp.name, NULL);

    if (!fgets(buf, 256, file)) return 0;
    if (sscanf(buf, " osc1 %f %d %f", &tmp.bass, &i,
               &tmp.viola) != 3)
        return 0;
    tmp.cello = (unsigned char)i;

    if (!fgets(buf, 256, file)) return 0;
    if (sscanf(buf, " osc2 %f %d %f", &tmp.violin, &i,
               &tmp.osc2_pulsewidth) != 3)
        return 0;
    tmp.speed = (unsigned char)i;

    if (!fgets(buf, 256, file)) return 0;
    if (sscanf(buf, " sync %d", &i) != 1)
        return 0;
    tmp.vibrato_chorus = (unsigned char)i;

    if (!fgets(buf, 256, file)) return 0;
    if (sscanf(buf, " balance %f", &tmp.mix) != 1)
        return 0;

    if (!fgets(buf, 256, file)) return 0;
    if (sscanf(buf, " lfo %f %d %f %f", &tmp.attack, &i,
               &tmp.bass_decay, &tmp.tone) != 4)
        return 0;
    tmp.decay = (unsigned char)i;

    if (format == 1) {

        if (!fgets(buf, 256, file)) return 0;
        if (sscanf(buf, " eg1 %f %f %f %f %f %f %f",
                   &tmp.eg1_attack_time, &tmp.eg1_decay_time,
                   &tmp.eg1_sustain_level, &tmp.eg1_release_time,
                   &tmp.eg1_vel_sens, &tmp.eg1_amount_o,
                   &tmp.eg1_amount_f) != 7)
            return 0;

        if (!fgets(buf, 256, file)) return 0;
        if (sscanf(buf, " eg2 %f %f %f %f %f %f %f",
                   &tmp.eg2_attack_time, &tmp.eg2_decay_time,
                   &tmp.eg2_sustain_level, &tmp.eg2_release_time,
                   &tmp.eg2_vel_sens, &tmp.eg2_amount_o,
                   &tmp.eg2_amount_f) != 7)
            return 0;

    } else {

        if (!fgets(buf, 256, file)) return 0;
        if (sscanf(buf, " eg1 %f %f %f %f %f %f",
                   &tmp.eg1_attack_time, &tmp.eg1_decay_time,
                   &tmp.eg1_sustain_level, &tmp.eg1_release_time,
                   &tmp.eg1_amount_o, &tmp.eg1_amount_f) != 6)
            return 0;

        if (!fgets(buf, 256, file)) return 0;
        if (sscanf(buf, " eg2 %f %f %f %f %f %f",
                   &tmp.eg2_attack_time, &tmp.eg2_decay_time,
                   &tmp.eg2_sustain_level, &tmp.eg2_release_time,
                   &tmp.eg2_amount_o, &tmp.eg2_amount_f) != 6)
            return 0;

        tmp.eg1_vel_sens = 0.0f;
        tmp.eg2_vel_sens = 0.0f;
    }

    if (!fgets(buf, 256, file)) return 0;
    if (sscanf(buf, " vcf %f %f %d", &tmp.vcf_cutoff, &tmp.vcf_qres, &i) != 3)
        return 0;
    tmp.vcf_mode = (unsigned char)i;

    if (!fgets(buf, 256, file)) return 0;
    if (sscanf(buf, " glide %f", &tmp.glide_time) != 1)
        return 0;

    if (!fgets(buf, 256, file)) return 0;
    if (sscanf(buf, " volume %f", &tmp.volume) != 1)
        return 0;

    if (!fgets(buf, 256, file)) return 0;
    if (sscanf(buf, " neko-dssi patch %3s", buf2) != 1) return 0;
    if (strcmp(buf2, "end")) return 0;

    memcpy(patch, &tmp, sizeof(neko_patch_t));
*/
    return 1;  /* -FIX- error handling yet to be implemented */
}

int
neko_data_decode_patches(const unsigned char *encoded, neko_patch_t *patches)
{
   int j, n, i0, i1, i2, i3;
    const unsigned char *ep = encoded;
    neko_patch_t *tmp, *pp;

//    if (strncmp(ep, "Xp0 ", 4)) {
//        /* fprintf(stderr, "bad header\n"); */
//        return 0;  /* bad format */
//    }
    ep += 4;

    tmp = (neko_patch_t *)malloc(32 * sizeof(neko_patch_t));
    if (!tmp)
        return 0;  /* out of memory */
    
    for (j = 0; j < 32; j++) {
        pp = &tmp[j];

//        parse_name(ep, pp->name, &n);
        if (!n) {
            /* fprintf(stderr, "failed in name\n"); */
            break;
        }
        ep += n;

/*	if (sscanf(ep, " %f %d %f %f %d %f %d %f %f %d %f %f%n",
                   &pp->bass, &i0, &pp->viola,
                   &pp->violin, &i1, &pp->osc2_pulsewidth,
                   &i2, &pp->mix, &pp->attack,
                   &i3, &pp->bass_decay, &pp->tone,
                   &n) != 12) {
*/            /* fprintf(stderr, "failed in oscs\n"); */
 //           break;
 //       }
        pp->cello = (unsigned char)i0;
        pp->speed = (unsigned char)i1;
        pp->vibrato_chorus = (unsigned char)i2;
        pp->decay = (unsigned char)i3;
        ep += n;
        }
        ep += n;

        while (*ep == ' ') ep++;
    
//    if (j != 32 || strcmp(ep, "end")) {
 //       /* fprintf(stderr, "decode failed, j = %d, *ep = 0x%02x\n", j, *ep); */
//        free(tmp);
//        return 0;  /* too few patches, or otherwise bad format */
 //   }

    memcpy(patches, tmp, 32 * sizeof(neko_patch_t));

    free(tmp);

    return 1;
}
