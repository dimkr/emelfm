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
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include "../emelfm.h"

static void
add_disk_usage(gchar *filename, gint *total, gint *files, gint *dirs)
{
  struct stat statbuf;

  if (lstat(filename, &statbuf) != 0)
  {
    status_errno();
    return;
  }
  (*total) += statbuf.st_size;
  if (S_ISDIR(statbuf.st_mode))
  {
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    gchar path[PATH_MAX];

    (*dirs)++;
    if ((dp = opendir(filename)) == NULL)
    {
      status_errno();
      status_message("Warning: couldn't open directory ");
      status_message(filename);
      status_message("\n");
      return;
    }

    while ((entry = readdir(dp)) != NULL)
    {
      if (STREQ(entry->d_name, ".") || STREQ(entry->d_name, ".."))
        continue;

      g_snprintf(path, PATH_MAX, "%s/%s", filename, entry->d_name);
      if (lstat(path, &statbuf) != 0)
      {
        status_errno();
        continue;
      }

      (*total) += statbuf.st_size;
      if (S_ISDIR(statbuf.st_mode))
        add_disk_usage(path, total, files, dirs);
      else    
        (*files)++;
    }
    closedir(dp);
  }
  else
    (*files)++;
}

static void
du()
{
  GList *base, *tmp;
  FileInfo *info;
  gint total = 0, files = 0, dirs = 0;
  GString *text;
  
  base = tmp = get_selection(curr_view);
  for (; tmp != NULL; tmp = tmp->next)
  {
    info = tmp->data;
    add_disk_usage(info->filename, &total, &files, &dirs);
  }

  text = g_string_new("Total Size: ");
  if (total < 10240) /* less than 10K */
    g_string_sprintfa(text, "%d bytes\n", total);
  else if (total < 1048576) /* less than a meg */
    g_string_sprintfa(text, "%.2f Kbytes\n", (total / 1024.0));
  else
    g_string_sprintfa(text, "%.2f Mbytes\n", (total / 1048576.0));;

  g_string_sprintfa(text, "in  %d Files\nand %d Directories\n", files, dirs);
  status_message(text->str);
  status_message("----end-of-output----\n");
  g_string_free(text, TRUE);
}

gint
init_plugin(Plugin *p)
{
  p->name = "Disk Usage";
  p->description = "Calculates the disk usage of the selected files.";
  p->plugin_cb = du;

  return TRUE;
}


