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
#include <libgda/gda-util.h>
#include <libgda/gda-server-provider-impl.h>
#include <libgda/gda-server-provider-extra.h>
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
static gboolean gda_oslc_provider_supports_feature(GdaServerProvider *provider,
                                                   GdaConnection *cnc,
                                                   GdaConnectionFeature feature);
static GdaWorker *gda_oslc_provider_create_worker(GdaServerProvider *provider,
                                                  gboolean for_cnc);
static GdaConnection *gda_oslc_provider_create_connection(GdaServerProvider *provider);
static gchar *gda_oslc_provider_statement_to_sql(GdaServerProvider *provider,
                                                 GdaConnection *cnc,
                                                 GdaStatement *stmt,
                                                 GdaSet *params,
                                                 GdaStatementSqlFlag flags,
                                                 GSList **params_used,
                                                 GError **error);
static GdaSqlStatement *gda_oslc_provider_statement_rewrite(GdaServerProvider *provider,
                                                            GdaConnection *cnc,
                                                            GdaStatement *stmt,
                                                            GdaSet *params,
                                                            GError **error);
static gboolean gda_oslc_provider_open_connection(GdaServerProvider *provider,
                                                  GdaConnection *cnc,
                                                  GdaQuarkList *params,
                                                  GdaQuarkList *auth);
static gboolean gda_oslc_provider_prepare_connection(GdaServerProvider *provider,
                                                     GdaConnection *cnc,
                                                     GdaQuarkList *params,
                                                     GdaQuarkList *auth);
static gboolean gda_oslc_provider_close_connection(GdaServerProvider *provider,
                                                   GdaConnection *cnc);
static gboolean gda_oslc_provider_statement_prepare(GdaServerProvider *provider,
                                                    GdaConnection *cnc,
                                                    GdaStatement *stmt,
                                                    GError **error);
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
	gda_oslc_provider_supports_feature,
	gda_oslc_provider_create_worker,
	gda_oslc_provider_create_connection,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	gda_oslc_provider_statement_to_sql,
	NULL,
	gda_oslc_provider_statement_rewrite,
	gda_oslc_provider_open_connection,
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
	gda_oslc_provider_statement_prepare,
	gda_oslc_provider_statement_execute,

	NULL, NULL, NULL, NULL, /* padding */
};

static void
gda_oslc_provider_class_init (GdaOslcProviderClass *klass)
{
	GdaServerProviderClass *provider_class = GDA_SERVER_PROVIDER_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	/* set virtual functions */
	gda_server_provider_set_impl_functions (provider_class,
                                          GDA_SERVER_PROVIDER_FUNCTIONS_BASE,
                                          (gpointer) &oslc_base_functions);

	gda_server_provider_set_impl_functions (provider_class,
                                          GDA_SERVER_PROVIDER_FUNCTIONS_META,
                                          (gpointer) NULL);

	gda_server_provider_set_impl_functions (provider_class,
                                          GDA_SERVER_PROVIDER_FUNCTIONS_XA,
                                          (gpointer) NULL);

  G_OBJECT_CLASS(klass)->finalize = gda_oslc_provider_finalize;
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
gda_oslc_provider_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY (type == 0)) {
		static GMutex registering;
		static GTypeInfo info = {
			sizeof (GdaOslcProviderClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gda_oslc_provider_class_init,
			NULL, NULL,
			sizeof (GdaOslcProvider),
			0,
			(GInstanceInitFunc) gda_oslc_provider_init,
			0
		};
		g_mutex_lock (&registering);
		if (type == 0)
			type = g_type_register_static (GDA_TYPE_SERVER_PROVIDER, "GdaOslcProvider", &info, 0);
		g_mutex_unlock (&registering);
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

static const gchar *gda_oslc_provider_get_server_version(GdaServerProvider *provider,
                                                         GdaConnection *cnc)
{
  g_warning("gda_oslc_provider_get_server_version not implemented");
  return "0.0";
}

static gboolean gda_oslc_provider_supports_feature(GdaServerProvider *provider,
                                                   GdaConnection *cnc,
                                                   GdaConnectionFeature feature)
{
  g_warning("gda_oslc_provider_supports_feature not implemented");
  return FALSE;
}

static GdaWorker *gda_oslc_provider_create_worker(GdaServerProvider *provider,
                                                  gboolean for_cnc)
{
  static GdaWorker *unique_worker = NULL;

  if (for_cnc)
    return gda_worker_new();
  return gda_worker_new_unique(&unique_worker, TRUE);
}

static GdaConnection *gda_oslc_provider_create_connection(GdaServerProvider *provider)
{
  g_warning("gda_oslc_provider_create_connection not implemented");
  return NULL;
}

static gchar *gda_oslc_provider_statement_to_sql(GdaServerProvider *provider,
                                                 GdaConnection *cnc,
                                                 GdaStatement *stmt,
                                                 GdaSet *params,
                                                 GdaStatementSqlFlag flags,
                                                 GSList **params_used,
                                                 GError **error)
{
  g_warning("gda_oslc_provider_statement_to_sql not implemented");
  return NULL;
}

static GdaSqlStatement *gda_oslc_provider_statement_rewrite(GdaServerProvider *provider,
                                                            GdaConnection *cnc,
                                                            GdaStatement *stmt,
                                                            GdaSet *params,
                                                            GError **error)
{
  g_warning("gda_oslc_provider_statement_rewrite not implemented");
  return NULL;
}

static gboolean gda_oslc_provider_open_connection(GdaServerProvider *provider,
                                                  GdaConnection *cnc,
                                                  GdaQuarkList *params,
                                                  GdaQuarkList *auth)
{
  GFile *f;
  GInputStream *is;
  GError *err;
  gssize res;
  gchar buf[4096];
  OslcConnectionData *ocd;

  err = NULL;

  ocd = g_malloc(sizeof(OslcConnectionData));
  ocd->root = g_string_new(NULL);

  f = g_file_new_for_uri("https://rtc.nginfot.net:9443/ccm/rootservices");
  is = G_INPUT_STREAM(g_file_read(f, NULL, &err));

  if (!is)
    g_error("input stream broken");

  while ((res = g_input_stream_read(is, buf, 4096, NULL, &err)) > 0) {
    g_string_append_len(ocd->root, buf, res);
    g_message("%li bytes read", res);
  }

  if (!res)
    g_message("read operation complete");
  else if (res == -1)
    g_error("read operation broken");

  g_object_unref(is);
  g_object_unref(f);

  gda_connection_internal_set_provider_data(cnc, (GdaServerProviderConnectionData *)ocd, NULL);

  return FALSE;
}

static gboolean gda_oslc_provider_prepare_connection(GdaServerProvider *provider,
                                                     GdaConnection *cnc,
                                                     GdaQuarkList *params,
                                                     GdaQuarkList *auth)
{
  g_warning("gda_oslc_provider_prepare_connection not implemented");
  return FALSE;
}

static gboolean gda_oslc_provider_close_connection(GdaServerProvider *provider,
                                                   GdaConnection *cnc)
{
  g_warning("gda_oslc_provider_close_connection not implemented");
  return FALSE;
}

static gboolean gda_oslc_provider_statement_prepare(GdaServerProvider *provider,
                                                    GdaConnection *cnc,
                                                    GdaStatement *stmt,
                                                    GError **error)
{
  g_warning("gda_oslc_provider_statement_prepare not implemented");
  return FALSE;
}

static GObject *gda_oslc_provider_statement_execute(GdaServerProvider *provider,
                                                    GdaConnection *cnc,
                                                    GdaStatement *stmt,
                                                    GdaSet *params,
                                                    GdaStatementModelUsage model_usage,
                                                    GType *col_types,
                                                    GdaSet **last_inserted_row,
                                                    GError **error)
{
  g_warning("gda_oslc_provider_statement_execute not implemented");
  return NULL;
}
