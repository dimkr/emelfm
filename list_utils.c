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

void
free_glist_data(GList **list)
{
  GList *tmp;

  for (tmp = *list; tmp != NULL; tmp = tmp->next)
  {
    if (tmp->data != NULL)
      g_free(tmp->data);
  }

  g_list_free(*list);
  *list = NULL;
}

void
free_clist_data(GtkWidget *clist)
{
  gint i;

  for (i = 0; i < GTK_CLIST(clist)->rows; i++)
    g_free(gtk_clist_get_row_data(GTK_CLIST(clist), i));
}

GList *
clist_data_to_glist(GtkWidget *clist)
{
  gint i;
  GList *list = NULL;

  for (i = 0; i < GTK_CLIST(clist)->rows; i++)
    list = g_list_append(list, gtk_clist_get_row_data(GTK_CLIST(clist), i));
  return list;
}

GList *
string_glist_find(GList *list, gchar *search_text)
{
  GList *tmp;
  gchar *curr;

  for (tmp = list; tmp != NULL; tmp = tmp->next)
  {
    curr = tmp->data;
    if (STREQ(curr, search_text)) return tmp;
  }
  return NULL;
}

void
clist_select_rows(GtkWidget *clist, GList *rows)
{
  GList *tmp;
  gint row;

  if (rows == NULL)
  {
    gtk_clist_select_row(GTK_CLIST(clist), 0, 0);
    GTK_CLIST(clist)->focus_row = 0;
  }
  else
  {
    for (tmp = rows; tmp != NULL; tmp = tmp->next)
    {
      row = (gint)tmp->data;
      if (row < GTK_CLIST(clist)->rows)
      {
        gtk_clist_select_row(GTK_CLIST(clist), row, 0);
        GTK_CLIST(clist)->focus_row = row;
      }
      else
      {
        gtk_clist_select_row(GTK_CLIST(clist), GTK_CLIST(clist)->rows-1, 0);
        GTK_CLIST(clist)->focus_row = GTK_CLIST(clist)->rows-1;
      }
    } 
  }
  gtk_widget_draw_focus(clist);
  gtk_widget_grab_focus(clist);
}

gboolean
clist_row_is_selected(GtkWidget *clist, gint row)
{
  GList *tmp;

  for (tmp = GTK_CLIST(clist)->selection; tmp != NULL; tmp = tmp->next)
    if (row == (gint)tmp->data)
      return TRUE;
  return FALSE;
}

