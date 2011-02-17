/* nekostring - main.c */

/* nekostring polyphonic string ensemble
 *
 * (C) 2006 Gordon JC Pearce and others
 *
 * For further information on contributors and the history of nekostring,
 * please see the README
 * this file contains elements from Sean Bolton's Xsynth-DSSI
 *
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <string.h>


#include <gtk/gtk.h>
#include <lo/lo.h>

#include "interface.h"
#include "support.h"
#include "widgets.h"
#include "callbacks.h"

// define a few globals
lo_address osc_host_address;
char *     osc_host_url;
char *     osc_quit_path;
char *     osc_control_path;

static void osc_error(int num, const char *msg, const char *path) {
    printf(" error: liblo server error %d in path \"%s\": %s\n", num, (path ? path : "(null)"), msg);
}
static char *osc_build_path(char *base_path, char *method) {
    char buffer[256];
    char *full_path;

    snprintf(buffer, 256, "%s%s", base_path, method);
    if (!(full_path = strdup(buffer))) {
        printf(": out of memory!\n");
        exit(1);
    }
    return strdup(buffer);
}

void osc_action_handler(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data) {

//	printf("%s\n",user_data);

}

int osc_control_handler(const char *path, const char *types, lo_arg **argv, int argc, lo_message msg, void *user_data) {

    int port;
    float value;

    if (argc < 2) {
        printf("error: too few arguments to osc_control_handler\n");
        return 1;
    }

    port = argv[0]->i;
    value = argv[1]->f;
    printf(" osc_control_handler: control %d now %f\n", port, value);

    update_voice_widget(port, value);

    return 0;



}

// Here's where the fun starts

int main (int argc, char *argv[])
{
  // first we'll define the necessary OSC things
  char *host, *port, *path, *tmp_url; // FIXME - tmp_url?
  lo_server osc_server;
  
  GtkWidget *window1;
//	GtkAdjustment *adj;

  gtk_set_locale ();
  gtk_init (&argc, &argv);

   if (argc != 5) {
        fprintf(stderr, "usage: %s <osc url> <plugin dllname> <plugin label> <user-friendly id>\n", argv[0]);
        exit(1);
    }

	// sort out the OSC addresses
    osc_host_url = argv[1];
    host = lo_url_get_hostname(osc_host_url);
    port = lo_url_get_port(osc_host_url);
    path = lo_url_get_path(osc_host_url);
    osc_host_address = lo_address_new(host, port);
    // now we define the handlers
    osc_quit_path      = osc_build_path(path, "/quit");
    osc_control_path   = osc_build_path(path, "/control");
    
    // start the OSC server
    osc_server = lo_server_new(NULL, osc_error);
    
    // add the callbacks
    lo_server_add_method(osc_server, osc_quit_path, "", osc_action_handler, "quit");
    lo_server_add_method(osc_server, osc_control_path, "if", osc_control_handler, NULL);


  int i;


  add_pixmap_directory (PACKAGE_DATA_DIR "/" PACKAGE "/pixmaps");

  /*
   * The following code was added by Glade to create one of each component
   * (except popup menus), just so that you see something after building
   * the project. Delete any components that you don't want shown initially.
   */
  window1 = create_window1 ();
  gtk_widget_show (window1);




  printf("%d\n",argc);
  for (i=0; i<argc; i++) {
	  printf("%s\n",argv[i]);
  }

  gtk_main ();
  
  
    lo_server_free(osc_server);
    free(host);
    free(port);
    free(path);
  free(osc_quit_path);
  return 0;
}
