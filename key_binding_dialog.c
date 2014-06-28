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
static GtkWidget *key_entry;
static GtkWidget *action_entry;
static GtkWidget *system_combo;
static GtkWidget *if_combo;
static GtkWidget *plugin_combo;
static GtkWidget *shell_radio_button;
static GtkWidget *system_radio_button;
static GtkWidget *if_radio_button;
static GtkWidget *plugin_radio_button;
static GtkWidget *ctrl_check;
static GtkWidget *alt_check;
static GtkWidget *shift_check;

static void
ok_cb(GtkWidget *widget, KeyBinding **kb)
{
  if (STREQ(gtk_entry_get_text(GTK_ENTRY(key_entry)), ""))
  {
    status_message("You must choose a key to bind\n");
    return;
  }

  if (*kb == NULL)
    *kb = g_new0(KeyBinding, 1);

  copy_entry_to_str(key_entry, (*kb)->key_name);
  (*kb)->keyval = gdk_keyval_from_name((*kb)->key_name);

  (*kb)->state = 0;
  if (GTK_TOGGLE_BUTTON(ctrl_check)->active)
    (*kb)->state |= GDK_CONTROL_MASK;
  if (GTK_TOGGLE_BUTTON(alt_check)->active)
    (*kb)->state |= GDK_MOD1_MASK;
  if (GTK_TOGGLE_BUTTON(shift_check)->active)
    (*kb)->state |= GDK_SHIFT_MASK;

  if (GTK_TOGGLE_BUTTON(shell_radio_button)->active)
  {
    copy_entry_to_str(action_entry, (*kb)->action);
  }
  else if (GTK_TOGGLE_BUTTON(system_radio_button)->active)
  {
    g_snprintf((*kb)->action, sizeof((*kb)->action), "SYSTEM_%s",
            gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(system_combo)->entry)));
  }
  else if (GTK_TOGGLE_BUTTON(if_radio_button)->active)
  {
    g_snprintf((*kb)->action, sizeof((*kb)->action), "INTERFACE:%s",
            gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(if_combo)->entry)));
  }
  else
  {
    g_snprintf((*kb)->action, sizeof((*kb)->action), "PLUGIN:%s",
            gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(plugin_combo)->entry)));
  }

  gtk_widget_destroy(dialog);
  gtk_main_quit();
}

static void
key_entry_cb(GtkWidget *entry, GdkEventKey *key)
{
  gtk_entry_set_text(GTK_ENTRY(entry), get_key_name(key->keyval));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctrl_check),
                               key->state & GDK_CONTROL_MASK);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(alt_check),
                               key->state & GDK_MOD1_MASK);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(shift_check),
                               key->state & GDK_SHIFT_MASK);
  gtk_signal_emit_stop_by_name(GTK_OBJECT(entry), "key_press_event");
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
  gtk_widget_set_sensitive(if_combo, FALSE);
  gtk_widget_set_sensitive(plugin_combo, FALSE);
  gtk_widget_grab_focus(action_entry);
}

static void
system_radio_cb(GtkWidget *widget, gpointer data)
{
  gtk_widget_set_sensitive(action_entry, FALSE);
  gtk_widget_set_sensitive(system_combo, TRUE);
  gtk_widget_set_sensitive(if_combo, FALSE);
  gtk_widget_set_sensitive(plugin_combo, FALSE);
  gtk_widget_grab_focus(GTK_COMBO(system_combo)->entry);
}

static void
if_radio_cb(GtkWidget *widget, gpointer data)
{
  gtk_widget_set_sensitive(action_entry, FALSE);
  gtk_widget_set_sensitive(system_combo, FALSE);
  gtk_widget_set_sensitive(if_combo, TRUE);
  gtk_widget_set_sensitive(plugin_combo, FALSE);
  gtk_widget_grab_focus(GTK_COMBO(if_combo)->entry);
}

static void
plugin_radio_cb(GtkWidget *widget, gpointer data)
{
  gtk_widget_set_sensitive(action_entry, FALSE);
  gtk_widget_set_sensitive(system_combo, FALSE);
  gtk_widget_set_sensitive(if_combo, FALSE);
  gtk_widget_set_sensitive(plugin_combo, TRUE);
  gtk_widget_grab_focus(GTK_COMBO(plugin_combo)->entry);
}

void
create_key_binding_dialog(KeyBinding **kb)
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

  table = add_framed_table(dialog_vbox, _("Key: "), 2, 3, FALSE, 10);
  gtk_table_set_row_spacings(GTK_TABLE(table), 5);
  key_entry = add_entry_to_table(table, "", 0, 3, 0, 1);
  gtk_signal_connect(GTK_OBJECT(key_entry), "key_press_event",
                     GTK_SIGNAL_FUNC(key_entry_cb), NULL);
  ctrl_check = add_check_button_to_table(table, _("Ctrl"), FALSE, NULL, NULL,
                                         0, 1, 1, 2);
  alt_check = add_check_button_to_table(table, _("Alt"), FALSE,NULL, NULL,
                                        1, 2, 1, 2);
  shift_check = add_check_button_to_table(table, _("Shift"), FALSE, NULL, NULL,
                                          2, 3, 1, 2);

  table = add_framed_table(dialog_vbox, _("Action: "), 4, 2, FALSE, 0);
  gtk_table_set_row_spacings(GTK_TABLE(table), 5);
  shell_radio_button = gtk_radio_button_new_with_label(NULL, _("Command: "));
  system_radio_button = gtk_radio_button_new_with_label(
               gtk_radio_button_group(GTK_RADIO_BUTTON(shell_radio_button)),
               _("System: "));
  if_radio_button = gtk_radio_button_new_with_label(
               gtk_radio_button_group(GTK_RADIO_BUTTON(shell_radio_button)),
               _("Interface: "));
  plugin_radio_button = gtk_radio_button_new_with_label(
               gtk_radio_button_group(GTK_RADIO_BUTTON(shell_radio_button)),
               _("Plugin: "));

  gtk_signal_connect(GTK_OBJECT(shell_radio_button), "clicked",
                     GTK_SIGNAL_FUNC(shell_radio_cb), NULL);
  gtk_signal_connect(GTK_OBJECT(system_radio_button), "clicked",
                     GTK_SIGNAL_FUNC(system_radio_cb), NULL);
  gtk_signal_connect(GTK_OBJECT(if_radio_button), "clicked",
                     GTK_SIGNAL_FUNC(if_radio_cb), NULL);
  gtk_signal_connect(GTK_OBJECT(plugin_radio_button), "clicked",
                     GTK_SIGNAL_FUNC(plugin_radio_cb), NULL);

  gtk_widget_show(shell_radio_button);
  gtk_widget_show(system_radio_button);
  gtk_widget_show(if_radio_button);
  gtk_widget_show(plugin_radio_button);

  gtk_table_attach_defaults(GTK_TABLE(table), shell_radio_button, 0, 1, 0, 1);
  gtk_table_attach_defaults(GTK_TABLE(table), system_radio_button, 0, 1, 1, 2);
  gtk_table_attach_defaults(GTK_TABLE(table), if_radio_button, 0, 1, 2, 3);
  gtk_table_attach_defaults(GTK_TABLE(table), plugin_radio_button, 0, 1, 3, 4);

  action_entry = add_entry_to_table(table, "", 1, 2, 0, 1);

  system_combo = gtk_combo_new();
  gtk_table_attach_defaults(GTK_TABLE(table), system_combo, 1, 2, 1, 2);
  gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(system_combo)->entry), FALSE);
  gtk_widget_show(system_combo);

  for (i = 0; i < n_sys_ops; i++)         /* + 7 to skip "SYSTEM_" */
    tmp = g_list_append(tmp, g_strdup(system_ops[i].name + 7)); 

  gtk_combo_set_popdown_strings(GTK_COMBO(system_combo), tmp);
  free_glist_data(&tmp);

  if_combo = gtk_combo_new();
  gtk_table_attach_defaults(GTK_TABLE(table), if_combo, 1, 2, 2, 3);
  gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(if_combo)->entry), FALSE);
  gtk_widget_show(if_combo);

  for (i = 0; i < n_interface_ops; i++)
    tmp = g_list_append(tmp, g_strdup(interface_ops[i].name + 10));

  gtk_combo_set_popdown_strings(GTK_COMBO(if_combo), tmp);
  free_glist_data(&tmp);

  plugin_combo = gtk_combo_new();
  gtk_table_attach_defaults(GTK_TABLE(table), plugin_combo, 1, 2, 3, 4);
  gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(plugin_combo)->entry), FALSE);
  gtk_widget_show(plugin_combo);

  for (tmp2 = cfg.plugins; tmp2 != NULL; tmp2 = tmp2->next)
  {
    Plugin *p = tmp2->data;
    tmp = g_list_append(tmp, g_strdup(p->name));
  }
  gtk_combo_set_popdown_strings(GTK_COMBO(plugin_combo), tmp);
  free_glist_data(&tmp);
  tmp = NULL;

  gtk_widget_set_sensitive(action_entry, FALSE);
  gtk_widget_set_sensitive(system_combo, FALSE);
  gtk_widget_set_sensitive(if_combo, FALSE);
  gtk_widget_set_sensitive(plugin_combo, FALSE);

  if (*kb != NULL)
  {
    gtk_entry_set_text(GTK_ENTRY(key_entry), (*kb)->key_name);
    if ((*kb)->state & GDK_CONTROL_MASK)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ctrl_check), TRUE);
    if ((*kb)->state & GDK_MOD1_MASK)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(alt_check), TRUE);
    if ((*kb)->state & GDK_SHIFT_MASK)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(shift_check), TRUE);
      
    if (strncmp((*kb)->action, "SYSTEM_", 7) == 0)
    {
      gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(system_combo)->entry),
                        (*kb)->action+7);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(system_radio_button), TRUE);
      gtk_widget_set_sensitive(system_combo, TRUE);
      gtk_widget_grab_focus(GTK_COMBO(system_combo)->entry);
    }
    else if (strncmp((*kb)->action, "INTERFACE:", 10) == 0)
    {
      gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(if_combo)->entry), (*kb)->action+10);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(if_radio_button), TRUE);
      gtk_widget_set_sensitive(if_combo, TRUE);
      gtk_widget_grab_focus(GTK_COMBO(system_combo)->entry);
    }
    else if (strncmp((*kb)->action, "PLUGIN:", 7) == 0)
    {
      gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(plugin_combo)->entry),
                        (*kb)->action+7);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(plugin_radio_button), TRUE);
      gtk_widget_set_sensitive(plugin_combo, TRUE);
      gtk_widget_grab_focus(GTK_COMBO(plugin_combo)->entry);
    }
    else
    {
      gtk_entry_set_text(GTK_ENTRY(action_entry), (*kb)->action);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(shell_radio_button), TRUE);
      gtk_widget_set_sensitive(action_entry, TRUE);
      gtk_widget_grab_focus(action_entry);
    }
  }
  else
  {
    gtk_widget_set_sensitive(action_entry, TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(shell_radio_button), TRUE);
  }

  gtk_signal_connect(GTK_OBJECT(action_entry), "activate", 
                     GTK_SIGNAL_FUNC(ok_cb), kb);
  gtk_widget_grab_focus(key_entry);

  add_button(action_area, _("Ok"), TRUE, 0, ok_cb, kb);
  add_button(action_area, _("Info"), TRUE, 0, help_cb, NULL);
  add_button(action_area, _("Cancel"), TRUE, 0, cancel_cb, kb);

  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
  gtk_widget_show(dialog);
}


