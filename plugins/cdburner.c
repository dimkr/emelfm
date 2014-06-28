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
#include "../emelfm.h"

static GtkWidget *dialog;
static GtkWidget *scsi_info_entry;
static GtkWidget *disktype_option;
static GtkWidget *audio_source_option;
static GtkWidget *session_option;
static GtkWidget *speed_option;
static GtkWidget *file_clist;

enum {AUDIO, ROCK_RIDGE, JOLIET};
enum {CD, WAV, MP3};
enum {SINGLE, MULTI_START, MULTI_CONTINUE, MULTI_CLOSE};
static gint disktype;
static gint audio_source;
static gint session;
static gint speed;

static void
burn_cb(GtkWidget *widget)
{
  gchar *s[4] = {"single session", "multi-session start",
                 "multi-session continue", "multi-session close"};
  gchar *dt[3] = {"audio", "rock ridge", "joliet"};
  gchar *source[3] = {"another cd", "wav's", "mp3's"};
  printf("Burning an %s %s cd from %s at %dx Speed\n", s[session], dt[disktype],
         (disktype == AUDIO ? source[audio_source] : "files"), speed);

  gtk_widget_set_sensitive(app.main_window, TRUE);
  gtk_widget_grab_focus(curr_view->clist);
  gtk_widget_destroy(dialog);
  gtk_main_quit();
}

static void
probe_cb(GtkWidget *widget)
{
  exec_in_xterm("cdrecord -scanbus | less");
}

static void
disable_sound_option(GtkWidget *widget, gpointer data)
{
  gtk_widget_set_sensitive(audio_source_option, FALSE);
  disktype = GPOINTER_TO_INT(data);
}

static void
enable_sound_option(GtkWidget *widget, gpointer data)
{
  gtk_widget_set_sensitive(audio_source_option, TRUE);
  disktype = GPOINTER_TO_INT(data);
}

static void
set_audio_source(GtkWidget *widget, gpointer data)
{
  audio_source = GPOINTER_TO_INT(data);
}

static void
set_session(GtkWidget *widget, gpointer data)
{
  session = GPOINTER_TO_INT(data);
}

static void
set_speed(GtkWidget *widget, gpointer data)
{
  speed = GPOINTER_TO_INT(data);
}

static void
cancel_cb(GtkWidget *widget)
{
  gtk_widget_set_sensitive(app.main_window, TRUE);
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
cdburner_dialog()
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *sw;
  GtkWidget *table;
  GtkWidget *menu;
  GList *base, *tmp;
  
  dialog = gtk_dialog_new();
  dialog_vbox = GTK_DIALOG(dialog)->vbox;
  action_area = GTK_DIALOG(dialog)->action_area;
  gtk_container_set_border_width(GTK_CONTAINER(dialog_vbox), 5);
  gtk_box_set_spacing(GTK_BOX(dialog_vbox), 5);
  gtk_signal_connect(GTK_OBJECT(dialog), "key_press_event",
                     GTK_SIGNAL_FUNC(key_press_cb), NULL);

  sw = add_sw(dialog_vbox, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC, TRUE, 5);
  gtk_widget_set_usize(sw, 0, 200);

  file_clist = gtk_clist_new(1);
  gtk_clist_column_titles_hide(GTK_CLIST(file_clist));
  gtk_clist_set_reorderable(GTK_CLIST(file_clist), TRUE);
  gtk_clist_set_selection_mode(GTK_CLIST(file_clist), GTK_SELECTION_BROWSE);
  gtk_clist_set_column_width(GTK_CLIST(file_clist), 0, 30);
  gtk_container_add(GTK_CONTAINER(sw), file_clist);
  gtk_widget_show(file_clist);

  base = tmp = get_selection(curr_view);
  for (; tmp != NULL; tmp = tmp->next)
  {
    gchar *buf[1] = { ((FileInfo *)tmp->data)->filename };
    gtk_clist_append(GTK_CLIST(file_clist), buf);
  }
  g_list_free(base);

  table = add_framed_table(dialog_vbox, _("CD Writer: "), 2, 3, FALSE, 0);
  add_label_to_table(table, _("id,bus,lun: "), 1.0, 0, 1, 0, 1);
  scsi_info_entry = add_entry_to_table(table, "0,0,0", 1, 2, 0, 1);
  add_button_to_table(table, _("Probe"), probe_cb, NULL, 2, 3, 0, 1);
  add_label_to_table(table, _("Speed: "), 1.0, 0, 1, 1, 2);
  speed_option = gtk_option_menu_new();
  gtk_table_attach(GTK_TABLE(table), speed_option, 1, 2, 1, 2,
                   0, 0, 0, 0);
  gtk_widget_show(speed_option);
  menu = gtk_menu_new();
  add_menu_item(menu, "1", set_speed, GINT_TO_POINTER(1));
  add_menu_item(menu, "2", set_speed, GINT_TO_POINTER(2));
  add_menu_item(menu, "4", set_speed, GINT_TO_POINTER(4));
  add_menu_item(menu, "6", set_speed, GINT_TO_POINTER(6));
  add_menu_item(menu, "8", set_speed, GINT_TO_POINTER(8));
  gtk_option_menu_set_menu(GTK_OPTION_MENU(speed_option), menu);
  speed = 1;

  table = add_framed_table(dialog_vbox, _("Burn Options: "), 3, 2, FALSE, 0);
  add_label_to_table(table, _("Disk Type: "), 1.0, 0, 1, 0, 1);
  disktype_option = gtk_option_menu_new();
  gtk_table_attach_defaults(GTK_TABLE(table), disktype_option, 1, 2, 0, 1);
  gtk_widget_show(disktype_option);
  menu = gtk_menu_new();
  add_menu_item(menu, _("Audio CD"), enable_sound_option, 
                GINT_TO_POINTER(AUDIO));
  add_menu_item(menu, _("RockRidge Data CD"), disable_sound_option, 
                GINT_TO_POINTER(ROCK_RIDGE));
  add_menu_item(menu, _("Joliet (Windows) Data CD"), disable_sound_option, 
                GINT_TO_POINTER(JOLIET));
  gtk_option_menu_set_menu(GTK_OPTION_MENU(disktype_option), menu);
  disktype = AUDIO;

  add_label_to_table(table, _("Audio Data Source: "), 1.0, 0, 1, 1, 2);
  audio_source_option = gtk_option_menu_new();
  gtk_table_attach_defaults(GTK_TABLE(table), audio_source_option, 1, 2, 1, 2);
  gtk_widget_show(audio_source_option);
  menu = gtk_menu_new();
  add_menu_item(menu, _("Wav Files"), set_audio_source, GINT_TO_POINTER(WAV));
  add_menu_item(menu, _("MP3 Files"), set_audio_source, GINT_TO_POINTER(MP3));
  add_menu_item(menu, _("CD-ROM"), set_audio_source, GINT_TO_POINTER(CD));
  gtk_option_menu_set_menu(GTK_OPTION_MENU(audio_source_option), menu);
  audio_source = WAV;

  add_label_to_table(table, _("Session Type: "), 1.0, 0, 1, 2, 3);
  session_option = gtk_option_menu_new();
  gtk_table_attach_defaults(GTK_TABLE(table), session_option, 1, 2, 2, 3);
  gtk_widget_show(session_option);
  menu = gtk_menu_new();
  add_menu_item(menu, _("Single Session (Close Disk)"), set_session, 
                GINT_TO_POINTER(SINGLE));
  add_menu_item(menu, _("Start Multisession Disk"), set_session, 
                GINT_TO_POINTER(MULTI_START));
  add_menu_item(menu, _("Continue Multisession Disk"), set_session, 
                GINT_TO_POINTER(MULTI_CONTINUE));
  add_menu_item(menu, _("Close Multisession Disk"), set_session, 
                GINT_TO_POINTER(MULTI_CLOSE));
  gtk_option_menu_set_menu(GTK_OPTION_MENU(session_option), menu);
  session = SINGLE;

  add_button(action_area, _("Burn"), TRUE, 0, burn_cb, NULL);
  add_button(action_area, _("Cancel"), TRUE, 0, cancel_cb, NULL);

  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
  gtk_widget_show(dialog);
  gtk_widget_set_sensitive(app.main_window, FALSE);
  gtk_main();
}

gint
init_plugin(Plugin *p)
{
  p->name = "CD-Burner";
  p->description = "A CD Burning plugin.";
  p->plugin_cb = cdburner_dialog;

  return TRUE;
}


