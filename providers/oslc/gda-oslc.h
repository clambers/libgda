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

#ifndef __GDA_OSLC_H__
#define __GDA_OSLC_H__

/*
 * Provider name
 */
#define OSLC_PROVIDER_NAME "OSLC"

#include <libgda/libgda.h>
#include <libgda/gda-connection.h>
#include <libgda/gda-connection-private.h>

typedef struct {
  GdaServerProviderConnectionData parent;
  GString *root;
} OslcConnectionData;

#endif
