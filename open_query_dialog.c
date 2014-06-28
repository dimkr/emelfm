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
#include "emelfm.h"

static GtkWidget *dialog;

static void
enter_command_cb(GtkWidget *widget)
{
  gtk_widget_set_sensitive(dialog, FALSE);
  open_with_cb(NULL);
  gtk_widget_destroy(dialog);
}

static void
view_file_cb(GtkWidget *widget)
{
  view_cb(NULL);
  gtk_widget_set_sensitive(app.main_window, TRUE);
  gtk_widget_destroy(dialog);
}

static void
cancel_cb(GtkWidget *widget)
{
  gtk_widget_set_sensitive(app.main_window, TRUE);
  gtk_widget_destroy(dialog);
}

static void
key_press_cb(GtkWidget    *widget,
             GdkEventKey  *event,
             gpointer     data)
{
  if (event->keyval == GDK_Escape)
    cancel_cb(NULL);
}

void
create_open_query_dialog(gchar *filename)
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *table;
  GtkWidget *wid;
  GString *message;

  dialog = gtk_dialog_new();
  dialog_vbox = GTK_DIALOG(dialog)->vbox;
  action_area = GTK_DIALOG(dialog)->action_area;
  gtk_container_set_border_width(GTK_CONTAINER(dialog_vbox), 5);
  gtk_container_set_border_width(GTK_CONTAINER(action_area), 5);
  gtk_signal_connect(GTK_OBJECT(dialog), "key_press_event",
                     GTK_SIGNAL_FUNC(key_press_cb), NULL);

  message = g_string_new(_("What would you like to do with this file?\n"));
  g_string_sprintfa(message, _("Filename:  %s"), filename);
  wid = add_label(dialog_vbox, message->str, 0.5, FALSE, 2);
  gtk_label_set_justify(GTK_LABEL(wid), GTK_JUSTIFY_LEFT);
  g_string_free(message, TRUE);

  table = add_table(action_area, 3, 1, TRUE, TRUE, 5);
  gtk_table_set_row_spacings(GTK_TABLE(table), 5);
  gtk_table_set_col_spacings(GTK_TABLE(table), 5);

  add_button_to_table(table, _("Enter a command to Open with"),
                      enter_command_cb, NULL, 0, 1, 0, 1);
  add_button_to_table(table, _("View File"), view_file_cb, NULL, 0, 1, 1, 2);
  add_button_to_table(table, _("Cancel"), cancel_cb, NULL, 0, 1, 2, 3);

  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
  gtk_widget_set_sensitive(app.main_window, FALSE);
  gtk_widget_show(dialog);
}

