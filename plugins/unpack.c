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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "../emelfm.h"

static gchar *tmp_dir = "/tmp/.emelfm-unpack";
static gchar *last_package;
static GtkWidget *dialog;

static void
yes_cb(GtkWidget *widget)
{
  GString *command = g_string_new("");

  if ((strstr(last_package, ".tar.gz") != NULL) ||
      (strstr(last_package, ".tgz") != NULL))
    g_string_sprintf(command, "cd %s; tar cvf - . | gzip - > %s", tmp_dir,
                   last_package);
  else if (strstr(last_package, ".tar.bz2") != NULL)
    g_string_sprintf(command, "cd %s; tar cvf - . | bzip2 - > %s", tmp_dir,
                     last_package);
  else if (strstr(last_package, ".tar") != NULL)
    g_string_sprintf(command, "cd %s; tar cvf %s .", tmp_dir, last_package);
  else if (strstr(last_package, ".zip") != NULL)
    g_string_sprintf(command, "rm %s; cd %s; zip -r %s .", last_package,
                     tmp_dir, last_package);

  exec_in_xterm(command->str);
  g_string_free(command, TRUE);
  g_free(last_package);
  gtk_widget_destroy(dialog);
}

static void
no_cb(GtkWidget *widget)
{
  g_free(last_package);
  gtk_widget_destroy(dialog);
}

static void
delete_event_cb(GtkWidget *widget)
{
  /* this is just here so the user can't close the dialog without clicking
   * one of the buttons
   */
}

static void
key_press_cb(GtkWidget *widget, GdkEventKey *event, gpointer *data)
{
  if (event->keyval == GDK_Escape)
    no_cb(NULL);
}

static void
query_user()
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  gchar *message;

  dialog = gtk_dialog_new();
  dialog_vbox = GTK_DIALOG(dialog)->vbox;
  action_area = GTK_DIALOG(dialog)->action_area;
  gtk_container_set_border_width(GTK_CONTAINER(dialog_vbox), 5);
  gtk_signal_connect(GTK_OBJECT(dialog), "delete_event",
                     GTK_SIGNAL_FUNC(delete_event_cb), NULL);
  gtk_signal_connect(GTK_OBJECT(dialog), "key_press_event",
                     GTK_SIGNAL_FUNC(key_press_cb), NULL);
  
  message = g_strdup_printf(_("Repack Package: %s"), last_package);
  add_label(dialog_vbox, message, 0.0, TRUE, 0);
  g_free(message);

  gtk_widget_grab_focus(
    add_button(action_area, _("Yes"), TRUE, 0, yes_cb, NULL));
  add_button(action_area, _("No"), TRUE, 0, no_cb, NULL);

  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
  gtk_widget_show(dialog);
}

static gint
check_dir(gpointer data)
{
  FileView *view = (FileView *)data;
  if (strstr(view->dir, tmp_dir) == NULL)
  { /* user left the temp directory */
    gchar *dir = g_strdup(last_package), *s;
    /* Change back to the directory the package is in */
    if ((s = strrchr(dir, '/')) != NULL)
    {
      *s = '\0';
      change_dir(view, dir);
    }
    g_free(dir);
    /* ask if they want to rebuild the package */
    query_user();
    return FALSE;
  }

  return TRUE;
}

static void
unpack()
{
  GString *command;
  GList *tmp;
  FileInfo *info;
  
  if (strstr(curr_view->dir, tmp_dir) != NULL)
  { /* It would be bad to call this plugin recursively */
    status_message("Recursive Unpack is not supported\n");
    return;
  }

  set_cursor(GDK_WATCH);
  /* delete the tmp_dir if it exists already */
  if (access(tmp_dir, F_OK) == 0)
    file_delete(tmp_dir);
  file_mkdir(tmp_dir); 

  /* unpack the tarball into the temp directory */
  tmp = get_selection(curr_view);
  info = tmp->data;
  command = g_string_new("");
  last_package = g_strdup_printf("%s/%s", curr_view->dir, info->filename);
  if ((strstr(info->filename, ".tar.gz") != NULL) ||
      (strstr(info->filename, ".tgz") != NULL))
    g_string_sprintf(command, "cd %s; gunzip -c %s | tar xvf -", tmp_dir,
                    last_package);
  else if (strstr(info->filename, ".tar.bz2") != NULL)
    g_string_sprintf(command, "cd %s; bzip2 -d -c %s | tar xvf -", tmp_dir,
                      last_package);
  else if (strstr(info->filename, ".tar") != NULL)
    g_string_sprintf(command, "cd %s; tar xvf %s", tmp_dir, last_package);
  else if (strstr(info->filename, ".zip") != NULL)
    g_string_sprintf(command, "cd %s; unzip %s", tmp_dir, last_package);
  else
  {
    status_message("Unrecognized package type\n");
    g_string_free(command, TRUE);
    g_free(last_package);
    set_cursor(GDK_LEFT_PTR);
    return;
  }

  system(command->str);
  g_string_free(command, TRUE);
  change_dir(curr_view, tmp_dir);
  set_cursor(GDK_LEFT_PTR);
  gtk_timeout_add(500, check_dir, curr_view);
}

gint
init_plugin(Plugin *p)
{
  p->name = "Unpack";
  p->description = "Unpack a package into a temp directory for quick access.\n"
                   "Supports tar, tar.gz, tar.bz2, and zip archives";
  p->plugin_cb = unpack;

  return TRUE;
}


