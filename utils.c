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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "emelfm.h"

/* Allocates a new string in lower case: must be freed */
gchar *
str_to_lower(gchar *string)
{
  gchar *new_string;

  if (string == NULL)
    return NULL;

  if ((new_string = g_strdup(string)) == NULL)
    return NULL;

  g_strdown(new_string);

  return new_string;
}

gint
S_ISEXE(mode_t mode)
{
  if ((S_IXUSR & mode) || (S_IXGRP & mode) || (S_IXOTH & mode))
    return 1;
  return 0;
}

/* This executes the file(1) command and searches the output for the 
 * given string.
 * Used in is_text and is_executable
 */
static gboolean
grep_file_output(gchar *filename, gchar *string)
{
  FILE *pipe;
  gchar line[MAX_LEN];

  g_snprintf(line, sizeof(line), "file \"%s\"", filename);
  if ((pipe = popen(line, "r")) == NULL)
  {
    fprintf(stderr, _("Unable to open pipe for file command\n"));
    return FALSE;
  }

  while (fgets(line, MAX_LEN, pipe))
  {
    if (strstr(line, string))
    {
      pclose(pipe);
      return TRUE;
    }
  }
  pclose(pipe);
  return FALSE;
}

gboolean
is_text(gchar *filename)
{
  return grep_file_output(filename, "text");
}

gboolean
is_executable(FileInfo *info)
{
  if (S_ISEXE(info->statbuf.st_mode))
    return grep_file_output(info->filename, "executable");

  return FALSE;
}

void
view_file(gchar *filename)
{
  gchar command[2*NAME_MAX];

  g_return_if_fail(filename != NULL);

  g_snprintf(command, sizeof(command), "%s \"%s\"", cfg.viewer_command, filename);
  file_exec(command);
}

void
show_help_file(gchar *filename)
{
  gchar fullpath[PATH_MAX+NAME_MAX];
  g_snprintf(fullpath, sizeof(fullpath), "%s/%s", DOC_DIR, filename);
  create_view_file_dialog(fullpath);
}

gchar *
get_key_name(gint keyval)
{
  return gdk_keyval_name(gdk_keyval_to_lower(keyval));
}

void
copy_entry_to_str(GtkWidget *entry, gchar *str)
{
  strcpy(str, gtk_entry_get_text(GTK_ENTRY(entry)));
}

void
free_data(gpointer data)
{
  if (data != NULL)
    g_free(data);
}

void
chomp(gchar *text)
{
  if (text[strlen(text)-1] == '\n')
    text[strlen(text)-1] = '\0';
}

/* This is the function that replaces %f etc. with their appropriate values
 * The GString returned needs to be freed.
 *
 * If for_each is non-null then %f will be replace by its value (which should
 * be a filename) otherwise, %f will be replaced by all selected filenames.
 */
GString *
expand_macros(gchar *text, gchar *for_each)
{
  GString *command_string = g_string_new("");
  gchar *s, *free_this, *command_copy;
  FileInfo *info;

  disable_refresh();
  command_copy = g_strdup(text);
  free_this = s = command_copy;
  while ((s = strchr(command_copy, '%')) != NULL)
  {
    *s++ = '\0';
    g_string_append(command_string, command_copy);
    command_copy = s+1;
    switch (*s)
    {
      case 'f':
        {
          if (for_each != NULL)
            g_string_sprintfa(command_string, "\"%s\"", for_each);
          else
          {
            GList *base, *tmp;

            base = tmp = get_selection(curr_view);
            if (tmp == NULL)
            {
              status_message(_("No files selected\n"));
              g_free(free_this);
              g_string_free(command_string, TRUE);
              reenable_refresh();
              return NULL;
            }
            else
            {
              for (; tmp != NULL; tmp = tmp->next)
              {
                info = tmp->data;
                g_string_sprintfa(command_string, "\"%s\" ", info->filename);
              }
            }
            g_list_free(base);
          }
        }
        break;
      case 'F':
        {
          GList *base, *tmp;

          base = tmp = get_selection(other_view);
          if (tmp == NULL)
          {
            status_message(_("No files selected in other pane\n"));
            g_free(free_this);
            g_string_free(command_string, TRUE);
            reenable_refresh();
            return NULL;
          }
          else
          {
            for (; tmp != NULL; tmp = tmp->next)
            {
              info = tmp->data;
              g_string_sprintfa(command_string, "\"%s/%s\" ", other_view->dir,
                                                              info->filename);
            }
          }
          g_list_free(base);
        }
        break;
      case 'd':
        g_string_sprintfa(command_string, "\"%s\"", curr_view->dir);
        break;
      case 'D':
        g_string_sprintfa(command_string, "\"%s\"", other_view->dir);
        break;
      case '{':
        if ((s = strchr(command_copy, '}')) == NULL)
        {
          status_message(_("No matching '}' found in action text.\n"));
          g_free(free_this);
          g_string_free(command_string, TRUE);
          reenable_refresh();
          return NULL;
        }
        else
        {
          gchar *user_input;

          *s++ = '\0';
          create_user_prompt(command_copy, "", FALSE, &user_input);
          gtk_main();
          command_copy = s;
          if (user_input != NULL)
          {
            g_string_append(command_string, user_input);
            g_free(user_input);
          }
          else
          {
            g_free(free_this);
            g_string_free(command_string, TRUE);
            reenable_refresh();
            return NULL;
          }
        }
        break;
      default:
        g_string_append_c(command_string, '%');
        g_string_append_c(command_string, *s);
        break;
    }
  }
  g_string_append(command_string, command_copy);
  g_free(free_this);
  reenable_refresh();
  return command_string;
}

void
exec_system_command(gchar *command)
{
  int i;
  for (i = 0; i < n_sys_ops; i++)
  {
    if (STREQ(command, system_ops[i].name))
    {
      system_ops[i].func(NULL);
      break;
    }
  }
}

void
exec_interface_op(gchar *command)
{
  int i;
  for (i = 0; i < n_interface_ops; i++)
  {
    if (STREQ(command, interface_ops[i].name))
    {
      interface_ops[i].func(NULL);
      break;
    }
  }
}

void
do_command(gchar *command)
{
  GString *command_copy;

  if (strncmp(command, "SYSTEM_", 7) == 0)
    exec_system_command(command);
  else if (strncmp(command, "INTERFACE:", 10) == 0)
    exec_interface_op(command);
  else if ((command_copy = expand_macros(command, NULL)) != NULL)
  {
    command = command_copy->str;
    if ((strncmp(command, "x ", 2) == 0) && (strlen(command) > 2))
      exec_in_xterm(command+2);
    else
    {
      if (command[strlen(command)-1] == '&')
      {
        command[strlen(command)-1] = '\0';
        file_exec(command);
      }
      else 
        exec_and_capture_output_threaded(command);
    }
    g_string_free(command_copy, TRUE);
  }
}

