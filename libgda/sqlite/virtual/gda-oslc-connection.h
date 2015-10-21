/*
 * Copyright (C) 2011 - 2014 Vivien Malerba <malerba@gnome-db.org>
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

#ifndef __GDA_OSLC_CONNECTION_H__
#define __GDA_OSLC_CONNECTION_H__

#include <virtual/gda-vconnection-data-model.h>
#include <libgda/gda-data-model-ldap.h>

#define GDA_TYPE_OSLC_CONNECTION            (gda_oslc_connection_get_type())
#define GDA_OSLC_CONNECTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST (obj, GDA_TYPE_OSLC_CONNECTION, GdaOslcConnection))
#define GDA_OSLC_CONNECTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST (klass, GDA_TYPE_OSLC_CONNECTION, GdaOslcConnectionClass))
#define GDA_IS_OSLC_CONNECTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE (obj, GDA_TYPE_OSLC_CONNECTION))
#define GDA_IS_OSLC_CONNECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GDA_TYPE_OSLC_CONNECTION))

G_BEGIN_DECLS

typedef struct _GdaOslcConnection GdaOslcConnection;
typedef struct _GdaOslcConnectionClass GdaOslcConnectionClass;

struct _GdaOslcConnection {
	GdaVconnectionDataModel parent;
};

struct _GdaOslcConnectionClass {
	GdaVconnectionDataModelClass parent_class;

	void (*_gda_reserved1) (void);
	void (*_gda_reserved2) (void);
	void (*_gda_reserved3) (void);
	void (*_gda_reserved4) (void);
};

GType gda_oslc_connection_get_type(void) G_GNUC_CONST;

G_END_DECLS

#endif
