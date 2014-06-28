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
#include "emelfm.h"

static GtkWidget *dialog;
static GtkWidget *name_entry;
static GtkWidget *action_entry;

static void
ok_cb(GtkWidget *widget, UserCommand **command)
{
  if (*command == NULL)
    *command = g_new0(UserCommand, 1);

  strncpy((*command)->name, gtk_entry_get_text(GTK_ENTRY(name_entry)),
          sizeof((*command)->name));
  strncpy((*command)->action, gtk_entry_get_text(GTK_ENTRY(action_entry)),
          sizeof((*command)->action));

  gtk_widget_destroy(dialog);
  gtk_main_quit();
}

static void
help_cb(GtkWidget *widget)
{
  show_help_file("help.txt");
}

static void
cancel_cb(GtkWidget *widget)
{
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
delete_event_cb(GtkWidget *widget)
{
  /* this is just here so the user can't close the dialog without clicking
   * one of the buttons
   */
}

void
create_user_command_dialog(UserCommand **command)
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *table;

  dialog = gtk_dialog_new();
  dialog_vbox = GTK_DIALOG(dialog)->vbox;
  action_area = GTK_DIALOG(dialog)->action_area;
  gtk_container_set_border_width(GTK_CONTAINER(dialog_vbox), 5);
  gtk_box_set_spacing(GTK_BOX(dialog_vbox), 5);
  gtk_signal_connect(GTK_OBJECT(dialog), "delete_event", delete_event_cb, NULL);
  gtk_signal_connect(GTK_OBJECT(dialog), "key_press_event",
                     GTK_SIGNAL_FUNC(key_press_cb), NULL);

  table = add_table(dialog_vbox, 2, 2, FALSE, FALSE, 0);
  gtk_table_set_row_spacings(GTK_TABLE(table), 5);

  add_label_to_table(table, _("Name: "), 0.0, 0, 1, 0, 1);
  add_label_to_table(table, _("Command: "), 0.0, 0, 1, 1, 2);
  if (*command != NULL)
  {
    name_entry = add_entry_to_table(table, (*command)->name, 1, 2, 0, 1);
    action_entry = add_entry_to_table(table, (*command)->action, 1, 2, 1, 2);
  }
  else
  {
    name_entry = add_entry_to_table(table, "", 1, 2, 0, 1);
    action_entry = add_entry_to_table(table, "", 1, 2, 1, 2);
  }

  gtk_signal_connect(GTK_OBJECT(name_entry), "activate", 
                     GTK_SIGNAL_FUNC(ok_cb), command);

  gtk_signal_connect(GTK_OBJECT(action_entry), "activate", 
                     GTK_SIGNAL_FUNC(ok_cb), command);

  gtk_widget_grab_focus(name_entry);

  add_button(action_area, _("Ok"), TRUE, 0, ok_cb, command);
  add_button(action_area, _("Info"), TRUE, 0, help_cb, NULL);
  add_button(action_area, _("Cancel"), TRUE, 0, cancel_cb, NULL);

  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
  gtk_widget_show(dialog);
}


