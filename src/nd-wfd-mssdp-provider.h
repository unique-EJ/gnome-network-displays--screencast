/* nd-wfd-mssdp-provider.h
 *
 * Copyright 2022 Christian Glombek <lorbus@fedoraproject.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <libgssdp/gssdp.h>
#include <gio/gio.h>
#include "nd-provider.h"

G_BEGIN_DECLS

#define ND_TYPE_WFD_MSSDP_PROVIDER (nd_wfd_mssdp_provider_get_type ())
G_DECLARE_FINAL_TYPE (NdWFDMssdpProvider, nd_wfd_mssdp_provider, ND, WFD_MSSDP_PROVIDER, GObject)

NdWFDMssdpProvider * nd_wfd_mssdp_provider_new (GSSDPClient * client);

GSSDPClient *  nd_wfd_mssdp_provider_get_client (NdWFDMssdpProvider *provider);

gboolean nd_wfd_mssdp_provider_browse (NdWFDMssdpProvider *provider,
                                       GError           * error);

G_END_DECLS
