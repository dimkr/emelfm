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
#include "emelfm.h"

void
close_cb(GtkWidget *widget, GtkWidget *dialog)
{
  gtk_widget_destroy(dialog);
}

static void
key_press_cb(GtkWidget    *widget,
             GdkEventKey  *event,
             gpointer     data)
{
  if (event->keyval == GDK_Escape)
    close_cb(NULL, widget);
}

void
create_view_file_dialog(gchar *filename)
{
  FILE *f;
  GtkWidget *dialog;
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *table;
  GtkWidget *text_area;
  GtkWidget *scrollbar;
  gchar text[MAX_LEN];

  if ((f = fopen(filename, "r")) == NULL)
  {
    status_message(_("Unable to open file for reading: "));
    status_message(filename);
    status_message("\n");
    return;
  }

  dialog = gtk_dialog_new();
  dialog_vbox = GTK_DIALOG(dialog)->vbox;
  action_area = GTK_DIALOG(dialog)->action_area;

  gtk_window_set_title(GTK_WINDOW(dialog), filename);
  gtk_container_set_border_width(GTK_CONTAINER(dialog_vbox), 5);
  gtk_container_set_border_width(GTK_CONTAINER(action_area), 5);
  gtk_box_set_spacing(GTK_BOX(dialog_vbox), 5);
  gtk_box_set_homogeneous(GTK_BOX(action_area), TRUE);
  gtk_signal_connect(GTK_OBJECT(dialog), "key_press_event",
                     GTK_SIGNAL_FUNC(key_press_cb), NULL);
  
  table = gtk_table_new(1, 2, FALSE);
  gtk_box_pack_start(GTK_BOX(dialog_vbox), table, TRUE, TRUE, FALSE);
  gtk_widget_show(table);

  text_area = gtk_text_new(NULL, NULL);
  gtk_text_set_editable(GTK_TEXT(text_area), FALSE);
  gtk_text_set_word_wrap(GTK_TEXT(text_area), TRUE);
  gtk_table_attach(GTK_TABLE(table), text_area, 0, 1, 0, 1,
    GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0);
  gtk_widget_show(text_area);
  gtk_widget_grab_focus(text_area);

  scrollbar = gtk_vscrollbar_new(GTK_TEXT(text_area)->vadj);
  gtk_table_attach(GTK_TABLE(table), scrollbar, 1, 2, 0, 1,
    GTK_FILL, GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0);
  gtk_widget_show(scrollbar);

  add_button(action_area, _("Close"), FALSE, 5, close_cb, dialog);
  
  while (fgets(text, sizeof(text), f) != NULL)
    gtk_text_insert(GTK_TEXT(text_area), app.output_font, NULL, NULL, text, strlen(text));

  fclose(f);

  gtk_widget_show(dialog);
  if (app.output_font != NULL)
  {
    gtk_widget_set_usize(text_area,
      (gdk_char_width(app.output_font, 'z') * 80 + scrollbar->allocation.width),
      (gdk_char_height(app.output_font, 'z') * 50));
  }
  else
    gtk_widget_set_usize(text_area, 500, 400);
}


