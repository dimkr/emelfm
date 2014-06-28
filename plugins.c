/* vi:set ts=2 sw=2 et:
 *
 * emelFM
 * Copyright (C) 1999 Michael Clark.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <string.h>
#include <stdio.h>
#include "emelfm.h"

void
destroy_plugin(Plugin *p)
{
  void (*unload)(Plugin *);

  if (g_module_symbol(p->module, "unload_plugin", (gpointer *)&unload))
    unload(p);
  
  g_module_close(p->module);
  g_free(p);
}

void
unload_all_plugins()
{
  GList *tmp;

  for (tmp = cfg.plugins; tmp != NULL; tmp = tmp->next)
  {
    if (tmp->data != NULL)
    {
      Plugin *p = tmp->data;
      destroy_plugin(p);
    }
  }

  g_list_free(cfg.plugins);
  cfg.plugins = NULL;
}

Plugin *
load_plugin(gchar *filename)
{
  Plugin *p;
  gint (*init)(Plugin *);
  
  p = g_new0(Plugin, 1);
  strncpy(p->filename, filename, sizeof(p->filename));
  if ((p->module = g_module_open(p->filename, 0)) == NULL)
  {
    printf("Failed to load module: %s\n", g_module_error());
    g_free(p);
    return NULL;
  }

  if (!g_module_symbol(p->module, "init_plugin", (gpointer *)&init))
  {
    printf("Couldn't find init_plugin in module: %s\n", g_module_error());
    g_free(p);
    return NULL;
  }

  if (!init(p))
  {
    printf("Couldn't initialize plugin.\n");
    g_free(p);
    return NULL;
  }

  p->load = TRUE;
  return p;
}

void
do_plugin_action(GtkWidget *widget, Plugin *p)
{
  if (p == NULL)
  {
    status_message("Plugin not found!\n");
    return;
  }

  /* disable/reenable here so that plugin doesn't have to worry about it */
  disable_refresh();
  p->plugin_cb();
  reenable_refresh();
}

Plugin *
get_plugin_by_name(gchar *name)
{
  GList *tmp;

  for (tmp = cfg.plugins; tmp != NULL; tmp = tmp->next)
  {
    Plugin *p = tmp->data;
    if (STREQ(name, p->name))
      return p;
  }

  return NULL;
}

