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

#undef GDA_DISABLE_DEPRECATED

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n-lib.h>
#include <gio/gio.h>
#include <virtual/gda-oslc-connection.h>
#include <libgda/gda-util.h>
#include <libgda/gda-server-provider-impl.h>
#include <libgda/gda-server-provider-extra.h>
#include <libgda/gda-data-model-iter.h>
#include <libgda/gda-data-model-array.h>
#include <libgda/gda-statement-extra.h>
#include <libgda/sql-parser/gda-sql-parser.h>
#include "gda-oslc.h"
#include "gda-oslc-provider.h"

static void gda_oslc_provider_class_init (GdaOslcProviderClass *klass);
static void gda_oslc_provider_init (GdaOslcProvider *provider, GdaOslcProviderClass *klass);
static void gda_oslc_provider_finalize(GObject *object);

static const gchar *gda_oslc_provider_get_name (GdaServerProvider *provider);
static const gchar *gda_oslc_provider_get_version(GdaServerProvider *provider);
static const gchar *gda_oslc_provider_get_server_version(GdaServerProvider *provider,
                                                         GdaConnection *cnc);
static GdaConnection *gda_oslc_provider_create_connection(GdaServerProvider *provider);
static gboolean gda_oslc_provider_prepare_connection(GdaServerProvider *provider,
                                                     GdaConnection *cnc,
                                                     GdaQuarkList *params,
                                                     GdaQuarkList *auth);
static gboolean gda_oslc_provider_close_connection(GdaServerProvider *provider,
                                                   GdaConnection *cnc);
static GObject *gda_oslc_provider_statement_execute(GdaServerProvider *provider,
                                                    GdaConnection *cnc,
                                                    GdaStatement *stmt,
                                                    GdaSet *params,
                                                    GdaStatementModelUsage model_usage,
                                                    GType *col_types,
                                                    GdaSet **last_inserted_row,
                                                    GError **error);

static GObjectClass *parent_class = NULL;

GdaServerProviderBase oslc_base_functions = {
	gda_oslc_provider_get_name,
	gda_oslc_provider_get_version,
	gda_oslc_provider_get_server_version,
	NULL,
	NULL,
	gda_oslc_provider_create_connection,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	gda_oslc_provider_prepare_connection,
	gda_oslc_provider_close_connection,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	gda_oslc_provider_statement_execute,

	NULL, NULL, NULL, NULL, /* padding */
};

static void
gda_oslc_provider_class_init (GdaOslcProviderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GdaServerProviderClass *provider_class = GDA_SERVER_PROVIDER_CLASS(klass);

	parent_class = g_type_class_peek_parent(klass);

	gda_server_provider_set_impl_functions(provider_class,
                                         GDA_SERVER_PROVIDER_FUNCTIONS_BASE,
                                         (gpointer)&oslc_base_functions);

	gda_server_provider_set_impl_functions(provider_class,
                                         GDA_SERVER_PROVIDER_FUNCTIONS_META,
                                         (gpointer)NULL);

	gda_server_provider_set_impl_functions(provider_class,
                                         GDA_SERVER_PROVIDER_FUNCTIONS_XA,
                                         (gpointer)NULL);

  object_class->finalize = gda_oslc_provider_finalize;
}

static void
gda_oslc_provider_init (G_GNUC_UNUSED GdaOslcProvider *oslc_prev,
                        G_GNUC_UNUSED GdaOslcProviderClass *klass)
{
  /* nothing to see here */
}

static void
gda_oslc_provider_finalize(GObject *object)
{
  GdaOslcProvider *provider = (GdaOslcProvider *) object;

  g_return_if_fail(GDA_IS_OSLC_PROVIDER(provider));

  parent_class->finalize(object);
}

GType
gda_oslc_provider_get_type(void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0)) {
		static GMutex registering;
		static GTypeInfo info = {
			sizeof(GdaOslcProviderClass),
			(GBaseInitFunc)NULL,
			(GBaseFinalizeFunc)NULL,
			(GClassInitFunc)gda_oslc_provider_class_init,
			NULL, NULL,
			sizeof(GdaOslcProvider),
			0,
			(GInstanceInitFunc)gda_oslc_provider_init,
			0
		};
		g_mutex_lock(&registering);
		if (type == 0)
			type = g_type_register_static(GDA_TYPE_VPROVIDER_DATA_MODEL,
                                    "GdaOslcProvider", &info, 0);
		g_mutex_unlock(&registering);
	}

	return type;
}

/*
 * Get provider name request
 */
static const gchar *
gda_oslc_provider_get_name (G_GNUC_UNUSED GdaServerProvider *provider)
{
	return OSLC_PROVIDER_NAME;
}

/*
 * Get provider's version, no need to change this
 */
static const gchar *
gda_oslc_provider_get_version (G_GNUC_UNUSED GdaServerProvider *provider)
{
	return PACKAGE_VERSION;
}

static const gchar *
gda_oslc_provider_get_server_version(GdaServerProvider *provider,
                                     GdaConnection *cnc)
{
  return "0.0";
}

static GdaConnection *
gda_oslc_provider_create_connection(GdaServerProvider *provider)
{
  GdaConnection *cnc;

  g_return_val_if_fail(GDA_IS_OSLC_PROVIDER(provider), NULL);

  cnc = g_object_new(GDA_TYPE_OSLC_CONNECTION, "provider", provider, NULL);

  return cnc;
}

static void
gda_oslc_free_cnc_data(OslcConnectionData *cdata)
{
  g_free(cdata);
}

static gboolean
gda_oslc_provider_prepare_connection(GdaServerProvider *provider,
                                     GdaConnection *cnc,
                                     GdaQuarkList *params,
                                     GdaQuarkList *auth)
{
  GFile *f;
  GInputStream *is;
  gssize res;
  gchar buf[4096];
  OslcConnectionData *cdata;
  GdaServerProviderBase *parent_functions;
  const gchar *uri;
  GError *err;

  g_return_val_if_fail(GDA_IS_OSLC_PROVIDER(provider), FALSE);
  g_return_val_if_fail(GDA_IS_CONNECTION(cnc), FALSE);

  err = NULL;

  parent_functions = gda_server_provider_get_impl_functions_for_class(parent_class, GDA_SERVER_PROVIDER_FUNCTIONS_BASE);

  if (parent_functions->prepare_connection) {
    if (!parent_functions->prepare_connection(provider, cnc, params, auth))
      return FALSE;
  }

  cdata = g_malloc(sizeof(OslcConnectionData));
  cdata->root = g_string_new(NULL);

  uri = gda_quark_list_find(params, "URI");
  if (!uri) {
    gda_connection_add_event_string(cnc, "%s", _("The connection string must contain a URI value"));
    return FALSE;
  }

  g_message("%s", uri);

  f = g_file_new_for_uri(uri);
  is = G_INPUT_STREAM(g_file_read(f, NULL, &err));

  if (!is)
    g_error("input stream broken");

  while ((res = g_input_stream_read(is, buf, 4096, NULL, &err)) > 0) {
    g_string_append_len(cdata->root, buf, res);
    g_message("%li bytes read", res);
  }

  if (!res)
    g_message("read operation complete");
  else if (res == -1)
    g_error("read operation broken");

  g_object_unref(is);
  g_object_unref(f);

  gda_virtual_connection_internal_set_provider_data(GDA_VIRTUAL_CONNECTION(cnc),
                                                    cdata,
                                                    (GDestroyNotify)gda_oslc_free_cnc_data);

  return TRUE;
}

static gboolean
gda_oslc_provider_close_connection(GdaServerProvider *provider,
                                   GdaConnection *cnc)
{
  g_message("OSLC: close_connection() stub");
  return FALSE;
}

static GObject *
gda_oslc_provider_statement_execute(GdaServerProvider *provider,
                                    GdaConnection *cnc,
                                    GdaStatement *stmt,
                                    GdaSet *params,
                                    GdaStatementModelUsage model_usage,
                                    GType *col_types,
                                    GdaSet **last_inserted_row,
                                    GError **error)
{
  g_message("OSLC: statement_execute() stub");
  return NULL;
}
