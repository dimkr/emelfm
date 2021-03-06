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
static GtkWidget *label_entry;
static GtkWidget *action_entry;
static GtkWidget *system_combo;
static GtkWidget *shell_radio_button;
static GtkWidget *system_radio_button;

static void
ok_cb(GtkWidget *widget, Button **button)
{
  if (*button == NULL)
    *button = g_new0(Button, 1);

  strncpy((*button)->label, gtk_entry_get_text(GTK_ENTRY(label_entry)),
          sizeof((*button)->label));
  if (GTK_TOGGLE_BUTTON(shell_radio_button)->active)
  {
    strncpy((*button)->action, gtk_entry_get_text(GTK_ENTRY(action_entry)),
            sizeof((*button)->action));
  }
  else if (GTK_TOGGLE_BUTTON(system_radio_button)->active)
  {
    g_snprintf((*button)->action, sizeof((*button)->action), "SYSTEM_%s",
            gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(system_combo)->entry)));
  }

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

static void
shell_radio_cb(GtkWidget *widget, gpointer data)
{
  gtk_widget_set_sensitive(action_entry, TRUE);
  gtk_widget_set_sensitive(system_combo, FALSE);
  gtk_widget_grab_focus(action_entry);
}

static void
system_radio_cb(GtkWidget *widget, gpointer data)
{
  gtk_widget_set_sensitive(action_entry, FALSE);
  gtk_widget_set_sensitive(system_combo, TRUE);
  gtk_widget_grab_focus(GTK_COMBO(system_combo)->entry);
}

void
create_button_dialog(Button **button)
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *table;
  GList *tmp = NULL, *tmp2 = NULL;
  int i;

  dialog = gtk_dialog_new();
  dialog_vbox = GTK_DIALOG(dialog)->vbox;
  action_area = GTK_DIALOG(dialog)->action_area;
  gtk_container_set_border_width(GTK_CONTAINER(dialog_vbox), 5);
  gtk_box_set_spacing(GTK_BOX(dialog_vbox), 5);
  gtk_signal_connect(GTK_OBJECT(dialog), "delete_event", delete_event_cb, NULL);
  gtk_signal_connect(GTK_OBJECT(dialog), "key_press_event",
                     GTK_SIGNAL_FUNC(key_press_cb), NULL);

  table = add_table(dialog_vbox, 1, 2, FALSE, FALSE, 10);
  gtk_table_set_row_spacings(GTK_TABLE(table), 5);
  label_entry = add_entry_to_table(table, "", 1, 2, 0, 1);
  add_label_to_table(table, _("Label: "), 0.0, 0, 1, 0, 1);

  table = add_framed_table(dialog_vbox, _("Action: "), 3, 2, FALSE, 0);
  gtk_table_set_row_spacings(GTK_TABLE(table), 5);
  shell_radio_button = add_radio_button_to_table(table, _("Command: "), NULL,
               FALSE, shell_radio_cb, NULL, 0, 1, 0, 1);
  system_radio_button = add_radio_button_to_table(table, _("System: "),
               gtk_radio_button_group(GTK_RADIO_BUTTON(shell_radio_button)),
               FALSE, system_radio_cb, NULL, 0, 1, 1, 2);

  action_entry = add_entry_to_table(table, "", 1, 2, 0, 1);

  system_combo = gtk_combo_new();
  gtk_table_attach_defaults(GTK_TABLE(table), system_combo, 1, 2, 1, 2);
  gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(system_combo)->entry), FALSE);
  gtk_widget_show(system_combo);

  for (i = 0; i < n_sys_ops; i++)         /* + 7 to skip "SYSTEM_" */
    tmp = g_list_append(tmp, g_strdup(system_ops[i].name + 7)); 

  gtk_combo_set_popdown_strings(GTK_COMBO(system_combo), tmp);
  free_glist_data(&tmp);
  tmp = NULL;

  if (*button != NULL)
  {
    gtk_entry_set_text(GTK_ENTRY(label_entry), (*button)->label);
    if (strncmp((*button)->action, "SYSTEM_", 7) == 0)
    {
      gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(system_combo)->entry),
                         (*button)->action + 7);
      gtk_widget_set_sensitive(action_entry, FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(system_radio_button), TRUE);
    }
    else
    {
      gtk_entry_set_text(GTK_ENTRY(action_entry), (*button)->action);
      gtk_widget_set_sensitive(system_combo, FALSE);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(shell_radio_button), TRUE);
    }
  }
  else
  {
    gtk_widget_set_sensitive(system_combo, FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(shell_radio_button), TRUE);
  }

  gtk_signal_connect(GTK_OBJECT(label_entry), "activate", 
                     GTK_SIGNAL_FUNC(ok_cb), button);
  gtk_signal_connect(GTK_OBJECT(action_entry), "activate", 
                     GTK_SIGNAL_FUNC(ok_cb), button);
  gtk_widget_grab_focus(label_entry);

  add_button(action_area, _("Ok"), TRUE, 0, ok_cb, button);
  add_button(action_area, _("Info"), TRUE, 0, help_cb, NULL);
  add_button(action_area, _("Cancel"), TRUE, 0, cancel_cb, NULL);

  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
  gtk_widget_show(dialog);
}


