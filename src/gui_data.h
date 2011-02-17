/* nekostring - gui_data.h */

/* nekostring polyphonic string ensemble
 *
 * (C) 2006 Gordon JC Pearce and others
 *
 * For further information on contributors and the history of nekostring,
 * please see the README
 *
*/

#ifndef _GUI_DATA_H
#define _GUI_DATA_H

#include <stdio.h>

#include "neko_types.h"
#include "neko_voice.h"

#define NEKO_OLD_PATCH_SIZE_PACKED     209  /* Steve Brookes' original patch save file format */
#define NEKO_OLD_PATCH_SIZE_WITH_NAME  224  /* the above followed by 15 characters of name */

/* gui_data.c */
void gui_data_import_patch(neko_patch_t *neko_patch,
                           unsigned char *old_patch, int unpack_name);
int  gui_data_write_patch(FILE *file, neko_patch_t *patch);
int  gui_data_save(char *filename, int start, int end, char **message);
void gui_data_mark_dirty_patch_sections(int start_patch, int end_patch);
int  gui_data_load(const char *filename, int position, char **message);
void gui_data_friendly_patches(void);
void gui_data_send_dirty_patch_sections(void);
int  gui_data_patch_compare(neko_patch_t *patch1, neko_patch_t *patch2);

/* gui_friendly_patches.c */
extern int            friendly_patch_count;
extern neko_patch_t friendly_patches[];

/* neko_data.c */
extern neko_patch_t neko_init_voice;

int   neko_data_read_patch(FILE *file, neko_patch_t *patch);
int   neko_data_decode_patches(const char *encoded, neko_patch_t *patches);

#endif /* _GUI_DATA_H */
