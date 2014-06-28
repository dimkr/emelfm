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

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "emelfm.h"

App app;
Config cfg;
FileView *curr_view;
FileView *other_view;

Column all_columns[MAX_COLUMNS] = {
  {"Filename", 180, FILENAME, TRUE, name_sort},
  {"Size", 80, SIZE, TRUE, size_sort},
  {"Modified", 80, MODIFIED, TRUE, date_sort},
  {"Accessed", 80, ACCESSED, FALSE, date_sort},
  {"Changed", 80, CHANGED, FALSE, date_sort},
  {"Permissions", 80, PERM, TRUE, perm_sort},
  {"Owner", 60, OWNER, TRUE, user_sort},
  {"Group", 60, GROUP, TRUE, group_sort}
};

SystemOp system_ops[] = {
  {"SYSTEM_CHOWN", ownership_cb},
  {"SYSTEM_CONFIGURE", configure_cb},
  {"SYSTEM_COPY", copy_cb},
  {"SYSTEM_COPY_AS", copy_as_cb},
  {"SYSTEM_DELETE", delete_cb},
  {"SYSTEM_EXECUTE", execute_cb},
  {"SYSTEM_EXECUTE_WITH_ARGS", execute_with_args_cb},
  {"SYSTEM_EXECUTE_IN_XTERM", execute_in_xterm_cb},
  {"SYSTEM_EXECUTE_IN_XTERM_WITH_ARGS", execute_in_xterm_with_args_cb},
  {"SYSTEM_INFO", file_info_cb},
  {"SYSTEM_FIND", find_cb},
  {"SYSTEM_MKDIR", mkdir_cb},
  {"SYSTEM_MOUNT", mount_cb},
  {"SYSTEM_MOVE", move_cb},
  {"SYSTEM_MOVE_AS", move_as_cb},
  {"SYSTEM_OPEN", open_cb},
  {"SYSTEM_OPEN_DIR_IN_OTHER_PANE", open_in_other_pane_cb},
  {"SYSTEM_OPEN_WITH", open_with_cb},
  {"SYSTEM_PERMISSIONS", permissions_cb},
  {"SYSTEM_QUIT", quit_cb},
  {"SYSTEM_REFRESH", refresh_cb},
  {"SYSTEM_RENAME", rename_cb},
  {"SYSTEM_SYMLINK", symlink_cb},
  {"SYSTEM_SYMLINK_AS", symlink_as_cb},
  {"SYSTEM_SYNC_DIRS", sync_dirs_cb},
  {"SYSTEM_TAG", toggle_tag_cb},
  {"SYSTEM_UMOUNT", umount_cb},
  {"SYSTEM_VIEW", view_cb}
};

SystemOp interface_ops[] = {
  {"INTERFACE:UpDir", updir_cb},
  {"INTERFACE:Go Home", cd_home},
  {"INTERFACE:Switch Panels", switch_views_cb},
  {"INTERFACE:Toggle Left Panel size", toggle_left_panel_cb},
  {"INTERFACE:Toggle Right Panel size", toggle_right_panel_cb},
  {"INTERFACE:Toggle Hidden Files", toggle_hidden_cb},
  {"INTERFACE:Open/Close Output Window", toggle_output_window_cb},
  {"INTERFACE:Goto Command Line", goto_command_line_cb},
  {"INTERFACE:Menu", show_menu_cb},
  {"INTERFACE:User Menu", show_user_menu_cb},
};

gint n_sys_ops = sizeof(system_ops) / sizeof(system_ops[0]);
gint n_interface_ops = sizeof(interface_ops) / sizeof(interface_ops[0]);

GdkColor EXE_COLOR;
GdkColor DIR_COLOR;
GdkColor LNK_COLOR;
GdkColor DEV_COLOR;
GdkColor COL_COLOR;
GdkColor TAG_COLOR;
GdkColor SOCK_COLOR;
GdkColor DRAG_HILIGHT;
GdkColor CLIST_COLOR;
GdkColor SELECT_COLOR;

static gint
update_status_bar(gpointer data)
{
  static gint last_selected_rows;
  static gint last_total_rows;
  gchar status_text[128];
  gint selected_rows, total_rows;

  if (curr_view->tagged != NULL)
    selected_rows = g_list_length(curr_view->tagged);
  else
    selected_rows = g_list_length(GTK_CLIST(curr_view->clist)->selection);
  total_rows = GTK_CLIST(curr_view->clist)->rows;

  if ((selected_rows != last_selected_rows) || (total_rows != last_total_rows))
  {
    last_selected_rows = selected_rows;
    last_total_rows = total_rows;
    g_snprintf(status_text, sizeof(status_text), _("%d of %d files selected"),
               selected_rows, total_rows);
    gtk_label_set_text(GTK_LABEL(app.status_bar), status_text);
  }

  return TRUE;
}

static void
add_user_command(gchar *name, gchar *action)
{
  UserCommand *command = g_new0(UserCommand, 1);

  strncpy(command->name, name, sizeof(command->name));
  strncpy(command->action, action, sizeof(command->action));
  cfg.user_commands = g_list_append(cfg.user_commands, command);
}

static void
add_user_button(gchar *label, gchar *action)
{
  Button *button = g_new0(Button, 1);

  strncpy(button->label, label, sizeof(button->label));
  strncpy(button->action, action, sizeof(button->action));
  cfg.buttons = g_list_append(cfg.buttons, button);
}

static void
add_toolbar_button(gchar *label, gchar *action, gchar *tooltip,
                   gboolean capture_output)
{
  ToolbarButton *tb = g_new0(ToolbarButton, 1);

  strncpy(tb->label, label, sizeof(tb->label));
  strncpy(tb->action, action, sizeof(tb->action));
  strncpy(tb->tooltip, tooltip, sizeof(tb->tooltip));
  tb->capture_output = capture_output;
  cfg.toolbar_buttons = g_list_append(cfg.toolbar_buttons, tb);
}

static void
add_key_binding(gint state, gchar *key_name, gchar *action)
{
  KeyBinding *kb = g_new0(KeyBinding, 1);

  kb->state = state;
  strncpy(kb->key_name, key_name, sizeof(kb->key_name));
  kb->keyval = gdk_keyval_from_name(key_name);
  strncpy(kb->action, action, sizeof(kb->action));
  cfg.key_bindings = g_list_append(cfg.key_bindings, kb);
}

static void
do_upgrade()
{
  gtk_main();
}

int 
main(int argc, char *argv[])
{
  gboolean first_run = TRUE;

#ifdef ENABLE_NLS
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
#endif

  all_columns[0].title = _("Filename");
  all_columns[1].title = _("Size");
  all_columns[2].title = _("Modified");
  all_columns[3].title = _("Accessed");
  all_columns[4].title = _("Changed");
  all_columns[5].title = _("Permissions");
  all_columns[6].title = _("Owner");
  all_columns[7].title = _("Group");

  /* Set up handler for sigchild so that we don't get zombies */
  set_sigchild_handler();

  /* Gtk initialization stuff */
  g_thread_init(NULL);
  gtk_set_locale();
  gtk_init(&argc, &argv);

  /* Prepare and read config files */
  set_config_dir();
  if (!read_config_file())
  {
    /* Set Default values */
    app.left_view.show_hidden = FALSE;
    app.right_view.show_hidden = FALSE;
    cfg.confirm_delete = TRUE;
    cfg.confirm_overwrite = TRUE;
    cfg.start_with_last_dir_left = FALSE;
    cfg.start_with_last_dir_right = FALSE;
    cfg.use_internal_viewer = TRUE;
    cfg.auto_refresh_enabled = TRUE;
    cfg.window_width = 640;
    cfg.window_height = 480;
    cfg.vpane_position = 325;
    cfg.expand_popup_menu = FALSE;
    cfg.use_vi_keys = FALSE;
    cfg.view_type = BOTH_PANES;
    cfg.dir_history_max_length = 15;
    cfg.command_history_max_length = 10;
    cfg.scrollbar_pos = GTK_CORNER_TOP_LEFT;
    strncpy(cfg.list_font,
            "-*-helvetica-medium-r-normal-*-*-120-*-*-p-*-*",
            sizeof(cfg.list_font));
    strncpy(cfg.output_font,
            "-*-courier-medium-r-normal-*-*-120-*-*-*-*-*",
            sizeof(cfg.output_font));
    strncpy(cfg.xterm_command, "xterm", sizeof(cfg.xterm_command));
    chdir(getenv("HOME"));
    getcwd(app.left_view.dir, PATH_MAX);
    strncpy(app.right_view.dir, app.left_view.dir, PATH_MAX);
    strncpy(cfg.left_startup_dir, app.left_view.dir, PATH_MAX);
    strncpy(cfg.right_startup_dir, app.left_view.dir, PATH_MAX);

    /* Setup the default colors */
    gdk_color_parse("forest green", &EXE_COLOR);
    gdk_color_parse("blue", &DIR_COLOR);
    gdk_color_parse("sky blue", &LNK_COLOR);
    gdk_color_parse("orange", &DEV_COLOR);
    gdk_color_parse("purple", &SOCK_COLOR);
    gdk_color_parse("dark gray", &COL_COLOR);
    gdk_color_parse("yellow", &TAG_COLOR);
    gdk_color_parse("light gray", &SELECT_COLOR);
    gdk_color_parse("wheat", &DRAG_HILIGHT);
  }
  else
    first_run = FALSE;

  if (!read_buttons_file())
  {
    /* Default Button Setup */
    add_user_button(_("Copy"), "SYSTEM_COPY");
    add_user_button(_("Move"), "SYSTEM_MOVE");
    add_user_button(_("Rename"), "SYSTEM_RENAME");
    add_user_button(_("SymLink"), "SYSTEM_SYMLINK");
    add_user_button(_("Delete"), "SYSTEM_DELETE");
    add_user_button(_("MkDir"), "SYSTEM_MKDIR");
    add_user_button(_("File Info"), "SYSTEM_INFO");
    add_user_button(_("Refresh"), "SYSTEM_REFRESH");
    add_user_button(_("Configure"), "SYSTEM_CONFIGURE");
    add_user_button(_("Quit"), "SYSTEM_QUIT");
  }

  if (!read_filetypes_file())
  {
    /* Setup some default filetypes */
    add_filetype("jpeg,jpg,png,xpm,gif", "xv,gimp", _("Image Files"));
    add_filetype("c,cpp,h,pl,java", "Edit With Vi@x vi,Edit With Emacs@x emacs", _("Source Code Files"));
    add_filetype("o,so,a", "View Symbols@x nm %f | less", _("Object Files"));
    add_filetype("htm,html", "netscape,x lynx", _("HTML Documents"));
    add_filetype("tar", "PLUGIN:Unpack,Unpack@x tar xvf %f,Unpack in other panel@x cd %D; tar xvf %d/%f,View Contents@x tar tvf %f | less", _("Plain Tarballs"));
    add_filetype("tar.gz,tgz", "PLUGIN:Unpack,Unpack@x gunzip -c %f | tar xvf -,Unpack in other panel@x cd %D; gunzip -c %d/%f | tar xvf -,View Contents@x gunzip -c %f | tar tvf - | less", _("Gzip Tarballs"));
    add_filetype("tar.bz2","PLUGIN:Unpack,Unpack@x bzip2 -d -c %f | tar xvf -,Unpack in other panel@x cd %D; bzip2 -d -c %d/%f | tar xvf -,View Contents@x bzip2 -d -c %f | tar tvf - | less", _("Bzip2 Tarballs"));
    add_filetype("zip", "PLUGIN:Unpack,Unzip@x unzip %f,Unzip in other panel@x cd %D; unzip %d/%f,View Contents@x unzip -l %f | less", _("ZIP Archives"));
    add_filetype("rpm", "View Info@x rpm -qlip %f | less,Install@su rpm -Uvh %f | less", _("RPM Packages"));
    add_filetype("pdf", "acroread,xpdf", _("Adobe Acrobat Documents"));
    add_filetype("ps", "gv", _("PostScript Documents"));
    add_filetype("txt", "PLUGIN:View", _("Text Documents"));
  }

  if (!read_bookmarks_file())
  {
    gchar *home = getenv("HOME");
    /* Setup some default bookmarks */
    if (home != NULL)
      cfg.bookmarks = g_list_append(cfg.bookmarks, g_strdup(home));
    cfg.bookmarks = g_list_append(cfg.bookmarks, g_strdup("/usr/local"));
    cfg.bookmarks = g_list_append(cfg.bookmarks, g_strdup("/usr"));
    cfg.bookmarks = g_list_append(cfg.bookmarks, g_strdup("/mnt"));
    cfg.bookmarks = g_list_append(cfg.bookmarks, g_strdup("/"));
  }
    
  if (!read_user_commands_file())
  {
    /* Setup some default user commands */
    add_user_command(_("Find SUID and SGID files"), "find . -perm +4000 -or -perm +2000");
    add_user_command(_("Find which RPM this came from"), "rpm -qf %f");
    add_user_command(_("Print"), "lpr %f");
    add_user_command(_("Make Patch"), "diff -c %d %D > %{Filename for patch:}");
  }

  if (!read_toolbar_file())
  {
    /* Setup the default toolbar */
    add_toolbar_button("df", "df", _("Available disk space"), FALSE);
    add_toolbar_button("free", "free", _("Memory information"), FALSE);
    add_toolbar_button("X", "xterm &", _("Open an Xterm"), FALSE);
  }

  if (!read_keys_file())
  {
    /* Default Key Bindings */
    add_key_binding(0, "F1", "SYSTEM_REFRESH");
    add_key_binding(0, "F2", "SYSTEM_INFO");
    add_key_binding(0, "F3", "SYSTEM_FIND");
    add_key_binding(0, "F4", "SYSTEM_VIEW");
    add_key_binding(0, "F5", "SYSTEM_COPY");
    add_key_binding(0, "F6", "SYSTEM_MOVE");
    add_key_binding(0, "F7", "SYSTEM_MKDIR");
    add_key_binding(0, "F8", "SYSTEM_DELETE");
    add_key_binding(0, "Return", "SYSTEM_OPEN");
    add_key_binding(0, "Right", "SYSTEM_OPEN");
    add_key_binding(0, "BackSpace", "INTERFACE:UpDir");
    add_key_binding(0, "Left", "INTERFACE:UpDir");
    add_key_binding(0, "space", "INTERFACE:Switch Panels");
    add_key_binding(0, "Tab", "INTERFACE:Switch Panels");
    add_key_binding(0, "Delete", "SYSTEM_DELETE");
    add_key_binding(0, "Insert", "SYSTEM_TAG");
    add_key_binding(0, "Home", "INTERFACE:Go Home");
    add_key_binding(GDK_CONTROL_MASK, "c", "SYSTEM_COPY");
    add_key_binding(GDK_CONTROL_MASK | GDK_SHIFT_MASK, "c", "SYSTEM_COPY_AS");
    add_key_binding(GDK_CONTROL_MASK, "m", "SYSTEM_MOVE");
    add_key_binding(GDK_CONTROL_MASK | GDK_SHIFT_MASK, "m", "SYSTEM_MOVE_AS");
    add_key_binding(GDK_CONTROL_MASK, "s", "SYSTEM_SYMLINK");
    add_key_binding(GDK_CONTROL_MASK | GDK_SHIFT_MASK, "s", "SYSTEM_SYMLINK_AS");
    add_key_binding(GDK_CONTROL_MASK, "r", "SYSTEM_RENAME");
    add_key_binding(GDK_CONTROL_MASK, "d", "SYSTEM_MKDIR");
    add_key_binding(GDK_CONTROL_MASK, "l", "SYSTEM_REFRESH");
    add_key_binding(GDK_CONTROL_MASK, "x", "SYSTEM_SYNC_DIRS");
    add_key_binding(GDK_CONTROL_MASK, "f", "SYSTEM_FIND");
    add_key_binding(GDK_CONTROL_MASK, "t", "SYSTEM_TAG");
    add_key_binding(GDK_CONTROL_MASK, "q", "SYSTEM_QUIT");
    add_key_binding(GDK_CONTROL_MASK, "h", "INTERFACE:Toggle Hidden Files");
    add_key_binding(GDK_CONTROL_MASK, "w", "INTERFACE:Open/Close Output Window");
    add_key_binding(GDK_CONTROL_MASK, "z", "INTERFACE:Goto Command Line");
    add_key_binding(GDK_CONTROL_MASK, "comma", "INTERFACE:Toggle Left Panel size");
    add_key_binding(GDK_CONTROL_MASK, "period", "INTERFACE:Toggle Right Panel size");
    add_key_binding(GDK_CONTROL_MASK, "p", "INTERFACE:Menu");
    add_key_binding(GDK_CONTROL_MASK, "bracketleft", "INTERFACE:Plugins Menu");
    add_key_binding(GDK_CONTROL_MASK, "bracketright", "INTERFACE:User Menu");
  }

  if (cfg.right_startup_dir != NULL)
    strncpy(app.right_view.dir, cfg.right_startup_dir,
            sizeof(app.right_view.dir));
  if (cfg.left_startup_dir != NULL)
    strncpy(app.left_view.dir, cfg.left_startup_dir, sizeof(app.left_view.dir));

  if (argv[1] != NULL)
  {
    strncpy(app.left_view.dir, argv[1], sizeof(app.left_view.dir));
    if (argv[2] != NULL)
      strncpy(app.right_view.dir, argv[2], sizeof(app.right_view.dir));
  }

  /* Build GUI */
  create_main_window();
  initialize_filters(&app.right_view);
  initialize_filters(&app.left_view);
  change_dir(&app.right_view, app.right_view.dir);
  change_dir(&app.left_view, app.left_view.dir);
  sort_list(&app.right_view, name_sort, GTK_SORT_ASCENDING, 0);
  sort_list(&app.left_view, name_sort, GTK_SORT_ASCENDING, 0);

  /* can't use status_message() cuz that would open the output window */
  gtk_text_insert(GTK_TEXT(app.output_text), app.output_font, NULL, NULL,
    _("Type 'keys' to print a list of the current keyboard shortcuts\n"), -1);

  gtk_timeout_add(1000, update_status_bar, NULL);
  cfg.check_config_id = gtk_timeout_add(1000, check_config_files, NULL);
  if (cfg.auto_refresh_enabled)
    cfg.auto_refresh_id = gtk_timeout_add(1000, auto_refresh, NULL);

  if (!first_run && !STREQ(cfg.version, VERSION))
    do_upgrade();

  gtk_main();

  exit(0);
}

