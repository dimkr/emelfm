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

static void
confirm_cb(GtkWidget *widget, guint *answer)
{
  if (answer != NULL)
    *answer = GPOINTER_TO_UINT(gtk_object_get_user_data(GTK_OBJECT(widget)));

  gtk_widget_destroy(dialog);
  gtk_main_quit();
}

static void
delete_event_cb(GtkWidget *widget)
{
  /* this is just here so the user can't close the dialog without clicking
   * one of the buttons
   */
}

static void
key_press_cb(GtkWidget    *widget,
             GdkEventKey  *event,
             guint        *answer)
{
  if (event->keyval == GDK_Escape)
  {
    if (answer != NULL)
      *answer = CANCEL;
    gtk_widget_destroy(dialog);
    gtk_main_quit();
  }
}

void
create_confirm_dialog(gchar *label_text, guint *answer, guint buttons)
{
  GtkWidget *button;
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;

  dialog = gtk_dialog_new();
  dialog_vbox = GTK_DIALOG(dialog)->vbox;
  action_area = GTK_DIALOG(dialog)->action_area;
  gtk_container_set_border_width(GTK_CONTAINER(dialog_vbox), 5);
  gtk_signal_connect(GTK_OBJECT(dialog), "delete_event",
                     GTK_SIGNAL_FUNC(delete_event_cb), NULL);
  gtk_signal_connect(GTK_OBJECT(dialog), "key_press_event",
                     GTK_SIGNAL_FUNC(key_press_cb), answer);
  
  add_label(dialog_vbox, label_text, 0, TRUE, 5);

  if (buttons & YES)
  {
    button = add_button(action_area, _("Yes"), TRUE, 0, confirm_cb, answer);
    gtk_object_set_user_data(GTK_OBJECT(button), GUINT_TO_POINTER(YES));
    gtk_widget_grab_focus(button);
  }

  if (buttons & OK)
  {
    button = add_button(action_area, _("Ok"), TRUE, 0, confirm_cb, answer);
    gtk_object_set_user_data(GTK_OBJECT(button), GUINT_TO_POINTER(OK));
    gtk_widget_grab_focus(button);
  }

  if (buttons & YES_TO_ALL)
  {
    button = add_button(action_area, _("Yes To All"), TRUE, 0, confirm_cb,
                        answer);
    gtk_object_set_user_data(GTK_OBJECT(button), GUINT_TO_POINTER(YES_TO_ALL));
  }

  if (buttons & APPLY_TO_ALL)
  {
    button = add_button(action_area, _("Apply To All"), TRUE, 0, confirm_cb,
                        answer);
    gtk_object_set_user_data(GTK_OBJECT(button), GUINT_TO_POINTER(APPLY_TO_ALL));
  }

  if (buttons & NO)
  {
    button = add_button(action_area, _("No"), TRUE, 0, confirm_cb, answer);
    gtk_object_set_user_data(GTK_OBJECT(button), GUINT_TO_POINTER(NO));
  }

  if (buttons & CANCEL)
  {
    button = add_button(action_area, _("Cancel"), TRUE, 0, confirm_cb, answer);
    gtk_object_set_user_data(GTK_OBJECT(button), GUINT_TO_POINTER(CANCEL));
  }

  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_widget_show(dialog);
}

void
create_confirm_del_dialog(gchar *filename, guint *answer)
{
  gchar label_text[MAX_LEN];

  g_snprintf(label_text, sizeof(label_text),
             _("Are you sure you want to delete: %s"), filename);
  create_confirm_dialog(label_text, answer, (YES | YES_TO_ALL | NO | CANCEL));
}

void
create_confirm_overwrite_dialog(gchar *filename, guint *answer)
{
  gchar label_text[MAX_LEN];

  g_snprintf(label_text, sizeof(label_text),
             _("%s already exists! Overwrite this file?"), filename);
  create_confirm_dialog(label_text, answer, (YES | YES_TO_ALL | NO | CANCEL));
}

