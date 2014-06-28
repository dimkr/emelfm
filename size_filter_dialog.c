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
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "emelfm.h"

static GtkWidget *dialog;
static GtkWidget *operation_combo;
static GtkWidget *size_entry;
static GtkWidget *units_combo;

static void
ok_cb(GtkWidget *widget, FileView *view)
{
  gchar *s;
  double size;

  view->size_filter.active = TRUE;
  set_filter_menu_active(view);

  s = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(operation_combo)->entry));
  if (STREQ(s, _("Bigger Than")))
    view->size_filter.op = GT;
  else
    view->size_filter.op = LT;

  s = gtk_entry_get_text(GTK_ENTRY(size_entry));
  size = atof(s);

  s = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(units_combo)->entry));
  if (STREQ(s, _("Bytes")))
    view->size_filter.size = (off_t)size;
  else if (STREQ(s, _("KBytes")))
    view->size_filter.size = (off_t)(size * (1 << 10));
  else
    view->size_filter.size = (off_t)(size * (1 << 20));

  gtk_widget_set_sensitive(app.main_window, TRUE);
  gtk_widget_destroy(dialog);
  refresh_list(view);
}

static void
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
create_size_filter_dialog(FileView *view)
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *hbox;
  gchar size_string[32];
  GList *tmp = NULL;

  dialog = gtk_dialog_new();
  dialog_vbox = GTK_DIALOG(dialog)->vbox;
  action_area = GTK_DIALOG(dialog)->action_area;
  gtk_container_set_border_width(GTK_CONTAINER(dialog_vbox), 5);
  gtk_box_set_spacing(GTK_BOX(dialog_vbox), 5);
  gtk_signal_connect(GTK_OBJECT(dialog), "key_press_event",
                     GTK_SIGNAL_FUNC(key_press_cb), view);

  add_label(dialog_vbox, _("Size Filter: "), 0.0, FALSE, 5);

  hbox = add_hbox(dialog_vbox, FALSE, 5, TRUE, 5);
  
  operation_combo = gtk_combo_new();
  gtk_widget_set_usize(operation_combo, 110, 0);
  gtk_box_pack_start(GTK_BOX(hbox), operation_combo, FALSE, FALSE, 0);
  gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(operation_combo)->entry), FALSE);
  gtk_widget_show(operation_combo);

  tmp = g_list_append(tmp, g_strdup(_("Bigger Than")));
  tmp = g_list_append(tmp, g_strdup(_("Smaller Than")));
  gtk_combo_set_popdown_strings(GTK_COMBO(operation_combo), tmp);
  free_glist_data(&tmp);
  tmp = NULL;

  switch (view->size_filter.op)
  {
    case GT:
      gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(operation_combo)->entry),
                         _("Bigger Than"));
      break;
    case LT:
      gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(operation_combo)->entry),
                         _("Smaller Than"));
      break;
    default:
      gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(operation_combo)->entry),
                         _("Bigger Than"));
      break;
  }

  size_entry = add_entry(hbox, "", FALSE, 0);
  gtk_widget_set_usize(size_entry, 60, 0);

  units_combo = gtk_combo_new();
  gtk_widget_set_usize(units_combo, 80, 0);
  gtk_box_pack_start(GTK_BOX(hbox), units_combo, FALSE, FALSE, 0);
  gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(units_combo)->entry), FALSE);
  gtk_widget_show(units_combo);

  tmp = g_list_append(tmp, g_strdup(_("Bytes")));
  tmp = g_list_append(tmp, g_strdup(_("KBytes")));
  tmp = g_list_append(tmp, g_strdup(_("MBytes")));
  gtk_combo_set_popdown_strings(GTK_COMBO(units_combo), tmp);
  free_glist_data(&tmp);
  tmp = NULL;

  if (view->size_filter.size < (1 << 10))
  {
    g_snprintf(size_string, sizeof(size_string), "%ld", view->size_filter.size);
    gtk_entry_set_text(GTK_ENTRY(size_entry), size_string);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(units_combo)->entry), _("Bytes"));
  }
  else if (view->size_filter.size < (1 << 20))
  {
    g_snprintf(size_string, sizeof(size_string), "%.2f", (double)((double)view->size_filter.size / (double)(1 << 10)));
    gtk_entry_set_text(GTK_ENTRY(size_entry), size_string);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(units_combo)->entry), _("KBytes"));
  }
  else
  {
    g_snprintf(size_string, sizeof(size_string), "%.2f", (double)((double)view->size_filter.size / (double)(1 << 20)));
    gtk_entry_set_text(GTK_ENTRY(size_entry), size_string);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(units_combo)->entry), _("MBytes"));
  }

  add_button(action_area, _("Ok"), FALSE, 0, ok_cb, view);
  add_button(action_area, _("Cancel"), FALSE, 0, cancel_cb, view);

  gtk_widget_show(dialog);
  gtk_widget_set_sensitive(app.main_window, FALSE);
}


