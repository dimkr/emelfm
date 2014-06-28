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

#include <stdlib.h>
#include "emelfm.h"

void
updir_cb(GtkWidget *widget)
{
  change_dir(curr_view, "..");
}

void
cd_home(GtkWidget *widget)
{
  gchar *home;
  if ((home = getenv("HOME")) != NULL)
    change_dir(curr_view, home);
  else
    status_message(_("Environment variable 'HOME' not set."));
}

void
switch_views_cb(GtkWidget *widget)
{
  if ((GTK_CLIST(other_view->clist)->rows > 0))
  {
    FileView *view = other_view;
    clist_select_rows(view->clist, view->old_selection);
  }
  else
  {
    refresh_list(other_view);
    gtk_widget_grab_focus(curr_view->clist);
  }
}

void
toggle_left_panel_cb(GtkWidget *widget)
{
  toggle_panel_size(&app.left_view);
}

void
toggle_right_panel_cb(GtkWidget *widget)
{
  toggle_panel_size(&app.right_view);
}

void
toggle_hidden_cb(GtkWidget *widget)
{
  gtk_signal_emit_by_name(GTK_OBJECT(curr_view->hidden_toggle), "clicked");
}

void
toggle_output_window_cb(GtkWidget *widget)
{
  show_hide_output_text(NULL);
}

void
goto_command_line_cb(GtkWidget *widget)
{
  gtk_widget_grab_focus(GTK_COMBO(app.command_line)->entry);
  if (cfg.output_text_hidden)
    show_hide_output_text(NULL);
}

void
show_menu_cb(GtkWidget *widget)
{
  show_menu(0, 0);
}

void
show_user_menu_cb(GtkWidget *widget)
{
  show_user_command_menu(0, 0);
}

