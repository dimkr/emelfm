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

static void
for_each_dialog()
{
  gchar *s;
  GString *command;
  GList *base, *tmp;
  FileInfo *info;

  create_user_prompt(_("Macros:\n"
                     "%f = Filename\n"
                     "%d = Active Directory\n"
                     "%D = Inactive Directory\n"
                     "%{Prompt message} = Prompt user for input\n\n"
                     "Enter an action to execute on each\n"
                     "selected file:"),
                     "", FALSE, &s);
  gtk_main();

  if (s != NULL)
  {
    base = tmp = get_selection(curr_view);

    if (strchr(s, '%') == NULL)
    {
      command = g_string_sized_new(sizeof(s));
      for (; tmp != NULL; tmp = tmp->next)
      {
        info = tmp->data;
        g_string_sprintf(command, "%s \"%s\"", s, info->filename);

        do_command(command->str);
      }
      g_string_free(command, TRUE);
    }
    else
    {
      for (; tmp != NULL; tmp = tmp->next)
      {
        info = tmp->data;
        if ((command = expand_macros(s, info->filename)) != NULL)
        {
          do_command(command->str);
          g_string_free(command, TRUE);
        }
      }
    }
    g_list_free(base);
  }
}

gint
init_plugin(Plugin *p)
{
  p->name = "For Each";
  p->description = "Execute a command on each file separately.";
  p->plugin_cb = for_each_dialog;

  return TRUE;
}
