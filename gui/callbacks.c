/* nekostring - callbacks.c */

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

#include <gtk/gtk.h>
#include <math.h>
#include <lo/lo.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "widgets.h"
#include "../src/neko_types.h"
#include "../src/neko_ports.h"





void update_voice_widget(int port, float value)
{
    struct neko_port_descriptor *xpd;
    GtkAdjustment *adj;
    float cval, a, b, c, d;


//i    if (port < XSYNTH_PORT_WAVEFORM || port >= XSYNTH_PORTS_COUNT) {
//        return;
//    }

    xpd = &neko_port_description[port];

        if (value < xpd->lower_bound)
        value = xpd->lower_bound;
    else if (value > xpd->upper_bound)
        value = xpd->upper_bound;

    //internal_gui_update_only = 1;

//    switch (xpd->type) {

//      case XSYNTH_PORT_TYPE_LINEAR:
                  a=xpd->a; b=xpd->b; c=xpd->c;
                  // used in "on_voice_slider_change" value = (xpd->a * cval + xpd->b) * cval + xpd->c;
          d = sqrt(b*b - 4*a*(c-value));
                cval=(-b+d)/(2*a);
        //cval = (value - xpd->c) / xpd->b;  // assume xpd->a == 0, which was always true for nekobee
        printf(" update_voice_widget: change of %s to %f => %f\n", xpd->name, value, cval);

        adj = (GtkAdjustment *)voice_widget[port];
        adj->value = cval * 10.0f;
        // gtk_signal_emit_by_name (GTK_OBJECT (adj), "changed");        does not cause call to on_voice_slider_change callback
        gtk_signal_emit_by_name (GTK_OBJECT (adj), "value_changed");  // causes call to on_voice_slider_change callback 
//        break;
/*
      case XSYNTH_PORT_TYPE_LOGARITHMIC:
                cval = log(value / xpd->a) / (xpd->c * log(xpd->b));
                if (port == XSYNTH_PORT_TUNING) {
            if (cval < -1.2f)   cval = -1.2f;
                if (cval > 1.2f) cval = 1.2f;
                }
                else {
        if (cval > 1.0f - 1.0e-6f)
            cval = 1.0f;
        }
         printf(" update_voice_widget: change of %s to %f => %f\n", xpd->name, value, cval);
        adj = (GtkAdjustment *)voice_widget[port];
        adj->value = cval * 10.0f;
        gtk_signal_emit_by_name (GTK_OBJECT (adj), "value_changed");  // causes call to on_voice_slider_change callback 
        break;
      default:
                        printf(" update_voice_widget: don't know what this is...\n");
        break;


      }
  //  internal_gui_update_only = 0;
  
  */
}




void on_voice_slider_change( GtkWidget *widget, gpointer data )
{
//   GtkAdjustment *adj;


	int index = (int)data;
   // struct xsynth_port_descriptor *xpd = &xsynth_port_description[index];
    //float cval = GTK_ADJUSTMENT(widget)->value; // / 10.0f;
	float cval = gtk_range_get_adjustment(GTK_RANGE(widget))->value; // / 10.0f;
//    float value;

	//printf("from slider: %d -> %f\n",index, cval);
	
//	adj= gtk_range_get_adjustment(GTK_RANGE(vs_viola));
//	adj->value=cval;
//	gtk_signal_emit_by_name (GTK_OBJECT (adj), "value_changed");
	
	
  //  if (internal_gui_update_only) {
  //      /* GDB_MESSAGE(GDB_GUI, " on_voice_slider_change: skipping further action\n"); */
  //      return;
  //  }

 //   if (xpd->type == XSYNTH_PORT_TYPE_LINEAR) {

   //     value = (xpd->a * cval + xpd->b) * cval + xpd->c;  /* linear or quadratic */

 //   } else { /* XSYNTH_PORT_TYPE_LOGARITHMIC */

    //    value = xpd->a * exp(xpd->c * cval * log(xpd->b));

 //   }

 //   GDB_MESSAGE(GDB_GUI, " on_voice_slider_change: slider %d changed to %10.6f => %10.6f\n",
  //          index, GTK_ADJUSTMENT(widget)->value, value);

    lo_send(osc_host_address, osc_control_path, "if", index, cval);
}
