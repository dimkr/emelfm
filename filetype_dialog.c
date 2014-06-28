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

static GtkWidget *_dialog;
static GtkWidget *_actions_list;
static GtkWidget *_desc_entry;
static GtkWidget *_ext_entry;
static gint _write_file;


static void
add_action_cb(GtkWidget *widget)
{
  gchar *new_name, *new_action;

  gtk_widget_set_sensitive(_dialog, FALSE);
  create_filetype_action_dialog("", "", &new_name, &new_action);
  gtk_main();

  if (new_action != NULL)
  {
    gchar *buf[2];
    
    if (STREQ(new_name, ""))
    {
      buf[0] = new_action;
      buf[1] = "";
    }
    else
    {
      buf[0] = new_name;
      buf[1] = new_action;
    }
    gtk_clist_append(GTK_CLIST(_actions_list), buf);

    g_free(new_name);
    g_free(new_action);
  }
  gtk_widget_set_sensitive(_dialog, TRUE);
}

static void
edit_action_cb(GtkWidget *widget)
{
  gint selected_row;
  gchar *col0_text, *col1_text;
  gchar *new_name, *new_action;

  if (GTK_CLIST(_actions_list)->selection != NULL)
    selected_row = (gint)GTK_CLIST(_actions_list)->selection->data;
  else
    return;

  if (selected_row < 0)
    return;

  gtk_widget_set_sensitive(_dialog, FALSE);
  gtk_clist_get_text(GTK_CLIST(_actions_list), selected_row, 0, &col0_text);
  gtk_clist_get_text(GTK_CLIST(_actions_list), selected_row, 1, &col1_text);
  if (STREQ(col1_text, ""))
    create_filetype_action_dialog("", col0_text, &new_name, &new_action);
  else
    create_filetype_action_dialog(col0_text, col1_text, &new_name, &new_action);
  gtk_main();

  if (new_action != NULL)
  {
    if (STREQ(new_name, ""))
    {
      gtk_clist_set_text(GTK_CLIST(_actions_list), selected_row, 0, new_action);
      gtk_clist_set_text(GTK_CLIST(_actions_list), selected_row, 1, "");
    }
    else
    {
      gtk_clist_set_text(GTK_CLIST(_actions_list), selected_row, 0, new_name);
      gtk_clist_set_text(GTK_CLIST(_actions_list), selected_row, 1, new_action);
    }
    g_free(new_action);
    g_free(new_name);
  }
  gtk_widget_set_sensitive(_dialog, TRUE);
}
  
static void
remove_action_cb(GtkWidget *widget)
{
  gint selected_row;
  
  if (GTK_CLIST(_actions_list)->selection != NULL)
    selected_row = (gint)GTK_CLIST(_actions_list)->selection->data;
  else
    return;

  if (selected_row < 0)
    return;

  gtk_clist_remove(GTK_CLIST(_actions_list), selected_row);
}

static void
set_default_action_cb(GtkWidget *widget)
{
  gint selected_row;
  
  if (GTK_CLIST(_actions_list)->selection != NULL)
    selected_row = (gint)GTK_CLIST(_actions_list)->selection->data;
  else
    return;

  if (selected_row == 0)
    return;

  gtk_clist_row_move(GTK_CLIST(_actions_list), selected_row, 0);
}

static void
done_cb(GtkWidget *widget, FileType **ft)
{
  gchar *description = gtk_entry_get_text(GTK_ENTRY(_desc_entry));
  gchar *extensions = gtk_entry_get_text(GTK_ENTRY(_ext_entry));
  gchar *col0_text, *col1_text;
  gint i;

  if (STREQ(extensions, ""))
  {
    status_message(_("Extensions field cannot be blank\n"));
    return;
  }
  if (STREQ(description, ""))
    description = extensions;
  if (GTK_CLIST(_actions_list)->rows < 1)
  {
    status_message(_("At least one action is required\n"));
    return;
  }

  if (*ft == NULL)
    *ft = g_new0(FileType, 1);

  strncpy((*ft)->description, description,
          sizeof((*ft)->description));
  strncpy((*ft)->extensions, extensions,
          sizeof((*ft)->extensions));
  strncpy((*ft)->actions, "", sizeof((*ft)->actions));

  for (i = 0; i < GTK_CLIST(_actions_list)->rows; i++)
  {
    gtk_clist_get_text(GTK_CLIST(_actions_list), i, 0, &col0_text);
    gtk_clist_get_text(GTK_CLIST(_actions_list), i, 1, &col1_text);
    strncat((*ft)->actions, col0_text, sizeof((*ft)->actions));

    if (!STREQ(col1_text, ""))
    {
      strncat((*ft)->actions, "@", sizeof((*ft)->actions));
      strncat((*ft)->actions, col1_text, sizeof((*ft)->actions));
    }

    if ((i+1) != GTK_CLIST(_actions_list)->rows) /* not after the last one */
      strncat((*ft)->actions, ",", sizeof((*ft)->actions));
  }

  if (_write_file)
  {
    write_filetypes_file();
    touch_config_dir();
  }

  gtk_widget_destroy(_dialog);
  gtk_main_quit();
}
  
static void
cancel_cb(GtkWidget *widget)
{
  gtk_widget_destroy(_dialog);
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

/* This is the filetype dialog with that lets you add, remove and change
 * actions and extensions.
 * ft: a FileType object to edit. This will already be in the filetypes list
 *     if we are editing an existing filetype. If NULL is passed as the
 *     filetype then we will have to create a new filetype and add it to the
 *     filetypes list.
 * write_file: tells us if we need to write the filetypes file or not.
 */
void
create_filetype_dialog(FileType **ft, gboolean write_file)
{
  GtkWidget *sw;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *table;
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  gchar *action, *s, *sep, *free_this;
  gchar *buf[2];
  
  _write_file = write_file;

  _dialog = gtk_dialog_new();
  gtk_widget_set_usize(_dialog, 250, 250);
  dialog_vbox = GTK_DIALOG(_dialog)->vbox;
  action_area = GTK_DIALOG(_dialog)->action_area;
  gtk_signal_connect(GTK_OBJECT(_dialog), "key_press_event",
                     GTK_SIGNAL_FUNC(key_press_cb), NULL);

  gtk_container_set_border_width(GTK_CONTAINER(dialog_vbox), 5);
  gtk_box_set_spacing(GTK_BOX(dialog_vbox), 5);
  gtk_container_set_border_width(GTK_CONTAINER(action_area), 5);

  table = add_table(dialog_vbox, 2, 2, FALSE, FALSE, 0);
  gtk_table_set_row_spacings(GTK_TABLE(table), 5);
  add_label_to_table(table, _("Description:"), 0.8, 0, 1, 0, 1);
  _desc_entry = add_entry_to_table(table, "", 1, 2, 0, 1);
  add_label_to_table(table, _("Extension(s):"), 0.8, 0, 1, 1, 2);
  _ext_entry = add_entry_to_table(table, "", 1, 2, 1, 2);

  if (*ft != NULL)
  {
    gtk_entry_set_text(GTK_ENTRY(_desc_entry), (*ft)->description);
    gtk_entry_set_text(GTK_ENTRY(_ext_entry), (*ft)->extensions);
  }

  vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 2);
  gtk_widget_show(vbox);

  sw = add_sw(vbox, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC, TRUE, 0);
  
  _actions_list = gtk_clist_new(2);
  gtk_clist_set_selection_mode(GTK_CLIST(_actions_list), GTK_SELECTION_BROWSE);
  gtk_clist_set_reorderable(GTK_CLIST(_actions_list), TRUE);
  gtk_clist_column_titles_passive(GTK_CLIST(_actions_list));
  gtk_clist_set_column_visibility(GTK_CLIST(_actions_list), 1, FALSE);
  gtk_container_add(GTK_CONTAINER(sw), _actions_list);
  gtk_widget_show(_actions_list);

  if ((*ft != NULL)
      && !STREQ((*ft)->actions, "")
      && ((s = g_strdup((*ft)->actions)) != NULL))
  {
    free_this = action = s;
    while ((s = strchr(action, ',')) != NULL)
    {
      *s++ = '\0';
      if ((sep = strchr(action, '@')) != NULL)
      {
        *sep++ = '\0';
        buf[1] = sep;
      }
      else
        buf[1] = "";

      buf[0] = action;
      gtk_clist_append(GTK_CLIST(_actions_list), buf);
      action = s;
    }
    if ((sep = strchr(action, '@')) != NULL)
    {
      *sep++ = '\0';
      buf[1] = sep;
    }
    else
      buf[1] = "";

    buf[0] = action;
    gtk_clist_append(GTK_CLIST(_actions_list), buf);
    g_free(free_this);
  }

  hbox = add_hbox(vbox, FALSE, 5, FALSE, 0);
  add_button(hbox, _("Add"), TRUE, 0, add_action_cb, NULL);
  add_button(hbox, _("Edit"), TRUE, 0, edit_action_cb, NULL);
  add_button(hbox, _("Remove"), TRUE, 0, remove_action_cb, NULL);
  add_button(hbox, _("Set Default"), TRUE, 0, set_default_action_cb, NULL);
  add_framed_widget(dialog_vbox, "Actions: ", vbox, TRUE, 0);
  gtk_widget_grab_focus(_desc_entry);

  add_button(action_area, _("Done"), TRUE, 0, done_cb, ft);
  add_button(action_area, _("Cancel"), TRUE, 0, cancel_cb, NULL);

  gtk_window_set_position(GTK_WINDOW(_dialog), GTK_WIN_POS_MOUSE);
  gtk_widget_show(_dialog);
}


