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

#include "emelfm.h"

static GtkWidget *dialog;
static GtkWidget *show_in_menu_check;

static void
ok_cb(GtkWidget *widget, Plugin **p)
{
  (*p)->show_in_menu = GTK_TOGGLE_BUTTON(show_in_menu_check)->active;

  gtk_widget_destroy(dialog);
  gtk_main_quit();
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
create_plugin_info_dialog(Plugin **p)
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

  table = add_table(dialog_vbox, 4, 2, FALSE, FALSE, 0);
  gtk_table_set_row_spacings(GTK_TABLE(table), 5);

  add_label_to_table(table, _("Name: "), 0.0, 0, 1, 0, 1);
  add_label_to_table(table, _("Description: "), 0.0, 0, 1, 1, 2);
  add_label_to_table(table, _("Filename: "), 0.0, 0, 1, 2, 3);
  add_label_to_table(table, (*p)->name, 0.0, 1, 2, 0, 1);
  add_label_to_table(table, (*p)->description, 0.0, 1, 2, 1, 2);
  add_label_to_table(table, (*p)->filename, 0.0, 1, 2, 2, 3);

  show_in_menu_check = add_check_button_to_table(table,
                       _("Show in popup menu?"),
                        (*p)->show_in_menu, NULL, NULL, 0, 2, 3, 4);

  add_button(action_area, _("Ok"), TRUE, 0, ok_cb, p);
  add_button(action_area, _("Cancel"), TRUE, 0, cancel_cb, NULL);

  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
  gtk_widget_show(dialog);
}


