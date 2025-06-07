/* nd-wfd-mssdp-provider.c
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

#include "gnome-network-displays-config.h"
#include "nd-wfd-mssdp-provider.h"
#include "nd-sink.h"
#include "nd-wfd-mice-sink.h"

struct _NdWFDMssdpProvider
{
  GObject               parent_instance;

  GPtrArray            *sinks;
  GSSDPClient          *gssdp_client;

  gboolean              discover;
};

enum {
  PROP_CLIENT = 1,

  PROP_DISCOVER,

  PROP_LAST = PROP_DISCOVER,
};

static void nd_wfd_mssdp_provider_provider_iface_init (NdProviderIface *iface);
static GList * nd_wfd_mssdp_provider_provider_get_sinks (NdProvider *provider);

G_DEFINE_TYPE_EXTENDED (NdWFDMssdpProvider, nd_wfd_mssdp_provider, G_TYPE_OBJECT, 0,
                        G_IMPLEMENT_INTERFACE (ND_TYPE_PROVIDER,
                                               nd_wfd_mssdp_provider_provider_iface_init);
                       )

static GParamSpec * props[PROP_LAST] = { NULL, };

static void
nd_wfd_mssdp_provider_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  NdWFDMssdpProvider *provider = ND_WFD_MSSDP_PROVIDER (object);

  switch (prop_id)
    {
    case PROP_CLIENT:
      g_value_set_object (value, provider->gssdp_client);
      break;

    case PROP_DISCOVER:
      g_value_set_boolean (value, provider->discover);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}


static void
nd_wfd_mssdp_provider_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  NdWFDMssdpProvider *provider = ND_WFD_MSSDP_PROVIDER (object);

  switch (prop_id)
    {
    case PROP_CLIENT:
      /* Construct only */
      provider->gssdp_client = g_value_dup_object (value);
      break;

    case PROP_DISCOVER:
      provider->discover = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}


static void
nd_wfd_mssdp_provider_finalize (GObject *object)
{
  NdWFDMssdpProvider *provider = ND_WFD_MSSDP_PROVIDER (object);

  g_clear_pointer (&provider->sinks, g_ptr_array_unref);
  g_clear_object (&provider->gssdp_client);

  G_OBJECT_CLASS (nd_wfd_mssdp_provider_parent_class)->finalize (object);
}

static void
nd_wfd_mssdp_provider_class_init (NdWFDMssdpProviderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = nd_wfd_mssdp_provider_get_property;
  object_class->set_property = nd_wfd_mssdp_provider_set_property;
  object_class->finalize = nd_wfd_mssdp_provider_finalize;

  props[PROP_CLIENT] =
    g_param_spec_object ("client", "Client",
                         "The GSSDPClient used to find sinks.",
                         GSSDP_TYPE_CLIENT,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, PROP_LAST, props);

  g_object_class_override_property (object_class, PROP_DISCOVER, "discover");
}

static gboolean
compare_sinks (NdWFDMiceSink *a, NdWFDMiceSink *b)
{
  gchar *a_name = NULL;
  gchar *b_name = NULL;
  gchar *a_ip = NULL;
  gchar *b_ip = NULL;
  gint a_iface = 0;
  gint b_iface = 0;

  g_object_get (a, "name", &a_name, NULL);
  g_object_get (b, "name", &b_name, NULL);
  g_object_get (a, "ip", &a_ip, NULL);
  g_object_get (b, "ip", &b_ip, NULL);

  return g_str_equal (a_name, b_name) &&
         g_str_equal (a_ip, b_ip) &&
         (a_iface == b_iface);
}

static void
resource_available_cb (GSSDPResourceBrowser *browser,
                       const gchar          *usn,
                       gpointer              locations,
                       NdWFDMssdpProvider   *provider)
{
  NdWFDMiceSink *sink = NULL;
  // TODO
  gchar *name = NULL;
  gchar *ip = NULL;
  gint interface = 0;
  GList *l = NULL;

  g_debug ("NdWFDMssdpProvider: Found resource with USN \"%s\"", usn);

  for (l = locations; l; l = l->next) {
    g_print("  Location: %s\n", (char *) l->data);
  }
/*
  sink = nd_wfd_mice_sink_new (usn, ip, NULL, 0);
  if (g_ptr_array_find_with_equal_func (provider->sinks, sink,
                                        (GEqualFunc) compare_sinks, NULL))
    {
      g_debug ("NdWFDMssdpProvider: Duplicate entry \"%s\" (%s)",
               name,
               ip);
      g_object_unref (sink);
      return;
    }
  g_object_unref (browser);

  g_debug ("NdWFDMssdpProvider: Creating sink \"%s\" (%s) on interface %i", name, ip, interface);
  g_ptr_array_add (provider->sinks, sink);
  g_signal_emit_by_name (provider, "sink-added", sink);
*/
}

static void
resource_unavailable_cb (GSSDPResourceBrowser *browser,
                         const gchar          *usn,
                         NdWFDMssdpProvider   *provider)
{
  g_debug ("NdWFDMssdpProvider: SSDP resource  \"%s\" became unavailable", usn);

  for (gint i = 0; i < provider->sinks->len; i++)
    {
      g_autoptr(NdWFDMiceSink) sink = g_object_ref (g_ptr_array_index (provider->sinks, i));

      gchar *remote_name = NULL;
      gint interface = 0;
      g_object_get (sink, "name", &remote_name, NULL);
      // TODO
      if (g_str_equal (remote_name, usn))
        {
          g_debug ("NdWFDMssdpProvider: Removing sink \"%s\"", remote_name);
          g_ptr_array_remove_index (provider->sinks, i);
          g_signal_emit_by_name (provider, "sink-removed", sink);
          break;
        }
      else
        g_debug ("NdWFDMssdpProvider: Keeping sink \"%s\"", remote_name);
    }
}

static void
nd_wfd_mssdp_provider_init (NdWFDMssdpProvider *provider)
{
  g_autoptr(GError) error = NULL;

  provider->discover = TRUE;
  provider->sinks = g_ptr_array_new_with_free_func (g_object_unref);
}

/******************************************************************
* NdProvider interface implementation
******************************************************************/

static void
nd_wfd_mssdp_provider_provider_iface_init (NdProviderIface *iface)
{
  iface->get_sinks = nd_wfd_mssdp_provider_provider_get_sinks;
}

static GList *
nd_wfd_mssdp_provider_provider_get_sinks (NdProvider *provider)
{
  NdWFDMssdpProvider *wfd_mssdp_provider = ND_WFD_MSSDP_PROVIDER (provider);
  GList *res = NULL;

  for (gint i = 0; i < wfd_mssdp_provider->sinks->len; i++)
    res = g_list_prepend (res, g_ptr_array_index (wfd_mssdp_provider->sinks, i));

  return res;
}

/******************************************************************
* NdWFDMssdpProvider public functions
******************************************************************/

NdWFDMssdpProvider *
nd_wfd_mssdp_provider_new (GSSDPClient *client)
{
  return g_object_new (ND_TYPE_WFD_MSSDP_PROVIDER,
                       "client", client,
                       NULL);
}

gboolean
nd_wfd_mssdp_provider_browse (NdWFDMssdpProvider *provider, GError * error)
{
  if (!provider->gssdp_client)
    {
      g_warning ("NdWFDMssdpProvider: No Avahi client found");
      return FALSE;
    }

  GSSDPResourceBrowser *gssdp_browser = gssdp_resource_browser_new(provider->gssdp_client, "urn:dial-multiscreen-org:service:dial:1");
  if (!gssdp_browser) {
      g_warning ("NdWFDMssdpProvider: Couldn't create GSSDP browser");
      return FALSE;
  }

  g_signal_connect (gssdp_browser,
                    "resource-available",
                    G_CALLBACK (resource_available_cb),
                    provider);

  g_signal_connect (gssdp_browser,
                    "resource-unavailable",
                    G_CALLBACK (resource_unavailable_cb),
                    provider);

  gssdp_resource_browser_set_active(gssdp_browser, TRUE);

  return TRUE;
}
