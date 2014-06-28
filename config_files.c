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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include "emelfm.h"

#define parse_bool(a, b) \
    if (strstr(line, a)) \
    { \
      value = (strchr(line, '=')+1); \
      b = STREQ(value, "TRUE"); \
      continue; \
    }

#define parse_str(a, b) \
    if (strstr(line, a)) \
    { \
      value = (strchr(line, '=')+1); \
      strncpy(b, value, sizeof(b)); \
      continue; \
    }

#define parse_int(a, b) \
    if (strstr(line, a)) \
    { \
      value = (strchr(line, '=')+1); \
      b = atoi(value); \
      continue; \
    }

#define parse_color(a, b) \
    if (strstr(line, a)) \
    { \
      unsigned int red, green, blue; \
      value = (strchr(line, '=')+1); \
      sscanf(value, "%04X,%04X,%04X", &red, &green, &blue); \
      b.red = red; \
      b.green = green; \
      b.blue = blue; \
    }

static void
write_data(gpointer data, gpointer file)
{
  fprintf((FILE *)file, "%s\n", (gchar *)data);
}

void 
set_config_dir()
{
  gchar *home_dir = getenv("HOME");
  struct stat stat_buf;

  if (home_dir)
  {
    g_snprintf(cfg.config_dir, sizeof(cfg.config_dir), "%s/.emelfm", home_dir);
    if (chdir(cfg.config_dir))
    {
      fprintf(stderr, "Creating config dir\n");
      if (mkdir(cfg.config_dir, 0777))
      {
        fprintf(stderr, "Couldn't make the config dir\n");
        return;
      }
    }
  }
  else
  {
    fprintf(stderr, "HOME environtment variable not set\n");
    return;
  }

  if (stat(cfg.config_dir, &stat_buf) == 0)
    cfg.config_mtime = stat_buf.st_mtime;
}

gint
check_config_files(gpointer data)
{
  struct stat stat_buf;

  if (stat(cfg.config_dir, &stat_buf) == 0)
  {
    if (stat_buf.st_mtime != cfg.config_mtime)
    { /* Config has changed */
      read_config_file();
      read_filetypes_file();
      read_bookmarks_file();
      read_user_commands_file();
      read_toolbar_file();
      read_keys_file();
      read_buttons_file();
      read_plugins_file();
      cfg.config_mtime = stat_buf.st_mtime;
      recreate_main_window();
    }
  }

  return TRUE;
}

/* This function should be called once we are done writing all config files
 * to let any other instances no that they need to re-read the files
 */
void
touch_config_dir()
{
  struct stat stat_buf;

  if (utime(cfg.config_dir, NULL) == 0)
  {
    if (stat(cfg.config_dir, &stat_buf) == 0)
      cfg.config_mtime = stat_buf.st_mtime;
    else
    {
      status_errno();
      printf("Couldn't stat the config dir\n");
    }
  }
  else
  {
    status_errno();
    printf("Couldn't touch the config dir\n");
  }
}

void
write_filetypes_file()
{
  FILE *f;
  gchar filetypes_file[PATH_MAX+10];
  GList *tmp;

  g_snprintf(filetypes_file, sizeof(filetypes_file),
             "%s/filetypes", cfg.config_dir);

  if ( (f = fopen(filetypes_file, "w")) == NULL )
  {
    fprintf(stderr, "Unable to open filetypes file for write\n");
    return;
  }

  for (tmp = cfg.filetypes; tmp != NULL; tmp = tmp->next)
  {
    FileType *ft = tmp->data;

    fprintf(f, "%s;%s;%s\n", ft->description, ft->extensions, ft->actions);
  }
  fclose(f);
}

gboolean
read_filetypes_file()
{
  FILE *f;
  gchar filetypes_file[PATH_MAX+10];
  gchar line[MAX_LEN];
  gchar *extensions;
  gchar *actions;
  gchar *description;
  gchar *s;

  g_snprintf(filetypes_file, sizeof(filetypes_file), "%s/filetypes", cfg.config_dir);
  if ( (f = fopen(filetypes_file, "r")) == NULL )
    return FALSE;

  free_glist_data(&cfg.filetypes);
  while ( fgets(line, MAX_LEN, f) )
  {
    if (line[0] == '\n') continue;

    if ((s = strstr(line, ";")) != NULL)
      *s = '\0';
    else
      continue;

    description = g_strdup(line);
    s++;
    extensions = s;

    if ((s = strstr(extensions, ";")) != NULL)
      *s = '\0';
    else
      continue;
    extensions = g_strdup(extensions);
    s++;
    actions = s;

    chomp(actions);

    add_filetype(extensions, actions, description);

    g_free(description);
    g_free(extensions);
  }
  fclose(f);

  return TRUE;
}

void
write_bookmarks_file()
{
  FILE *f;
  gchar bookmarks_file[PATH_MAX+10];

  g_snprintf(bookmarks_file, sizeof(bookmarks_file),
             "%s/bookmarks", cfg.config_dir);
  if ( (f = fopen(bookmarks_file, "w")) == NULL )
  {
    fprintf(stderr, "Unable to open bookmarks file for writing\n");
    return;
  }

  g_list_foreach(cfg.bookmarks, write_data, f);
  fclose(f);
}

gboolean
read_bookmarks_file()
{
  FILE *f;
  gchar bookmarks_file[PATH_MAX+NAME_MAX];
  gchar line[MAX_LEN];

  g_snprintf(bookmarks_file, sizeof(bookmarks_file),
             "%s/bookmarks", cfg.config_dir);
  if ( (f = fopen(bookmarks_file, "r")) == NULL )
    return FALSE;

  free_glist_data(&cfg.bookmarks);
  while (fgets(line, MAX_LEN, f))
  {
    if (line[0] == '\n') continue;

    chomp(line);
    cfg.bookmarks = g_list_append(cfg.bookmarks, g_strdup(line));
  }
  fclose(f);

  return TRUE;
}
  
void
write_buttons_file()
{
  FILE *f;
  gchar filename[PATH_MAX+NAME_MAX];
  GList *tmp;

  g_snprintf(filename, sizeof(filename), "%s/buttons", cfg.config_dir);
  if ((f = fopen(filename, "w")) == NULL)
  {
    fprintf(stderr, "Unable to open buttons file for writing\n");
    return;
  }

  for (tmp = cfg.buttons; tmp != NULL; tmp = tmp->next)
  {
    Button *button = tmp->data;

    fprintf(f, "%s=%s\n", button->label, button->action);
  }
  fclose(f);
}

gboolean
read_buttons_file()
{
  FILE *f;
  gchar filename[PATH_MAX+NAME_MAX];
  gchar line[MAX_LEN], *s;
  Button *button;

  g_snprintf(filename, sizeof(filename), "%s/buttons", cfg.config_dir);
  if ( (f = fopen(filename, "r")) == NULL )
    return FALSE;

  free_glist_data(&cfg.buttons);
  while ( fgets(line, MAX_LEN, f) )
  {
    if (line[0] == '\n') continue;

    if ((s = strchr(line, '=')) != NULL)
      *s++ = '\0';
    else
      continue;

    button = g_new0(Button, 1);
    strncpy(button->label, line, sizeof(button->label));
    chomp(s);
    strncpy(button->action, s, sizeof(button->action));
    cfg.buttons = g_list_append(cfg.buttons, button);
  }
  fclose(f);

  return TRUE;
}

void
write_user_commands_file()
{
  FILE *f;
  gchar filename[PATH_MAX+NAME_MAX];
  GList *tmp;

  g_snprintf(filename, sizeof(filename), "%s/user_commands", cfg.config_dir);
  if ((f = fopen(filename, "w")) == NULL)
  {
    fprintf(stderr, "Unable to open user_commands file for writing\n");
    return;
  }

  for (tmp = cfg.user_commands; tmp != NULL; tmp = tmp->next)
  {
    UserCommand *command = tmp->data;

    fprintf(f, "%s=%s\n", command->name, command->action);
  }
  fclose(f);
}

gboolean
read_user_commands_file()
{
  FILE *f;
  gchar filename[PATH_MAX+NAME_MAX];
  gchar line[MAX_LEN], *s;
  UserCommand *command;

  g_snprintf(filename, sizeof(filename), "%s/user_commands", cfg.config_dir);
  if ( (f = fopen(filename, "r")) == NULL )
    return FALSE;

  free_glist_data(&cfg.user_commands);
  while ( fgets(line, MAX_LEN, f) )
  {
    if (line[0] == '\n') continue;

    if ((s = strchr(line, '=')) != NULL)
      *s++ = '\0';
    else
      continue;

    command = g_new0(UserCommand, 1);
    strncpy(command->name, line, sizeof(command->name));
    chomp(s);
    strncpy(command->action, s, sizeof(command->action));
    cfg.user_commands = g_list_append(cfg.user_commands, command);
  }
  fclose(f);

  return TRUE;
}

void
write_toolbar_file()
{
  FILE *f;
  gchar filename[PATH_MAX+NAME_MAX];
  GList *tmp;

  g_snprintf(filename, sizeof(filename), "%s/toolbar", cfg.config_dir);
  if ((f = fopen(filename, "w")) == NULL)
  {
    fprintf(stderr, "Unable to open toolbar file for writing\n");
    return;
  }

  for (tmp = cfg.toolbar_buttons; tmp != NULL; tmp = tmp->next)
  {
    ToolbarButton *tb = tmp->data;

    fprintf(f, "%s\n%s\n%s\n%s\n", tb->label, tb->tooltip, tb->action,
            (tb->capture_output ? "TRUE" : "FALSE"));
  }
  fclose(f);
}

gboolean
read_toolbar_file()
{
  FILE *f;
  gchar filename[PATH_MAX+NAME_MAX];
  gchar line[MAX_LEN];
  ToolbarButton tb;

  g_snprintf(filename, sizeof(filename), "%s/toolbar", cfg.config_dir);
  if ( (f = fopen(filename, "r")) == NULL )
    return FALSE;

  free_glist_data(&cfg.toolbar_buttons);
  while ( fgets(tb.label, sizeof(tb.label), f) )
  {
    if (!(fgets(tb.tooltip, sizeof(tb.tooltip), f) 
          && fgets(tb.action, sizeof(tb.action), f)
          && fgets(line, sizeof(line), f)))
    {
      fprintf(stderr, "Bad toolbar file\n");
      break;
    }

    chomp(tb.label);
    chomp(tb.tooltip);
    chomp(tb.action);
    chomp(line);
    tb.capture_output = STREQ(line, "TRUE") ? TRUE : FALSE;
    cfg.toolbar_buttons = g_list_append(cfg.toolbar_buttons,
                                        g_memdup(&tb, sizeof(ToolbarButton)));
  }
  fclose(f);

  return TRUE;
}

void
write_keys_file()
{
  FILE *f;
  gchar filename[PATH_MAX+NAME_MAX];
  GList *tmp;

  g_snprintf(filename, sizeof(filename), "%s/keys", cfg.config_dir);
  if ((f = fopen(filename, "w")) == NULL)
  {
    fprintf(stderr, "Unable to open toolbar file for writing\n");
    return;
  }

  for (tmp = cfg.key_bindings; tmp != NULL; tmp = tmp->next)
  {
    KeyBinding *kb = tmp->data;

    fprintf(f, "%d %s %s\n", kb->state, kb->key_name, kb->action);
  }
  fclose(f);
}

gboolean
read_keys_file()
{
  FILE *f;
  gchar filename[PATH_MAX+NAME_MAX];
  gchar line[MAX_LEN];
  KeyBinding *kb;
  gchar *s, *key_name;

  g_snprintf(filename, sizeof(filename), "%s/keys", cfg.config_dir);
  if ( (f = fopen(filename, "r")) == NULL )
    return FALSE;

  free_glist_data(&cfg.key_bindings);
  while (fgets(line, MAX_LEN, f))
  {
    kb = g_new0(KeyBinding, 1);

    if ((s = strchr(line, ' ')) != NULL)
      *s++ = '\0';
    else
    {
      g_free(kb);
      continue;
    }

    kb->state = atoi(line);

    key_name = s;
    if ((s = strchr(key_name, ' ')) != NULL)
      *s++ = '\0';
    else
    {
      g_free(kb);
      continue;
    }

    strncpy(kb->key_name, key_name, sizeof(kb->key_name));
    kb->keyval = gdk_keyval_from_name(key_name);
    
    chomp(s);
    strncpy(kb->action, s, sizeof(kb->action));

    cfg.key_bindings = g_list_append(cfg.key_bindings, kb);
  }
  fclose(f);

  return TRUE;
}

void
write_plugins_file()
{
  FILE *f;
  gchar filename[PATH_MAX+NAME_MAX];
  GList *tmp;

  g_snprintf(filename, sizeof(filename), "%s/plugins", cfg.config_dir);
  if ((f = fopen(filename, "w")) == NULL)
  {
    fprintf(stderr, "Unable to open plugins file for writing\n");
    return;
  }

  for (tmp = cfg.plugins; tmp != NULL; tmp = tmp->next)
  {
    Plugin *p = tmp->data;

    fprintf(f, "%s:%s\n", p->filename, (p->show_in_menu ? "TRUE" : "FALSE"));
  }
  fclose(f);
}

gint
read_plugins_file()
{
  FILE *f;
  gchar filename[PATH_MAX+NAME_MAX];
  gchar plugin_path[PATH_MAX+NAME_MAX], *s;
  Plugin *p;

  g_snprintf(filename, sizeof(filename), "%s/plugins", cfg.config_dir);
  if ( (f = fopen(filename, "r")) == NULL )
    return FALSE;

  unload_all_plugins();
  while ( fgets(plugin_path, sizeof(plugin_path), f) )
  {
    chomp(plugin_path);
    if ((s = strrchr(plugin_path, ':')) != NULL)
    {
      *s++ = '\0';
      if ((p = load_plugin(plugin_path)) != NULL)
      {
        p->show_in_menu = STREQ(s, "TRUE");
        cfg.plugins = g_list_append(cfg.plugins, p);
      }
    }
    else
    {
      if ((p = load_plugin(plugin_path)) != NULL)
        cfg.plugins = g_list_append(cfg.plugins, p);
    }
  }
  fclose(f);

  return TRUE;
}

void
write_config_file()
{
  FILE *f;
  gchar config_file[PATH_MAX+10];
  int i;

  g_snprintf(config_file, sizeof(config_file), "%s/settings", cfg.config_dir);
  if ((f = fopen(config_file, "w")) == NULL)
  {
    fprintf(stderr, "Unable to open config file for writing\n");
    return;
  }

  fprintf(f, "VERSION=%s\n", VERSION);
  fprintf(f, "SHOW_HIDDEN_LEFT=%s\n", (app.left_view.show_hidden ? "TRUE" : "FALSE"));
  fprintf(f, "SHOW_HIDDEN_RIGHT=%s\n", (app.right_view.show_hidden ? "TRUE" : "FALSE"));
  fprintf(f, "CONFIRM_DELETE=%s\n", (cfg.confirm_delete ? "TRUE" : "FALSE"));
  fprintf(f, "CONFIRM_OVERWRITE=%s\n", (cfg.confirm_overwrite
                                        ? "TRUE" : "FALSE"));
  fprintf(f, "USE_INTERNAL_VIEWER=%s\n", (cfg.use_internal_viewer
                                          ? "TRUE" : "FALSE"));
  fprintf(f, "AUTO_REFRESH_ENABLED=%s\n", (cfg.auto_refresh_enabled
                                           ? "TRUE" : "FALSE"));
  fprintf(f, "VIEWER_COMMAND=%s\n", cfg.viewer_command);
  fprintf(f, "XTERM_COMMAND=%s\n", cfg.xterm_command);
  fprintf(f, "LEFT_DIR=%s\n", (cfg.start_with_last_dir_left ? app.left_view.dir : cfg.left_startup_dir));
  fprintf(f, "RIGHT_DIR=%s\n", (cfg.start_with_last_dir_right ? app.right_view.dir : cfg.right_startup_dir));
  fprintf(f, "START_WITH_LAST_DIR_LEFT=%s\n", (cfg.start_with_last_dir_left
                                               ? "TRUE" : "FALSE"));
  fprintf(f, "START_WITH_LAST_DIR_RIGHT=%s\n", (cfg.start_with_last_dir_right
                                               ? "TRUE" : "FALSE"));
  fprintf(f, "DIR_HISTORY=%d\n", cfg.dir_history_max_length);
  fprintf(f, "COMMAND_HISTORY=%d\n", cfg.command_history_max_length);
  fprintf(f, "EXPANDED_POPUP_MENU=%s\n", (cfg.expand_popup_menu
                                          ? "TRUE" : "FALSE"));
  fprintf(f, "USE_VI_KEYS=%s\n", (cfg.use_vi_keys ? "TRUE" : "FALSE"));
  fprintf(f, "WINDOWS_RIGHT_CLICK=%s\n", (cfg.windows_right_click ? "TRUE" : "FALSE"));
  fprintf(f, "WINDOW_HEIGHT=%d\n", app.main_window->allocation.height);
  fprintf(f, "WINDOW_WIDTH=%d\n", app.main_window->allocation.width);
  fprintf(f, "OUTPUT_TEXT_HIDDEN=%s\n", (cfg.output_text_hidden
                                          ? "TRUE" : "FALSE"));
  fprintf(f, "VPANE_POSITION=%d\n", (cfg.output_text_hidden
                           ? cfg.vpane_position
                           : GTK_PANED(app.vpane)->child1->allocation.height));
  if (cfg.view_type == BOTH_PANES)
    fprintf(f, "VIEW_TYPE=BOTH_PANES\n");
  else
    fprintf(f, "VIEW_TYPE=%s\n", (cfg.view_type == LEFT_PANE
                                  ?"LEFT_PANE" : "RIGHT_PANE"));
  fprintf(f, "LIST_FONT=%s\n", cfg.list_font);
  fprintf(f, "OUTPUT_FONT=%s\n", cfg.output_font);
  fprintf(f, "SCROLLBAR_POS=%d\n", cfg.scrollbar_pos);

  for (i = 0; i < MAX_COLUMNS; i++)
  {
    if (all_columns[i].is_visible)
    {
      gint r, l;
      r = GTK_CLIST(app.right_view.clist)->column[i].area.width;
      l = GTK_CLIST(app.left_view.clist)->column[i].area.width;
      fprintf(f, "COLUMN=%d,%d\n", i, (r > l ? r : l));
    }
  }

  fprintf(f, "COL_COLOR=%04X,%04X,%04X\n", COL_COLOR.red, COL_COLOR.green, COL_COLOR.blue);
  fprintf(f, "DIR_COLOR=%04X,%04X,%04X\n", DIR_COLOR.red, DIR_COLOR.green, DIR_COLOR.blue);
  fprintf(f, "DEV_COLOR=%04X,%04X,%04X\n", DEV_COLOR.red, DEV_COLOR.green, DEV_COLOR.blue);
  fprintf(f, "EXE_COLOR=%04X,%04X,%04X\n", EXE_COLOR.red, EXE_COLOR.green, EXE_COLOR.blue);
  fprintf(f, "LNK_COLOR=%04X,%04X,%04X\n", LNK_COLOR.red, LNK_COLOR.green, LNK_COLOR.blue);
  fprintf(f, "SOCK_COLOR=%04X,%04X,%04X\n", SOCK_COLOR.red, SOCK_COLOR.green, SOCK_COLOR.blue);
  fprintf(f, "TAG_COLOR=%04X,%04X,%04X\n", TAG_COLOR.red, TAG_COLOR.green, TAG_COLOR.blue);
  fprintf(f, "SELECT_COLOR=%04X,%04X,%04X\n", SELECT_COLOR.red, SELECT_COLOR.green, SELECT_COLOR.blue);
  fprintf(f, "DRAG_HILIGHT=%04X,%04X,%04X\n", DRAG_HILIGHT.red, DRAG_HILIGHT.green, DRAG_HILIGHT.blue);

  fclose(f);
}

gboolean
read_config_file()
{
  FILE *f;
  gchar config_file[PATH_MAX+10];
  gchar line[MAX_LEN];
  gchar *value;
  int clear_default_columns = TRUE;

  g_snprintf(config_file, sizeof(config_file), "%s/settings", cfg.config_dir);
  if ((f = fopen(config_file, "r")) == NULL)
    return FALSE;

  while (fgets(line, MAX_LEN, f))
  {
    chomp(line);

    parse_str("VERSION", cfg.version);
    parse_bool("SHOW_HIDDEN_LEFT", app.left_view.show_hidden);
    parse_bool("SHOW_HIDDEN_RIGHT", app.right_view.show_hidden);
    parse_bool("CONFIRM_DELETE", cfg.confirm_delete);
    parse_bool("CONFIRM_OVERWRITE", cfg.confirm_overwrite);
    parse_str("LEFT_DIR", cfg.left_startup_dir);
    parse_str("RIGHT_DIR", cfg.right_startup_dir);
    parse_bool("START_WITH_LAST_DIR_LEFT", cfg.start_with_last_dir_left);
    parse_bool("START_WITH_LAST_DIR_RIGHT", cfg.start_with_last_dir_right);
    parse_bool("AUTO_REFRESH_ENABLED", cfg.auto_refresh_enabled);
    parse_int("DIR_HISTORY", cfg.dir_history_max_length);
    parse_int("COMMAND_HISTORY", cfg.command_history_max_length);
    parse_bool("EXPANDED_POPUP_MENU", cfg.expand_popup_menu);
    parse_bool("USE_VI_KEYS", cfg.use_vi_keys);
    parse_bool("WINDOWS_RIGHT_CLICK", cfg.windows_right_click);
    parse_int("WINDOW_HEIGHT", cfg.window_height);
    parse_int("WINDOW_WIDTH", cfg.window_width);
    parse_bool("USE_INTERNAL_VIEWER", cfg.use_internal_viewer);
    parse_str("VIEWER_COMMAND", cfg.viewer_command);
    parse_str("XTERM_COMMAND", cfg.xterm_command);
    parse_bool("OUTPUT_TEXT_HIDDEN", cfg.output_text_hidden);
    parse_int("VPANE_POSITION", cfg.vpane_position);
    parse_str("LIST_FONT", cfg.list_font);
    parse_str("OUTPUT_FONT", cfg.output_font);
    parse_int("SCROLLBAR_POS", cfg.scrollbar_pos);
    parse_color("COL_COLOR", COL_COLOR);
    parse_color("DIR_COLOR", DIR_COLOR);
    parse_color("DEV_COLOR", DEV_COLOR);
    parse_color("EXE_COLOR", EXE_COLOR);
    parse_color("LNK_COLOR", LNK_COLOR);
    parse_color("SOCK_COLOR", SOCK_COLOR);
    parse_color("TAG_COLOR", TAG_COLOR);
    parse_color("SELECT_COLOR", SELECT_COLOR);
    parse_color("DRAG_HILIGHT", DRAG_HILIGHT);

    if (strstr(line, "VIEW_TYPE"))
    {
      value = (strchr(line, '=')+1);
      if (STREQ(value, "LEFT_PANE"))
        cfg.view_type = LEFT_PANE;
      else if (STREQ(value, "RIGHT_PANE"))
        cfg.view_type = RIGHT_PANE;
      else
        cfg.view_type = BOTH_PANES;
      continue;
    }

    if (strstr(line, "COLUMN"))
    {
      int i;
      gchar *s;
      if (clear_default_columns)
      {
        for (i = 0; i < MAX_COLUMNS; i++)
          all_columns[i].is_visible = FALSE;
        clear_default_columns = FALSE;
      }
      value = (strchr(line, '=')+1);
      if ((s = strchr(value, ',')) != NULL)
      {
        /* Now we store the size also */
        *s++ = '\0';
        i = atoi(value);
        all_columns[i].is_visible = TRUE;
        all_columns[i].size = atoi(s);
      }
      else
      {
        /* No size here.. just the col */
        i = atoi(value);
        all_columns[i].is_visible = TRUE;
      }
      continue;
    }
  }
  fclose(f);

  return TRUE;
}

