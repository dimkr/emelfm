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

#include "icons/icon_up.xpm"
#include "icons/icon_down.xpm"

static GtkWidget *dialog;
static GtkWidget *colorsel_dialog;
static GtkWidget *fontsel_dialog;

static GList *added_plugins, *removed_plugins;

/* General Page 1 widgets */
static GtkWidget *confirm_del_check;
static GtkWidget *confirm_ow_check;
static GtkWidget *auto_refresh_check;
static GtkWidget *right_startup_dir_entry;
static GtkWidget *left_startup_dir_entry;
static GtkWidget *start_with_last_left_check;
static GtkWidget *start_with_last_right_check;

/* General Page 2 widgets */
static GtkWidget *use_internal_check;
static GtkWidget *viewer_entry;
static GtkWidget *xterm_entry;
static GtkWidget *dir_history_entry;
static GtkWidget *command_history_entry;

/* Interface Page widgets */
static GtkWidget *list_font_entry;
static GtkWidget *output_font_entry;
static GtkWidget *top_right_radio;
static GtkWidget *top_left_radio;
static GtkWidget *bot_right_radio;
static GtkWidget *bot_left_radio;
static GtkWidget *win_right_click_check;
static GtkWidget *expand_check;
static GtkWidget *use_vi_keys_check;

/* Clists */
static GtkWidget *bookmarks_clist;
static GtkWidget *filetypes_clist;
static GtkWidget *user_commands_clist;
static GtkWidget *toolbar_clist;
static GtkWidget *keys_clist;
static GtkWidget *buttons_clist;
static GtkWidget *plugins_clist;
static GtkWidget *file_selection;
static GtkWidget *column_checks[MAX_COLUMNS];


/* Config Dialog Callbacks */
static void
ok_cb(GtkWidget *button)
{
  gint i;
  GList *tmp;

  cfg.confirm_delete = GTK_TOGGLE_BUTTON(confirm_del_check)->active;
  cfg.confirm_overwrite = GTK_TOGGLE_BUTTON(confirm_ow_check)->active;
  cfg.start_with_last_dir_left = GTK_TOGGLE_BUTTON(start_with_last_left_check)->active;
  cfg.start_with_last_dir_right = GTK_TOGGLE_BUTTON(start_with_last_right_check)->active;
  cfg.use_internal_viewer = GTK_TOGGLE_BUTTON(use_internal_check)->active;
  strncpy(cfg.left_startup_dir,
          gtk_entry_get_text(GTK_ENTRY(left_startup_dir_entry)),
          sizeof(cfg.left_startup_dir));
  strncpy(cfg.right_startup_dir,
          gtk_entry_get_text(GTK_ENTRY(right_startup_dir_entry)),
          sizeof(cfg.right_startup_dir));
  strncpy(cfg.viewer_command, gtk_entry_get_text(GTK_ENTRY(viewer_entry)),
          sizeof(cfg.viewer_command));
  strncpy(cfg.xterm_command, gtk_entry_get_text(GTK_ENTRY(xterm_entry)),
          sizeof(cfg.xterm_command));
  strncpy(cfg.list_font,
          gtk_entry_get_text(GTK_ENTRY(list_font_entry)),
          sizeof(cfg.list_font));
  strncpy(cfg.output_font, gtk_entry_get_text(GTK_ENTRY(output_font_entry)),
          sizeof(cfg.output_font));
  cfg.dir_history_max_length = atoi(
             gtk_entry_get_text(GTK_ENTRY(dir_history_entry)));
  cfg.command_history_max_length = atoi(
         gtk_entry_get_text(GTK_ENTRY(command_history_entry)));

  cfg.auto_refresh_enabled = GTK_TOGGLE_BUTTON(auto_refresh_check)->active;
  if (cfg.auto_refresh_id != 0)
    gtk_timeout_remove(cfg.auto_refresh_id);
  if (cfg.auto_refresh_enabled)
    cfg.auto_refresh_id = gtk_timeout_add(1000, auto_refresh, NULL);

  cfg.expand_popup_menu = GTK_TOGGLE_BUTTON(expand_check)->active;
  cfg.use_vi_keys = GTK_TOGGLE_BUTTON(use_vi_keys_check)->active;
  cfg.windows_right_click = GTK_TOGGLE_BUTTON(win_right_click_check)->active;

  if (GTK_TOGGLE_BUTTON(top_left_radio)->active)
    cfg.scrollbar_pos = GTK_CORNER_BOTTOM_RIGHT;
  else if (GTK_TOGGLE_BUTTON(top_right_radio)->active)
    cfg.scrollbar_pos = GTK_CORNER_BOTTOM_LEFT;
  else if (GTK_TOGGLE_BUTTON(bot_left_radio)->active)
    cfg.scrollbar_pos = GTK_CORNER_TOP_RIGHT;
  else if (GTK_TOGGLE_BUTTON(bot_right_radio)->active)
    cfg.scrollbar_pos = GTK_CORNER_TOP_LEFT;

  free_glist_data(&cfg.bookmarks);
  cfg.bookmarks = clist_data_to_glist(bookmarks_clist);

  free_glist_data(&cfg.filetypes);
  cfg.filetypes = clist_data_to_glist(filetypes_clist);

  free_glist_data(&cfg.user_commands);
  cfg.user_commands = clist_data_to_glist(user_commands_clist);

  free_glist_data(&cfg.toolbar_buttons);
  cfg.toolbar_buttons = clist_data_to_glist(toolbar_clist);

  free_glist_data(&cfg.key_bindings);
  cfg.key_bindings = clist_data_to_glist(keys_clist);

  free_glist_data(&cfg.buttons);
  cfg.buttons = clist_data_to_glist(buttons_clist);

  free_glist_data(&cfg.plugins);
  cfg.plugins = clist_data_to_glist(plugins_clist);
  for (tmp = removed_plugins; tmp != NULL; tmp = tmp->next)
    destroy_plugin((Plugin*)tmp->data);
  g_list_free(removed_plugins);
  g_list_free(added_plugins);

  for (i = 0; i < MAX_COLUMNS; i++)
    all_columns[i].is_visible = GTK_TOGGLE_BUTTON(column_checks[i])->active;

  /* the dialog needs to be destroyed before we refresh the main window */
  gtk_widget_set_sensitive(app.main_window, TRUE);
  gtk_widget_destroy(dialog);

  recreate_main_window();

  write_filetypes_file();
  write_bookmarks_file();
  write_user_commands_file();
  write_toolbar_file();
  write_keys_file();
  write_buttons_file();
  write_plugins_file();
  write_config_file();
  touch_config_dir();
}

static void
cancel_cb(GtkWidget *button)
{
  GList *tmp;

  free_glist_data(&removed_plugins);
  for (tmp = added_plugins; tmp != NULL; tmp = tmp->next)
    destroy_plugin((Plugin*)tmp->data);
  g_list_free(added_plugins);

  free_clist_data(filetypes_clist);
  free_clist_data(buttons_clist);
  free_clist_data(toolbar_clist);
  free_clist_data(keys_clist);
  free_clist_data(user_commands_clist);

  gtk_widget_set_sensitive(app.main_window, TRUE);
  gtk_widget_destroy(dialog);
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

/*
 * General Page Callbacks
 */
static void
start_with_last_cb(GtkWidget *check)
{
  if (check == start_with_last_left_check)
    gtk_widget_set_sensitive(left_startup_dir_entry,
                             !GTK_TOGGLE_BUTTON(check)->active);
  if (check == start_with_last_right_check)
    gtk_widget_set_sensitive(right_startup_dir_entry,
                             !GTK_TOGGLE_BUTTON(check)->active);
}

static void
use_internal_cb(GtkWidget *check_button)
{
  gtk_widget_set_sensitive(viewer_entry,
                           !GTK_TOGGLE_BUTTON(check_button)->active);
}

/* 
 * FileType Page Callbacks
 */
static void 
add_filetype_cb(GtkWidget *button, GtkWidget *clist)
{
  FileType *ft = NULL;

  gtk_widget_set_sensitive(dialog, FALSE);
  create_filetype_dialog(&ft, TRUE);
  gtk_main();

  if (ft != NULL)
  {
    gchar *buf[2];
    gint i;

    buf[0] = ft->extensions;
    buf[1] = ft->description;
    i = gtk_clist_append(GTK_CLIST(clist), buf);
    gtk_clist_set_row_data(GTK_CLIST(clist), i, ft);
  }
  gtk_widget_set_sensitive(dialog, TRUE);
}

static void
edit_filetype_cb(GtkWidget *button, GtkWidget *clist)
{
  gint selected_row;
  FileType *ft;
  
  if (GTK_CLIST(clist)->selection != NULL)
    selected_row = (gint)GTK_CLIST(clist)->selection->data;
  else
    return;

  gtk_widget_set_sensitive(dialog, FALSE);
  ft = gtk_clist_get_row_data(GTK_CLIST(clist), selected_row);
  create_filetype_dialog(&ft, FALSE);
  gtk_main();

  gtk_clist_set_text(GTK_CLIST(clist), selected_row, 0, ft->extensions);
  gtk_clist_set_text(GTK_CLIST(clist), selected_row, 1, ft->description);
  gtk_widget_set_sensitive(dialog, TRUE);
}

/*
 * User Commands Page Callbacks
 */
static void
add_user_command_cb(GtkWidget *widget, GtkWidget *clist)
{
  UserCommand *command = NULL;
  
  gtk_widget_set_sensitive(dialog, FALSE);
  create_user_command_dialog(&command);
  gtk_main();
  if (command != NULL)
  {
    gchar *buf[1] = { command->name };
    gint row;
    row = gtk_clist_append(GTK_CLIST(clist), buf);
    gtk_clist_set_row_data(GTK_CLIST(clist), row, command);
  }
  gtk_widget_set_sensitive(dialog, TRUE);
}

static void
edit_user_command_cb(GtkWidget *widget, GtkWidget *clist)
{
  gint selected_row;
  UserCommand *command;

  if (GTK_CLIST(clist)->selection != NULL)
    selected_row = (gint)GTK_CLIST(clist)->selection->data;
  else
    return;

  gtk_widget_set_sensitive(dialog, FALSE);
  command = gtk_clist_get_row_data(GTK_CLIST(clist), selected_row);
  create_user_command_dialog(&command);
  gtk_main();
  gtk_clist_set_text(GTK_CLIST(clist), selected_row, 0, command->name);
  gtk_widget_set_sensitive(dialog, TRUE);
}

/*
 * Toolbar Callbacks
 */
static void
add_toolbar_button_cb(GtkWidget *widget, GtkWidget *clist)
{
  ToolbarButton *tb = NULL;

  gtk_widget_set_sensitive(dialog, FALSE);
  create_toolbar_button_dialog(&tb);
  gtk_main();
  if (tb != NULL)
  {
    gchar *buf[2] = { tb->label, tb->action };
    gint row;
    row = gtk_clist_append(GTK_CLIST(clist), buf);
    gtk_clist_set_row_data(GTK_CLIST(clist), row, tb);
  }
  gtk_widget_set_sensitive(dialog, TRUE);
}

static void
edit_toolbar_button_cb(GtkWidget *widget, GtkWidget *clist)
{
  gint selected_row;
  ToolbarButton *tb;

  if (GTK_CLIST(clist)->selection != NULL)
    selected_row = (gint)GTK_CLIST(clist)->selection->data;
  else
    return;

  gtk_widget_set_sensitive(dialog, FALSE);
  tb = gtk_clist_get_row_data(GTK_CLIST(clist), selected_row);
  create_toolbar_button_dialog(&tb);
  gtk_main();
  gtk_clist_set_text(GTK_CLIST(clist), selected_row, 0, tb->label);
  gtk_clist_set_text(GTK_CLIST(clist), selected_row, 1, tb->action);
  gtk_widget_set_sensitive(dialog, TRUE);
}

/*
 * Key Callbacks
 */
static void
key_state_to_str(gint state, gchar *str)
{
  strcpy(str, "");
  if (state & GDK_CONTROL_MASK)
    strcat(str, "C");
  if (state & GDK_MOD1_MASK)
    strcat(str, "A");
  if (state & GDK_SHIFT_MASK)
    strcat(str, "S");
}

static void
key_add_cb(GtkWidget *widget, GtkWidget *clist)
{
  KeyBinding *kb = NULL;

  gtk_widget_set_sensitive(dialog, FALSE);
  create_key_binding_dialog(&kb);
  gtk_main();
  if (kb != NULL)
  {
    gchar mod[4];
    gchar *buf[3] = { mod, kb->key_name, kb->action };
    gint row;

    key_state_to_str(kb->state, mod);
    row = gtk_clist_append(GTK_CLIST(clist), buf);
    gtk_clist_set_row_data(GTK_CLIST(clist), row, kb);
  }
  gtk_widget_set_sensitive(dialog, TRUE);
}

static void
key_edit_cb(GtkWidget *widget, GtkWidget *clist)
{
  gint selected_row;
  KeyBinding *kb;
  gchar mod[4];

  if (GTK_CLIST(clist)->selection != NULL)
    selected_row = (gint)GTK_CLIST(clist)->selection->data;
  else
    return;

  gtk_widget_set_sensitive(dialog, FALSE);
  kb = gtk_clist_get_row_data(GTK_CLIST(clist), selected_row);
  create_key_binding_dialog(&kb);
  gtk_main();

  key_state_to_str(kb->state, mod);
  gtk_clist_set_text(GTK_CLIST(clist), selected_row, 0, mod);
  gtk_clist_set_text(GTK_CLIST(clist), selected_row, 1, kb->key_name);
  gtk_clist_set_text(GTK_CLIST(clist), selected_row, 2, kb->action);
  gtk_widget_set_sensitive(dialog, TRUE);
}

/*
 * Button Callbacks
 */
static void
add_button_cb(GtkWidget *widget, GtkWidget *clist)
{
  Button *button = NULL;

  gtk_widget_set_sensitive(dialog, FALSE);
  create_button_dialog(&button);
  gtk_main();

  if (button != NULL)
  {
    gchar *buf[1] = { button->label };
    gint row = gtk_clist_append(GTK_CLIST(clist), buf);
    gtk_clist_set_row_data(GTK_CLIST(clist), row, button);
  }
  gtk_widget_set_sensitive(dialog, TRUE);
}

static void
edit_button_cb(GtkWidget *widget, GtkWidget *clist)
{
  gint selected_row;
  Button *button;

  if (GTK_CLIST(clist)->selection != NULL)
    selected_row = (gint)GTK_CLIST(clist)->selection->data;
  else
    return;

  gtk_widget_set_sensitive(dialog, FALSE);
  button = gtk_clist_get_row_data(GTK_CLIST(clist), selected_row);
  create_button_dialog(&button);
  gtk_main();
  gtk_clist_set_text(GTK_CLIST(clist), selected_row, 0, button->label);
  gtk_widget_set_sensitive(dialog, TRUE);
}

/*
 * Plugin Callbacks
 */
static void
add_plugin_ok_cb(GtkWidget *widget, GtkWidget *clist)
{
  Plugin *p;
  gchar *buf[2];
  gint row;

  p = load_plugin(gtk_file_selection_get_filename(
                  GTK_FILE_SELECTION(file_selection)));
  if (p == NULL)
  {
    status_message("Unable to load module.\n");
    gtk_widget_set_sensitive(dialog, TRUE);
    gtk_widget_destroy(file_selection);
    return;
  }

  buf[0] = p->name;
  buf[1] = _("YES");
  p->show_in_menu = TRUE;
  row = gtk_clist_append(GTK_CLIST(clist), buf);
  gtk_clist_set_row_data(GTK_CLIST(clist), row, p);
  added_plugins = g_list_append(added_plugins, p);

  gtk_widget_set_sensitive(dialog, TRUE);
  gtk_widget_destroy(file_selection);
}

static void
add_plugin_cancel_cb(GtkWidget *widget)
{
  gtk_widget_set_sensitive(dialog, TRUE);
  gtk_widget_destroy(file_selection);
}

static void
add_plugin_cb(GtkWidget *widget, GtkWidget *clist)
{
  gchar path[PATH_MAX];
  file_selection = gtk_file_selection_new(
                            _("Please select the module you want to add."));
  g_snprintf(path, sizeof(path), "%s/", PLUGINS_DIR);
  gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection), path);

  gtk_signal_connect(
     GTK_OBJECT(GTK_FILE_SELECTION(file_selection)->ok_button), "clicked",
     GTK_SIGNAL_FUNC(add_plugin_ok_cb), clist);
  gtk_signal_connect(
     GTK_OBJECT(GTK_FILE_SELECTION(file_selection)->cancel_button), "clicked",
     GTK_SIGNAL_FUNC(add_plugin_cancel_cb), NULL);
  gtk_widget_show(file_selection);
  gtk_widget_set_sensitive(dialog, FALSE);
}

static void
plugin_info_cb(GtkWidget *widget, GtkWidget *clist)
{
  Plugin *p;
  gint selected_row;

  if (GTK_CLIST(clist)->selection != NULL)
    selected_row = (gint)GTK_CLIST(clist)->selection->data;
  else
    return;

  gtk_widget_set_sensitive(dialog, FALSE);
  p = gtk_clist_get_row_data(GTK_CLIST(clist), selected_row);
  create_plugin_info_dialog(&p);
  gtk_main();
  gtk_clist_set_text(GTK_CLIST(clist), selected_row, 1,
                    (p->show_in_menu ? "YES" : "NO"));
  gtk_widget_set_sensitive(dialog, TRUE);
}

static void
remove_plugin_cb(GtkWidget *widget, GtkWidget *clist)
{
  Plugin *p;
  gint selected_row;

  if (GTK_CLIST(clist)->selection != NULL)
    selected_row = (gint)GTK_CLIST(clist)->selection->data;
  else
    return;

  p = gtk_clist_get_row_data(GTK_CLIST(clist), selected_row);
  removed_plugins = g_list_append(removed_plugins, p);
  gtk_clist_remove(GTK_CLIST(clist), selected_row);
}

/*
 * Interface Page Callbacks
 */
static GdkColor *_color;

static void
color_select_ok_cb(GtkWidget *widget,  GtkWidget *button)
{
  gdouble rgb[3];
  GtkStyle *style  = gtk_style_new();

  gtk_color_selection_get_color(GTK_COLOR_SELECTION(
    GTK_COLOR_SELECTION_DIALOG(colorsel_dialog)->colorsel), rgb);

  _color->red = rgb[0] * 65535.0;
  _color->green = rgb[1] * 65535.0;
  _color->blue = rgb[2] * 65535.0;

  style->bg[GTK_STATE_NORMAL] = *_color;
  style->bg[GTK_STATE_PRELIGHT] = *_color;
  style->bg[GTK_STATE_INSENSITIVE] = *_color;
  gtk_widget_set_style(button, style);

  gtk_widget_set_sensitive(dialog, TRUE);
  gtk_widget_destroy(colorsel_dialog);
}

static void
color_select_cancel_cb(GtkWidget *button)
{
  gtk_widget_set_sensitive(dialog, TRUE);
  gtk_widget_destroy(colorsel_dialog);
}

static void
color_select_cb(GtkWidget *button, GdkColor *color)
{
  gdouble rgb[3];

  colorsel_dialog = gtk_color_selection_dialog_new(_("Choose Color"));
  gtk_widget_destroy(GTK_COLOR_SELECTION_DIALOG(colorsel_dialog)->help_button);

  rgb[0] = color->red / 65535.0;
  rgb[1] = color->green / 65535.0;
  rgb[2] = color->blue / 65535.0;
  gtk_color_selection_set_color(GTK_COLOR_SELECTION(
    GTK_COLOR_SELECTION_DIALOG(colorsel_dialog)->colorsel), rgb);

  _color = color;

  gtk_signal_connect(GTK_OBJECT(
    GTK_COLOR_SELECTION_DIALOG(colorsel_dialog)->ok_button),
    "clicked", GTK_SIGNAL_FUNC(color_select_ok_cb), button);
  gtk_signal_connect(GTK_OBJECT(
    GTK_COLOR_SELECTION_DIALOG(colorsel_dialog)->cancel_button),
    "clicked", GTK_SIGNAL_FUNC(color_select_cancel_cb), NULL);

  gtk_widget_show(colorsel_dialog);
  gtk_widget_set_sensitive(dialog, FALSE);
}

static void
add_color_button(GtkWidget *table, gchar *title, GdkColor *color,
                 gint top, gint bot)
{
  GtkWidget *wid;
  GtkStyle *style;

  add_label_to_table(table, title, 0, 0, 1, top, bot);
  wid = add_button_to_table(table, "", color_select_cb, color, 1, 2, top, bot);
  style = gtk_style_new();
  style->bg[GTK_STATE_NORMAL] = *color;
  style->bg[GTK_STATE_PRELIGHT] = *color;
  style->bg[GTK_STATE_INSENSITIVE] = *color;
  gtk_widget_set_style(wid, style);
}

static void
font_select_ok_cb(GtkWidget *button, GtkWidget *entry)
{
  gchar *font_name = gtk_font_selection_dialog_get_font_name(
                     GTK_FONT_SELECTION_DIALOG(fontsel_dialog));
  gtk_entry_set_text(GTK_ENTRY(entry), font_name);
  gtk_widget_set_sensitive(dialog, TRUE);
  gtk_widget_destroy(fontsel_dialog);
}

static void
font_select_cancel_cb(GtkWidget *button)
{
  gtk_widget_set_sensitive(dialog, TRUE);
  gtk_widget_destroy(fontsel_dialog);
}

static void
font_select_cb(GtkWidget *button, GtkWidget *entry)
{
  fontsel_dialog = gtk_font_selection_dialog_new(_("Choose Font"));
  gtk_font_selection_dialog_set_font_name(
      GTK_FONT_SELECTION_DIALOG(fontsel_dialog),
      gtk_entry_get_text(GTK_ENTRY(entry)));
  gtk_signal_connect(GTK_OBJECT(
    GTK_FONT_SELECTION_DIALOG(fontsel_dialog)->ok_button),
    "clicked", GTK_SIGNAL_FUNC(font_select_ok_cb), entry);
  gtk_signal_connect(GTK_OBJECT(
    GTK_FONT_SELECTION_DIALOG(fontsel_dialog)->cancel_button),
    "clicked", GTK_SIGNAL_FUNC(font_select_cancel_cb), NULL);

  gtk_widget_show(fontsel_dialog);
  gtk_widget_set_sensitive(dialog, FALSE);
}

/**/
static void
clist_up_cb(GtkWidget *widget, GtkWidget *clist)
{
  gint selected_row;

  if (GTK_CLIST(clist)->selection != NULL)
    selected_row = (gint)GTK_CLIST(clist)->selection->data;
  else
    return;

  if (selected_row < 1)
    return;

  gtk_clist_swap_rows(GTK_CLIST(clist), selected_row,
                                        selected_row-1);
  gtk_clist_select_row(GTK_CLIST(clist), selected_row-1, 0);
}

static void
clist_down_cb(GtkWidget *widget, GtkWidget *clist)
{
  gint selected_row;

  if (GTK_CLIST(clist)->selection != NULL)
    selected_row = (gint)GTK_CLIST(clist)->selection->data;
  else
    return;

  if (selected_row >= (GTK_CLIST(clist)->rows - 1))
    return;

  gtk_clist_swap_rows(GTK_CLIST(clist), selected_row, selected_row+1);
  gtk_clist_select_row(GTK_CLIST(clist), selected_row+1, 0);
}

static void
clist_remove_cb(GtkWidget *widget, GtkWidget *clist)
{
  gint selected_row;

  if (GTK_CLIST(clist)->selection != NULL)
    selected_row = (gint)GTK_CLIST(clist)->selection->data;
  else
    return;

  g_free(gtk_clist_get_row_data(GTK_CLIST(clist), selected_row));
  gtk_clist_remove(GTK_CLIST(clist), selected_row);
}

static void
config_clist_select_row_cb(GtkWidget *clist)
{
  GtkWidget *notebook;
  gint page;

  if (GTK_CLIST(clist)->selection != NULL)
    page = (gint)GTK_CLIST(clist)->selection->data;
  else
    return;

  notebook = GTK_WIDGET(gtk_object_get_user_data(GTK_OBJECT(clist)));
  gtk_notebook_set_page(GTK_NOTEBOOK(notebook), page);
}

static GtkWidget *
add_clist(GtkWidget *box, gint cols, gchar **titles)
{
  GtkWidget *sw;
  GtkWidget *clist;

  sw = add_sw(box, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC, TRUE, 0);
  clist = gtk_clist_new_with_titles(cols, titles);
  gtk_clist_column_titles_passive(GTK_CLIST(clist));
  gtk_clist_set_reorderable(GTK_CLIST(clist), TRUE);
  gtk_clist_set_selection_mode(GTK_CLIST(clist), GTK_SELECTION_BROWSE);
  gtk_container_add(GTK_CONTAINER(sw), clist);
  gtk_widget_show(clist);

  return clist;
}

static GtkWidget *
add_page(GtkWidget *notebook, gchar *label_text, GtkWidget *clist)
{
  GtkWidget *frame;
  GtkWidget *label;
  GtkWidget *vbox;
  GtkWidget *vvbox;
  gchar *buf[1];

  buf[0] = label_text;
  gtk_clist_append(GTK_CLIST(clist), buf);

  vvbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vvbox);

  frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
  gtk_box_pack_start(GTK_BOX(vvbox), frame, FALSE, TRUE, 0);
  gtk_widget_show(frame);

  label = gtk_label_new(label_text);
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_misc_set_padding(GTK_MISC(label), 2, 1);
  gtk_container_add(GTK_CONTAINER(frame), label);
  gtk_widget_show(label);

  vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
  gtk_container_add(GTK_CONTAINER(vvbox), vbox);
  gtk_widget_show(vbox);

  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vvbox, NULL);
  return vbox;
}

void
create_config_dialog(gint page)
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *notebook;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *frame;
  GtkWidget *table;
  GtkWidget *config_clist;
  gchar *titles[3];
  gchar *buf[3];
  gchar label_text[MAX_LEN];
  GList *tmp;
  gint i;

  added_plugins = removed_plugins = NULL;

  dialog = gtk_dialog_new();
  dialog_vbox = GTK_DIALOG(dialog)->vbox;
  action_area = GTK_DIALOG(dialog)->action_area;
  gtk_container_set_border_width(GTK_CONTAINER(dialog_vbox), 5);
  gtk_box_set_spacing(GTK_BOX(dialog_vbox), 5);
  gtk_container_set_border_width(GTK_CONTAINER(action_area), 5);
  add_button(action_area, _("OK"), TRUE, 0, ok_cb, NULL);
  add_button(action_area, _("Cancel"), TRUE, 0, cancel_cb, NULL);
  gtk_signal_connect(GTK_OBJECT(dialog), "delete_event", delete_event_cb, NULL);
  gtk_signal_connect(GTK_OBJECT(dialog), "key_press_event",
                     GTK_SIGNAL_FUNC(key_press_cb), NULL);
  
  /* the main hbox - contains the clist and the frame */
  hbox = add_hbox(dialog_vbox, FALSE, 5, TRUE, 0);

  /* the config clist */
  titles[0] = _("Configuration");
  config_clist = gtk_clist_new_with_titles(1, titles);
  gtk_clist_set_selection_mode(GTK_CLIST(config_clist), GTK_SELECTION_BROWSE);
  gtk_clist_column_titles_passive(GTK_CLIST(config_clist));
  gtk_signal_connect(GTK_OBJECT(config_clist), "select_row",
                     GTK_SIGNAL_FUNC(config_clist_select_row_cb), NULL);
  gtk_widget_set_usize(config_clist, 120, 0);
  gtk_box_pack_start(GTK_BOX(hbox), config_clist, FALSE, FALSE, 0);
  gtk_widget_show(config_clist);

  /* the main frame - holds the notebook*/
  frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
  gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 0);
  gtk_widget_show(frame);

  /* the notebook */
  notebook = gtk_notebook_new();
  gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
  gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
  gtk_container_add(GTK_CONTAINER(frame), notebook);
  gtk_object_set_user_data(GTK_OBJECT(config_clist), notebook);
  gtk_widget_show(notebook);
  
  /* General Tab */
  vbox = add_page(notebook, _("General - Page 1"), config_clist);
  table = add_framed_table(vbox, _("Options: "), 3, 2, FALSE, 2);
  confirm_del_check = add_check_button_to_table(table,
                                     _("Confirm Delete"),
                                     cfg.confirm_delete, NULL, NULL,
                                     0, 2, 0, 1);
  confirm_ow_check = add_check_button_to_table(table,
                                    _("Confirm Overwrite"),
                                    cfg.confirm_overwrite, NULL, NULL,
                                    0, 2, 1, 2);
  auto_refresh_check = add_check_button_to_table(table,
                                      _("Auto refresh"),
                                      cfg.auto_refresh_enabled, NULL, NULL,
                                      0, 2, 2, 3);

  table = add_framed_table(vbox, _("Directories: "), 4, 2, FALSE, 2);
  gtk_table_set_row_spacings(GTK_TABLE(table), 0);
  add_label_to_table(table, _("Left Startup Dir: "), 0, 0, 1, 0, 1);
  left_startup_dir_entry = add_entry_to_table(table,
                                              cfg.left_startup_dir,
                                              1, 2, 0, 1);
  start_with_last_left_check = add_check_button_to_table(table,
                             _("Last Active Directory"),
                             cfg.start_with_last_dir_left, start_with_last_cb,
                             NULL, 1, 2, 1, 2);
  add_label_to_table(table, _("Right Startup Dir: "), 0, 0, 1, 2, 3);
  right_startup_dir_entry = add_entry_to_table(table,
                                               cfg.right_startup_dir,
                                               1, 2, 2, 3);
  start_with_last_right_check = add_check_button_to_table(table,
                             _("Last Active Directory"),
                             cfg.start_with_last_dir_right, start_with_last_cb,
                             NULL, 1, 2, 3, 4);
  if (GTK_TOGGLE_BUTTON(start_with_last_left_check)->active)
    gtk_widget_set_sensitive(left_startup_dir_entry, FALSE);
  if (GTK_TOGGLE_BUTTON(start_with_last_right_check)->active)
    gtk_widget_set_sensitive(right_startup_dir_entry, FALSE);
  
  /* General - Page 2 */
  vbox = add_page(notebook,_("General - Page 2"), config_clist);
  table = add_framed_table(vbox, _("Commands: "), 3, 2, FALSE, 2);
  add_label_to_table(table, _("Viewer Command: "), 0, 0, 1, 0, 1);
  viewer_entry = add_entry_to_table(table, cfg.viewer_command,
                                                   1, 2, 0, 1);
  use_internal_check = add_check_button_to_table(table,
                 _("Use internal viewer"), cfg.use_internal_viewer,
                 use_internal_cb, NULL, 0, 2, 1, 2);
  if (GTK_TOGGLE_BUTTON(use_internal_check)->active)
    gtk_widget_set_sensitive(viewer_entry, FALSE);

  add_label_to_table(table, _("Xterm Command: "), 0, 0, 1, 2, 3);
  xterm_entry = add_entry_to_table(table, cfg.xterm_command,
                                                  1, 2, 2, 3);

  table = add_framed_table(vbox, _("Histories: "), 2, 2, FALSE, 2);
  add_label_to_table(table, _("Directory History Size: "), 0, 0, 1, 0, 1);
  g_snprintf(label_text, sizeof(label_text), "%d", cfg.dir_history_max_length);
  dir_history_entry = add_entry_to_table(table, label_text,
                                                        1, 2, 0, 1);
  add_label_to_table(table, _("Command History Size: "), 0, 0, 1, 1, 2);
  g_snprintf(label_text, sizeof(label_text), "%d",
             cfg.command_history_max_length);
  command_history_entry = add_entry_to_table(table, label_text,
                                                            1, 2, 1, 2);
  /* Columns Tab */
  vbox = add_page(notebook,_("Columns"), config_clist);

  for (i = 0; i < MAX_COLUMNS; i++)
  {
    column_checks[i] = add_check_button(vbox, all_columns[i].title,
                                        0, FALSE, 0, NULL, NULL);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(column_checks[i]),
                                 all_columns[i].is_visible);
  }
  gtk_widget_set_sensitive(column_checks[FILENAME], FALSE);

  /* Bookmarks Tab */
  vbox = add_page(notebook,_("Bookmarks"), config_clist);

  titles[0] = _("Bookmarks");
  bookmarks_clist = add_clist(vbox, 1, titles);

  for (tmp = cfg.bookmarks; tmp != NULL; tmp = tmp->next)
  {
    buf[0] = (gchar *)tmp->data;
    i = gtk_clist_append(GTK_CLIST(bookmarks_clist), buf);
    gtk_clist_set_row_data(GTK_CLIST(bookmarks_clist), i, g_strdup(tmp->data));
  }
  gtk_widget_show(bookmarks_clist);

  hbox = add_hbox(vbox, TRUE, 0, FALSE, 5);
  add_button_with_icon(hbox, icon_up_xpm, FALSE, 5, clist_up_cb,
                       bookmarks_clist);
  add_button_with_icon(hbox, icon_down_xpm, FALSE, 5, clist_down_cb,
                       bookmarks_clist);
  add_button(hbox, _("Remove"), TRUE, 5, clist_remove_cb, bookmarks_clist);

  /* Filetypes Tab */
  vbox = add_page(notebook,_("Filetypes"), config_clist);
  
  titles[0] = _("Extensions");
  titles[1] = _("Description");
  filetypes_clist = add_clist(vbox, 2, titles);

  for (tmp = cfg.filetypes; tmp != NULL; tmp = tmp->next)
  {
    FileType *ft = tmp->data;

    buf[0] = ft->extensions;
    buf[1] = ft->description;

    i = gtk_clist_append(GTK_CLIST(filetypes_clist), buf);
    gtk_clist_set_row_data(GTK_CLIST(filetypes_clist), i,
                           g_memdup(ft, sizeof(FileType)));
  }

  hbox = add_hbox(vbox, TRUE, 0, FALSE, 5);
  add_button_with_icon(hbox, icon_up_xpm, TRUE, 5, clist_up_cb,
                       filetypes_clist);
  add_button_with_icon(hbox, icon_down_xpm, TRUE, 5, clist_down_cb,
                       filetypes_clist);
  add_button(hbox, _("Add"), TRUE, 5, add_filetype_cb, filetypes_clist);
  add_button(hbox, _("Edit"), TRUE, 5, edit_filetype_cb, filetypes_clist);
  add_button(hbox, _("Remove"), TRUE, 5, clist_remove_cb, filetypes_clist);

  /* User Commands Tab */
  vbox = add_page(notebook, _("User Commands"), config_clist);

  titles[0] = _("Name");
  user_commands_clist = add_clist(vbox, 1, titles);

  for (tmp = cfg.user_commands; tmp != NULL; tmp = tmp->next)
  {
    UserCommand *command = tmp->data;

    buf[0] = command->name;

    i = gtk_clist_append(GTK_CLIST(user_commands_clist), buf);
    gtk_clist_set_row_data(GTK_CLIST(user_commands_clist), i,
                           g_memdup(command, sizeof(UserCommand)));
  }

  hbox = add_hbox(vbox, TRUE, 0, FALSE, 5);
  add_button_with_icon(hbox, icon_up_xpm, TRUE, 5, clist_up_cb, user_commands_clist);
  add_button_with_icon(hbox, icon_down_xpm, TRUE, 5, clist_down_cb, user_commands_clist);
  add_button(hbox, _("Add"), TRUE, 5, add_user_command_cb, user_commands_clist);
  add_button(hbox, _("Edit"), TRUE, 5, edit_user_command_cb, user_commands_clist);
  add_button(hbox, _("Remove"), TRUE, 5, clist_remove_cb, user_commands_clist);

  /* Toolbar Tab */
  vbox = add_page(notebook, _("Toolbar Buttons"), config_clist);

  titles[0] = _("Label");
  titles[1] = _("Action");
  toolbar_clist = add_clist(vbox, 2, titles);

  for (tmp = cfg.toolbar_buttons; tmp != NULL; tmp = tmp->next)
  {
    ToolbarButton *tb = tmp->data;

    buf[0] = tb->label;
    buf[1] = tb->action;

    i = gtk_clist_append(GTK_CLIST(toolbar_clist), buf);
    gtk_clist_set_row_data(GTK_CLIST(toolbar_clist), i, 
                           g_memdup(tb, sizeof(ToolbarButton)));
  }

  hbox = add_hbox(vbox, TRUE, 0, FALSE, 5);
  add_button_with_icon(hbox, icon_up_xpm, TRUE, 5, clist_up_cb, toolbar_clist);
  add_button_with_icon(hbox, icon_down_xpm, TRUE, 5, clist_down_cb,
                       toolbar_clist);
  add_button(hbox, _("Add"), TRUE, 5, add_toolbar_button_cb, toolbar_clist);
  add_button(hbox, _("Edit"), TRUE, 5, edit_toolbar_button_cb, toolbar_clist);
  add_button(hbox, _("Remove"), TRUE, 5, clist_remove_cb, toolbar_clist);

  /* Keys Tab */
  vbox = add_page(notebook, _("Key Bindings"), config_clist);

  titles[0] = _("Mod");
  titles[1] = _("Key");
  titles[2] = _("Action");
  keys_clist = add_clist(vbox, 3, titles);
  gtk_clist_set_column_width(GTK_CLIST(keys_clist), 1, 50);

  for (tmp = cfg.key_bindings; tmp != NULL; tmp = tmp->next)
  {
    KeyBinding *kb = tmp->data;
    gchar mod[4];

    key_state_to_str(kb->state, mod);

    buf[0] = mod;
    buf[1] = kb->key_name;
    buf[2] = kb->action;

    i = gtk_clist_append(GTK_CLIST(keys_clist), buf);
    gtk_clist_set_row_data(GTK_CLIST(keys_clist), i, 
                           g_memdup(kb, sizeof(KeyBinding)));
  }

  hbox = add_hbox(vbox, TRUE, 0, FALSE, 5);
  add_button_with_icon(hbox, icon_up_xpm, TRUE, 5, clist_up_cb, keys_clist);
  add_button_with_icon(hbox, icon_down_xpm, TRUE, 5, clist_down_cb, keys_clist);
  add_button(hbox, _("Add"), TRUE, 5, key_add_cb, keys_clist);
  add_button(hbox, _("Edit"), TRUE, 5, key_edit_cb, keys_clist);
  add_button(hbox, _("Remove"), TRUE, 5, clist_remove_cb, keys_clist);

  /* Buttons Tab */
  vbox = add_page(notebook,_("Buttons"), config_clist);

  titles[0] = _("Label");
  buttons_clist = add_clist(vbox, 1, titles);

  for (tmp = cfg.buttons; tmp != NULL; tmp = tmp->next)
  {
    Button *button = tmp->data;

    buf[0] = button->label;
    i = gtk_clist_append(GTK_CLIST(buttons_clist), buf);
    gtk_clist_set_row_data(GTK_CLIST(buttons_clist), i,
                           g_memdup(button, sizeof(Button)));
  }

  hbox = add_hbox(vbox, TRUE, 0, FALSE, 5);
  add_button_with_icon(hbox, icon_up_xpm, TRUE, 5, clist_up_cb, buttons_clist);
  add_button_with_icon(hbox, icon_down_xpm, TRUE, 5, clist_down_cb,
                       buttons_clist);
  add_button(hbox, _("Add"), TRUE, 5, add_button_cb, buttons_clist);
  add_button(hbox, _("Edit"), TRUE, 5, edit_button_cb, buttons_clist);
  add_button(hbox, _("Remove"), TRUE, 5, clist_remove_cb, buttons_clist);
  
  /* Plugins Tab */
  vbox = add_page(notebook, _("Plugins"), config_clist);

  titles[0] = _("Name");
  titles[1] = _("Show in Menu");
  plugins_clist = add_clist(vbox, 2, titles);
  gtk_clist_set_column_width(GTK_CLIST(plugins_clist), 0, 200);

  for (tmp = cfg.plugins; tmp != NULL; tmp = tmp->next)
  {
    Plugin *plugin = tmp->data;

    buf[0] = plugin->name;
    buf[1] = (plugin->show_in_menu ? _("YES") : _("NO"));
    i = gtk_clist_append(GTK_CLIST(plugins_clist), buf);
    gtk_clist_set_row_data(GTK_CLIST(plugins_clist), i,
                           g_memdup(plugin, sizeof(Plugin)));
  }

  hbox = add_hbox(vbox, TRUE, 0, FALSE, 5);
  add_button_with_icon(hbox, icon_up_xpm, TRUE, 5, clist_up_cb, plugins_clist);
  add_button_with_icon(hbox, icon_down_xpm, TRUE, 5, clist_down_cb,
                       plugins_clist);
  add_button(hbox, _("Add"), TRUE, 5, add_plugin_cb, plugins_clist);
  add_button(hbox, _("Info"), TRUE, 5, plugin_info_cb, plugins_clist);
  add_button(hbox, _("Remove"), TRUE, 5, remove_plugin_cb, plugins_clist);

  /* Interface Page1 */
  vbox = add_page(notebook,_("Interface - Colors"), config_clist);

  table = add_table(vbox, 9, 2, FALSE, FALSE, 0);
  gtk_table_set_row_spacings(GTK_TABLE(table), 5);
  add_color_button(table, _("Active List Column Button"), &COL_COLOR, 0, 1);
  add_color_button(table, _("Executable"), &EXE_COLOR, 1, 2);
  add_color_button(table, _("Symbolic Link"), &LNK_COLOR, 2, 3);
  add_color_button(table, _("Special Device"), &DEV_COLOR, 3, 4);
  add_color_button(table, _("Socket"), &SOCK_COLOR, 4, 5);
  add_color_button(table, _("Directory"), &DIR_COLOR, 5, 6);
  add_color_button(table, _("Tagged"), &TAG_COLOR, 6, 7);
  add_color_button(table, _("Inactive Selected"), &SELECT_COLOR, 7, 8);
  add_color_button(table, _("Drag Highlight"), &DRAG_HILIGHT, 8, 9);

  /* Interface Page2 */
  vbox = add_page(notebook,_("Interface - Misc."), config_clist);
  table = add_table(vbox, 2, 2, FALSE, FALSE, 0);
  gtk_table_set_row_spacings(GTK_TABLE(table), 5);
  list_font_entry = add_entry_to_table(table, cfg.list_font, 0, 1, 0, 1);
  add_button_to_table(table, _("List Font"), font_select_cb,
                      list_font_entry, 1, 2, 0, 1);
  output_font_entry = add_entry_to_table(table, cfg.output_font, 0, 1, 1, 2);
  add_button_to_table(table, _("Output Font"), font_select_cb, 
                      output_font_entry, 1, 2, 1, 2);

  table = add_framed_table(vbox, _("Scrollbar Position: "), 2, 2, FALSE, 5);
  top_right_radio = add_radio_button_to_table(table, "Top-Right",
    NULL, (cfg.scrollbar_pos == GTK_CORNER_BOTTOM_LEFT), NULL, NULL, 1, 2, 0, 1);
  top_left_radio = add_radio_button_to_table(table, "Top-Left", 
    gtk_radio_button_group(GTK_RADIO_BUTTON(top_right_radio)),
    (cfg.scrollbar_pos == GTK_CORNER_BOTTOM_RIGHT), NULL, NULL, 0, 1, 0, 1);
  bot_right_radio = add_radio_button_to_table(table,
    "Bottom-Right",
    gtk_radio_button_group(GTK_RADIO_BUTTON(top_right_radio)),
    (cfg.scrollbar_pos == GTK_CORNER_TOP_LEFT), NULL, NULL, 1, 2, 1, 2);
  bot_left_radio = add_radio_button_to_table(table,
    "Bottom-Left",
    gtk_radio_button_group(GTK_RADIO_BUTTON(top_right_radio)),
    (cfg.scrollbar_pos == GTK_CORNER_TOP_RIGHT), NULL, NULL, 0, 1, 1, 2);

  win_right_click_check = add_check_button(vbox,
               _("Windows Right Click Behavior"), cfg.windows_right_click,
               FALSE, 0, NULL, NULL);
  expand_check = add_check_button(vbox, _("Expanded Right Click Menu"),
                                  cfg.expand_popup_menu, FALSE, 0, NULL, NULL);
  use_vi_keys_check = add_check_button(vbox, _("Use Vi keys"),
                                       cfg.use_vi_keys, FALSE, 0,  NULL, NULL);

  gtk_clist_select_row(GTK_CLIST(config_clist), page, 0);

  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_widget_show(dialog);
  gtk_widget_set_sensitive(app.main_window, FALSE);
  gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(app.main_window));
}

