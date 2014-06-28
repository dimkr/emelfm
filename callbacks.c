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
#include <unistd.h>
#include <string.h>
#include "emelfm.h"

void
copy_cb(GtkWidget *widget)
{
  FileInfo *info;
  GList *tmp, *base;
  gchar src[PATH_MAX+NAME_MAX], dest[PATH_MAX+NAME_MAX];
  gboolean check = cfg.confirm_overwrite;
  gint result;

  set_cursor(GDK_WATCH);
  disable_refresh();
  base = tmp = get_selection(curr_view);

  for (; tmp != NULL; tmp = tmp->next)
  {
    info = tmp->data;
    g_snprintf(src, sizeof(src), "%s/%s", curr_view->dir, info->filename);
    g_snprintf(dest, sizeof(dest), "%s/%s", other_view->dir, info->filename);
    if (check && (access(dest, F_OK) == 0))
    {
      gtk_widget_set_sensitive(app.main_window, FALSE);
      result = op_with_ow_check(src, dest, file_copy);
      gtk_widget_set_sensitive(app.main_window, TRUE);
      if (result == YES_TO_ALL)
        check = FALSE;
      else if (result == CANCEL)
        break;
    }
    else
      file_copy(src, dest);
  }

  
  g_list_free(base);
  refresh_list(other_view);
  refresh_list(other_view);
  set_cursor(GDK_LEFT_PTR);
  reenable_refresh();
}

void
copy_as_cb(GtkWidget *widget)
{
  FileInfo *info;
  GList *tmp, *base;
  gchar label[NAME_MAX+32];
  gchar dest[PATH_MAX+NAME_MAX];
  gchar src[PATH_MAX+NAME_MAX];
  gchar *new_name;
  gboolean check = cfg.confirm_overwrite;
  gint result;

  set_cursor(GDK_WATCH);
  gtk_widget_set_sensitive(app.main_window, FALSE);
  disable_refresh();
  base = tmp = get_selection(curr_view);

  for (; tmp != NULL; tmp = tmp->next)
  {
    info = tmp->data;
    strncpy(src, info->filename, sizeof(src));

    g_snprintf(label, sizeof(label), _("Enter new filename for: %s"), src);
    create_user_prompt(label, src, FALSE, &new_name);
    gtk_main();

    if (new_name != NULL)
    {
      g_snprintf(dest, sizeof(dest), "%s/%s", other_view->dir, new_name);
      g_snprintf(src, sizeof(src), "%s/%s", curr_view->dir, info->filename);
      if (check && (access(dest, F_OK) == 0))
      {
        gtk_widget_set_sensitive(app.main_window, FALSE);
        result = op_with_ow_check(src, dest, file_copy);
        gtk_widget_set_sensitive(app.main_window, TRUE);
        if (result == YES_TO_ALL)
          check = FALSE;
        else if (result == CANCEL)
          break;
      }
      else
        file_copy(src, dest);
      g_free(new_name);
    }
  }

  g_list_free(base);
  gtk_widget_set_sensitive(app.main_window, TRUE);
  refresh_list(other_view);
  refresh_list(other_view);
  set_cursor(GDK_LEFT_PTR);
  reenable_refresh();
}

void
move_cb(GtkWidget *widget)
{
  FileInfo *info;
  GList *tmp, *base;
  gchar src[PATH_MAX+NAME_MAX], dest[PATH_MAX+NAME_MAX];
  gboolean check = cfg.confirm_overwrite;
  gint result;

  set_cursor(GDK_WATCH);
  disable_refresh();
  base = tmp = get_selection(curr_view);

  for (; tmp != NULL; tmp = tmp->next)
  {
    info = tmp->data;
    g_snprintf(src, sizeof(src), "%s/%s", curr_view->dir, info->filename);
    g_snprintf(dest, sizeof(dest), "%s/%s", other_view->dir, info->filename);
    if (check && (access(dest, F_OK) == 0))
    {
      gtk_widget_set_sensitive(app.main_window, FALSE);
      result = op_with_ow_check(src, dest, file_move);
      gtk_widget_set_sensitive(app.main_window, TRUE);
      if (result == YES_TO_ALL)
        check = FALSE;
      else if (result == CANCEL)
        break;
    }
    else
      file_move(src, dest);
  }
  g_list_free(base);

  refresh_list(other_view);
  refresh_list(other_view);
  gtk_clist_unselect_all(GTK_CLIST(curr_view->clist));
  set_cursor(GDK_LEFT_PTR);
  reenable_refresh();
}

void
move_as_cb(GtkWidget *widget)
{
  FileInfo *info;
  GList *tmp, *base;
  gchar label[NAME_MAX+32];
  gchar dest[PATH_MAX+NAME_MAX];
  gchar src[PATH_MAX+NAME_MAX];
  gchar *new_name;
  gboolean check = cfg.confirm_overwrite;
  gint result;

  set_cursor(GDK_WATCH);
  gtk_widget_set_sensitive(app.main_window, FALSE);
  disable_refresh();
  base = tmp = get_selection(curr_view);

  for (; tmp != NULL; tmp = tmp->next)
  {
    info = tmp->data;
    strncpy(src, info->filename, sizeof(src));

    g_snprintf(label, sizeof(label), _("Enter new filename for: %s"), src);
    create_user_prompt(label, src, FALSE, &new_name);
    gtk_main();

    if (new_name != NULL)
    {
      g_snprintf(dest, sizeof(dest), "%s/%s", other_view->dir, new_name);
      g_snprintf(src, sizeof(src), "%s/%s", curr_view->dir, info->filename);
      if (check && (access(dest, F_OK) == 0))
      {
        gtk_widget_set_sensitive(app.main_window, FALSE);
        result = op_with_ow_check(src, dest, file_move);
        gtk_widget_set_sensitive(app.main_window, TRUE);
        if (result == YES_TO_ALL)
          check = FALSE;
        else if (result == CANCEL)
          break;
      }
      else
        file_move(src, dest);
      g_free(new_name);
    }
  }

  g_list_free(base);
  gtk_widget_set_sensitive(app.main_window, TRUE);
  refresh_list(other_view);
  refresh_list(other_view);
  gtk_clist_unselect_all(GTK_CLIST(curr_view->clist));
  set_cursor(GDK_LEFT_PTR);
  reenable_refresh();
}

void
delete_cb(GtkWidget *widget)
{
  FileInfo *info;
  gchar filename[PATH_MAX+NAME_MAX];
  GList *tmp, *base;
  gint confirm_button;
  gboolean check = cfg.confirm_delete;

  set_cursor(GDK_WATCH);
  gtk_widget_set_sensitive(app.main_window, FALSE);
  disable_refresh();
  base = tmp = get_selection(curr_view);

  for (; tmp != NULL; tmp = tmp->next)
  {
    info = tmp->data;
    g_snprintf(filename, sizeof(filename), "%s/%s",
               curr_view->dir, info->filename);
    if (check)
    {
      create_confirm_del_dialog(info->filename, &confirm_button);
      gtk_main();
      if (confirm_button == YES)
      {
        file_delete(filename);
      }
      else if (confirm_button == YES_TO_ALL)
      {
        file_delete(filename);
        check = FALSE;
      }
      else if (confirm_button == CANCEL)
      {
        break;
      }
    }
    else
      file_delete(filename);
  }
  g_list_free(base);

  gtk_widget_set_sensitive(app.main_window, TRUE);
  refresh_list(curr_view);
  gtk_clist_unselect_all(GTK_CLIST(curr_view->clist));
  set_cursor(GDK_LEFT_PTR);
  reenable_refresh();
}

void
symlink_cb(GtkWidget *widget)
{
  FileInfo *info;
  gchar src[PATH_MAX+NAME_MAX], dest[PATH_MAX+NAME_MAX];
  GList *tmp, *base;

  set_cursor(GDK_WATCH);
  disable_refresh();
  base = tmp = get_selection(curr_view);

  for (; tmp != NULL; tmp = tmp->next)
  {
    info = tmp->data;
    g_snprintf(src, sizeof(src), "%s/%s", curr_view->dir, info->filename);
    g_snprintf(dest, sizeof(dest),  "%s/%s", other_view->dir, info->filename);

    if (file_symlink(src, dest) == -1)
      status_errno();
  }
  g_list_free(base);

  refresh_list(other_view);
  refresh_list(other_view);
  set_cursor(GDK_LEFT_PTR);
  reenable_refresh();
}

void
symlink_as_cb(GtkWidget *widget)
{
  FileInfo *info;
  gchar label[NAME_MAX+32];
  gchar *new_name;
  gchar dest[PATH_MAX+NAME_MAX];
  gchar src[PATH_MAX+NAME_MAX];
  GList *tmp, *base;

  set_cursor(GDK_WATCH);
  gtk_widget_set_sensitive(app.main_window, FALSE);
  disable_refresh();
  base = tmp = get_selection(curr_view);

  for (; tmp != NULL; tmp = tmp->next)
  {
    info = tmp->data;
    strncpy(src, info->filename, sizeof(src));

    g_snprintf(label, sizeof(label), _("Enter name for symlink to: %s"), src);
    create_user_prompt(label, src, FALSE, &new_name);
    gtk_main();

    if (new_name != NULL)
    {
      g_snprintf(dest, sizeof(dest), "%s/%s", other_view->dir, new_name);
      g_snprintf(src, sizeof(src), "%s/%s", curr_view->dir, info->filename);
      if (file_symlink(src, dest) == -1)
        status_errno();
      g_free(new_name);
    }
  }

  g_list_free(base);
  gtk_widget_set_sensitive(app.main_window, TRUE);
  refresh_list(other_view);
  refresh_list(other_view);
  set_cursor(GDK_LEFT_PTR);
  reenable_refresh();
}

void
permissions_cb(GtkWidget *widget)
{
  FileInfo *info;
  GList *tmp, *base;
  gchar path[PATH_MAX+NAME_MAX];

  guint answer;
  gboolean recurse;
  GString *mode;

  set_cursor(GDK_WATCH);
  gtk_widget_set_sensitive(app.main_window, FALSE);
  disable_refresh();
  base = tmp = get_selection(curr_view);

  for (; tmp != NULL; tmp = tmp->next)
  {
    info = tmp->data;
    create_permissions_dialog(info, &answer, &recurse, &mode);
    gtk_main();

    if (answer == OK)
    {
      g_snprintf(path, sizeof(path), "%s/%s", curr_view->dir, info->filename);
      file_chmod(path, mode->str, recurse);
      g_string_free(mode, TRUE);
    }
    else if (answer == APPLY_TO_ALL)
    {
      for (; tmp != NULL; tmp = tmp->next)
      {
        info = tmp->data;
        g_snprintf(path, sizeof(path), "%s/%s", curr_view->dir, info->filename);
        file_chmod(path, mode->str, recurse);
      }
      g_string_free(mode, TRUE);
      break;
    }
    else if (answer == CANCEL)
      break;
  }

  g_list_free(base);
  gtk_widget_set_sensitive(app.main_window, TRUE);
  refresh_list(curr_view);
  set_cursor(GDK_LEFT_PTR);
  reenable_refresh();
}

void
ownership_cb(GtkWidget *widget)
{
  FileInfo *info;
  GList *tmp, *base;
  gchar path[PATH_MAX+NAME_MAX];

  uid_t owner_id;
  gid_t group_id;
  gboolean recurse;
  guint answer;

  set_cursor(GDK_WATCH);
  gtk_widget_set_sensitive(app.main_window, FALSE);
  disable_refresh();
  base = tmp = get_selection(curr_view);

  for (; tmp != NULL; tmp = tmp->next)
  {
    info = tmp->data;
    create_ownership_dialog(info, &answer, &owner_id, &group_id, &recurse);
    gtk_main();

    if (answer == OK)
    {
      g_snprintf(path, sizeof(path), "%s/%s", curr_view->dir, info->filename);
      file_chown(path, owner_id, group_id, recurse);
    }
    else if (answer == APPLY_TO_ALL)
    {
      for (; tmp != NULL; tmp = tmp->next)
      {
        info = tmp->data;
        g_snprintf(path, sizeof(path), "%s/%s", curr_view->dir, info->filename);
        file_chown(path, owner_id, group_id, recurse);
      }
      break;
    }
    else if (answer == CANCEL)
      break;
  }

  g_list_free(base);
  gtk_widget_set_sensitive(app.main_window, TRUE);
  refresh_list(curr_view);
  set_cursor(GDK_LEFT_PTR);
  reenable_refresh();
}

void
file_info_cb(GtkWidget *widget)
{
  FileInfo *info;
  GList *tmp, *base;

  set_cursor(GDK_WATCH);
  gtk_widget_set_sensitive(app.main_window, FALSE);
  disable_refresh();
  base = tmp = get_selection(curr_view);

  for (; tmp != NULL; tmp = tmp->next)
  {
    info = tmp->data;
    create_file_info_dialog(info);
    gtk_main();
  }

  g_list_free(tmp);
  gtk_widget_set_sensitive(app.main_window, TRUE);
  focus_row(curr_view, curr_view->row, FALSE, FALSE, TRUE);
  set_cursor(GDK_LEFT_PTR);
  reenable_refresh();
}

void
rename_cb(GtkWidget *widget)
{
  FileInfo *info;
  GList *tmp, *base, *new_list = NULL;
  gchar label[NAME_MAX+32];
  gchar src[PATH_MAX+NAME_MAX];
  gchar dest[PATH_MAX+NAME_MAX];
  gchar *new_name;
  gboolean check = cfg.confirm_overwrite;
  gint result;

  set_cursor(GDK_WATCH);
  gtk_widget_set_sensitive(app.main_window, FALSE);
  disable_refresh();
  base = tmp = get_selection(curr_view);

  for (; tmp != NULL; tmp = tmp->next)
  {
    info = tmp->data;
    strncpy(src, info->filename, sizeof(src));

    g_snprintf(label, sizeof(label), _("Enter new filename for: %s"), src);
    create_user_prompt(label, src, FALSE, &new_name);
    gtk_main();

    if (new_name != NULL)
    {
      g_snprintf(dest, sizeof(dest), "%s/%s", curr_view->dir, new_name);
      g_snprintf(src, sizeof(src), "%s/%s", curr_view->dir, info->filename);
      if (check && (access(dest, F_OK) == 0))
      {
        gtk_widget_set_sensitive(app.main_window, FALSE);
        result = op_with_ow_check(src, dest, file_move);
        gtk_widget_set_sensitive(app.main_window, TRUE);
        if (result == YES_TO_ALL)
          check = FALSE;
        else if (result == CANCEL)
          break;
      }
      else
        file_move(src, dest);

      new_list = g_list_append(new_list, new_name);
    }
  }

  g_list_free(base);
  refresh_list(curr_view);
  if (new_list != NULL)
  {
    gtk_clist_unselect_all(GTK_CLIST(curr_view->clist));
    for (tmp = new_list; tmp != NULL; tmp = tmp->next)
      select_row_by_filename(curr_view, (gchar *)tmp->data);
    free_glist_data(&new_list);
  }
  gtk_widget_set_sensitive(app.main_window, TRUE);
  set_cursor(GDK_LEFT_PTR);
  reenable_refresh();
}

void
sync_dirs_cb(GtkWidget *widget)
{
  change_dir(other_view, curr_view->dir);
}

void
mkdir_cb(GtkWidget *widget)
{
  gchar *new_dir;
  gchar path[PATH_MAX];

  set_cursor(GDK_WATCH);
  gtk_widget_set_sensitive(app.main_window, FALSE);
  disable_refresh();
  create_user_prompt(_("Enter name for new directory:"), "", FALSE, &new_dir);
  gtk_main();

  if (new_dir != NULL)
  {
    g_snprintf(path, sizeof(path), "%s/%s", curr_view->dir, new_dir);
    if (file_mkdir(path) != 0)
      status_errno();
    refresh_list(curr_view);
    gtk_clist_unselect_all(GTK_CLIST(curr_view->clist));
    select_row_by_filename(curr_view, new_dir);
    g_free(new_dir);
  }

  gtk_widget_set_sensitive(app.main_window, TRUE);
  set_cursor(GDK_LEFT_PTR);
  reenable_refresh();
}

void
view_cb(GtkWidget *widget)
{
  FileInfo *info;
  GList *tmp, *base;

  disable_refresh();
  base = tmp = get_selection(curr_view);

  if (cfg.use_internal_viewer)
  {
    for (; tmp != NULL; tmp = tmp->next)
    {
      info = tmp->data;
      create_view_file_dialog(info->filename);
    }
  }
  else
  {
    gchar command[PATH_MAX];
    strncpy(command, cfg.viewer_command, sizeof(command));
    for (; tmp != NULL; tmp = tmp->next)
    {
      info = tmp->data;
      strncat(command, " \"", sizeof(command));
      strncat(command, info->filename, sizeof(command));
      strncat(command, "\"", sizeof(command));
    }
    file_exec(command);
  }

  g_list_free(base);
  reenable_refresh();
  gtk_widget_grab_focus(curr_view->clist);
}

void
toggle_tag_cb()
{
  GList *tmp = GTK_CLIST(curr_view->clist)->selection;
  FileInfo *info;
  gint row;

  gtk_signal_emit_by_name(GTK_OBJECT(curr_view->clist), "end-selection");

  for (; tmp != NULL; tmp = tmp->next)
  {
    row = (gint)tmp->data;
    info = gtk_clist_get_row_data(GTK_CLIST(curr_view->clist), row);
    if (g_list_find(curr_view->tagged, info) != NULL)
    {
      curr_view->tagged = g_list_remove(curr_view->tagged, info);
      gtk_clist_set_background(GTK_CLIST(curr_view->clist), row, &CLIST_COLOR);
    }
    else
    {
      curr_view->tagged = g_list_append(curr_view->tagged, info);
      gtk_clist_set_background(GTK_CLIST(curr_view->clist), row, &TAG_COLOR);
    }
  }
  focus_row(curr_view, curr_view->row+1, TRUE, TRUE, TRUE);
}

void
refresh_cb(GtkWidget *widget)
{
  set_cursor(GDK_WATCH);
  disable_refresh();
  /* Since the refresh_list will switch the curr_view and other_view... */
  refresh_list(other_view);
  refresh_list(other_view);
  reenable_refresh();
  set_cursor(GDK_LEFT_PTR);
}

void
configure_cb(GtkWidget *widget)
{
  create_config_dialog(GENERAL_1);
  gtk_widget_grab_focus(curr_view->clist);
}

void
quit_cb(GtkWidget *widget)
{
  set_cursor(GDK_WATCH);
  write_filetypes_file();
  write_bookmarks_file();
  write_user_commands_file();
  write_toolbar_file();
  write_keys_file();
  write_buttons_file();
  write_plugins_file();
  write_config_file();
  touch_config_dir();
  gtk_main_quit();
}

void find_cb(GtkWidget *widget)
{
  gchar *s, *pattern;
  gint i, last;

  gtk_widget_set_sensitive(app.main_window, FALSE);
  disable_refresh();
  create_user_prompt(_("Enter a filename or partial filename to find:"),
                     curr_view->last_find, TRUE, &s);
  gtk_main();

  if (s != NULL)
  {
    strncpy(curr_view->last_find, s, sizeof(curr_view->last_find));
    pattern = g_strdup_printf("*%s*", s);
    if (GTK_CLIST(curr_view->clist)->selection != NULL)
    {
      last = (gint)GTK_CLIST(curr_view->clist)->selection->data;
      if (last == (GTK_CLIST(curr_view->clist)->rows - 1))
        i = 0;
      else
        i = last + 1;
    }
    else
    {
      i = 0;
      last = GTK_CLIST(curr_view->clist)->rows - 1;
      if (last < 0)
      {
        g_free(pattern);
        g_free(s);
        reenable_refresh();
        gtk_widget_set_sensitive(app.main_window, TRUE);
        return;
      }
    }

    while (i != last)
    {
      FileInfo *info = gtk_clist_get_row_data(GTK_CLIST(curr_view->clist), i);
      if (gtk_pattern_match_simple(pattern, info->filename))
      {
        focus_row(curr_view, i, TRUE, TRUE, TRUE);
        break;
      }

      i++;
      if (i >= GTK_CLIST(curr_view->clist)->rows)
        i = 0;
    }
    if (i == last)
      focus_row(curr_view, last, TRUE, FALSE, TRUE);
    g_free(pattern);
    g_free(s);
  }
  else
    focus_row(curr_view, curr_view->row, FALSE, FALSE, TRUE);

  reenable_refresh();
  gtk_widget_set_sensitive(app.main_window, TRUE);
}

void
open_cb(GtkWidget *widget)
{
  FileInfo *info;
  gchar *ext, *action, *filename, *sep;

  disable_refresh();
  if ((info = get_first_selected(curr_view)) == NULL)
  {
    reenable_refresh();
    return;
  }

  if (is_dir(info))
  {
    gchar path[PATH_MAX];
    g_snprintf(path, sizeof(path), "%s/%s", curr_view->dir, info->filename);
    change_dir(curr_view, path);
    reenable_refresh();
    return;
  }

  ext = strchr(info->filename, '.');
  if (ext == NULL) /* no extension */
  {
    if (S_ISEXE(info->statbuf.st_mode))
    {
      gchar command_line[NAME_MAX+2];
      g_snprintf(command_line, sizeof(command_line), "./%s", info->filename);
      exec_and_capture_output_threaded(command_line);
    }
    else if (is_text(info->filename))
    {
      if (cfg.use_internal_viewer)
        create_view_file_dialog(info->filename);
      else
        view_file(info->filename);
    }
    else
    {
      create_open_query_dialog(info->filename);
    }
    reenable_refresh();
    return;
  }

  if (ext == info->filename) /* its a dot file */
  {
    if (is_text(info->filename))
    {
      if (cfg.use_internal_viewer)
        create_view_file_dialog(info->filename);
      else
        view_file(info->filename);
    }
    else
    {
      create_open_query_dialog(info->filename);
    }
    reenable_refresh();
    return;
  }

  /* the file has an extension */
  do
  {
    ext++; /* get rid of the . prefix */
    action = get_default_action_for_ext(ext);
    if (action != NULL)
    {
      if ((sep = strchr(action, '@')) != NULL)
        exec_filetype_action(sep+1);
      else
        exec_filetype_action(action);
      g_free(action);
      reenable_refresh();
      return;
    }
  } while ((ext = strchr(ext, '.')) != NULL);

  /* Didn't find any matching extensions */
  filename = g_strdup(info->filename);
  create_init_filetype_dialog(filename);
  g_free(filename);
  reenable_refresh();
}

void
open_with_cb(GtkWidget *widget)
{
  gchar *command;

  gtk_widget_set_sensitive(app.main_window, FALSE);
  create_user_prompt(_("Enter command:"), "", FALSE, &command);
  gtk_main();

  gtk_widget_set_sensitive(app.main_window, TRUE);
  gtk_widget_grab_focus(curr_view->clist);
  g_return_if_fail(command != NULL);

  if (strchr(command, '%') == NULL)
  {
    GString *command_line = g_string_new(command);
    GList *base, *tmp;

    disable_refresh();
    base = tmp = get_selection(curr_view);
    for (; tmp != NULL; tmp = tmp->next)
    {
      FileInfo *info = tmp->data;
      g_string_sprintfa(command_line , " \"%s\"", info->filename);
    }

    do_command(command_line->str);
    g_string_free(command_line, TRUE);
    reenable_refresh();
  }
  else
  {
    GString *new_command;
    if ((new_command = expand_macros(command, NULL)) != NULL)
    {
      do_command(new_command->str);
      g_string_free(new_command, TRUE);
    }
  }

  g_free(command);
}

void
open_in_other_pane_cb(GtkWidget *widget)
{
  gchar *path;
  FileInfo *info;
  
  disable_refresh();
  info = gtk_clist_get_row_data(GTK_CLIST(curr_view->clist), curr_view->row);
  path = g_strdup_printf("%s/%s", curr_view->dir, info->filename);
  change_dir(other_view, path);
  g_free(path);
  focus_row(other_view, other_view->row, FALSE, FALSE, TRUE);
  reenable_refresh();
}

void
mount_cb(GtkWidget *widget)
{
  gchar *mount_point;
  FileInfo *info;
  
  set_cursor(GDK_WATCH);
  disable_refresh();
  info = gtk_clist_get_row_data(GTK_CLIST(curr_view->clist), curr_view->row);
  mount_point = g_strdup_printf("%s/%s", curr_view->dir, info->filename);
  file_mount(mount_point);
  change_dir(curr_view, mount_point);
  g_free(mount_point);
  set_cursor(GDK_LEFT_PTR);
  reenable_refresh();
}

void
umount_cb(GtkWidget *widget)
{
  gchar *mount_point;
  FileInfo *info;
  
  set_cursor(GDK_WATCH);
  disable_refresh();
  info = gtk_clist_get_row_data(GTK_CLIST(curr_view->clist), curr_view->row);
  mount_point = g_strdup_printf("%s/%s", curr_view->dir, info->filename);
  file_umount(mount_point);
  g_free(mount_point);
  set_cursor(GDK_LEFT_PTR);
  reenable_refresh();
}

void
execute_cb(GtkWidget *widget)
{
  FileInfo *info;
  gchar *command;

  disable_refresh();
  if ((info = get_first_selected(curr_view)) == NULL)
  {
    reenable_refresh();
    return;
  }

  command = g_strdup_printf("./%s", info->filename);
  exec_and_capture_output_threaded(command);
  g_free(command);
  reenable_refresh();
}

void
execute_with_args_cb(GtkWidget *widget)
{
  FileInfo *info;
  gchar *command, *args;

  disable_refresh();
  if ((info = get_first_selected(curr_view)) == NULL)
  {
    reenable_refresh();
    return;
  }
  
  create_user_prompt("Enter arguments: ", "", FALSE, &args);
  gtk_main();
  if (args != NULL)
  {
    command = g_strdup_printf("./%s %s", info->filename, args);
    exec_and_capture_output_threaded(command);
    g_free(command);
    g_free(args);
  }

  reenable_refresh();
}

void
execute_in_xterm_cb(GtkWidget *widget)
{
  FileInfo *info;
  gchar *command;

  disable_refresh();
  if ((info = get_first_selected(curr_view)) == NULL)
  {
    reenable_refresh();
    return;
  }

  command = g_strdup_printf("./%s", info->filename);
  exec_in_xterm(command);
  g_free(command);
  reenable_refresh();
}

void
execute_in_xterm_with_args_cb(GtkWidget *widget)
{
  FileInfo *info;
  gchar *command, *args;

  disable_refresh();
  if ((info = get_first_selected(curr_view)) == NULL)
  {
    reenable_refresh();
    return;
  }
  
  create_user_prompt("Enter arguments: ", "", FALSE, &args);
  gtk_main();
  if (args != NULL)
  {
    command = g_strdup_printf("./%s %s", info->filename, args);
    exec_in_xterm(command);
    g_free(command);
    g_free(args);
  }

  reenable_refresh();
}

