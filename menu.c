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

/* 9-20-2000: Modified by Aurelien Gateau
 *            Added code for Named actions for filetypes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "emelfm.h"

/*
 * Bookmark Callbacks
 */
static void
goto_bookmark(GtkWidget *menu_item, FileView  *view)
{
  gchar *dir = gtk_widget_get_name(GTK_WIDGET(menu_item));

  change_dir(view, dir);
}

static void
bookmark_add_cb(GtkWidget *widget, FileView *view)
{
  GtkWidget *menu_item = gtk_menu_item_new_with_label(view->dir);

  gtk_widget_set_name(GTK_WIDGET(menu_item), view->dir);
  gtk_signal_connect(GTK_OBJECT(menu_item), "activate",
                     GTK_SIGNAL_FUNC(goto_bookmark), curr_view);
  gtk_menu_append(GTK_MENU(curr_view->bookmark_menu), menu_item);
  gtk_widget_show(menu_item);

  menu_item = gtk_menu_item_new_with_label(view->dir);
  gtk_widget_set_name(GTK_WIDGET(menu_item), view->dir);
  gtk_signal_connect(GTK_OBJECT(menu_item), "activate",
                     GTK_SIGNAL_FUNC(goto_bookmark), other_view);
  gtk_menu_append(GTK_MENU(other_view->bookmark_menu), menu_item);
  gtk_widget_show(menu_item);

  cfg.bookmarks = g_list_append(cfg.bookmarks, g_strdup(view->dir));
  write_bookmarks_file();
  touch_config_dir();
}

static void
edit_bookmarks_cb(GtkWidget *widget)
{
  create_config_dialog(BOOKMARKS);
}

/*
 * Right Click Menu Callbacks
 */
static void
choose_action_cb(GtkWidget *widget)
{
  exec_filetype_action(gtk_widget_get_name(widget));
}

static void
edit_filetype_cb(GtkWidget *widget)
{
  FileInfo *info;
  FileType *ft;
  gchar *ext;

  info = gtk_clist_get_row_data(GTK_CLIST(curr_view->clist), curr_view->row);

  if ((ext = strchr(info->filename, '.')) == NULL)
  {
    status_message(_("File does not have an extension\n"));
    return;
  }

  do
  {
    ext++;
    if ((ft = get_filetype_for_ext(ext)) != NULL)
    {
      gtk_widget_set_sensitive(app.main_window, FALSE);
      create_filetype_dialog(&ft, TRUE);
      gtk_main();
      gtk_widget_set_sensitive(app.main_window, TRUE);
      return;
    }
  } while ((ext = strchr(ext, '.')) != NULL);

  status_message(_("Filetype not found.\n"));
  return;
}

/*
 * Filter Menu Callbacks
 */
static void
filename_filter_cb(GtkWidget *widget, FileView *view)
{
  create_filename_filter_dialog(view);
}

static void
size_filter_cb(GtkWidget *widget, FileView *view)
{
  create_size_filter_dialog(view);
}

static void
date_filter_cb(GtkWidget *widget, FileView *view)
{
  create_date_filter_dialog(view);
}

static void
filter_dirs_cb(GtkWidget *widget, FileView *view)
{
  view->filter_directories = GTK_CHECK_MENU_ITEM(widget)->active;
  refresh_list(view);
}

static void
remove_filters_cb(GtkWidget *widget, FileView *view)
{
  remove_filters(view);
}

/*
 * User Command Callback
 */
static void
exec_user_command(GtkWidget *widget)
{
  GString *command;

  if ((command = expand_macros(gtk_widget_get_name(widget), NULL)) != NULL)
  {
    do_command(command->str);
    g_string_free(command, TRUE);
  }
}

static void
edit_user_menu_cb(GtkWidget *widget)
{
  create_config_dialog(USER_COMMANDS);
}

/* Plugins Menu Callbacks */
static void
edit_plugins_cb(GtkWidget *widget)
{
  create_config_dialog(PLUGINS);
}

/* */
GtkWidget *
create_filelist_menu_bar(FileView *view)
{
  GtkWidget *menu_bar;
  GtkWidget *menu;
  menu_bar = gtk_menu_bar_new();

  /* Bookmark Menu */
  view->bookmark_menu = gtk_menu_new();
  add_menu_item(view->bookmark_menu, _("Add Bookmark"), bookmark_add_cb, view);
  add_menu_item(view->bookmark_menu, _("Edit Bookmarks"), edit_bookmarks_cb, NULL);
  add_menu_separator(view->bookmark_menu);
  view->bookmark_menu_item = add_submenu(menu_bar, _("_Bookmarks"), view->bookmark_menu);

  /* Filter Menu */
  menu = gtk_menu_new();
  add_menu_item(menu, _("Filename Filter"), filename_filter_cb, view);
  add_menu_item(menu, _("Size Filter"), size_filter_cb, view);
  add_menu_item(menu, _("Date Filter"), date_filter_cb, view);
  add_menu_separator(menu);
  add_menu_check_button(menu, _("Filter Directories"), FALSE,
                        filter_dirs_cb, view);
  add_menu_separator(menu);
  add_menu_item(menu, _("Remove All Filters"), remove_filters_cb, view);
  view->filter_menu_item = add_submenu(menu_bar, _("_Filters"), menu);
  if (view->filename_filter.active
      || view->size_filter.active
      || view->date_filter.active)
  {
    set_filter_menu_active(view);
  }

  return menu_bar;
}

static GtkWidget *
create_plugins_menu()
{
  GList *tmp;
  GtkWidget *menu;

  menu = gtk_menu_new();
  add_menu_item(menu, _("Edit Plugins..."), edit_plugins_cb, NULL);
  add_menu_separator(menu);

  for (tmp = cfg.plugins; tmp != NULL; tmp = tmp->next)
  {
    Plugin *p = tmp->data;
    if (p->show_in_menu)
      add_menu_item(menu, p->name, do_plugin_action, p);
  }
  return menu;
}

static GtkWidget *
create_user_command_menu()
{
  GList *tmp;
  GtkWidget *menu;
  GtkWidget *menu_item;

  menu = gtk_menu_new();
  add_menu_item(menu, _("Edit User Commands..."), edit_user_menu_cb, NULL);
  add_menu_separator(menu);

  for (tmp = cfg.user_commands; tmp != NULL; tmp = tmp->next)
  {
    UserCommand *command = tmp->data;

    menu_item = gtk_menu_item_new_with_label(command->name);
    gtk_widget_set_name(GTK_WIDGET(menu_item), command->action);
    gtk_signal_connect(GTK_OBJECT(menu_item), "activate",
                       GTK_SIGNAL_FUNC(exec_user_command), NULL);
    gtk_menu_append(GTK_MENU(menu), menu_item);
    gtk_widget_show(menu_item);
  }

  return menu;
}

static void
append_standard_menu_items(GtkWidget *menu)
{
  if (cfg.expand_popup_menu)
  {
    add_menu_item(menu, _("File Info..."), file_info_cb, NULL);
    add_menu_item(menu, _("Permissions..."), permissions_cb, NULL);
    add_menu_item(menu, _("User/Group..."), ownership_cb, NULL);
    add_menu_separator(menu);
    add_menu_item(menu, _("Rename..."), rename_cb, NULL);
    add_menu_item(menu, _("Copy"), copy_cb, NULL);
    add_menu_item(menu, _("Copy As..."), copy_as_cb, NULL);
    add_menu_item(menu, _("Move"), move_cb, NULL);
    add_menu_item(menu, _("Move As..."), move_as_cb, NULL);
    add_menu_item(menu, _("SymLink"), symlink_cb, NULL);
    add_menu_item(menu, _("SymLink As..."), symlink_as_cb, NULL);
    add_menu_item(menu, _("MkDir..."), mkdir_cb, NULL);
    add_menu_item(menu, _("Delete"), delete_cb, NULL);
  }
  else
  {
    GtkWidget *sub_menu = gtk_menu_new();
    GtkWidget *menu_item;

    add_menu_item(sub_menu, _("File Info..."), file_info_cb, NULL);
    add_menu_item(sub_menu, _("Permissions..."), permissions_cb, NULL);
    add_menu_item(sub_menu, _("User/Group..."), ownership_cb, NULL);
    menu_item = add_menu_item(menu, _("Properties"), NULL, NULL);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), sub_menu);

    sub_menu = gtk_menu_new();
    add_menu_item(sub_menu, _("Rename..."), rename_cb, NULL);
    add_menu_item(sub_menu, _("Copy"), copy_cb, NULL);
    add_menu_item(sub_menu, _("Copy As..."), copy_as_cb, NULL);
    add_menu_item(sub_menu, _("Move"), move_cb, NULL);
    add_menu_item(sub_menu, _("Move As..."), move_as_cb, NULL);
    add_menu_item(sub_menu, _("SymLink"), symlink_cb, NULL);
    add_menu_item(sub_menu, _("SymLink As..."), symlink_as_cb, NULL);
    add_menu_item(sub_menu, _("MkDir..."), mkdir_cb, NULL);
    add_menu_item(sub_menu, _("Delete"), delete_cb, NULL);
    menu_item = add_menu_item(menu, _("Operations"), NULL, NULL);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), sub_menu);
  }
}

void
create_menus()
{
  GtkWidget *menu_item;

  app.plugins_menu = create_plugins_menu();
  app.user_command_menu = create_user_command_menu();

  /* Standard Popup Menu) */
  if (app.main_menu != NULL)
    gtk_widget_destroy(app.main_menu);
  app.main_menu = gtk_menu_new();
  add_menu_item(app.main_menu, _("Open"), open_cb, NULL);
  add_menu_item(app.main_menu, _("Open with..."), open_with_cb, NULL);
  add_menu_item(app.main_menu, _("View"), view_cb, NULL);
  app.main_ft_menu.actions_menu = add_menu_item(app.main_menu,
                           _("Choose action"), NULL, NULL);
  app.main_ft_menu.edit_ft_menu = add_menu_item(app.main_menu,
                           _("Edit filetype..."), edit_filetype_cb, NULL);
  add_menu_separator(app.main_menu);
  append_standard_menu_items(app.main_menu);
  add_menu_separator(app.main_menu);

  menu_item = add_menu_item(app.main_menu, _("User"), NULL, NULL);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), app.user_command_menu);
  menu_item = add_menu_item(app.main_menu, _("Plugins"), NULL, NULL);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), app.plugins_menu);
  add_menu_separator(app.main_menu);
  add_menu_item(app.main_menu, _("Configure..."), configure_cb, NULL);

  /* Popup menu for directories */
  if (app.dir_menu != NULL)
    gtk_widget_destroy(app.dir_menu);
  app.dir_menu = gtk_menu_new();
  add_menu_item(app.dir_menu, _("Open"), open_cb, NULL);
  add_menu_item(app.dir_menu, _("Open in Other Panel"),
                open_in_other_pane_cb, NULL);
  add_menu_item(app.dir_menu, _("Mount"), mount_cb, NULL);
  add_menu_item(app.dir_menu, _("UnMount"), umount_cb, NULL);

  add_menu_separator(app.dir_menu);
  append_standard_menu_items(app.dir_menu);
  add_menu_separator(app.dir_menu);

  menu_item = add_menu_item(app.dir_menu, _("User"), NULL, NULL);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), create_user_command_menu());
  menu_item = add_menu_item(app.dir_menu, _("Plugins"), NULL, NULL);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), create_plugins_menu());
  add_menu_separator(app.dir_menu);
  add_menu_item(app.dir_menu, _("Configure..."), configure_cb, NULL);

  /* Popup menu for executables */
  if (app.exec_menu != NULL)
    gtk_widget_destroy(app.exec_menu);
  app.exec_menu = gtk_menu_new();
  add_menu_item(app.exec_menu, _("Execute"), execute_cb, NULL);
  add_menu_item(app.exec_menu, _("Execute with args..."),
                execute_with_args_cb, NULL);
  add_menu_item(app.exec_menu, _("Execute in xterm"),
                execute_in_xterm_cb, NULL);
  add_menu_item(app.exec_menu, _("Execute in xterm with args..."),
                execute_in_xterm_with_args_cb, NULL);
  app.exec_ft_menu.actions_menu = add_menu_item(app.exec_menu,
                           _("Choose action"), NULL, NULL);
  app.exec_ft_menu.edit_ft_menu = add_menu_item(app.exec_menu,
                           _("Edit filetype..."), edit_filetype_cb, NULL);
  add_menu_separator(app.exec_menu);
  append_standard_menu_items(app.exec_menu);
  add_menu_separator(app.exec_menu);

  menu_item = add_menu_item(app.exec_menu, _("User"), NULL, NULL);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), create_user_command_menu());
  menu_item = add_menu_item(app.exec_menu, _("Plugins"), NULL, NULL);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), create_plugins_menu());
  add_menu_separator(app.exec_menu);
  add_menu_item(app.exec_menu, _("Configure..."), configure_cb, NULL);
}

static GtkWidget *
create_actions_menu(gchar *actions)
{
  gchar *s, *sep, *free_this;
  GtkWidget *menu = gtk_menu_new();
  GtkWidget *menu_item;

  actions = free_this = g_strdup(actions);

  while ((s = strchr(actions, ',')) != NULL)
  {
    *s++ = '\0';
    if ((sep = strchr(actions, '@')) != NULL)
    {
      *sep++ = '\0';
      menu_item = add_menu_item(menu, actions, choose_action_cb, NULL);
      gtk_widget_set_name(GTK_WIDGET(menu_item), sep);
    }
    else
    {
      menu_item = add_menu_item(menu, actions, choose_action_cb, NULL);
      gtk_widget_set_name(GTK_WIDGET(menu_item), actions);
    }
    actions = s;
  }

  /* get the last action */
  if ((sep = strchr(actions, '@')) != NULL)
  {
    *sep++ = '\0';
    menu_item = add_menu_item(menu, actions, choose_action_cb, NULL);
    gtk_widget_set_name(GTK_WIDGET(menu_item), sep);
  }
  else
  {
    menu_item = add_menu_item(menu, actions, choose_action_cb, NULL);
    gtk_widget_set_name(GTK_WIDGET(menu_item), actions);
  }
  g_free(free_this);
  return menu;
}

static void
set_filetype_menus(FileTypeMenu *ft_menu, gchar *filename)
{
  gchar *ext, *actions; 

  if ((ext = strchr(filename, '.')) != NULL)
  {
    do
    { /* Check all possible extensions for a matching filetype */
      ext++; /* get rid of the leading '.' */
      actions = get_actions_for_ext(ext);
      if (actions != NULL)
      {
        gtk_widget_set_sensitive(ft_menu->actions_menu, TRUE);
        gtk_widget_set_sensitive(ft_menu->edit_ft_menu, TRUE);
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(ft_menu->actions_menu), 
                                  create_actions_menu(actions));
        g_free(actions);
        return;
      }
    } while ((ext = strchr(ext, '.')) != NULL);
  }

  /* None of the extensions had a filetype */
  gtk_widget_set_sensitive(ft_menu->actions_menu, FALSE);
  gtk_widget_set_sensitive(ft_menu->edit_ft_menu, FALSE);
}

void
show_menu(guint button, guint32 time)
{
  FileInfo *info;
  
  disable_refresh();
  if (curr_view->tagged != NULL)
    info = curr_view->tagged->data;
  else
    info = gtk_clist_get_row_data(GTK_CLIST(curr_view->clist), curr_view->row);
  
  if (is_dir(info))
  {
    gtk_menu_popup(GTK_MENU(app.dir_menu), NULL, NULL, NULL, NULL,
                   button, time);
  }
  else if (is_executable(info))
  {
    set_filetype_menus(&app.exec_ft_menu, info->filename);
    gtk_menu_popup(GTK_MENU(app.exec_menu), NULL, NULL, NULL, NULL,
                   button, time);
  }
  else
  {
    set_filetype_menus(&app.main_ft_menu, info->filename);
    gtk_menu_popup(GTK_MENU(app.main_menu), NULL, NULL, NULL, NULL,
                   button, time);
  }

  reenable_refresh();
  return;
}

void
show_plugins_menu(guint button, guint32 time)
{
  gtk_menu_popup(GTK_MENU(app.plugins_menu), NULL, NULL, NULL, NULL,
                 button, time);
}

void
show_user_command_menu(guint button, guint32 time)
{
  gtk_menu_popup(GTK_MENU(app.user_command_menu), NULL, NULL, NULL, NULL,
                 button, time);
}

void
load_bookmarks()
{
  GList *tmp;
  GtkWidget *menu_item;

  for (tmp = cfg.bookmarks; tmp != NULL; tmp = tmp->next)
  {
    gchar *bookmark = tmp->data;

    menu_item = gtk_menu_item_new_with_label(bookmark);
    gtk_widget_set_name(GTK_WIDGET(menu_item), bookmark);
    gtk_signal_connect(GTK_OBJECT(menu_item), "activate",
            GTK_SIGNAL_FUNC(goto_bookmark), &app.left_view);
    gtk_menu_append(GTK_MENU(app.left_view.bookmark_menu), menu_item);
    gtk_widget_show(menu_item);

    menu_item = gtk_menu_item_new_with_label(bookmark);
    gtk_widget_set_name(GTK_WIDGET(menu_item), bookmark);
    gtk_signal_connect(GTK_OBJECT(menu_item), "activate",
            GTK_SIGNAL_FUNC(goto_bookmark), &app.right_view);
    gtk_menu_append(GTK_MENU(app.right_view.bookmark_menu), menu_item);
    gtk_widget_show(menu_item);
  }
}

