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
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GtkWidget *dialog;
static GtkWidget *date_entry;
static GtkWidget *operation_combo;

void
ok_cb(GtkWidget *widget, FileView *view)
{
  gchar date_string[MAX_LEN];
  gchar *s;
  struct tm tm_time;

  view->date_filter.active = TRUE;
  set_filter_menu_active(view);

  g_snprintf(date_string, sizeof(date_string),
             "%s 00:00:00",
             gtk_entry_get_text(GTK_ENTRY(date_entry)));
  strptime(date_string, "%m-%d-%Y %T", &tm_time);
  view->date_filter.time = mktime(&tm_time);

  s = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(operation_combo)->entry));
  if (STREQ(s, _("Accessed Since")))
  {
    view->date_filter.op = GT;
    view->date_filter.time_type = ATIME;
  }
  else if (STREQ(s, _("Accessed Before")))
  {
    view->date_filter.op = LT;
    view->date_filter.time_type = ATIME;
  }
  else if (STREQ(s, _("Modified Since")))
  {
    view->date_filter.op = GT;
    view->date_filter.time_type = MTIME;
  }
  else if (STREQ(s, _("Modified Before")))
  {
    view->date_filter.op = LT;
    view->date_filter.time_type = MTIME;
  }
  else if (STREQ(s, _("Changed Since")))
  {
    view->date_filter.op = GT;
    view->date_filter.time_type = CTIME;
  }
  else if (STREQ(s, _("Changed Before")))
  {
    view->date_filter.op = LT;
    view->date_filter.time_type = CTIME;
  }

  gtk_widget_set_sensitive(app.main_window, TRUE);
  gtk_widget_destroy(dialog);
  refresh_list(view);
}

void
cancel_cb(GtkWidget *widget, FileView *view)
{
  gtk_widget_set_sensitive(app.main_window, TRUE);
  gtk_widget_destroy(dialog);
  gtk_widget_grab_focus(view->clist);
}

static void
key_press_cb(GtkWidget    *widget,
             GdkEventKey  *event,
             FileView     *view)
{
  if (event->keyval == GDK_Escape)
    cancel_cb(NULL, view);
}

void
create_date_filter_dialog(FileView *view)
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *hbox;
  gchar date_string[11];
  struct tm *tm_ptr;
  GList *tmp = NULL;

  dialog = gtk_dialog_new();
  dialog_vbox = GTK_DIALOG(dialog)->vbox;
  action_area = GTK_DIALOG(dialog)->action_area;
  gtk_container_set_border_width(GTK_CONTAINER(dialog_vbox), 5);
  gtk_box_set_spacing(GTK_BOX(dialog_vbox), 5);
  gtk_signal_connect(GTK_OBJECT(dialog), "key_press_event",
                     GTK_SIGNAL_FUNC(key_press_cb), view);

  add_label(dialog_vbox, _("Date Filter: "), 0.0, FALSE, 5);

  hbox = add_hbox(dialog_vbox, FALSE, 5, TRUE, 5);

  operation_combo = gtk_combo_new();
  gtk_widget_set_usize(operation_combo, 130, 0);
  gtk_box_pack_start(GTK_BOX(hbox), operation_combo, FALSE, FALSE, 0);
  gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(operation_combo)->entry), FALSE);
  gtk_widget_show(operation_combo);

  tmp = g_list_append(tmp, g_strdup(_("Accessed Since")));
  tmp = g_list_append(tmp, g_strdup(_("Accessed Before")));
  tmp = g_list_append(tmp, g_strdup(_("Modified Since")));
  tmp = g_list_append(tmp, g_strdup(_("Modified Before")));
  tmp = g_list_append(tmp, g_strdup(_("Changed Since")));
  tmp = g_list_append(tmp, g_strdup(_("Changed Before")));
  gtk_combo_set_popdown_strings(GTK_COMBO(operation_combo), tmp);
  free_glist_data(&tmp);
  switch (view->date_filter.time_type)
  {
    case ATIME:
      if (view->date_filter.op == GT)
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(operation_combo)->entry),
                           _("Accessed Since"));
      else
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(operation_combo)->entry),
                           _("Accessed Before"));
      break;
    case MTIME:
      if (view->date_filter.op == GT)
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(operation_combo)->entry),
                           _("Modified Since"));
      else
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(operation_combo)->entry),
                           _("Modified Before"));
      break;
    case CTIME:
      if (view->date_filter.op == GT)
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(operation_combo)->entry),
                           _("Changed Since"));
      else
        gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(operation_combo)->entry),
                           _("Changed Before"));
      break;
    default:
      gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(operation_combo)->entry),
                         _("Accessed Since"));
      break;
  }

  tm_ptr = localtime(&(view->date_filter.time));
  strftime(date_string, sizeof(date_string), "%m-%d-%Y", tm_ptr);
  date_entry = add_entry(hbox, date_string, FALSE, 0);
  gtk_widget_set_usize(date_entry, 100, 0);

  add_button(action_area, _("Ok"), FALSE, 0, ok_cb, view);
  add_button(action_area, _("Cancel"), FALSE, 0, cancel_cb, view);

  gtk_widget_show(dialog);
  gtk_widget_set_sensitive(app.main_window, FALSE);
}


