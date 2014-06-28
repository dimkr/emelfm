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

#include "../emelfm.h"

static GtkWidget *dialog;
static GtkWidget *filename_entry;
static GtkWidget *pkg_type_option;
static gint pkg_type;
enum { TAR_GZ, TAR_BZ2, TAR, ZIP };

static void
pack_cb(GtkWidget *widget)
{
  GString *command;
  gchar *entry_text;
  GList *tmp, *base;

  command = g_string_new("");
  entry_text = gtk_entry_get_text(GTK_ENTRY(filename_entry));
  tmp = base = get_selection(curr_view);

  switch (pkg_type)
  {
  case TAR_GZ:
    g_string_assign(command, "tar cvf - ");
    for (; tmp != NULL; tmp = tmp->next)
      g_string_sprintfa(command, "%s ", ((FileInfo *)tmp->data)->filename);
    g_string_sprintfa(command, "| gzip - > %s.tar.gz", entry_text);
    break;
  case TAR_BZ2:
    g_string_assign(command, "tar cvf - ");
    for (; tmp != NULL; tmp = tmp->next)
      g_string_sprintfa(command, "%s ", ((FileInfo *)tmp->data)->filename);
    g_string_sprintfa(command, "| bzip2 - > %s.tar.bz2", entry_text);
    break;
  case TAR: 
    g_string_sprintf(command, "tar cvf %s.tar ", entry_text);
    for (; tmp != NULL; tmp = tmp->next)
      g_string_sprintfa(command, "%s ", ((FileInfo *)tmp->data)->filename);
    break;
  case ZIP:
    g_string_sprintf(command, "zip -r %s.zip ", entry_text);
    for (; tmp != NULL; tmp = tmp->next)
      g_string_sprintfa(command, "%s ", ((FileInfo *)tmp->data)->filename);
    break;
  default:
    break;
  }

  exec_in_xterm(command->str);
  g_string_free(command, TRUE);
      
  gtk_widget_set_sensitive(app.main_window, TRUE);
  gtk_widget_grab_focus(curr_view->clist);
  gtk_widget_destroy(dialog);
  gtk_main_quit();
}

static void
cancel_cb(GtkWidget *widget)
{
  gtk_widget_set_sensitive(app.main_window, TRUE);
  gtk_widget_destroy(dialog);
  gtk_main_quit();
}

static void
key_press_cb(GtkWidget    *widget,
             GdkEventKey  *event,
             gpointer     data)
{
  if (event->keyval == GDK_Escape)
    cancel_cb(NULL);
}

static void
set_pkg_type(GtkWidget *widget, gpointer data)
{
  pkg_type = GPOINTER_TO_INT(data);
}

static void
pack_dialog()
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *table;
  GtkWidget *menu;
  
  dialog = gtk_dialog_new();
  dialog_vbox = GTK_DIALOG(dialog)->vbox;
  action_area = GTK_DIALOG(dialog)->action_area;
  gtk_container_set_border_width(GTK_CONTAINER(dialog_vbox), 5);
  gtk_box_set_spacing(GTK_BOX(dialog_vbox), 5);
  gtk_signal_connect(GTK_OBJECT(dialog), "key_press_event",
                     GTK_SIGNAL_FUNC(key_press_cb), NULL);

  table = add_framed_table(dialog_vbox, _("Archive Options: "), 1, 3, FALSE, 0);
  add_label_to_table(table, _("Filename: "), 1.0, 0, 1, 0, 1);
  filename_entry = add_entry_to_table(table, "", 1, 2, 0, 1);
  pkg_type_option = gtk_option_menu_new();
  gtk_table_attach_defaults(GTK_TABLE(table), pkg_type_option, 2, 3, 0, 1);
  gtk_widget_show(pkg_type_option);
  menu = gtk_menu_new();
  add_menu_item(menu, _(".tar.gz"), set_pkg_type, GINT_TO_POINTER(TAR_GZ));
  add_menu_item(menu, _(".tar.bz2"), set_pkg_type, GINT_TO_POINTER(TAR_BZ2));
  add_menu_item(menu, _(".tar"), set_pkg_type, GINT_TO_POINTER(TAR));
  add_menu_item(menu, _(".zip"), set_pkg_type, GINT_TO_POINTER(ZIP));
  gtk_option_menu_set_menu(GTK_OPTION_MENU(pkg_type_option), menu);
  pkg_type = TAR_GZ;

  add_button(action_area, _("Pack"), TRUE, 0, pack_cb, NULL);
  add_button(action_area, _("Cancel"), TRUE, 0, cancel_cb, NULL);

  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
  gtk_widget_show(dialog);
  gtk_widget_set_sensitive(app.main_window, FALSE);
  gtk_main();
}

gint
init_plugin(Plugin *p)
{
  p->name = "Pack";
  p->description = "A file archiving utility.\n"
                   "Builds tar, tar.gz, tar.bz2, and zip archives";
  p->plugin_cb = pack_dialog;

  return TRUE;
}


