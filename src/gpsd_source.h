#ifndef __GPSD_SOURCE_H
#define __GPSD_SOURCE_H

#include <glib.h>
#include <gps.h>

typedef gboolean (*GpsdSourceFunc)(struct gps_data_t *gpsdata);

GSource *gpsd_source_new(const gchar *host, const gchar *port);

#endif /* __GPSD_SOURCE_H */
