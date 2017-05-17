#include <unistd.h>
#include <glib.h>
#include <errno.h>
#include <gps.h>

#include "nxjson.h"
#include "gpsd_source.h"

#define GPSD_TIMEOUT 100000
#define BUF_SIZE 4096

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
  gboolean ret = FALSE;
  static gchar buf[BUF_SIZE];
  GpsdSource *src = (GpsdSource *)gsource;
  gint nbytes = read(src->gpsdata.gps_fd, buf, BUF_SIZE);

  if (0 > nbytes)
  {
    fprintf(stderr, "error while reading from gpsd: %d, %s\n", errno, gps_errstr(errno));
  }
  else if (NULL != callback)
  {
    gchar **lines = g_strsplit(g_strstrip(buf), "\n", -1);
    gchar **line;
    printf("Got [%s]\n", buf);

    /* Decode all received lines */
    for (line = lines; *line; line++) {
      const nx_json *json = nx_json_parse(*line, 0);
      if (json) {
        const gchar *class = nx_json_get(json, "class")->text_value;
        printf("\tDecoded JSON, class: %s\n", class);
        nx_json_free(json);
      }
    }

    g_strfreev(lines);
    /* ret = callback(&(src->gpsdata)); */
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
  gps_stream(&(src->gpsdata), WATCH_ENABLE | WATCH_JSON, NULL);

  return (GSource *)src;
}

void gpsd_source_destroy(GSource *gsource)
{
  GpsdSource *src = (GpsdSource *)gsource;
  gps_close(&(src->gpsdata));
}
