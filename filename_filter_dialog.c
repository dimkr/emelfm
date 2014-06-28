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
static GtkWidget *pattern_entry;
static GtkWidget *invert_check;
static GtkWidget *case_sensitive_check;

static void
ok_cb(GtkWidget *widget, FileView *view)
{
  gchar *s;

  s = gtk_entry_get_text(GTK_ENTRY(pattern_entry));
  if (strlen(s) == 0)
  {
    status_message(_("Invalid filename pattern\n"));
    gtk_widget_set_sensitive(app.main_window, TRUE);
    gtk_widget_destroy(dialog);
    return;
  }

  view->filename_filter.active = TRUE;
  set_filter_menu_active(view);

  strncpy(view->filename_filter.pattern, s,
          sizeof(view->filename_filter.pattern));
  view->filename_filter.invert_mask = GTK_TOGGLE_BUTTON(invert_check)->active;
  view->filename_filter.case_sensitive = GTK_TOGGLE_BUTTON(case_sensitive_check)->active;

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
create_filename_filter_dialog(FileView *view)
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *table;
  GtkTooltips *tooltips;

  dialog = gtk_dialog_new();
  dialog_vbox = GTK_DIALOG(dialog)->vbox;
  action_area = GTK_DIALOG(dialog)->action_area;
  gtk_container_set_border_width(GTK_CONTAINER(dialog_vbox), 5);
  gtk_box_set_spacing(GTK_BOX(dialog_vbox), 5);
  gtk_signal_connect(GTK_OBJECT(dialog), "key_press_event",
                     GTK_SIGNAL_FUNC(key_press_cb), view);
  tooltips = gtk_tooltips_new();

  add_label(dialog_vbox, _("Filename Filter: "), 0.0, FALSE, 5);
  pattern_entry = add_entry(dialog_vbox, view->filename_filter.pattern,
                            FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(pattern_entry), "activate", 
                     GTK_SIGNAL_FUNC(ok_cb), view);
  gtk_widget_grab_focus(pattern_entry);

  add_label(dialog_vbox, _("Example: *.c,*.h"), 0.0, FALSE, 0);

  table = add_table(dialog_vbox, 1, 2, FALSE, FALSE, 0);
  invert_check = add_check_button_to_table(table, _("Invert"),
                                  view->filename_filter.invert_mask,
                                  NULL, NULL, 0, 1, 0, 1);
  case_sensitive_check = add_check_button_to_table(table, _("Case Sensitive"),
                                    view->filename_filter.case_sensitive,
                                    NULL, NULL, 1, 2, 0, 1);
  gtk_tooltips_set_tip(GTK_TOOLTIPS(tooltips), invert_check,
                       _("Show files that DO NOT match the given mask"), NULL);

  add_button(action_area, _("Ok"), TRUE, 0, ok_cb, view);
  add_button(action_area, _("Cancel"), TRUE, 0, cancel_cb, view);

  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
  gtk_widget_show(dialog);
  gtk_widget_set_sensitive(app.main_window, FALSE);
}


