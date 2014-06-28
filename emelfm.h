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

/* locale patch */
#include "emelfm_locale.h"

#ifndef __EMELFM_H__
#define __EMELFM_H__

#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gmodule.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

#define VERSION "0.9.2"
#define AUTHOR "Michael Clark <macst92@imap.pitt.edu>"
#define NAME_MAX 255
#define FONTNAME_MAX 128
#define MAX_LEN 1024
#define MAX_COLUMNS 8

#define STREQ(a,b) (strcmp((a),(b)) == 0)

typedef enum { FILENAME = 0, SIZE, MODIFIED, ACCESSED, CHANGED,
               PERM, OWNER, GROUP } ColumnFlags;
typedef enum { GENERAL_1 = 0, GENERAL_2, COLUMNS, BOOKMARKS, FILETYPES,
               USER_COMMANDS, TOOLBAR_BUTTONS, FN_KEYS, BUTTONS,
               PLUGINS, INTERFACE_1, INTERFACE_2 } ConfigDialogPages;
typedef enum { GT, LT, EQ } Operator;

typedef enum
{
  YES = 1 << 0,
  OK = 1 << 1,
  YES_TO_ALL = 1 << 2,
  APPLY_TO_ALL = 1 << 3,
  NO = 1 << 4,
  CANCEL = 1 << 5
} DialogButtons;

typedef enum { BOTH_PANES, LEFT_PANE, RIGHT_PANE } ViewType;

typedef struct _FileInfo
{
  gchar filename[NAME_MAX];
  struct stat statbuf;
} FileInfo;

typedef struct _FileType
{
  gchar description[NAME_MAX];
  gchar extensions[NAME_MAX];
  gchar actions[NAME_MAX];
} FileType;

typedef struct _Button
{
  gchar label[NAME_MAX];
  gchar action[NAME_MAX];
} Button;

typedef struct _ToolbarButton
{
  gchar label[NAME_MAX];
  gchar action[NAME_MAX];
  gchar tooltip[NAME_MAX];
  gboolean capture_output; /* This is no longer used, here for compatability */
} ToolbarButton;

typedef struct _Column
{
  gchar *title;
  gint size;
  gint id;
  gint is_visible;
  GtkCListCompareFunc sort_func;
} Column;

typedef struct _UserCommand
{
  gchar name[NAME_MAX];
  gchar action[NAME_MAX];
} UserCommand;

typedef struct _KeyBinding
{
  gint state;
  gint keyval;
  gchar key_name[NAME_MAX];
  gchar action[NAME_MAX];
} KeyBinding;

typedef struct _SystemOp
{
  gchar *name;
  GtkSignalFunc func;
} SystemOp;

typedef struct _Plugin
{
  gchar filename[PATH_MAX+NAME_MAX];
  gchar *name;
  gchar *description;
  gboolean show_in_menu;
  gboolean load;
  GModule *module;
  void (*plugin_cb)();
} Plugin;

typedef struct _FileTypeMenu
{
  GtkWidget *actions_menu;
  GtkWidget *edit_ft_menu;
} FileTypeMenu;

typedef struct _FileView
{
  GtkWidget *clist;
  GtkWidget *sort_arrows[MAX_COLUMNS];
  GtkWidget *dir_entry;
  GtkWidget *bookmark_menu;
  GtkWidget *bookmark_menu_item;
  GtkWidget *filter_menu_item;
  GtkWidget *hidden_toggle;
  GList *old_selection;
  GList *tagged;
  GList *dir_history;
  gchar last_find[NAME_MAX];
  gchar dir[PATH_MAX];
  time_t dir_mtime;

  gboolean filter_directories;
  struct
  {
    gchar pattern[NAME_MAX];
    gboolean invert_mask;
    gboolean case_sensitive;
    gboolean active;
  } filename_filter;
  struct
  {
    off_t size;
    Operator op;
    gboolean active;
  } size_filter;
  struct
  {
    time_t time;
    Operator op;
    enum {MTIME, ATIME, CTIME} time_type;
    gboolean active;
  } date_filter;

  gint row;
  gint last_row;
  gint selected_rows;
  gboolean show_hidden;
} FileView;

typedef struct _App
{
  FileView  left_view;
  FileView  right_view;
  GtkWidget *main_window;
  GtkWidget *left_view_box;
  GtkWidget *right_view_box;
  GtkWidget *button_col_sw;
  GtkWidget *hbox;
  GtkWidget *vpane;
  GtkWidget *status_bar;
  GtkWidget *toolbar;
  GtkWidget *toolbar_button_box;
  GtkWidget *main_menu;
  GtkWidget *dir_menu;
  GtkWidget *exec_menu;
  GtkWidget *plugins_menu;
  GtkWidget *user_command_menu;
  GtkWidget *drag_op_menu;
  FileTypeMenu main_ft_menu; /* need two separate filetype menus since */
  FileTypeMenu exec_ft_menu; /* menu items can't have more than one parent*/
  GtkWidget *output_text;
  GtkWidget *command_line_box;
  GtkWidget *command_line;
  GtkWidget *selection;
  GtkTooltips *tooltips;
  GdkFont *output_font;
} App;

typedef struct _Config
{
  gchar version[8];
  gchar viewer_command[NAME_MAX];
  gchar xterm_command[NAME_MAX];
  gchar left_startup_dir[PATH_MAX];
  gchar right_startup_dir[PATH_MAX];
  gchar list_font[FONTNAME_MAX];
  gchar output_font[FONTNAME_MAX];
  gchar config_dir[PATH_MAX];
  time_t config_mtime;
  gboolean confirm_delete;
  gboolean confirm_overwrite;
  gboolean start_with_last_dir_left;
  gboolean start_with_last_dir_right;
  gboolean output_text_hidden;
  gboolean expand_popup_menu;
  gboolean use_internal_viewer;
  gboolean use_vi_keys;
  gboolean windows_right_click;
  gint window_width;
  gint window_height;
  gint vpane_position;
  gint dir_history_max_length;
  gint command_history_max_length;
  gint auto_refresh_enabled;
  guint auto_refresh_id;
  guint check_config_id;
  GList *plugins;
  GList *buttons;
  GList *filetypes;
  GList *bookmarks;
  GList *command_history;
  GList *user_commands;
  GList *toolbar_buttons;
  GList *key_bindings;
  GtkCornerType scrollbar_pos;
  ViewType view_type;
} Config;


extern App app;
extern Config cfg;
extern FileView *curr_view;
extern FileView *other_view;

extern Column all_columns[MAX_COLUMNS];
extern SystemOp system_ops[];
extern SystemOp interface_ops[];
extern gint n_sys_ops;
extern gint n_interface_ops;


/* Colors */
extern GdkColor EXE_COLOR;
extern GdkColor DIR_COLOR;
extern GdkColor LNK_COLOR;
extern GdkColor DEV_COLOR;
extern GdkColor COL_COLOR;
extern GdkColor TAG_COLOR;
extern GdkColor SOCK_COLOR;
extern GdkColor CLIST_COLOR;
extern GdkColor DRAG_HILIGHT;
extern GdkColor SELECT_COLOR;

/* Widget Utils */
extern GtkWidget *add_button(GtkWidget *box, gchar *label, gint fill,
                            gint pad, GtkSignalFunc func, gpointer data);
extern GtkWidget *add_button_with_icon(GtkWidget *box, gchar **xpm, gint fill,
                            gint pad, GtkSignalFunc func, gpointer data);
extern GtkWidget *add_label(GtkWidget *box, gchar *text, gfloat align,
                            gint fill, gint pad);
extern GtkWidget *add_entry(GtkWidget *box, gchar *init_text, gint fill,
                            gint pad);
extern GtkWidget *add_check_button(GtkWidget *box, gchar *label, gint state,
                                   gint fill, gint pad, GtkSignalFunc func,
                                   gpointer data);
extern GtkWidget *add_radio_button(GtkWidget *box, gchar *label, GSList *group,
                                   gint fill, gint pad, GtkSignalFunc func,
                                   gpointer data);
extern GtkWidget *add_hbox(GtkWidget *box, gint homogen, gint spacing,
                           gint fill, gint pad);
extern GtkWidget *add_vbox(GtkWidget *box, gint homogen, gint spacing,
                           gint fill, gint pad);
extern GtkWidget *add_sw(GtkWidget *box, GtkPolicyType h_policy,
                         GtkPolicyType v_policy, gint fill, gint pad);
extern GtkWidget *add_separator(GtkWidget *box, gint fill, gint pad);
extern GtkWidget *add_table(GtkWidget *box, gint rows, gint cols, gint homogen,
                            gint fill, gint pad);
extern GtkWidget *add_label_to_table(GtkWidget *table, gchar *text,
                                     gfloat align, gint left, gint right,
                                     gint top, gint bottom);
extern GtkWidget *add_entry_to_table(GtkWidget *table, gchar *init_text,
                                     gint left, gint right,
                                     gint top, gint bottom);
extern GtkWidget *add_button_to_table(GtkWidget *table, gchar *label,
                             GtkSignalFunc func, gpointer data,
                             gint left, gint right, gint top, gint bottom);
extern GtkWidget *add_check_button_to_table(GtkWidget *table, gchar *label,
                             gint state, GtkSignalFunc func, gpointer data,
                             gint left, gint right, gint top, gint bottom);
extern GtkWidget *add_radio_button_to_table(GtkWidget *table, gchar *label,
                  GSList *group, gint state, GtkSignalFunc func, gpointer data,
                  gint left, gint right, gint top, gint bottom);
extern GtkWidget *add_menu_item(GtkWidget *menu, gchar *label,
                                GtkSignalFunc func, gpointer data);
extern GtkWidget *add_menu_check_button(GtkWidget *menu, gchar *label,
                                        gboolean state, GtkSignalFunc func,
                                        gpointer data);
extern GtkWidget *add_menu_separator(GtkWidget *menu);
extern GtkWidget *add_submenu(GtkWidget *menu_bar, gchar *label,
                              GtkWidget *menu);
extern GtkWidget *add_framed_table(GtkWidget *box, gchar *title, gint rows,
                                   gint cols, gint fill, gint pad);
extern GtkWidget *add_framed_widget(GtkWidget *box, gchar *title,
                                    GtkWidget *widget, gint fill, gint pad);
extern void set_cursor(GdkCursorType type);

/* List Utils */
extern void free_glist_data(GList **list);
extern void free_clist_data(GtkWidget *clist);
extern GList *clist_data_to_glist(GtkWidget *clist);
extern GList *string_glist_find(GList *list, gchar *search_text);
extern gboolean clist_row_is_selected(GtkWidget *clist, gint row);
extern void clist_select_rows(GtkWidget *clist, GList *rows);

/* Utils */
extern gchar *str_to_lower(gchar *string);
extern gint S_ISEXE(mode_t mode);
extern void free_data(gpointer data);
extern gchar *get_key_name(gint keyval);
extern void copy_entry_to_str(GtkWidget *entry, gchar *str);
extern gboolean is_text(gchar *filename);
extern gboolean is_executable(FileInfo *info);
extern void view_file(gchar *filename);
extern void show_help_file(gchar *filename);
extern void chomp(gchar *text);
extern GString *expand_macros(gchar *text, gchar *for_each);
extern void exec_system_command(gchar *command);
extern void exec_interface_op(gchar *command);
extern void do_command(gchar *command);

/* Filetypes */
extern gchar *get_actions_for_ext(gchar *ext);
extern gchar *get_default_action_for_ext(gchar *ext);
extern FileType *get_filetype_for_ext(gchar *ext);
extern void add_filetype(gchar *ext, gchar *prog, gchar *desc);
extern void exec_filetype_action(gchar *action);

/* Plugins */
extern void destroy_plugin(Plugin *p);
extern void unload_all_plugins();
extern Plugin *load_plugin(gchar *filename);
extern void do_plugin_action(GtkWidget *widget, Plugin *p);
extern Plugin *get_plugin_by_name(gchar *name);

/* Interface callbacks */
extern void updir_cb(GtkWidget *widget);
extern void cd_home(GtkWidget *widget);
extern void switch_views_cb(GtkWidget *widget);
extern void toggle_left_panel_cb(GtkWidget *widget);
extern void toggle_right_panel_cb(GtkWidget *widget);
extern void toggle_hidden_cb(GtkWidget *widget);
extern void toggle_output_window_cb(GtkWidget *widget);
extern void goto_command_line_cb(GtkWidget *widget);
extern void show_menu_cb(GtkWidget *widget);
extern void show_plugins_menu_cb(GtkWidget *widget);
extern void show_user_menu_cb(GtkWidget *widget);

/* Callbacks */
extern void copy_cb(GtkWidget *widget);
extern void copy_as_cb(GtkWidget *widget);
extern void move_cb(GtkWidget *widget);
extern void move_as_cb(GtkWidget *widget);
extern void rename_cb(GtkWidget *widget);
extern void delete_cb(GtkWidget *widget);
extern void symlink_cb(GtkWidget *widget);
extern void symlink_as_cb(GtkWidget *widget);
extern void mkdir_cb(GtkWidget *widget);
extern void permissions_cb(GtkWidget *widget);
extern void ownership_cb(GtkWidget *widget);
extern void file_info_cb(GtkWidget *widget);
extern void sync_dirs_cb(GtkWidget *widget);
extern void view_cb(GtkWidget *widget);
extern void toggle_tag_cb();
extern void refresh_cb(GtkWidget *widget);
extern void command_cb(GtkWidget *widget);
extern void open_cb(GtkWidget *widget);
extern void open_with_cb(GtkWidget *widget);
extern void open_in_other_pane_cb(GtkWidget *widget);
extern void mount_cb(GtkWidget *widget);
extern void umount_cb(GtkWidget *widget);
extern void configure_cb(GtkWidget *widget);
extern void quit_cb(GtkWidget *widget);
extern void find_cb(GtkWidget *widget);
extern void execute_cb(GtkWidget *widget);
extern void execute_with_args_cb(GtkWidget *widget);
extern void execute_in_xterm_cb(GtkWidget *widget);
extern void execute_in_xterm_with_args_cb(GtkWidget *widget);

/* Window */
extern void create_main_window();
extern void recreate_main_window();

/* Command Panel */
extern void status_message(gchar *msg);
extern void status_errno();
extern void show_hide_output_text(GtkWidget *widget);
extern GtkWidget *create_output_window();
extern GtkWidget *create_command_line();

/* FileView */
extern void toggle_panel_size(FileView *view);
extern GtkWidget *create_file_view(FileView *view);

/* Filelist */
extern void get_perm_string(gchar *string, gint len, mode_t mode);
extern gint is_dir(FileInfo *info);
extern void change_dir(FileView *view, gchar *path);
extern gint name_sort(GtkCList *clist, gconstpointer row1, gconstpointer row2);
extern gint size_sort(GtkCList *clist, gconstpointer row1, gconstpointer row2);
extern gint date_sort(GtkCList *clist, gconstpointer row1, gconstpointer row2);
extern gint perm_sort(GtkCList *clist, gconstpointer row1, gconstpointer row2);
extern gint user_sort(GtkCList *clist, gconstpointer row1, gconstpointer row2);
extern gint group_sort(GtkCList *clist, gconstpointer row1, gconstpointer row2);
extern void sort_list(FileView *view, GtkCListCompareFunc,
                      GtkSortType direction, gint col);
extern void focus_row(FileView *view, gint row, gboolean clear_selection,
                      gboolean center, gboolean grab_focus);
extern void select_row_by_filename(FileView *view, gchar *filename);
extern GList *get_selection(FileView *view);
extern FileInfo *get_first_selected(FileView *view);
extern void initialize_filters(FileView *view);
extern void set_filter_menu_active(FileView *view);
extern void remove_filters(FileView *view);
extern void refresh_list(FileView *view);
extern void disable_refresh();
extern void reenable_refresh();
extern gint auto_refresh(gpointer data);
extern void load_dir_list(FileView *view);

/* Menu */
extern GtkWidget *create_filelist_menu_bar(FileView *view);
extern GtkWidget *create_main_menu_bar();
extern void create_menus();
extern void show_menu(guint button, guint32 time);
extern void show_plugins_menu(guint button, guint32 time);
extern void show_user_command_menu(guint button, guint32 time);
extern void load_bookmarks();

/* File Ops */
extern void set_sigchild_handler();
extern gint file_copy(gchar *src, gchar *dest);
extern gint file_move(gchar *src, gchar *dest);
extern gint file_delete(gchar *file);
extern gint file_chmod(gchar *path, gchar *mode, gboolean recurse_dirs);
extern gint file_chown(gchar *path, uid_t owner_id, gid_t group_id,
                       gboolean recurse_dirs);
extern gint file_mkdir(gchar *path);
extern gint file_symlink(gchar *src, gchar *dest);
extern gint file_exec(gchar *command);
extern gint file_mount(gchar *mount_point);
extern gint file_umount(gchar *mount_point);
extern gint exec_in_xterm(gchar *command);
extern gint exec_as_root(gchar *command);
extern gint exec_and_capture_output(gchar **args);
extern gint exec_and_capture_output_threaded(gchar *command);

/* Dialogs */
extern void create_confirm_dialog(gchar *label_text, guint *answer,
                                  guint buttons);
extern void create_confirm_del_dialog(gchar *filename, guint *answer);
extern void create_confirm_overwrite_dialog(gchar *filename, guint *answer);
extern gint op_with_ow_check(gchar *src, gchar *dest,
                             gint (*op_func)(gchar *, gchar *));
extern void create_permissions_dialog(FileInfo *info, guint *answer_ret,
            gboolean *recurse_ret, GString **mode_ret);
extern void create_ownership_dialog(FileInfo *info, guint *answer_ret, 
            uid_t *owner_ret, gid_t *group_ret, gboolean *recurse_ret);
extern void create_filetype_dialog(FileType **ft, gboolean write_file);
extern void create_filetype_action_dialog(gchar *name, gchar *action,
                                          gchar **arg1, gchar **arg2);
extern void create_init_filetype_dialog(gchar *filename);
extern void create_config_dialog();
extern void create_add_ext_dialog();
extern void create_file_info_dialog();
extern void create_view_file_dialog(gchar *filename);
extern void create_filename_filter_dialog(FileView *view);
extern void create_size_filter_dialog(FileView *view);
extern void create_date_filter_dialog(FileView *view);
extern void create_glob_dialog();
extern void create_rename_ext_dialog();
extern void create_user_prompt(gchar *prompt, gchar *init_text,
                               gboolean select_text, gchar **string);
extern void create_user_command_dialog(UserCommand **command);
extern void create_toolbar_button_dialog(ToolbarButton **tb);
extern void create_button_dialog(Button **button);
extern void create_key_binding_dialog(KeyBinding **kb);
extern void create_plugin_info_dialog(Plugin **p);
extern void create_open_query_dialog(gchar *filename);

/* Config Functions */
extern void set_config_dir();
extern gint check_config_files();
extern void touch_config_dir();
extern void write_filetypes_file();
extern void write_bookmarks_file();
extern void write_user_commands_file();
extern void write_toolbar_file();
extern void write_keys_file();
extern void write_buttons_file();
extern void write_plugins_file();
extern void write_config_file();
extern gboolean read_filetypes_file();
extern gboolean read_bookmarks_file();
extern gboolean read_user_commands_file();
extern gboolean read_toolbar_file();
extern gboolean read_keys_file();
extern gboolean read_buttons_file();
extern gboolean read_plugins_file();
extern gboolean read_config_file();

#endif /* __EMELFM_H__ */

