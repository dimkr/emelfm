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

/* 9-22-2000: Modified by Aurelien Gateau 
 *            Added code for show hidden toggle button
 */

/* 10-9-2000: Modified by Aurelien Gateau 
 *            Added code for windows right click behavior
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "emelfm.h"

#include "icons/icon_dirparent.xpm"

/* Stuff for drag and drop */
enum { TARGET_URL, TARGET_STRING };

static GtkTargetEntry target_table[] = {
  { "text/uri-list", 0, TARGET_URL },
  { "text/plain",    0, TARGET_STRING }
};
static guint n_targets = sizeof(target_table) / sizeof(target_table[0]);

void
toggle_panel_size(FileView *view)
{
  gboolean was_inactive = (view == other_view);

  if (view == &app.left_view)
  {
    switch (cfg.view_type)
    {
    case BOTH_PANES:
      cfg.view_type = LEFT_PANE;
      gtk_widget_hide(app.button_col_sw);
      gtk_widget_hide(app.right_view_box);
      break;
    case LEFT_PANE:
      cfg.view_type = BOTH_PANES;
      gtk_widget_show(app.right_view_box);
      gtk_widget_show(app.button_col_sw);
      break;
    case RIGHT_PANE:
      cfg.view_type = LEFT_PANE;
      gtk_widget_hide(app.right_view_box);
      gtk_widget_show(app.left_view_box);
      break;
    default:
      break;
    }
  }
  else
  {
    switch (cfg.view_type)
    {
    case BOTH_PANES:
      cfg.view_type = RIGHT_PANE;
      gtk_widget_hide(app.button_col_sw);
      gtk_widget_hide(app.left_view_box);
      break;
    case LEFT_PANE:
      cfg.view_type = RIGHT_PANE;
      gtk_widget_hide(app.left_view_box);
      gtk_widget_show(app.right_view_box);
      break;
    case RIGHT_PANE:
      cfg.view_type = BOTH_PANES;
      gtk_widget_show(app.left_view_box);
      gtk_widget_show(app.button_col_sw);
      break;
    default:
      break;
    }
  }

  if (was_inactive)
    clist_select_rows(view->clist, view->old_selection);
  else
    focus_row(view, view->row, FALSE, TRUE, TRUE);
}

/* This function is called whenever we need to exchange curr_view and
 * other_view.
 */
static void
switch_views()
{
  gint i;
  gint num_cols;
  GtkStyle *style;
  FileView *temp;
  GList *tmp;

  temp = other_view;
  other_view = curr_view;
  curr_view = temp;

  gtk_signal_emit_by_name(GTK_OBJECT(other_view->clist), "end-selection");
  if (other_view->old_selection)
    g_list_free(other_view->old_selection);
  other_view->old_selection = g_list_copy(
                                GTK_CLIST(other_view->clist)->selection);
  for (tmp = other_view->old_selection; tmp != NULL; tmp = tmp->next)
    gtk_clist_set_background(GTK_CLIST(other_view->clist), (gint)tmp->data,
                             &SELECT_COLOR);
  for (tmp = curr_view->old_selection; tmp != NULL; tmp = tmp->next)
    gtk_clist_set_background(GTK_CLIST(curr_view->clist), (gint)tmp->data,
                             &CLIST_COLOR);

  gtk_clist_unselect_all(GTK_CLIST(other_view->clist));
  style = gtk_style_copy(gtk_widget_get_style(
                         GTK_CLIST(other_view->clist)->column[0].button));
  style->bg[GTK_STATE_NORMAL] = COL_COLOR;
  num_cols = GTK_CLIST(curr_view->clist)->columns;
  for (i = 0; i < num_cols; i++)
  {
    gtk_widget_set_style(GTK_CLIST(curr_view->clist)->column[i].button,
                         style);
    gtk_widget_set_style(curr_view->sort_arrows[i], style);
  }
  for (i = 0; i < num_cols; i++)
  {
    gtk_widget_restore_default_style(
          GTK_CLIST(other_view->clist)->column[i].button);
    gtk_widget_restore_default_style(other_view->sort_arrows[i]);
  }

  chdir(curr_view->dir);
}

/*
 * FILEVIEW CALLBACKS
 */
static void
updir_click_cb(GtkWidget *widget, GdkEventButton *event, FileView *view)
{
  if (event->button == 1)
  {
    gchar path[PATH_MAX];
    g_snprintf(path, sizeof(path), "%s/..", view->dir);
    change_dir(view, path);
  }
  else if (event->button == 3)
  {
    gchar *home;
    if ((home = getenv("HOME")) != NULL)
      change_dir(view, home);
    else
      status_message(_("Environment variable 'HOME' not set."));
  }
  gtk_signal_emit_stop_by_name(GTK_OBJECT(widget), "button_press_event");
  focus_row(view, view->row, TRUE, TRUE, TRUE);
}

static void
toggle_show_hidden_cb(GtkWidget *widget, FileView *view)
{
  view->show_hidden = GTK_TOGGLE_BUTTON(widget)->active;
  refresh_list(view);
}

static void
show_hide_file_view_cb(GtkWidget *widget, GdkEventButton *event, FileView *view)
{
  if (event->button == 1)
  {
    toggle_panel_size(view);
  }
  else if (event->button == 3)
  {
    if (view != curr_view)
      change_dir(curr_view, view->dir);
    else
      change_dir(other_view, view->dir);
  }
}

static void
mouse_click_cb(GtkWidget *widget, GdkEventButton *event, FileView *view)
{
  if (event->button == 1)
  { /* Left Button: selection */
    if (curr_view != view)
    {
      switch_views();
      gtk_widget_grab_focus(view->clist);
    }
  }
  else if (event->button == 2)
  { /* Middle Button: Drag and Drop */
    GtkTargetList *list;
    GdkDragContext *context;

    list = gtk_target_list_new(target_table, n_targets);
    context = gtk_drag_begin(curr_view->clist, list, GDK_ACTION_COPY |
      GDK_ACTION_MOVE | GDK_ACTION_LINK | GDK_ACTION_ASK,
      event->button, (GdkEvent *)event);
    gtk_drag_set_icon_default(context);
  }
  else if (event->button == 3)
  { /* Right Button: Menu */
    if (cfg.windows_right_click && (view->tagged == NULL))
    { /* if windows right click is enabled, we want to select the file */
      gint xmouse,ymouse,row,column;
      GdkModifierType mask;

      gdk_window_get_pointer(GTK_CLIST(view->clist)->clist_window,
                             &xmouse, &ymouse, &mask);
      gtk_clist_get_selection_info(GTK_CLIST(view->clist), xmouse, ymouse,
                                   &row, &column);
      /* don't screw up the current selection */
      if (!clist_row_is_selected(view->clist, row))
        gtk_clist_unselect_all(GTK_CLIST(view->clist));
      focus_row(view, row, FALSE, FALSE, TRUE);
    }

    if (event->state & GDK_SHIFT_MASK)
      show_plugins_menu(event->button, event->time);
    else if (event->state & GDK_CONTROL_MASK)
      show_user_command_menu(event->button, event->time);
    else
      show_menu(event->button, event->time);
  }
}

static void
select_row_cb(GtkWidget     *clist,
              gint           row,
              gint           col,
              GdkEvent      *event,
              FileView      *view)
{
  view->row = row;
  if (curr_view != view)
    switch_views();
  if (event && (event->type == GDK_2BUTTON_PRESS))
    open_cb(NULL);
}

static gint
find_filename_begining_with(gchar ch)
{
  gint i;

  /* search from the current row to the end of the list */
  for (i = curr_view->row+1; i < GTK_CLIST(curr_view->clist)->rows; i++)
  {
    FileInfo *info = gtk_clist_get_row_data(GTK_CLIST(curr_view->clist), i);
    
    if (info->filename[0] == ch)
      return i;
  }
  /* search from the first row to the current row */
  for (i = 0; i < curr_view->row; i++)
  {
    FileInfo *info = gtk_clist_get_row_data(GTK_CLIST(curr_view->clist), i);

    if (info->filename[0] == ch)
      return i;
  }
  return -1;
}

static gboolean
file_list_key_press_cb(GtkWidget *widget, GdkEventKey *event, FileView *view)
{
  GList *tmp;

  /* get rid of all the modifiers that we don't care about */
  event->state &= ~(GDK_LOCK_MASK | GDK_MOD2_MASK | GDK_MOD3_MASK |
                    GDK_MOD4_MASK | GDK_MOD5_MASK);
  for (tmp = cfg.key_bindings; tmp != NULL; tmp = tmp->next)
  {
    KeyBinding *kb = tmp->data;

    if ((event->state == kb->state) &&
        (gdk_keyval_to_lower(event->keyval) == kb->keyval))
    { /* key bindings are case insensitive */
      do_command(kb->action);
      gtk_signal_emit_stop_by_name(GTK_OBJECT(widget), "key_press_event");
      return TRUE;
    }
  }

  /* Alt+<letter> Shortcuts to activate the menus */
  if (event->state & GDK_MOD1_MASK)
  {
    switch (event->keyval)
    {
    case 'B': case 'b':
      gtk_signal_emit_by_name(GTK_OBJECT(view->bookmark_menu_item),
                              "activate-item");
      break;
    case 'F': case 'f':
      gtk_signal_emit_by_name(GTK_OBJECT(view->filter_menu_item),
                              "activate-item");
      break;
    default:
      break;
    }
    return TRUE;
  }

  if ((event->keyval < 0x100) && (event->state <= 1))
  { /* The key is a letter and the only possible modifier is Shift */
    if (cfg.use_vi_keys)
    {
      switch (event->keyval)
      {
      case 'j':
        event->keyval = GDK_Down;
        return TRUE;
      case 'k':
        event->keyval = GDK_Up;
        return TRUE;
      case 'l':
        open_cb(NULL);
        gtk_widget_grab_focus(curr_view->clist);
        return TRUE;
      case 'h':
        change_dir(view, "..");
        return TRUE;
      case 'g':
        focus_row(view, 0, TRUE, TRUE, TRUE);
        return TRUE;
      case 'G':
        focus_row(view, (GTK_CLIST(view->clist)->rows - 1), TRUE, TRUE, TRUE);
        return TRUE;
      case 't':
        toggle_tag_cb();
        return TRUE;
      case '/':
        find_cb(NULL);
        return TRUE;
      default:
        break;
      }
    }
    else
    {
      /* Select row beggining with letter entered */
      gint row = find_filename_begining_with(event->keyval);
      if (row > 0)
        focus_row(view, row, TRUE, TRUE, TRUE);
      return TRUE;
    }
  }
  return FALSE;
}

static void
column_button_cb(GtkWidget  *widget,
                 gint       col,
                 FileView   *view)
{
  GtkSortType direction;

  /* reverse the sorting direction if the list is already sorted by this col */
  if (GTK_CLIST(view->clist)->sort_column == col)
    direction = (GTK_CLIST(view->clist)->sort_type == GTK_SORT_ASCENDING
                 ? GTK_SORT_DESCENDING : GTK_SORT_ASCENDING);
  else
    direction = GTK_CLIST(view->clist)->sort_type;
    
  sort_list(view, all_columns[col].sort_func, direction, col);
}

static void
dir_entry_cb(GtkWidget *entry, FileView  *view)
{
  gchar *path = gtk_entry_get_text(GTK_ENTRY(entry));
  change_dir(view, path);
}

static void
dir_popwin_cb(GtkWidget *popwin, FileView *view)
{
  gchar *path = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(view->dir_entry)->entry));
  change_dir(view, path);
}

/*
 * D&D stuff
 */

/* This gets called when an option from the popup menu (Copy/Move/Link) is
 * chosen
 */
static void
drag_op_cb(GtkWidget *widget, gpointer data)
{
  gchar *dest_dir, *file_list;
  gchar dest[PATH_MAX+NAME_MAX];
  gchar *s, *filename;
  int (*op_func)(gchar *, gchar *) = data;

  gboolean check = cfg.confirm_overwrite;
  gint result;

  dest_dir = gtk_object_get_data(GTK_OBJECT(widget), "dest_dir");
  file_list = gtk_object_get_data(GTK_OBJECT(widget), "file_list");

  /* file_list is a string of the format:
   * file:/path/to/one_file\r\nfile:/path/to/second_file\r\n 
   * and so on
   */
  while ((s = strstr(file_list, "\r\n")) != NULL)
  {
    *s = '\0';
    file_list += 5; /* Skip over 'file:' */

    filename = strrchr(file_list, '/'); /* extract the filename */
    g_snprintf(dest, sizeof(dest), "%s%s", dest_dir, filename+1);
    if (check && (access(dest, F_OK) == 0))
    {
      gtk_widget_set_sensitive(app.main_window, FALSE);
      result = op_with_ow_check(file_list, dest, op_func);
      gtk_widget_set_sensitive(app.main_window, TRUE);
      if (result == YES_TO_ALL)
        check = FALSE;
      else if (result == CANCEL)
        break;
    }
    else
      op_func(file_list, dest);

    file_list = s + 2; /* Skip over the '\r\n' */
  }
    
  gtk_widget_destroy(app.drag_op_menu);
  refresh_list(other_view);
  refresh_list(other_view);
}

/* This is called by the drop handler function clist_drag_data_received */
static void
create_drag_op_menu(gchar *dest_dir, gchar *file_list)
{
  GtkWidget *item;

  if (app.drag_op_menu != NULL)
    gtk_widget_destroy(app.drag_op_menu);

  app.drag_op_menu = gtk_menu_new();
  item = add_menu_item(app.drag_op_menu, _("Copy"), drag_op_cb, file_copy);
  gtk_object_set_data_full(GTK_OBJECT(item), "dest_dir", g_strdup(dest_dir),
                           free_data);
  gtk_object_set_data_full(GTK_OBJECT(item), "file_list", g_strdup(file_list),
                           free_data);
  item = add_menu_item(app.drag_op_menu, _("Move"), drag_op_cb, file_move);
  gtk_object_set_data_full(GTK_OBJECT(item), "dest_dir", g_strdup(dest_dir),
                           free_data);
  gtk_object_set_data_full(GTK_OBJECT(item), "file_list", g_strdup(file_list),
                           free_data);
  item = add_menu_item(app.drag_op_menu, _("SymLink"), drag_op_cb, file_symlink);
  gtk_object_set_data_full(GTK_OBJECT(item), "dest_dir", g_strdup(dest_dir),
                           free_data);
  gtk_object_set_data_full(GTK_OBJECT(item), "file_list", g_strdup(file_list),
                           free_data);

  gtk_menu_popup(GTK_MENU(app.drag_op_menu), NULL, NULL, NULL, NULL, 0, 0);
}

/* This is called automatically when we recieve a drop */
static void
clist_drag_data_received(GtkWidget        *widget,
                         GdkDragContext   *context,
                         gint             x,
                         gint             y,
                         GtkSelectionData *data,
                         guint            drag_info,
                         guint            time,
                         FileView         *view)
{
  gint row, col;
  FileInfo *info;
  gchar dest_dir[PATH_MAX];

  gtk_clist_set_background(GTK_CLIST(view->clist), view->last_row,
                           &CLIST_COLOR);

  if (!gtk_clist_get_selection_info(GTK_CLIST(view->clist), x, y, &row, &col))
  {
    g_snprintf(dest_dir, sizeof(dest_dir), "%s/", view->dir);
  }
  else
  {
    info = gtk_clist_get_row_data(GTK_CLIST(view->clist), row);

    if (is_dir(info))
      g_snprintf(dest_dir, sizeof(dest_dir), "%s/%s/", view->dir, info->filename);
    else
      g_snprintf(dest_dir, sizeof(dest_dir), "%s/", view->dir);
  }

  create_drag_op_menu(dest_dir, (gchar *)data->data);
  gtk_drag_finish(context, FALSE, FALSE, time);
}

/* This is called automatically when a drag is initiated 
 * It build a URI list of the selected filenames to set as the drag data
 */
static void
clist_drag_data_get(GtkWidget            *widget,
                    GdkDragContext       *context,
                    GtkSelectionData     *data,
                    guint                info_arg,
                    guint                time,
                    FileView             *view)
{
  FileInfo *info;
  GList *base, *tmp;
  GString *uri_list = g_string_sized_new(PATH_MAX);

  disable_refresh();
  base = tmp = get_selection(view);
  
  for (; tmp != NULL; tmp = tmp->next)
  {
    info = tmp->data;
    g_string_sprintfa(uri_list, "file:%s/%s\r\n", view->dir, info->filename);
  }

  gtk_selection_data_set(data, data->target, 8, uri_list->str, uri_list->len);
  g_string_free(uri_list, TRUE);
  g_list_free(base);
  reenable_refresh();
}

/* This handles the highlighting of the rows */
static gboolean
clist_drag_motion(GtkWidget         *widget,
                  GdkDragContext    *context,
                  gint              x,
                  gint              y,
                  guint             time,
                  FileView          *view)
{
  gint row, col;

  gtk_clist_set_background(GTK_CLIST(view->clist), view->last_row,
                           &CLIST_COLOR);

  if (gtk_clist_get_selection_info(GTK_CLIST(view->clist), x, y, &row, &col))
  {
    gtk_clist_set_background(GTK_CLIST(view->clist), row, &DRAG_HILIGHT);
    view->last_row = row;
  }
    
  return TRUE;  
}

static void
clist_drag_leave(GtkWidget         *widget,
                 GdkDragContext    *context,
                 guint             time,
                 FileView          *view)
{
  gtk_clist_set_background(GTK_CLIST(view->clist), view->last_row,
                           &CLIST_COLOR);
}

GtkWidget *
create_file_view(FileView *view)
{
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *menu_bar;
  GtkWidget *button;
  GtkWidget *pixmapwid;
  GdkPixmap *pixmap;
  GdkBitmap *mask;
  GtkStyle *style;
  GdkFont *font;
  GtkWidget *sw;
  gint i;

  /* Top Pane */
  vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);

  menu_bar = create_filelist_menu_bar(view);
  gtk_widget_show(menu_bar);
  gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);

  hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);

  /* Show/Hide Button */
  if (view == &app.right_view)
    button = gtk_button_new_with_label("<");
  else
    button = gtk_button_new_with_label(">");
  GTK_WIDGET_UNSET_FLAGS(button, GTK_CAN_FOCUS);
  gtk_signal_connect(GTK_OBJECT(button), "button_press_event", 
                    GTK_SIGNAL_FUNC(show_hide_file_view_cb), view);
  gtk_tooltips_set_tip(GTK_TOOLTIPS(app.tooltips), button,
                       _("Left Click: Maximize/Minimize File View\n"
                         "Right Click: Sync Dirs"), NULL);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
  gtk_widget_show(button);

  /* Hidden files toggle button */
  view->hidden_toggle = gtk_toggle_button_new_with_label("H");
  GTK_WIDGET_UNSET_FLAGS(view->hidden_toggle, GTK_CAN_FOCUS);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(view->hidden_toggle),
                               view->show_hidden);
  gtk_signal_connect(GTK_OBJECT(view->hidden_toggle), "toggled",
                     GTK_SIGNAL_FUNC(toggle_show_hidden_cb), view);
  gtk_tooltips_set_tip(GTK_TOOLTIPS(app.tooltips), view->hidden_toggle,
                       _("Show/Hide hidden files"), NULL);
  gtk_box_pack_start(GTK_BOX(hbox), view->hidden_toggle, FALSE, FALSE, 0);
  gtk_widget_show(view->hidden_toggle);

  /* Dir Entry Combo */
  view->dir_entry = gtk_combo_new();
  gtk_combo_disable_activate(GTK_COMBO(view->dir_entry));
  gtk_combo_set_use_arrows_always(GTK_COMBO(view->dir_entry), TRUE);
  gtk_combo_set_case_sensitive(GTK_COMBO(view->dir_entry), TRUE);
  gtk_signal_connect(GTK_OBJECT(GTK_COMBO(view->dir_entry)->entry), 
              "activate", GTK_SIGNAL_FUNC(dir_entry_cb), view);
  gtk_signal_connect(GTK_OBJECT(GTK_COMBO(view->dir_entry)->popwin),
              "hide", GTK_SIGNAL_FUNC(dir_popwin_cb), view);
  gtk_box_pack_start(GTK_BOX(hbox), view->dir_entry, TRUE, TRUE, 0);
  gtk_widget_show(view->dir_entry);

  /* Up Dir Button */
  button = gtk_button_new();
  GTK_WIDGET_UNSET_FLAGS(button, GTK_CAN_FOCUS);
  gtk_signal_connect(GTK_OBJECT(button), "button_press_event",
                     GTK_SIGNAL_FUNC(updir_click_cb), view);
  gtk_tooltips_set_tip(GTK_TOOLTIPS(app.tooltips), button,
                       _("Left Click: Up Dir   Right Click: Home"), NULL);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
  gtk_widget_show(button);

  style = gtk_widget_get_style(app.main_window);
  pixmap = gdk_pixmap_create_from_xpm_d(app.main_window->window, &mask, 
                                        &style->bg[GTK_STATE_NORMAL], 
                                        icon_dirparent_xpm);
  pixmapwid = gtk_pixmap_new(pixmap, mask);
  gtk_container_add(GTK_CONTAINER(button), pixmapwid);
  gtk_widget_show(pixmapwid);

  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

  sw = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
          GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW(sw), cfg.scrollbar_pos);
  gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 0);
  gtk_widget_show(sw);

  /* File List */
  view->clist = gtk_clist_new(MAX_COLUMNS);
  for (i = 0; i < MAX_COLUMNS; i++)
  {
    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    GtkWidget *label = gtk_label_new(all_columns[i].title);

    view->sort_arrows[i] = gtk_arrow_new(GTK_ARROW_DOWN, GTK_SHADOW_IN);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), view->sort_arrows[i], FALSE, TRUE, 0);

    gtk_widget_show(label);
    gtk_widget_show(hbox);
    gtk_clist_set_column_widget(GTK_CLIST(view->clist), i, hbox);
    GTK_WIDGET_UNSET_FLAGS(GTK_CLIST(view->clist)->column[i].button,
                           GTK_CAN_FOCUS);
  } 
  gtk_clist_column_titles_show(GTK_CLIST(view->clist));
  gtk_clist_set_shadow_type(GTK_CLIST(view->clist), GTK_SHADOW_ETCHED_IN);
  gtk_clist_set_selection_mode(GTK_CLIST(view->clist), GTK_SELECTION_EXTENDED);
  gtk_clist_set_use_drag_icons(GTK_CLIST(view->clist), TRUE);
  gtk_clist_set_row_height(GTK_CLIST(view->clist), 0);
  
  font = gdk_font_load(cfg.list_font);
  if (font != NULL)
  {
    style = gtk_style_copy(gtk_widget_get_style(view->clist));
    style->font = font;
    gtk_widget_set_style(view->clist, style);
  }

  for (i = 0; i < MAX_COLUMNS; i++)
  {
    gtk_clist_set_column_width(GTK_CLIST(view->clist), i, all_columns[i].size);
    gtk_clist_set_column_visibility(GTK_CLIST(view->clist), i,
                                    all_columns[i].is_visible);
  }

  gtk_signal_connect(GTK_OBJECT(view->clist), "select_row",
                     GTK_SIGNAL_FUNC(select_row_cb), view);
  gtk_signal_connect(GTK_OBJECT(view->clist), "button_press_event",
                     GTK_SIGNAL_FUNC(mouse_click_cb), view);
  gtk_signal_connect(GTK_OBJECT(view->clist), "key_press_event",
                     GTK_SIGNAL_FUNC(file_list_key_press_cb), view);
  gtk_signal_connect(GTK_OBJECT(view->clist), "click_column",
                     GTK_SIGNAL_FUNC(column_button_cb), view);
  gtk_signal_connect(GTK_OBJECT(view->clist), "drag_data_get",
                     GTK_SIGNAL_FUNC(clist_drag_data_get), view);
  gtk_signal_connect(GTK_OBJECT(view->clist), "drag_motion",
                     GTK_SIGNAL_FUNC(clist_drag_motion), view);
  gtk_signal_connect(GTK_OBJECT(view->clist), "drag_leave",
                     GTK_SIGNAL_FUNC(clist_drag_leave), view);
  gtk_signal_connect(GTK_OBJECT(view->clist), "drag_data_received",
                     GTK_SIGNAL_FUNC(clist_drag_data_received), view);
  gtk_drag_dest_set(view->clist, GTK_DEST_DEFAULT_MOTION |
    GTK_DEST_DEFAULT_HIGHLIGHT | GTK_DEST_DEFAULT_DROP, target_table,
    n_targets, GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK |
    GDK_ACTION_ASK);

  gtk_container_add(GTK_CONTAINER(sw), view->clist);
  gtk_widget_show(view->clist);

  /* Set the CLIST_COLOR for resetting from the DRAG_HILIGHT color */
  {
    GtkStyle *style = gtk_widget_get_style(view->clist);
    CLIST_COLOR.red   = style->base[GTK_STATE_NORMAL].red;
    CLIST_COLOR.green = style->base[GTK_STATE_NORMAL].green;
    CLIST_COLOR.blue  = style->base[GTK_STATE_NORMAL].blue;
  }

  return vbox;
}

