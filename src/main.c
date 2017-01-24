#include <glib.h>
#include <gps.h>

#include "gpsd_source.h"

static gboolean cb(gpointer user_data)
{
  struct gps_data_t *gps_data = (struct gps_data_t *)user_data;

  if (0 == gps_data->online)
  {
    printf("OFFLINE\n");
  }
  else
  {
    switch (gps_data->fix.mode)
    {
      case MODE_2D:
        printf("2D FIX\n");
        break;
      case MODE_3D:
        printf("3D FIX\n");
        break;
      default:
        printf("NO FIX\n");
        break;
    }
  }
}

int main(int argc, char** argv)
{
  GMainLoop *loop = NULL;
  GSource *gpsd_src = NULL;

  gchar *host = NULL;
  gchar *port = NULL;

  /* Set up main loop */
  gpsd_src = gpsd_source_new(host, port);
  g_source_attach(gpsd_src, NULL);
  loop = g_main_loop_new (NULL, FALSE);
  g_source_set_callback(gpsd_src, cb, NULL, NULL);

  /* Create and run main loop */
  g_main_loop_run(loop);

  gpsd_source_destroy(gpsd_src);
  g_main_loop_unref(loop);
}
