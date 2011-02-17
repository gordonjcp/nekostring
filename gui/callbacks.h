/* nekostring - callbacks.h */

/* nekostring polyphonic string ensemble
 *
 * (C) 2006 Gordon JC Pearce and others
 *
 * For further information on contributors and the history of nekostring,
 * please see the README
 *
*/

#include <gtk/gtk.h>
#include <lo/lo.h>
void on_voice_slider_change( GtkWidget *widget, gpointer data );
void update_voice_widget(int port, float value);
extern lo_address osc_host_address;
extern char *     osc_control_path;
