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

#ifndef __GDA_OSLC_PROVIDER_H__
#define __GDA_OSLC_PROVIDER_H__

#include <virtual/gda-vprovider-data-model.h>

#define GDA_TYPE_OSLC_PROVIDER            (gda_oslc_provider_get_type())
#define GDA_OSLC_PROVIDER(obj)            (G_TYPE_CHECK_INSTANCE_CAST (obj, GDA_TYPE_OSLC_PROVIDER, GdaOslcProvider))
#define GDA_OSLC_PROVIDER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST (klass, GDA_TYPE_OSLC_PROVIDER, GdaOslcProviderClass))
#define GDA_IS_OSLC_PROVIDER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE (obj, GDA_TYPE_OSLC_PROVIDER))
#define GDA_IS_OSLC_PROVIDER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GDA_TYPE_OSLC_PROVIDER))

typedef struct _GdaOslcProvider      GdaOslcProvider;
typedef struct _GdaOslcProviderClass GdaOslcProviderClass;

struct _GdaOslcProvider {
	GdaVproviderDataModel provider;
};

struct _GdaOslcProviderClass {
	GdaVproviderDataModelClass parent_class;
};

G_BEGIN_DECLS

GType gda_oslc_provider_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif
