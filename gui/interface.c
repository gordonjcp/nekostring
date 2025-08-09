/* nekostring - interface.c */

/* nekostring polyphonic string ensemble
 *
 * (C) 2006 Gordon JC Pearce and others
 *
 * For further information on contributors and the history of nekostring,
 * please see the README
 *
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "widgets.h"

#include "../src/neko.h"
#include "../src/neko_ports.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

GtkWidget *voice_widget[NEKO_PORTS_COUNT];
  GtkWidget *window1;
  GtkWidget *notebook1;
  GtkWidget *vbox1;
  GtkWidget *hbox1;
  GtkWidget *frame_voicing;
  GtkWidget *alignment3;
  GtkWidget *hbox2;
  GtkWidget *vb_contrabass;
  GtkWidget *vs_bass;
  GtkWidget *label8;
  GtkWidget *vbox4;
  GtkWidget *vs_cello;
  GtkWidget *label10;
  GtkWidget *vbox5;
  GtkWidget *vs_viola;
  GtkWidget *label11;
  GtkWidget *vbox6;
  GtkWidget *vs_violin;
  GtkWidget *label12;
  GtkWidget *label_voicing;
  GtkWidget *frame_ensemble;
  GtkWidget *alignment2;
  GtkWidget *hbox3;
  GtkWidget *vbox7;
  GtkWidget *vs_cspeed;
  GtkWidget *label13;
  GtkWidget *vbox8;
  GtkWidget *vs_cdepth;
  GtkWidget *label14;
  GtkWidget *vbox9;
  GtkWidget *vs_vspeed;
  GtkWidget *label15;
  GtkWidget *vbox10;
  GtkWidget *vs_vdepth;
  GtkWidget *label16;
  GtkWidget *label_ensemble;
  GtkWidget *frame4;
  GtkWidget *alignment4;
  GtkWidget *hbox4;
  GtkWidget *vbox11;
  GtkWidget *vs_tone;
  GtkWidget *label18;
  GtkWidget *vbox12;
  GtkWidget *vs_volume;
  GtkWidget *label19;
  GtkWidget *label17;
  GtkWidget *hbox5;
  GtkWidget *image1;
  GtkWidget *frame5;
  GtkWidget *alignment5;
  GtkWidget *hbox6;
  GtkWidget *vbox17;
  GtkWidget *vscale19;
  GtkWidget *label25;
  GtkWidget *vbox18;
  GtkWidget *vscale20;
  GtkWidget *label26;
  GtkWidget *vbox19;
  GtkWidget *vscale21;
  GtkWidget *label27;
  GtkWidget *label23;
  GtkWidget *frame6;
  GtkWidget *alignment6;
  GtkWidget *vbox16;
  GtkWidget *combobox1;
  GtkWidget *btn_savepatch;
  GtkWidget *label24;
  GtkWidget *label_main;
  GtkWidget *image2;
  GtkWidget *label2;

GtkWidget*
create_window1 (void)
{

  window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_size_request (window1, 600, 300);
  gtk_window_set_title (GTK_WINDOW (window1), "nekostring");
  gtk_window_set_resizable (GTK_WINDOW (window1), FALSE);

  notebook1 = gtk_notebook_new ();
  gtk_widget_show (notebook1);
  gtk_container_add (GTK_CONTAINER (window1), notebook1);
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook1), GTK_POS_BOTTOM);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (notebook1), vbox1);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, TRUE, 0);
  gtk_widget_set_size_request (hbox1, -1, 172);

  frame_voicing = gtk_frame_new (NULL);
  gtk_widget_show (frame_voicing);
  gtk_box_pack_start (GTK_BOX (hbox1), frame_voicing, TRUE, TRUE, 0);
  gtk_widget_set_size_request (frame_voicing, 220, 80);

  alignment3 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment3);
  gtk_container_add (GTK_CONTAINER (frame_voicing), alignment3);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment3), 0, 0, 12, 0);

  hbox2 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox2);
  gtk_container_add (GTK_CONTAINER (alignment3), hbox2);

  vb_contrabass = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vb_contrabass);
  gtk_box_pack_start (GTK_BOX (hbox2), vb_contrabass, TRUE, TRUE, 0);
  gtk_widget_set_size_request (vb_contrabass, 50, 135);

  voice_widget[NEKO_PORT_BASS_LEVEL] = gtk_vscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 0, 0, 0, 0)));
  gtk_widget_show (voice_widget[NEKO_PORT_BASS_LEVEL]);
  gtk_box_pack_start (GTK_BOX (vb_contrabass), voice_widget[NEKO_PORT_BASS_LEVEL], TRUE, FALSE, 0);
  gtk_widget_set_size_request (voice_widget[NEKO_PORT_BASS_LEVEL], 50, 120);
  gtk_scale_set_draw_value (GTK_SCALE (voice_widget[NEKO_PORT_BASS_LEVEL]), FALSE);
  gtk_range_set_inverted (GTK_RANGE (voice_widget[NEKO_PORT_BASS_LEVEL]), TRUE);

  label8 = gtk_label_new ("Bass");
  gtk_widget_show (label8);
  gtk_box_pack_start (GTK_BOX (vb_contrabass), label8, FALSE, FALSE, 0);
  gtk_widget_set_size_request (label8, -1, 30);

  vbox4 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox4);
  gtk_box_pack_start (GTK_BOX (hbox2), vbox4, TRUE, TRUE, 0);
  gtk_widget_set_size_request (vbox4, 50, 135);

  vs_cello = gtk_vscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (.75, 0, 1, 0, 0, 0)));
  gtk_widget_show (vs_cello);
  gtk_box_pack_start (GTK_BOX (vbox4), vs_cello, TRUE, FALSE, 0);
  gtk_widget_set_size_request (vs_cello, 50, 120);
  gtk_scale_set_draw_value (GTK_SCALE (vs_cello), FALSE);
  gtk_range_set_inverted (GTK_RANGE (vs_cello), TRUE);

  label10 = gtk_label_new ("Cello");
  gtk_widget_show (label10);
  gtk_box_pack_start (GTK_BOX (vbox4), label10, FALSE, FALSE, 0);
  gtk_widget_set_size_request (label10, -1, 30);

  vbox5 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox5);
  gtk_box_pack_start (GTK_BOX (hbox2), vbox5, TRUE, TRUE, 0);
  gtk_widget_set_size_request (vbox5, 50, 135);

  vs_viola = gtk_vscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 0, 1, 0, 0, 0)));
  gtk_widget_show (vs_viola);
  gtk_box_pack_start (GTK_BOX (vbox5), vs_viola, TRUE, FALSE, 0);
  gtk_widget_set_size_request (vs_viola, 50, 120);
  gtk_scale_set_draw_value (GTK_SCALE (vs_viola), FALSE);
  gtk_range_set_inverted (GTK_RANGE (vs_viola), TRUE);

  label11 = gtk_label_new ("Viola");
  gtk_widget_show (label11);
  gtk_box_pack_start (GTK_BOX (vbox5), label11, FALSE, FALSE, 0);
  gtk_widget_set_size_request (label11, -1, 30);

  vbox6 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox6);
  gtk_box_pack_start (GTK_BOX (hbox2), vbox6, TRUE, TRUE, 0);
  gtk_widget_set_size_request (vbox6, 50, 135);

  vs_violin = gtk_vscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (.8, 0, 1, 0, 0, 0)));
  gtk_widget_show (vs_violin);
  gtk_box_pack_start (GTK_BOX (vbox6), vs_violin, TRUE, FALSE, 0);
  gtk_widget_set_size_request (vs_violin, 50, 120);
  gtk_scale_set_draw_value (GTK_SCALE (vs_violin), FALSE);
  gtk_range_set_inverted (GTK_RANGE (vs_violin), TRUE);

  label12 = gtk_label_new ("Violin");
  gtk_widget_show (label12);
  gtk_box_pack_start (GTK_BOX (vbox6), label12, FALSE, FALSE, 0);
  gtk_widget_set_size_request (label12, -1, 30);

  label_voicing = gtk_label_new ("<b>Voicing</b>");
  gtk_widget_show (label_voicing);
  gtk_frame_set_label_widget (GTK_FRAME (frame_voicing), label_voicing);
  gtk_label_set_use_markup (GTK_LABEL (label_voicing), TRUE);

  frame_ensemble = gtk_frame_new (NULL);
  gtk_widget_show (frame_ensemble);
  gtk_box_pack_start (GTK_BOX (hbox1), frame_ensemble, TRUE, TRUE, 0);
  gtk_widget_set_size_request (frame_ensemble, 220, -1);

  alignment2 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment2);
  gtk_container_add (GTK_CONTAINER (frame_ensemble), alignment2);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment2), 0, 0, 12, 0);

  hbox3 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox3);
  gtk_container_add (GTK_CONTAINER (alignment2), hbox3);

  vbox7 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox7);
  gtk_box_pack_start (GTK_BOX (hbox3), vbox7, TRUE, TRUE, 0);
  gtk_widget_set_size_request (vbox7, 50, 135);

  vs_cspeed = gtk_vscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (1, 0.5, 2, 0, 0, 0)));
  gtk_widget_show (vs_cspeed);
  gtk_box_pack_start (GTK_BOX (vbox7), vs_cspeed, TRUE, FALSE, 0);
  gtk_widget_set_size_request (vs_cspeed, 50, 120);
  gtk_scale_set_draw_value (GTK_SCALE (vs_cspeed), FALSE);
  gtk_range_set_inverted (GTK_RANGE (vs_cspeed), TRUE);

  label13 = gtk_label_new ("Speed");
  gtk_widget_show (label13);
  gtk_box_pack_start (GTK_BOX (vbox7), label13, FALSE, FALSE, 0);
  gtk_widget_set_size_request (label13, -1, 30);
  gtk_label_set_justify (GTK_LABEL (label13), GTK_JUSTIFY_CENTER);

  vbox8 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox8);
  gtk_box_pack_start (GTK_BOX (hbox3), vbox8, TRUE, TRUE, 0);
  gtk_widget_set_size_request (vbox8, 50, 135);

  vs_cdepth = gtk_vscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0.5, 0, 1, 0, 0, 0)));
  gtk_widget_show (vs_cdepth);
  gtk_box_pack_start (GTK_BOX (vbox8), vs_cdepth, TRUE, FALSE, 0);
  gtk_widget_set_size_request (vs_cdepth, 50, 120);
  gtk_scale_set_draw_value (GTK_SCALE (vs_cdepth), FALSE);
  gtk_range_set_inverted (GTK_RANGE (vs_cdepth), TRUE);

  label14 = gtk_label_new ("Depth");
  gtk_widget_show (label14);
  gtk_box_pack_start (GTK_BOX (vbox8), label14, FALSE, FALSE, 0);
  gtk_widget_set_size_request (label14, -1, 30);

  vbox9 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox9);
  gtk_box_pack_start (GTK_BOX (hbox3), vbox9, TRUE, TRUE, 0);
  gtk_widget_set_size_request (vbox9, 50, 135);

  vs_vspeed = gtk_vscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 0, 0, 0, 0)));
  gtk_widget_show (vs_vspeed);
  gtk_box_pack_start (GTK_BOX (vbox9), vs_vspeed, TRUE, FALSE, 0);
  gtk_widget_set_size_request (vs_vspeed, 50, 120);
  gtk_scale_set_draw_value (GTK_SCALE (vs_vspeed), FALSE);
  gtk_range_set_inverted (GTK_RANGE (vs_vspeed), TRUE);

  label15 = gtk_label_new ("Vibrato\nChorus");
  gtk_widget_show (label15);
  gtk_box_pack_start (GTK_BOX (vbox9), label15, FALSE, FALSE, 0);
  gtk_widget_set_size_request (label15, -1, 30);

  vbox10 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox10);
  gtk_box_pack_start (GTK_BOX (hbox3), vbox10, TRUE, TRUE, 0);
  gtk_widget_set_size_request (vbox10, 50, 135);

  vs_vdepth = gtk_vscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 0, 0, 0, 0)));
  gtk_widget_show (vs_vdepth);
  gtk_box_pack_start (GTK_BOX (vbox10), vs_vdepth, TRUE, FALSE, 0);
  gtk_widget_set_size_request (vs_vdepth, 50, 120);
  gtk_scale_set_draw_value (GTK_SCALE (vs_vdepth), FALSE);
//  gtk_scale_set_digits (GTK_SCALE (vs_vdepth), 2);
//  gtk_range_set_update_policy (GTK_RANGE (vs_vdepth), GTK_UPDATE_DISCONTINUOUS);
  gtk_range_set_inverted (GTK_RANGE (vs_vdepth), TRUE);

  label16 = gtk_label_new ("Mix");
  gtk_widget_show (label16);
  gtk_box_pack_start (GTK_BOX (vbox10), label16, FALSE, FALSE, 0);
  gtk_widget_set_size_request (label16, -1, 30);

  label_ensemble = gtk_label_new ("<b>Ensemble</b>");
  gtk_widget_show (label_ensemble);
  gtk_frame_set_label_widget (GTK_FRAME (frame_ensemble), label_ensemble);
  gtk_label_set_use_markup (GTK_LABEL (label_ensemble), TRUE);

  frame4 = gtk_frame_new (NULL);
  gtk_widget_show (frame4);
  gtk_box_pack_start (GTK_BOX (hbox1), frame4, TRUE, TRUE, 0);
  gtk_widget_set_size_request (frame4, 110, -1);

  alignment4 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment4);
  gtk_container_add (GTK_CONTAINER (frame4), alignment4);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment4), 0, 0, 12, 0);

  hbox4 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox4);
  gtk_container_add (GTK_CONTAINER (alignment4), hbox4);

  vbox11 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox11);
  gtk_box_pack_start (GTK_BOX (hbox4), vbox11, TRUE, TRUE, 0);
  gtk_widget_set_size_request (vbox11, 50, 135);

  vs_tone = gtk_vscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0.75, 0, 1, 0, 0, 0)));
  gtk_widget_show (vs_tone);
  gtk_box_pack_start (GTK_BOX (vbox11), vs_tone, TRUE, FALSE, 0);
  gtk_widget_set_size_request (vs_tone, 50, 120);
  gtk_scale_set_draw_value (GTK_SCALE (vs_tone), FALSE);
  gtk_range_set_inverted (GTK_RANGE (vs_tone), TRUE);

  label18 = gtk_label_new ("Tone");
  gtk_widget_show (label18);
  gtk_box_pack_start (GTK_BOX (vbox11), label18, FALSE, FALSE, 0);
  gtk_widget_set_size_request (label18, -1, 30);

  vbox12 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox12);
  gtk_box_pack_start (GTK_BOX (hbox4), vbox12, TRUE, TRUE, 0);
  gtk_widget_set_size_request (vbox12, 50, 135);

  vs_volume = gtk_vscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0.7, 0, 1, 0, 0, 0)));
  gtk_widget_show (vs_volume);
  gtk_box_pack_start (GTK_BOX (vbox12), vs_volume, TRUE, FALSE, 0);
  gtk_widget_set_size_request (vs_volume, 50, 120);
  gtk_scale_set_draw_value (GTK_SCALE (vs_volume), FALSE);
  gtk_range_set_inverted (GTK_RANGE (vs_volume), TRUE);

  label19 = gtk_label_new ("Volume");
  gtk_widget_show (label19);
  gtk_box_pack_start (GTK_BOX (vbox12), label19, FALSE, FALSE, 0);
  gtk_widget_set_size_request (label19, -1, 30);

  label17 = gtk_label_new ("<b>Output</b>");
  gtk_widget_show (label17);
  gtk_frame_set_label_widget (GTK_FRAME (frame4), label17);
  gtk_label_set_use_markup (GTK_LABEL (label17), TRUE);

  hbox5 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox5);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox5, TRUE, TRUE, 0);

  image1 = gtk_image_new_from_icon_name ("gtk-dialog-warning", GTK_ICON_SIZE_BUTTON);
  gtk_image_set_pixel_size (GTK_IMAGE(image1), 57);
  gtk_widget_show (image1);
  gtk_box_pack_start (GTK_BOX (hbox5), image1, TRUE, TRUE, 0);
  gtk_widget_set_size_request (image1, 1, -1);

  frame5 = gtk_frame_new (NULL);
  gtk_widget_show (frame5);
  gtk_box_pack_start (GTK_BOX (hbox5), frame5, TRUE, TRUE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame5), GTK_SHADOW_NONE);

  alignment5 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment5);
  gtk_container_add (GTK_CONTAINER (frame5), alignment5);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment5), 0, 0, 12, 0);

  hbox6 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox6);
  gtk_container_add (GTK_CONTAINER (alignment5), hbox6);

  vbox17 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox17);
  gtk_box_pack_start (GTK_BOX (hbox6), vbox17, TRUE, TRUE, 0);
  gtk_widget_set_size_request (vbox17, 50, 135);

  vscale19 = gtk_vscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0.36, 0, 1, 0, 0, 0)));
  gtk_widget_show (vscale19);
  gtk_box_pack_start (GTK_BOX (vbox17), vscale19, TRUE, FALSE, 0);
  gtk_widget_set_size_request (vscale19, 50, 60);
  gtk_scale_set_draw_value (GTK_SCALE (vscale19), FALSE);
  gtk_range_set_inverted (GTK_RANGE (vscale19), TRUE);

  label25 = gtk_label_new ("Attack");
  gtk_widget_show (label25);
  gtk_box_pack_start (GTK_BOX (vbox17), label25, FALSE, FALSE, 0);
  gtk_widget_set_size_request (label25, -1, 15);

  vbox18 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox18);
  gtk_box_pack_start (GTK_BOX (hbox6), vbox18, TRUE, TRUE, 0);
  gtk_widget_set_size_request (vbox18, 50, 135);

  vscale20 = gtk_vscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1, 0, 0, 0)));
  gtk_widget_show (vscale20);
  gtk_box_pack_start (GTK_BOX (vbox18), vscale20, TRUE, FALSE, 0);
  gtk_widget_set_size_request (vscale20, 50, 60);
  gtk_scale_set_draw_value (GTK_SCALE (vscale20), FALSE);
  gtk_range_set_inverted (GTK_RANGE (vscale20), TRUE);

  label26 = gtk_label_new ("Decay\n");
  gtk_widget_show (label26);
  gtk_box_pack_start (GTK_BOX (vbox18), label26, FALSE, FALSE, 0);
  gtk_widget_set_size_request (label26, -1, 15);

  vbox19 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox19);
  gtk_box_pack_start (GTK_BOX (hbox6), vbox19, TRUE, TRUE, 0);
  gtk_widget_set_size_request (vbox19, 50, 135);

  vscale21 = gtk_vscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0.04, 0, 1, 0, 0, 0)));
  gtk_widget_show (vscale21);
  gtk_box_pack_start (GTK_BOX (vbox19), vscale21, TRUE, FALSE, 0);
  gtk_widget_set_size_request (vscale21, 50, 60);
  gtk_scale_set_draw_value (GTK_SCALE (vscale21), FALSE);
  gtk_range_set_inverted (GTK_RANGE (vscale21), TRUE);

  label27 = gtk_label_new ("Bass Decay\n");
  gtk_widget_show (label27);
  gtk_box_pack_start (GTK_BOX (vbox19), label27, FALSE, FALSE, 0);
  gtk_widget_set_size_request (label27, -1, 15);

  label23 = gtk_label_new ("<b>Envelope</b>");
  gtk_widget_show (label23);
  gtk_frame_set_label_widget (GTK_FRAME (frame5), label23);
  gtk_label_set_use_markup (GTK_LABEL (label23), TRUE);

  frame6 = gtk_frame_new (NULL);
  gtk_widget_show (frame6);
  gtk_box_pack_start (GTK_BOX (hbox5), frame6, TRUE, TRUE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame6), GTK_SHADOW_NONE);

  alignment6 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment6);
  gtk_container_add (GTK_CONTAINER (frame6), alignment6);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment6), 0, 0, 12, 0);

  vbox16 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox16);
  gtk_container_add (GTK_CONTAINER (alignment6), vbox16);

  combobox1 = gtk_combo_box_new_text ();
  gtk_widget_show (combobox1);
  gtk_box_pack_start (GTK_BOX (vbox16), combobox1, TRUE, TRUE, 6);
  gtk_widget_set_size_request (combobox1, 1, -1);
  gtk_combo_box_append_text (GTK_COMBO_BOX (combobox1), "placeholder1");
  gtk_combo_box_append_text (GTK_COMBO_BOX (combobox1), "placeholder2");

  btn_savepatch = gtk_button_new_with_mnemonic ("Save Patch");
  gtk_widget_show (btn_savepatch);
  gtk_box_pack_start (GTK_BOX (vbox16), btn_savepatch, FALSE, FALSE, 8);
  gtk_widget_set_size_request (btn_savepatch, 1, -1);

  label24 = gtk_label_new ("<b>Patch</b>");
  gtk_widget_show (label24);
  gtk_frame_set_label_widget (GTK_FRAME (frame6), label24);
  gtk_label_set_use_markup (GTK_LABEL (label24), TRUE);

  label_main = gtk_label_new ("Controls");
  gtk_widget_show (label_main);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label_main);
  gtk_label_set_width_chars (GTK_LABEL (label_main), 10);

  image2 = gtk_image_new_from_icon_name ("gtk-bold", GTK_ICON_SIZE_BUTTON);
  gtk_image_set_pixel_size (GTK_IMAGE(image2), 209);
  gtk_widget_show (image2);
  gtk_container_add (GTK_CONTAINER (notebook1), image2);

  label2 = gtk_label_new ("Config");
  gtk_widget_show (label2);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), label2);
  gtk_label_set_width_chars (GTK_LABEL (label2), 10);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (window1, window1, "window1");
  GLADE_HOOKUP_OBJECT (window1, notebook1, "notebook1");
  GLADE_HOOKUP_OBJECT (window1, vbox1, "vbox1");
  GLADE_HOOKUP_OBJECT (window1, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (window1, frame_voicing, "frame_voicing");
  GLADE_HOOKUP_OBJECT (window1, alignment3, "alignment3");
  GLADE_HOOKUP_OBJECT (window1, hbox2, "hbox2");
  GLADE_HOOKUP_OBJECT (window1, vb_contrabass, "vb_contrabass");
  GLADE_HOOKUP_OBJECT (window1, voice_widget[NEKO_PORT_BASS_LEVEL], "vs_bass");
  GLADE_HOOKUP_OBJECT (window1, label8, "label8");
  GLADE_HOOKUP_OBJECT (window1, vbox4, "vbox4");
  GLADE_HOOKUP_OBJECT (window1, vs_cello, "vs_cello");
  GLADE_HOOKUP_OBJECT (window1, label10, "label10");
  GLADE_HOOKUP_OBJECT (window1, vbox5, "vbox5");
  GLADE_HOOKUP_OBJECT (window1, vs_viola, "vs_viola");
  GLADE_HOOKUP_OBJECT (window1, label11, "label11");
  GLADE_HOOKUP_OBJECT (window1, vbox6, "vbox6");
  GLADE_HOOKUP_OBJECT (window1, vs_violin, "vs_violin");
  GLADE_HOOKUP_OBJECT (window1, label12, "label12");
  GLADE_HOOKUP_OBJECT (window1, label_voicing, "label_voicing");
  GLADE_HOOKUP_OBJECT (window1, frame_ensemble, "frame_ensemble");
  GLADE_HOOKUP_OBJECT (window1, alignment2, "alignment2");
  GLADE_HOOKUP_OBJECT (window1, hbox3, "hbox3");
  GLADE_HOOKUP_OBJECT (window1, vbox7, "vbox7");
  GLADE_HOOKUP_OBJECT (window1, vs_cspeed, "vs_cspeed");
  GLADE_HOOKUP_OBJECT (window1, label13, "label13");
  GLADE_HOOKUP_OBJECT (window1, vbox8, "vbox8");
  GLADE_HOOKUP_OBJECT (window1, vs_cdepth, "vs_cdepth");
  GLADE_HOOKUP_OBJECT (window1, label14, "label14");
  GLADE_HOOKUP_OBJECT (window1, vbox9, "vbox9");
  GLADE_HOOKUP_OBJECT (window1, vs_vspeed, "vs_vspeed");
  GLADE_HOOKUP_OBJECT (window1, label15, "label15");
  GLADE_HOOKUP_OBJECT (window1, vbox10, "vbox10");
  GLADE_HOOKUP_OBJECT (window1, vs_vdepth, "vs_vdepth");
  GLADE_HOOKUP_OBJECT (window1, label16, "label16");
  GLADE_HOOKUP_OBJECT (window1, label_ensemble, "label_ensemble");
  GLADE_HOOKUP_OBJECT (window1, frame4, "frame4");
  GLADE_HOOKUP_OBJECT (window1, alignment4, "alignment4");
  GLADE_HOOKUP_OBJECT (window1, hbox4, "hbox4");
  GLADE_HOOKUP_OBJECT (window1, vbox11, "vbox11");
  GLADE_HOOKUP_OBJECT (window1, vs_tone, "vs_tone");
  GLADE_HOOKUP_OBJECT (window1, label18, "label18");
  GLADE_HOOKUP_OBJECT (window1, vbox12, "vbox12");
  GLADE_HOOKUP_OBJECT (window1, vs_volume, "vs_volume");
  GLADE_HOOKUP_OBJECT (window1, label19, "label19");
  GLADE_HOOKUP_OBJECT (window1, label17, "label17");
  GLADE_HOOKUP_OBJECT (window1, hbox5, "hbox5");
  GLADE_HOOKUP_OBJECT (window1, image1, "image1");
  GLADE_HOOKUP_OBJECT (window1, frame5, "frame5");
  GLADE_HOOKUP_OBJECT (window1, alignment5, "alignment5");
  GLADE_HOOKUP_OBJECT (window1, hbox6, "hbox6");
  GLADE_HOOKUP_OBJECT (window1, vbox17, "vbox17");
  GLADE_HOOKUP_OBJECT (window1, vscale19, "vscale19");
  GLADE_HOOKUP_OBJECT (window1, label25, "label25");
  GLADE_HOOKUP_OBJECT (window1, vbox18, "vbox18");
  GLADE_HOOKUP_OBJECT (window1, vscale20, "vscale20");
  GLADE_HOOKUP_OBJECT (window1, label26, "label26");
  GLADE_HOOKUP_OBJECT (window1, vbox19, "vbox19");
  GLADE_HOOKUP_OBJECT (window1, vscale21, "vscale21");
  GLADE_HOOKUP_OBJECT (window1, label27, "label27");
  GLADE_HOOKUP_OBJECT (window1, label23, "label23");
  GLADE_HOOKUP_OBJECT (window1, frame6, "frame6");
  GLADE_HOOKUP_OBJECT (window1, alignment6, "alignment6");
  GLADE_HOOKUP_OBJECT (window1, vbox16, "vbox16");
  GLADE_HOOKUP_OBJECT (window1, combobox1, "combobox1");
  GLADE_HOOKUP_OBJECT (window1, btn_savepatch, "btn_savepatch");
  GLADE_HOOKUP_OBJECT (window1, label24, "label24");
  GLADE_HOOKUP_OBJECT (window1, label_main, "label_main");
  GLADE_HOOKUP_OBJECT (window1, image2, "image2");
  GLADE_HOOKUP_OBJECT (window1, label2, "label2");
  
  
	gtk_signal_connect (GTK_OBJECT (voice_widget[NEKO_PORT_BASS_LEVEL]),
                        "value_changed", GTK_SIGNAL_FUNC(on_voice_slider_change),
                        (gpointer)NEKO_PORT_BASS_LEVEL);
	gtk_signal_connect (GTK_OBJECT (vs_cello),
                        "value_changed", GTK_SIGNAL_FUNC(on_voice_slider_change),
                        (gpointer)NEKO_PORT_CELLO_LEVEL);
	gtk_signal_connect (GTK_OBJECT (vs_viola),
                        "value_changed", GTK_SIGNAL_FUNC(on_voice_slider_change),
                        (gpointer)NEKO_PORT_VIOLA_LEVEL);
	gtk_signal_connect (GTK_OBJECT (vs_violin),
                        "value_changed", GTK_SIGNAL_FUNC(on_voice_slider_change),
                        (gpointer)NEKO_PORT_VIOLIN_LEVEL);
	gtk_signal_connect (GTK_OBJECT (vs_cspeed),
                        "value_changed", GTK_SIGNAL_FUNC(on_voice_slider_change),
                        (gpointer)NEKO_PORT_SPEED);
    gtk_signal_connect (GTK_OBJECT (vs_cdepth),
                        "value_changed", GTK_SIGNAL_FUNC(on_voice_slider_change),
                        (gpointer)NEKO_PORT_DEPTH);
	gtk_signal_connect (GTK_OBJECT (vs_vspeed),
                        "value_changed", GTK_SIGNAL_FUNC(on_voice_slider_change),
                        (gpointer)NEKO_PORT_VIBRATO_CHORUS);
	gtk_signal_connect (GTK_OBJECT (vs_tone),
                        "value_changed", GTK_SIGNAL_FUNC(on_voice_slider_change),
                        (gpointer)NEKO_PORT_TONE);
	gtk_signal_connect (GTK_OBJECT (vs_volume),
                        "value_changed", GTK_SIGNAL_FUNC(on_voice_slider_change),
                        (gpointer)NEKO_PORT_VOLUME);

  return window1;
}
