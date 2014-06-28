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
#include "../emelfm.h"

static GtkWidget *dialog;
static GtkWidget *pattern_entry;
static GtkWidget *invert_check;
static GtkWidget *case_sensitive_check;

static void
ok_cb(GtkWidget *widget)
{
  gchar *pattern, *s, *free_this;
  GList *patterns = NULL;
  gint i;
  gboolean invert, case_sensitive;

  if ((s = gtk_entry_get_text(GTK_ENTRY(pattern_entry))) == NULL)
    return;

  invert = GTK_TOGGLE_BUTTON(invert_check)->active;
  case_sensitive = GTK_TOGGLE_BUTTON(case_sensitive_check)->active;

  free_this = pattern = g_strdup(s);
  if (!case_sensitive)
    g_strdown(pattern);
  while ((s = strchr(pattern, ',')) != NULL)
  {
    *s = '\0';
    patterns = g_list_append(patterns, pattern);
    pattern = s+1;
  }
  patterns = g_list_append(patterns, pattern);

  gtk_clist_unselect_all(GTK_CLIST(curr_view->clist));
  for (i = 0; i < GTK_CLIST(curr_view->clist)->rows; i++)
  {
    FileInfo *info = gtk_clist_get_row_data(GTK_CLIST(curr_view->clist), i);
    GList *tmp;

    for (tmp = patterns; tmp != NULL; tmp = tmp->next)
    {
      pattern = tmp->data;
      if (!case_sensitive)
      {
        gchar *dup = g_strdup(info->filename);
        g_strdown(dup);
        if (gtk_pattern_match_simple(pattern, dup))
        {
          if (!invert)
            gtk_clist_select_row(GTK_CLIST(curr_view->clist), i, 0);
        }
        else if (invert)
          gtk_clist_select_row(GTK_CLIST(curr_view->clist), i, 0);
        g_free(dup);
      }
      else if (gtk_pattern_match_simple(pattern, info->filename))
      {
        if (!invert)
          gtk_clist_select_row(GTK_CLIST(curr_view->clist), i, 0);
      }
      else if (invert)
        gtk_clist_select_row(GTK_CLIST(curr_view->clist), i, 0);
    }
  }

  g_list_free(patterns);
  g_free(free_this);
  gtk_widget_set_sensitive(app.main_window, TRUE);
  gtk_widget_grab_focus(curr_view->clist);
  gtk_widget_destroy(dialog);
  gtk_main_quit();
}

static void
cancel_cb(GtkWidget *widget)
{
  gtk_widget_set_sensitive(app.main_window, TRUE);
  gtk_widget_grab_focus(curr_view->clist);
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
glob_dialog()
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *table;
  GtkTooltips *tooltips;
  gint selected_row;
  
  dialog = gtk_dialog_new();
  dialog_vbox = GTK_DIALOG(dialog)->vbox;
  action_area = GTK_DIALOG(dialog)->action_area;
  gtk_container_set_border_width(GTK_CONTAINER(dialog_vbox), 5);
  gtk_box_set_spacing(GTK_BOX(dialog_vbox), 5);
  gtk_signal_connect(GTK_OBJECT(dialog), "key_press_event",
                     GTK_SIGNAL_FUNC(key_press_cb), NULL);
  tooltips = gtk_tooltips_new();

  add_label(dialog_vbox, _("Glob Select: "), 0.0, FALSE, 5);

  pattern_entry = add_entry(dialog_vbox, "*", FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(pattern_entry), "activate",
                     GTK_SIGNAL_FUNC(ok_cb), NULL);
  gtk_widget_grab_focus(pattern_entry);

  if (GTK_CLIST(curr_view->clist)->selection != NULL)
  {
    GString *text;
    gchar *filename, *s;
    selected_row = (gint)GTK_CLIST(curr_view->clist)->selection->data;
    gtk_clist_get_text(GTK_CLIST(curr_view->clist), selected_row, 0, &filename);
    if ((s = strrchr(filename, '.')) != NULL)
    {
      text = g_string_new(s);
      g_string_prepend_c(text, '*');
      gtk_entry_set_text(GTK_ENTRY(pattern_entry), text->str);
      gtk_editable_select_region(GTK_EDITABLE(pattern_entry), 0, -1);
      g_string_free(text, TRUE);
    }
  }
    
  add_label(dialog_vbox, _("Example: *.c,*.h"), 0.0, FALSE, 0);
  
  table = add_table(dialog_vbox, 1, 2, FALSE, FALSE, 0);
  invert_check = add_check_button_to_table(table, _("Invert"), FALSE,
                                           NULL, NULL, 0, 1, 0, 1);
  case_sensitive_check = add_check_button_to_table(table, _("Case Sensitive"),
                                           TRUE, NULL, NULL, 1, 2, 0, 1);
  gtk_tooltips_set_tip(GTK_TOOLTIPS(tooltips), invert_check,
                    _("Select files that DO NOT match the given mask"), NULL);

  add_button(action_area, _("Ok"), TRUE, 0, ok_cb, NULL);
  add_button(action_area, _("Cancel"), TRUE, 0, cancel_cb, NULL);

  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
  gtk_widget_show(dialog);
  gtk_widget_set_sensitive(app.main_window, FALSE);
  gtk_main();
}

gint
init_plugin(Plugin *p)
{
  p->name = "Glob";
  p->description = "Select filenames matching a pattern.";
  p->plugin_cb = glob_dialog;

  return TRUE;
}


