#include <glib.h>
#include <gio/gio.h>

#include <errno.h>
#include <gps.h>

#include "nxjson.h"
#include "gpsd_source.h"

#define BUF_SIZE 4096

typedef struct {
  GSource g_source;
  struct gps_data_t gpsdata;
} GpsdSource;

static gboolean on_gpsd_data(GSocket *socket, GIOCondition condition, gpointer user_data)
{
  struct gps_data_t *gpsdata = (struct gps_data_t *)user_data;
  static gint pos = 0;
  static gchar buf[BUF_SIZE];

  if (G_IO_IN & condition)
  {
    while (BUF_SIZE > pos && 1 == g_socket_receive(socket, buf + pos, 1, NULL, NULL))
    {
      if ('\r' == buf[pos])
      {
        /* Skip \r */
      }
      else if ('\n' == buf[pos])
      {
        /* A full line, let's parse it */
        buf[pos] = '\0';
        printf("-- Read a line: [%s]\n", buf);
        pos = 0;
      }
      else
      {
        /* Continue */
        pos++;
      }
    }
  }

  /* TODO: Return FALSE on disconnect et. al. */
  return TRUE;
}

static gboolean dispatch(GSource *source, GSourceFunc callback, gpointer user_data)
{
  printf("--> Dispatch\n");
  GpsdSource *gpsdsource = (GpsdSource *)source;
  struct gps_data_t *gpsdata = &(gpsdsource->gpsdata);
  GpsdSourceFunc gpsd_callback = (GpsdSourceFunc)callback;

  if (NULL != gpsdata && NULL != gpsd_callback)
  {
    gpsd_callback(gpsdata, user_data);
    gpsdata->set = 0;
  }
  printf("<-- Dispatch\n");
  return TRUE;
}

static void finalize(GSource *source)
{
  GpsdSource *gpsdsource = (GpsdSource *)source;
  gps_close(&(gpsdsource->gpsdata));
}

static GSourceFuncs fcns = { NULL, NULL, dispatch, finalize };

GSource *gpsd_source_new(const gchar *host, const gchar *port)
{
  GSource *source = g_source_new(&fcns, sizeof(GpsdSource));
  struct gps_data_t *gpsdata = &(((GpsdSource *)source)->gpsdata);
  GSocket *socket;
  GSource *socketsource;

  if (0 != gps_open(host, port, gpsdata) != 0)
  {
    fprintf(stderr, "no gpsd running or network error: %d, %s\n", errno, gps_errstr(errno));
    return NULL;
  }

  socket = g_socket_new_from_fd(gpsdata->gps_fd, NULL);
  g_socket_set_blocking(socket, false);

  socketsource = g_socket_create_source(socket, G_IO_IN, NULL);
  g_source_set_callback(socketsource, (GSourceFunc)on_gpsd_data, gpsdata, NULL);
  g_source_add_child_source(source, (GSource *)socketsource);

  gps_stream(gpsdata, WATCH_ENABLE | WATCH_JSON, NULL);
  
  return source;
}
