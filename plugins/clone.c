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

#include <unistd.h>
#include "../emelfm.h"

static void
clone()
{
  FileInfo *info;
  GList *tmp, *base;
  gchar *new_name;
  gint check = cfg.confirm_overwrite;
  gint result;

  gtk_widget_set_sensitive(app.main_window, FALSE);
  base = tmp = get_selection(curr_view);

  for (; tmp != NULL; tmp = tmp->next)
  {
    info = tmp->data;
    create_user_prompt(_("This allows you to copy a file\n"
                         "to the current directory rather\n"
                         "than the opposite directory.\n\n"
                         "Enter New Filename: "),
                         info->filename, FALSE, &new_name);
    gtk_main();

    if (new_name != NULL)
    {
      if (check && (access(new_name, F_OK) == 0))
      {
        result = op_with_ow_check(info->filename, new_name, file_copy);
        if (result == YES_TO_ALL)
          check = FALSE;
        else if (result == CANCEL)
          break;
      }
      else
        file_copy(info->filename, new_name);
      g_free(new_name);
    }
  }

  g_list_free(tmp);
  gtk_widget_set_sensitive(app.main_window, TRUE);
  refresh_list(curr_view);
}

gint
init_plugin(Plugin *p)
{
  p->name = "Clone";
  p->description = "Copy a file to the current directory with a new filename.";
  p->plugin_cb = clone;

  return TRUE;
}


