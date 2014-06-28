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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include "emelfm.h"

typedef struct _DirHistoryEntry
{
  gchar path[PATH_MAX];
  gint row;
} DirHistoryEntry;

static GList *
history_find(GList *list, gchar *path)
{
  GList *tmp;
  DirHistoryEntry *entry;

  for (tmp = list; tmp != NULL; tmp = tmp->next)
  {
    entry = tmp->data;
    if (STREQ(entry->path, path))
      return tmp;
  }
  return NULL;
}

void
get_perm_string(gchar *buf, gint len, mode_t mode)
{
  gchar *perm_sets[] = {"---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"};
  gint u, g, o;

  u = (mode & S_IRWXU) >> 6;
  g = (mode & S_IRWXG) >> 3;
  o = (mode & S_IRWXO);

  g_snprintf(buf, len, "-%s%s%s", perm_sets[u], perm_sets[g], perm_sets[o]);
  
  if (S_ISLNK(mode))
    buf[0] = 'l';
  else if (S_ISDIR(mode))
    buf[0] = 'd';
  else if (S_ISBLK(mode))
    buf[0] = 'b';
  else if (S_ISCHR(mode))
    buf[0] = 'c';
  else if (S_ISFIFO(mode))
    buf[0] = 'f';
  else if (S_ISSOCK(mode))
    buf[0] = 's';

  if (mode & S_ISVTX)
    buf[9] = (buf[9] == '-') ? 'T' : 't';
  if (mode & S_ISGID)
    buf[6] = (buf[6] == '-') ? 'S' : 's';
  if (mode & S_ISUID)
    buf[3] = (buf[3] == '-') ? 'S' : 's';
}

gint
is_dir(FileInfo *info)
{
  if ( S_ISDIR(info->statbuf.st_mode) )
    return 1;
  if ( S_ISLNK(info->statbuf.st_mode) )
  {
    struct stat statbuf;
    stat(info->filename, &statbuf);
    if ( S_ISDIR(statbuf.st_mode) )
      return 1;
  }

  return 0;
}

void 
change_dir(FileView *view, gchar *path)
{
  GList *tmp = NULL;
  DirHistoryEntry *entry;

  if (access(path, F_OK) != 0)
  {
    status_message(_("That directory does not exist\n"));
    return;
  }
  if (access(path, R_OK) != 0)
  {
    status_message(_("You do not have read access on that directory\n"));
    return;
  }

  if (chdir(path))
  {
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(view->dir_entry)->entry),
                       view->dir);
    status_errno();
    return;
  }

  /* Save the last row before we reload the list */
  tmp = history_find(view->dir_history, view->dir);
  if (tmp != NULL)
  {
    entry = tmp->data;
    entry->row = view->row;
  }

  set_cursor(GDK_WATCH);
  getcwd(view->dir, PATH_MAX);
  load_dir_list(view);
  set_cursor(GDK_LEFT_PTR);

  tmp = history_find(view->dir_history, view->dir);
  if (tmp != NULL)
  {
    entry = tmp->data;

    focus_row(view, entry->row, TRUE, TRUE, TRUE);
    view->dir_history = g_list_remove_link(view->dir_history, tmp);
    view->dir_history = g_list_prepend(view->dir_history, entry);
    g_list_free(tmp);
  }
  else
  {
    entry = g_new0(DirHistoryEntry, 1);

    focus_row(view, 0, TRUE, TRUE, TRUE);
    strncpy(entry->path, view->dir, sizeof(entry->path));
    entry->row = 0;
    view->dir_history = g_list_prepend(view->dir_history, entry);
    if (g_list_length(view->dir_history) > cfg.dir_history_max_length)
    {
      tmp = g_list_nth(view->dir_history, cfg.dir_history_max_length);
      view->dir_history = g_list_remove_link(view->dir_history, tmp);
      free_glist_data(&tmp);
    }
  }
  gtk_combo_set_popdown_strings(GTK_COMBO(view->dir_entry),
                                view->dir_history);
}

gint
name_sort(GtkCList *clist, gconstpointer row1, gconstpointer row2)
{
  FileInfo *info1, *info2;

  info1 = ((GtkCListRow *)row1)->data;
  info2 = ((GtkCListRow *)row2)->data;

  if (STREQ(info1->filename, ".."))
    return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
  if (STREQ(info2->filename, ".."))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);

  if (is_dir(info1))
  {
    if (is_dir(info2))
      return strcmp(info1->filename, info2->filename);
    else
      return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
  }

  if (is_dir(info2))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);

  return strcmp(info1->filename, info2->filename);
}

gint
size_sort(GtkCList *clist, gconstpointer row1, gconstpointer row2)
{
  FileInfo *info1, *info2;

  info1 = ((GtkCListRow *)row1)->data;
  info2 = ((GtkCListRow *)row2)->data;

  if (STREQ(info1->filename, ".."))
    return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
  if (STREQ(info2->filename, ".."))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);

  if (is_dir(info1))
  {
    if (is_dir(info2))
      return (info1->statbuf.st_size - info2->statbuf.st_size);
    else
      return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
  }

  if (is_dir(info2))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);
  
  return (info1->statbuf.st_size - info2->statbuf.st_size);
}

gint
date_sort(GtkCList *clist, gconstpointer row1, gconstpointer row2)
{
  FileInfo *info1, *info2;

  info1 = ((GtkCListRow *)row1)->data;
  info2 = ((GtkCListRow *)row2)->data;

  if (STREQ(info1->filename, ".."))
    return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
  if (STREQ(info2->filename, ".."))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);

  if (is_dir(info1))
  {
    if (is_dir(info2))
      return (info1->statbuf.st_mtime - info2->statbuf.st_mtime);
    else
      return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
  }

  if (is_dir(info2))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);
  
  return (info1->statbuf.st_mtime - info2->statbuf.st_mtime);
}

gint
perm_sort(GtkCList *clist, gconstpointer row1, gconstpointer row2)
{
  FileInfo *info1, *info2;

  info1 = ((GtkCListRow *)row1)->data;
  info2 = ((GtkCListRow *)row2)->data;

  if (STREQ(info1->filename, ".."))
    return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
  if (STREQ(info2->filename, ".."))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);

  if (is_dir(info1))
  {
    if (is_dir(info2))
      return (info1->statbuf.st_mode - info2->statbuf.st_mode);
    else
      return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
  }

  if (is_dir(info2))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);
  
  return (info1->statbuf.st_mode - info2->statbuf.st_mode);
}
  
gint
user_sort(GtkCList *clist, gconstpointer row1, gconstpointer row2)
{
  FileInfo *info1, *info2;

  info1 = ((GtkCListRow *)row1)->data;
  info2 = ((GtkCListRow *)row2)->data;

  if (STREQ(info1->filename, ".."))
    return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
  if (STREQ(info2->filename, ".."))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);

  if (is_dir(info1))
  {
    if (is_dir(info2))
      return (info1->statbuf.st_uid - info2->statbuf.st_uid);
    else
      return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
  }

  if (is_dir(info2))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);
  
  return (info1->statbuf.st_uid - info2->statbuf.st_uid);
}

gint
group_sort(GtkCList *clist, gconstpointer row1, gconstpointer row2)
{
  FileInfo *info1, *info2;

  info1 = ((GtkCListRow *)row1)->data;
  info2 = ((GtkCListRow *)row2)->data;

  if (STREQ(info1->filename, ".."))
    return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
  if (STREQ(info2->filename, ".."))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);

  if (is_dir(info1))
  {
    if (is_dir(info2))
      return (info1->statbuf.st_gid - info2->statbuf.st_gid);
    else
      return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
  }

  if (is_dir(info2))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);
  
  return (info1->statbuf.st_gid - info2->statbuf.st_gid);
}

void
sort_list(FileView *view,
          GtkCListCompareFunc compare_func,
          GtkSortType direction,
          gint col)
{
  gtk_widget_hide(view->sort_arrows[GTK_CLIST(view->clist)->sort_column]);
  gtk_widget_show(view->sort_arrows[col]);
  gtk_arrow_set(GTK_ARROW(view->sort_arrows[col]),
                (direction == GTK_SORT_ASCENDING
                 ? GTK_ARROW_DOWN : GTK_ARROW_UP), GTK_SHADOW_IN);

  gtk_clist_set_compare_func(GTK_CLIST(view->clist), compare_func);
  gtk_clist_set_sort_type(GTK_CLIST(view->clist), direction);
  gtk_clist_set_sort_column(GTK_CLIST(view->clist), col);
  gtk_clist_sort(GTK_CLIST(view->clist));
}

void
focus_row(FileView *view,
          gint row,
          gboolean clear_selection,
          gboolean center,
          gboolean grab_focus)
{
  gtk_clist_freeze(GTK_CLIST(view->clist));
  if (clear_selection)
    gtk_clist_unselect_all(GTK_CLIST(view->clist));
  gtk_clist_select_row(GTK_CLIST(view->clist), row, 0);
  GTK_CLIST(view->clist)->focus_row = row;
  gtk_widget_draw_focus(view->clist);
  if (center)
    gtk_clist_moveto(GTK_CLIST(view->clist), row, 0, 0.5, 0.0);
  gtk_clist_thaw(GTK_CLIST(view->clist));
  if (grab_focus)
    gtk_widget_grab_focus(view->clist);
}

void
select_row_by_filename(FileView *view, gchar *filename)
{
  gint i;

  disable_refresh();
  for (i = 0; i < GTK_CLIST(view->clist)->rows; i++)
  {
    FileInfo *info = gtk_clist_get_row_data(GTK_CLIST(view->clist), i);
    if (STREQ(info->filename, filename))
    {
      focus_row(view, i, FALSE, TRUE, TRUE);
      break;
    }
  }
  reenable_refresh();
}

FileInfo *
get_first_selected(FileView *view)
{
  FileInfo *info = NULL;

  if (view->tagged != NULL)
    info = view->tagged->data;
  else
  {
    if (GTK_CLIST(view->clist)->selection != NULL)
    {
      info = gtk_clist_get_row_data(GTK_CLIST(view->clist),
                (gint)GTK_CLIST(view->clist)->selection->data);
    }
  }

  return info;
}

GList *
get_selection(FileView *view)
{
  GList *tmp;
  GList *row_data = NULL;

  gtk_signal_emit_by_name(GTK_OBJECT(view->clist), "end-selection");

  if (view->tagged != NULL)
    return g_list_copy(view->tagged);

  if (view == curr_view)
    tmp = GTK_CLIST(view->clist)->selection;
  else
    tmp = view->old_selection;

  for (; tmp != NULL; tmp = tmp->next)
  {
    FileInfo *info = gtk_clist_get_row_data(GTK_CLIST(view->clist),
                                            (gint)tmp->data);
    row_data = g_list_append(row_data, info);
  }

  return row_data;
}

void 
set_filter_menu_active(FileView *view)
{
  GtkStyle *style = gtk_style_copy(gtk_widget_get_style(
                    GTK_MENU_ITEM(view->filter_menu_item)->item.bin.child));
  GdkColor red;
  
  gdk_color_parse("red", &red);
  style->fg[GTK_STATE_NORMAL] = red;
  style->fg[GTK_STATE_PRELIGHT] = red;

  gtk_widget_set_style(GTK_MENU_ITEM(view->filter_menu_item)->item.bin.child,
                       style);
}

void
remove_filters(FileView *view)
{
  view->filename_filter.active = FALSE;
  view->size_filter.active = FALSE;
  view->date_filter.active = FALSE;

  gtk_widget_restore_default_style(GTK_MENU_ITEM(view->filter_menu_item)->item.bin.child);

  refresh_list(view);
}

void
initialize_filters(FileView *view)
{
  view->filter_directories = FALSE;

  view->filename_filter.active = FALSE;
  strncpy(view->filename_filter.pattern, "*",
          sizeof(view->filename_filter.pattern));
  view->filename_filter.case_sensitive = TRUE;

  view->size_filter.active = FALSE;
  view->size_filter.size = 0;
  view->size_filter.op = GT;

  view->date_filter.active = FALSE;
  view->date_filter.time = time(NULL);
  view->date_filter.time_type = ATIME;
  view->date_filter.op = GT;
}

static gboolean
match_filename_filter(FileView *view, FileInfo *info)
{
  gchar *s, *pattern, *free_this;
  gboolean result;

  if (is_dir(info) && !view->filter_directories)
     return TRUE;
  
  free_this = pattern = g_strdup(view->filename_filter.pattern);
  if (!view->filename_filter.case_sensitive)
    g_strdown(pattern);
  while ((s = strchr(pattern, ',')) != NULL)
  {
    *s = '\0';
    if (!view->filename_filter.case_sensitive)
    {
      gchar *dup = str_to_lower(info->filename);
      if (gtk_pattern_match_simple(pattern, dup))
      {
        g_free(free_this);
        g_free(dup);
        return !view->filename_filter.invert_mask;
      }
      g_free(dup);
    }
    else if (gtk_pattern_match_simple(pattern, info->filename))
    {
      g_free(free_this);
      return !view->filename_filter.invert_mask;
    }
    pattern = s+1;
  }

  if (!view->filename_filter.case_sensitive)
  {
    gchar *dup = str_to_lower(info->filename);
    result = gtk_pattern_match_simple(pattern, dup);
    g_free(dup);
  }
  else
    result = gtk_pattern_match_simple(pattern, info->filename);

  g_free(free_this);
  return (view->filename_filter.invert_mask ? !result : result);
}

static gboolean
match_size_filter(FileView *view, FileInfo *info)
{
  if (is_dir(info) && !view->filter_directories)
     return TRUE;

  switch (view->size_filter.op)
  {
    case GT:
      return (info->statbuf.st_size > view->size_filter.size);
      break;
    case LT:
      return (info->statbuf.st_size < view->size_filter.size);
      break;
    case EQ:
      return (info->statbuf.st_size == view->size_filter.size);
      break;
    default:
      return TRUE;
  }

  /* this can never happen */
  return TRUE;
}

static gboolean
match_date_filter(FileView *view, FileInfo *info)
{
  if (is_dir(info) && !view->filter_directories)
     return TRUE;

  switch (view->date_filter.time_type)
  {
    case ATIME:
      if (view->date_filter.op == GT)
        return (difftime(view->date_filter.time, info->statbuf.st_atime) < 0);
      else
        return (difftime(view->date_filter.time, info->statbuf.st_atime) > 0);
      break;
    case MTIME:
      if (view->date_filter.op == GT)
        return (difftime(view->date_filter.time, info->statbuf.st_mtime) < 0);
      else
        return (difftime(view->date_filter.time, info->statbuf.st_mtime) > 0);
      break;
    case CTIME:
      if (view->date_filter.op == GT)
        return (difftime(view->date_filter.time, info->statbuf.st_ctime) < 0);
      else
        return (difftime(view->date_filter.time, info->statbuf.st_ctime) > 0);
      break;
    default:
      return TRUE;
  }

  /* this cant happen */
  return TRUE;
}

/* NOTE: This function will cause curr_view and other_view to switch if */
/* curr_view isn't the view passed in */
void
refresh_list(FileView *view)
{
  GtkAdjustment *vadj;
  gfloat scrollbar_pos;
  GList *selection;

  g_return_if_fail(view != NULL);

  vadj = gtk_clist_get_vadjustment(GTK_CLIST(view->clist));
  scrollbar_pos = vadj->value;
  if (view == curr_view)
    selection = g_list_copy(GTK_CLIST(view->clist)->selection);
  else
    selection = g_list_copy(view->old_selection);
  gtk_clist_freeze(GTK_CLIST(view->clist));

  load_dir_list(view);

  gtk_adjustment_set_value(GTK_ADJUSTMENT(vadj), scrollbar_pos);
  gtk_clist_unselect_all(GTK_CLIST(view->clist));
  clist_select_rows(view->clist, selection);
  gtk_clist_thaw(GTK_CLIST(view->clist));
}

/* These functions must encapsulate any instructions that access the FileInfo
 * objects that are set as the 'row_data' of the FileView::clist objects.
 * This is because a refresh in the middle of such an operation would free
 * the FileInfo objects as a side-effect.
 */
static gint refresh_ref = 0;

void
disable_refresh()
{
  refresh_ref++;
  if (refresh_ref == 1)
  {
    gtk_timeout_remove(cfg.check_config_id);
    if (cfg.auto_refresh_enabled)
      gtk_timeout_remove(cfg.auto_refresh_id);
  }
}

void
reenable_refresh()
{
  refresh_ref--;
  if (refresh_ref < 0)
  {
    fprintf(stderr, _("Something is going wrong with the auto-refresh. Exit quickly\n"));
    if (cfg.output_text_hidden)
      show_hide_output_text(NULL);
    status_message(_("Something is going wrong with the auto-refresh.\n"
                   "Please send a bug report to author\n"));
  }

  if (refresh_ref == 0)
  {
    cfg.check_config_id = gtk_timeout_add(1000, check_config_files, NULL);
    if (cfg.auto_refresh_enabled)
      cfg.auto_refresh_id = gtk_timeout_add(1000, auto_refresh, NULL);
  }
}

/* This is the timeout function that gets installed if auto refresh is enabled
 */
gint
auto_refresh(gpointer data)
{
  struct stat statbuf;

  stat(curr_view->dir, &statbuf);
  if ((statbuf.st_mtime != curr_view->dir_mtime)
      && (curr_view->tagged == NULL))
  {
    GtkAdjustment *vadj;
    gfloat scrollbar_pos;
    GList *tmp;

    vadj = gtk_clist_get_vadjustment(GTK_CLIST(curr_view->clist));
    scrollbar_pos = vadj->value;
    
    tmp = g_list_copy(GTK_CLIST(curr_view->clist)->selection);
    gtk_clist_freeze(GTK_CLIST(curr_view->clist));
    load_dir_list(curr_view);

    gtk_adjustment_set_value(GTK_ADJUSTMENT(vadj), scrollbar_pos);
    gtk_clist_unselect_all(GTK_CLIST(curr_view->clist));
    for (; tmp != NULL; tmp = tmp->next)
    {
      gint i = (gint)tmp->data;
      if (i < GTK_CLIST(curr_view->clist)->rows)
        gtk_clist_select_row(GTK_CLIST(curr_view->clist), i, 0);
      if (tmp->next == NULL)
        GTK_CLIST(curr_view->clist)->focus_row = i;
    }
    gtk_widget_draw_focus(curr_view->clist);
    gtk_clist_thaw(GTK_CLIST(curr_view->clist));
  } 

  stat(other_view->dir, &statbuf);
  if ((statbuf.st_mtime != other_view->dir_mtime)
      && (other_view->tagged == NULL))
  {
    GtkAdjustment *vadj;
    gfloat scrollbar_pos;
    GList *tmp;

    vadj = gtk_clist_get_vadjustment(GTK_CLIST(other_view->clist));
    scrollbar_pos = vadj->value;
    load_dir_list(other_view);
    chdir(curr_view->dir);
    gtk_adjustment_set_value(GTK_ADJUSTMENT(vadj), scrollbar_pos);
    for (tmp = other_view->old_selection; tmp != NULL; tmp = tmp->next)
      gtk_clist_set_background(GTK_CLIST(other_view->clist), (gint)tmp->data,
                               &SELECT_COLOR);
  }

  return TRUE;
}

void
load_dir_list(FileView *view)
{
  DIR *dp;
  struct dirent *entry;
  struct stat statbuf;
  FileInfo info;

  g_return_if_fail(view != NULL);

  stat(view->dir, &statbuf); /* Stat before we opendir */
  if ((dp = opendir(view->dir)) == NULL)
  {
    fprintf(stderr, _("Unable to open directory: %s\n"), view->dir);
    return;
  }
  view->dir_mtime = statbuf.st_mtime;

  gtk_clist_freeze(GTK_CLIST(view->clist));
  gtk_clist_unselect_all(GTK_CLIST(view->clist));
  gtk_clist_clear(GTK_CLIST(view->clist));

  chdir(view->dir);
  while ((entry = readdir(dp)) != NULL)
  {
    FileInfo *this_info;
    struct tm *tm_ptr;
    struct passwd *pwd_buf;
    struct group *grp_buf;
    gchar filename_buf[NAME_MAX];
    gchar size_buf[20];
    gchar modified_buf[25];
    gchar accessed_buf[25];
    gchar changed_buf[25];
    gchar perm_buf[11];
    gchar uid_buf[20];
    gchar gid_buf[20];
    gchar *buf[MAX_COLUMNS];
    gint row;

    if (STREQ(entry->d_name, "."))
      continue;
    if (((entry->d_name[0] == '.') && (!STREQ(entry->d_name, "..")))
                                   && !view->show_hidden )
      continue;
    lstat(entry->d_name, &(info.statbuf));
    strncpy(info.filename, entry->d_name, sizeof(info.filename));

    if (view->filename_filter.active && !match_filename_filter(view, &info))
        continue;
    if (view->size_filter.active && !match_size_filter(view, &info))
        continue;
    if (view->date_filter.active && !match_date_filter(view, &info))
        continue;

    this_info = g_memdup(&info, sizeof(FileInfo));

    strcpy(filename_buf, this_info->filename);
    if (is_dir(this_info)) strcat(filename_buf, "/");
    buf[0] = filename_buf;

    if (this_info->statbuf.st_size < 10240) /* less than 10K */
    {
      g_snprintf(size_buf, sizeof(size_buf), _("%d bytes"),
                 (int)this_info->statbuf.st_size);
    }
    else if (this_info->statbuf.st_size < 1048576) /* less than a meg */
    {
      g_snprintf(size_buf, sizeof(size_buf), _("%.2f Kbytes"),
              ((float)this_info->statbuf.st_size / 1024.0));
    }
    else  /* more than a meg */
    {
      g_snprintf(size_buf, sizeof(size_buf), _("%.2f Mbytes"),
              ((float)this_info->statbuf.st_size / 1048576.0));
    }
    buf[1] = size_buf;

    tm_ptr = localtime(&(this_info->statbuf.st_mtime));
    strftime(modified_buf, sizeof(modified_buf), "%b  %d %H:%M", tm_ptr);
    buf[2] = modified_buf;

    tm_ptr = localtime(&(this_info->statbuf.st_atime));
    strftime(accessed_buf, sizeof(accessed_buf), "%b  %d %H:%M", tm_ptr);
    buf[3] = accessed_buf;

    tm_ptr = localtime(&(this_info->statbuf.st_ctime));
    strftime(changed_buf, sizeof(changed_buf), "%b  %d %H:%M", tm_ptr);
    buf[4] = changed_buf;

    get_perm_string(perm_buf, sizeof(perm_buf), this_info->statbuf.st_mode);
    buf[5] = perm_buf;


    if ((pwd_buf = getpwuid(this_info->statbuf.st_uid)) == NULL)
    {
      g_snprintf(uid_buf, sizeof(uid_buf), "%d",
                 (int)this_info->statbuf.st_uid);
      buf[6] = uid_buf;
    }
    else
    {
      buf[6] = pwd_buf->pw_name;
    }

    if ((grp_buf = getgrgid(this_info->statbuf.st_gid)) == NULL)
    {
      g_snprintf(gid_buf, sizeof(gid_buf), "%d",
                 (int)this_info->statbuf.st_gid);
      buf[7] = gid_buf;
    }
    else
    {
      buf[7] = grp_buf->gr_name;
    }

    row = gtk_clist_append(GTK_CLIST(view->clist), buf);
    gtk_clist_set_row_data_full(GTK_CLIST(view->clist), row,
                                this_info, free_data);
    switch (this_info->statbuf.st_mode & S_IFMT)
    {
      case S_IFLNK:
        gtk_clist_set_foreground(GTK_CLIST(view->clist), row, &LNK_COLOR);
        break;
      case S_IFDIR:
        gtk_clist_set_foreground(GTK_CLIST(view->clist), row, &DIR_COLOR);
        break;
      case S_IFCHR:
      case S_IFBLK:
        gtk_clist_set_foreground(GTK_CLIST(view->clist), row, &DEV_COLOR);
        break;
      case S_IFSOCK:
        gtk_clist_set_foreground(GTK_CLIST(view->clist), row, &SOCK_COLOR);
        break;
      case S_IFREG:
        if (S_ISEXE(this_info->statbuf.st_mode))
          gtk_clist_set_foreground(GTK_CLIST(view->clist), row, &EXE_COLOR);
        break;
      default:
        break;
    }
  }
  closedir(dp);

  gtk_clist_sort(GTK_CLIST(view->clist));
  gtk_clist_thaw(GTK_CLIST(view->clist));
  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(view->dir_entry)->entry), view->dir);

  g_list_free(view->tagged);
  view->tagged = NULL;
}

