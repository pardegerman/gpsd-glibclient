#ifndef __GPSD_SOURCE_H
#define __GPSD_SOURCE_H

#include <glib.h>

typedef gboolean (*GpsdSourceFunc)(gpointer data);

GSource *gpsd_source_new(const gchar *host, const gchar *port);
void gpsd_source_destroy(GSource *gsource);

#endif /* __GPSD_SOURCE_H */
