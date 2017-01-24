#include <glib.h>
#include <errno.h>
#include <gps.h>

#include "gpsd_source.h"

#define GPSD_TIMEOUT 100000

typedef struct {
  GSource g_source;
  struct gps_data_t gpsdata;
} GpsdSource;

static gboolean prepare(GSource *gsource, gint *timeout)
{
  GpsdSource *src = (GpsdSource *)gsource;
  gboolean ret = gps_waiting(&(src->gpsdata), GPSD_TIMEOUT);
  if (!ret)
  {
    printf("No data waiting from gpsd, waiting...\n");
    *timeout = 100;
  }
  else
  {
    *timeout = -1;
  }
  return ret;
}

static gboolean check(GSource *gsource)
{
  GpsdSource *src = (GpsdSource *)gsource;
  return gps_waiting(&(src->gpsdata), GPSD_TIMEOUT);
}

static gboolean dispatch(GSource *gsource, GSourceFunc callback, gpointer user_data)
{
  GpsdSource *src = (GpsdSource *)gsource;
  gboolean ret;
  gint nbytes = gps_read(&(src->gpsdata));
  if (0 > nbytes)
  {
    fprintf(stderr, "error while reading from gpsd: %d, %s\n", errno, gps_errstr(errno));
    ret = FALSE;
  }
  else if (0 == nbytes)
  {
    fprintf(stderr, "No data this time\n");
    ret = FALSE;
  }
  else if (NULL != callback)
  {
    ret = callback(&(src->gpsdata));
  }
  return ret;
}

static GSourceFuncs fcns = { prepare, check, dispatch, NULL };

GSource *gpsd_source_new(const gchar *host, const gchar *port)
{
  GpsdSource *src = (GpsdSource *)g_source_new(&fcns, sizeof(GpsdSource));

  if (0 != gps_open(host, port, &(src->gpsdata)) != 0)
  {
    fprintf(stderr, "no gpsd running or network error: %d, %s\n", errno, gps_errstr(errno));
    return NULL;
  }
  gps_stream(&(src->gpsdata), WATCH_ENABLE, NULL);

  return (GSource *)src;
}

void gpsd_source_destroy(GSource *gsource)
{
  GpsdSource *src = (GpsdSource *)gsource;
  gps_close(&(src->gpsdata));
}
