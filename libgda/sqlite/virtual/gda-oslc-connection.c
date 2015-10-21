/*
 * Copyright (C) 2015 Chris Lamberson <clamberson@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include <glib/gi18n-lib.h>
#include <string.h>
#include <gda-util.h>
#include "gda-oslc-connection.h"
#include <libgda/gda-connection-private.h>
#include <sql-parser/gda-sql-parser.h>
#include <libgda/gda-debug-macros.h>

static void gda_oslc_connection_class_init(GdaOslcConnectionClass *klass);
static void gda_oslc_connection_init(GdaOslcConnection *cnc, GdaOslcConnectionClass *klass);

static GObjectClass *parent_class = NULL;

GType
gda_oslc_connection_get_type(void)
{
  static GType type = 0;

  if (G_UNLIKELY(type == 0)) {
    static GMutex registering;
    if (type == 0) {
      static GTypeInfo info = {
        sizeof(GdaOslcConnectionClass),
        (GBaseInitFunc)NULL,
        (GBaseFinalizeFunc)NULL,
        (GClassInitFunc)gda_oslc_connection_class_init,
        NULL, NULL,
        sizeof(GdaOslcConnection),
        0,
        (GInstanceInitFunc)gda_oslc_connection_init,
        0
      };

      g_mutex_lock(&registering);
      if (type == 0)
        type = g_type_register_static(GDA_TYPE_VCONNECTION_DATA_MODEL, "GdaOslcConnection", &info, 0);
      g_mutex_unlock(&registering);
    }
  }

  return type;
}

static void
gda_oslc_connection_class_init(GdaOslcConnectionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  parent_class = g_type_class_peek_parent(klass);
}

static void
conn_opened_cb(GdaOslcConnection *cnc, G_GNUC_UNUSED gpointer data)
{
  g_message("connection opened");
}

static void
gda_oslc_connection_init(GdaOslcConnection *cnc, G_GNUC_UNUSED GdaOslcConnectionClass *klass)
{
  g_signal_connect(cnc, "opened", G_CALLBACK(conn_opened_cb), NULL);
}
