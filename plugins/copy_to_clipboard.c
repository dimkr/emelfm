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
#include "../emelfm.h"

static GtkWidget *clipboard;
static GtkWidget *window;

static void
copy_to_clipboard()
{
  FileInfo *info;
  GList *tmp, *base;

  gtk_entry_set_text(GTK_ENTRY(clipboard), "");
  base = tmp = get_selection(curr_view);

  for (; tmp != NULL; tmp = tmp->next)
  {
    info = tmp->data;
    if (strchr(info->filename, ' ') != NULL)
    {
      gtk_entry_append_text(GTK_ENTRY(clipboard), "\"");
      gtk_entry_append_text(GTK_ENTRY(clipboard), info->filename);
      gtk_entry_append_text(GTK_ENTRY(clipboard), "\"");
    }
    else
      gtk_entry_append_text(GTK_ENTRY(clipboard), info->filename);

    if (tmp->next != NULL)
      gtk_entry_append_text(GTK_ENTRY(clipboard), " ");
  }
  gtk_editable_select_region(GTK_EDITABLE(clipboard), 0, -1);
  gtk_editable_copy_clipboard(GTK_EDITABLE(clipboard));
}

gint
init_plugin(Plugin *p)
{
  /* This is a hidden entry field for that will hold the contents of the
   * clipboard.. we have to pack it somewhere to avoid a crash
   * but we do not show it
   */
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  clipboard = gtk_entry_new();
  gtk_container_add(GTK_CONTAINER(window), clipboard);
  
  p->name = "Copy To Clipboard";
  p->description = "Copy the selected filenames to the clipboard.";
  p->plugin_cb = copy_to_clipboard;

  return TRUE;
}

void
unload_plugin(Plugin *p)
{
  gtk_widget_destroy(window);
}


