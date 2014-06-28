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

#include <stdlib.h>
#include <string.h>
#include "emelfm.h"

/* Allocates and returns the actions string in the file types list with a
 * matching extension.. the value returned needs to be freed
 */
gchar *
get_actions_for_ext(gchar *ext)
{
  FileType *ft = get_filetype_for_ext(ext);

  if (ft != NULL)
    return g_strdup(ft->actions);
  
  return NULL;
}

/* same as above but returns the first action in the list */
gchar *
get_default_action_for_ext(gchar *ext)
{
  FileType *ft = get_filetype_for_ext(ext);

  if (ft != NULL)
  {
    gchar *action = g_strdup(ft->actions);
    gchar *s;

    if ((s = strchr(action, ',')) != NULL)
      *s = '\0';

    return action;
  }

  return NULL;
}

/* Search the filetype list for the extension */
FileType *
get_filetype_for_ext(gchar *ext)
{
  GList *tmp;
  gchar *ext_copy, *s, *s2, *free_this;

  ext = str_to_lower(ext);
  for (tmp = cfg.filetypes; tmp != NULL; tmp = tmp->next)
  {
    FileType *ft = tmp->data;

    free_this = ext_copy = g_strdup(ft->extensions);
    while ((s = s2 = strchr(ext_copy, ',')) != NULL)
    {
      while (*(s2-1) == ' ') /* get rid of whitespace */
        s2--;
      *s2 = '\0';
      while ((*ext_copy == '.') || (*ext_copy == ' '))
        ext_copy++;

      if (STREQ(ext_copy, ext))
      {
        g_free(free_this);
        g_free(ext);
        return ft;
      }
      ext_copy = s+1;
    }
    while ((*ext_copy == '.') || (*ext_copy == ' '))
      ext_copy++;
    if (STREQ(ext_copy, ext))
    {
      g_free(free_this);
      g_free(ext);
      return ft;
    }
    g_free(free_this);
  }
  g_free(ext);

  return NULL;
}

/* first checks to see if a filetype with the same extensions exists..
 * if not it creates and adds a new filetype
 */
void
add_filetype(gchar *extensions, gchar *actions, gchar *description)
{
  GList *tmp;
  FileType *ft;

  for (tmp = cfg.filetypes; tmp != NULL; tmp = tmp->next)
  {
    ft = tmp->data;

    if (STREQ(extensions, ft->extensions))
    {
      strncpy(ft->actions, actions, sizeof(ft->actions));
      strncpy(ft->description, description, sizeof(ft->description));
      return;
    }
  }

  {
    ft = g_new0(FileType, 1);
    strncpy(ft->description, description, sizeof(ft->description));
    strncpy(ft->extensions, extensions, sizeof(ft->extensions));
    strncpy(ft->actions, actions, sizeof(ft->actions));
    
    cfg.filetypes = g_list_append(cfg.filetypes, ft);
  }
}

/* The only thing different about this and a regular do_command is that if 
 * there is no %f in the action given it appends the selected files to the 
 * end of the action string
 * This is so the user can just enter like "xmms" as an action and not have
 * to remember to append the %f 
 */
void
exec_filetype_action(gchar *action)
{
  FileInfo *info;
  GString *command;

  g_return_if_fail(action != NULL);

  if (strchr(action, '%') == NULL)
  { /* There wasn't a macro found in the action */
    GList *tmp, *base;

    disable_refresh();
    base = tmp = get_selection(curr_view);
    if (tmp == NULL)
    {
      reenable_refresh();
      return;
    }

    command = g_string_sized_new(MAX_LEN);
    g_string_assign(command, action);

    for (; tmp != NULL; tmp = tmp->next)
    {
      info = tmp->data;
      g_string_sprintfa(command, " \"%s\"", info->filename);
    }
    do_command(command->str);
    g_list_free(base);
    g_string_free(command, TRUE);
    reenable_refresh();
  }
  else
  { /* There was a macro in the action */
    if ((command = expand_macros(action, NULL)) != NULL)
    {
      do_command(command->str);
      g_string_free(command, TRUE);
    }
  }
}


