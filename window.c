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

#include "emelfm.h"

/*
 * TOOLBAR FUNCTION CALLBACKS
 */
static void
toolbar_command(GtkWidget *widget, ToolbarButton *tb)
{
  do_command(tb->action);
}

static void
toolbar_click_cb(GtkWidget *widget, GdkEventButton *event, ToolbarButton *tb)
{
  if (event->button == 3)
  {
    gtk_widget_set_sensitive(app.main_window, FALSE);
    create_toolbar_button_dialog(&tb);
    gtk_main();

    gtk_label_set_text(GTK_LABEL(GTK_BIN(widget)->child), tb->label);
    gtk_tooltips_set_tip(GTK_TOOLTIPS(app.tooltips), widget, tb->tooltip, NULL);
    write_toolbar_file();
    touch_config_dir();
    gtk_widget_set_sensitive(app.main_window, TRUE);
    gtk_widget_grab_focus(curr_view->clist);
  }
}

/*
 * Main Window Callbacks
 */

static void
user_button_callback(GtkWidget *widget, gchar *action)
{
  do_command(action);
}

static void
user_button_click_cb(GtkWidget *widget, GdkEventButton *event, Button *button)
{
  if (event->button == 3)
  {
    gtk_widget_set_sensitive(app.main_window, FALSE);
    create_button_dialog(&button);
    gtk_main();

    gtk_label_set_text(GTK_LABEL(GTK_BIN(widget)->child), button->label);
    write_buttons_file();
    touch_config_dir();
    gtk_widget_set_sensitive(app.main_window, TRUE);
    gtk_widget_grab_focus(curr_view->clist);
  }
}

/*
 * WIDGET BUILDERS
 */
static void
create_button_column()
{
  GtkWidget *vbox, *wid;
  GList *tmp;

  vbox = gtk_vbox_new(FALSE, 10);

  for (tmp = cfg.buttons; tmp != NULL; tmp = tmp->next)
  {
    Button *button = tmp->data;
    wid = add_button(vbox, button->label, FALSE, 0,
                     user_button_callback, button->action);
    GTK_WIDGET_UNSET_FLAGS(wid, GTK_CAN_FOCUS);
    gtk_signal_connect(GTK_OBJECT(wid), "button_press_event",
                       GTK_SIGNAL_FUNC(user_button_click_cb), button);
  }

  app.button_col_sw = add_sw(app.hbox, GTK_POLICY_AUTOMATIC,
                             GTK_POLICY_AUTOMATIC, FALSE, 5);
  gtk_widget_set_usize(app.button_col_sw, 100, 0);
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(app.button_col_sw),
                                        vbox);
  gtk_widget_show(vbox);
  if (cfg.view_type != BOTH_PANES)
    gtk_widget_hide(app.button_col_sw);
}

static void
create_toolbar()
{
  GtkWidget *wid;
  GList *tmp;

  app.toolbar_button_box = add_hbox(app.toolbar, FALSE, 0, FALSE, FALSE);
  for (tmp = cfg.toolbar_buttons; tmp != NULL; tmp = tmp->next)
  {
    ToolbarButton *tb = tmp->data;

    wid = add_button(app.toolbar_button_box, tb->label, FALSE, 1,
                     toolbar_command, tb);
    GTK_WIDGET_UNSET_FLAGS(wid, GTK_CAN_FOCUS);
    gtk_tooltips_set_tip(GTK_TOOLTIPS(app.tooltips), wid, tb->tooltip, NULL);
    gtk_signal_connect(GTK_OBJECT(wid), "button_press_event",
                       GTK_SIGNAL_FUNC(toolbar_click_cb), tb);
  }
}

/* Main Window Setup */
void
create_main_window()
{
  GtkWidget *main_window_vbox;
  GtkWidget *wid;
  gchar label_text[MAX_LEN];
  
  app.main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect(GTK_OBJECT(app.main_window), "delete_event",
                     GTK_SIGNAL_FUNC(quit_cb), NULL);
  gtk_widget_set_usize(app.main_window, cfg.window_width, cfg.window_height);
  gtk_window_set_policy(GTK_WINDOW(app.main_window), TRUE, TRUE, FALSE);
  gtk_widget_realize(app.main_window);

  app.tooltips = gtk_tooltips_new();

  main_window_vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(app.main_window), main_window_vbox);
  gtk_widget_show(main_window_vbox);

  /* Welcome Label */
  g_snprintf(label_text, sizeof(label_text),
             _("Welcome to emelFM - Version %s"), VERSION);
  add_label(main_window_vbox, label_text, 0.5, FALSE, 0);
  
  app.hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(app.hbox);

  /* Menus */
  create_menus();

  /* Left Panel */
  app.left_view_box = create_file_view(&app.left_view);
  curr_view = &app.left_view;
  gtk_box_pack_start(GTK_BOX(app.hbox), app.left_view_box, TRUE, TRUE, 0);
  if (cfg.view_type == RIGHT_PANE)
    gtk_widget_hide(app.left_view_box);

  /* Button Column */
  if (cfg.buttons != NULL)
    create_button_column();

  /* Right Panel */
  app.right_view_box = create_file_view(&app.right_view);
  other_view = &app.right_view;
  gtk_box_pack_start(GTK_BOX(app.hbox), app.right_view_box, TRUE, TRUE, 0);
  if (cfg.view_type == LEFT_PANE)
    gtk_widget_hide(app.right_view_box);

  /* Command Panel */
  wid = create_output_window();
  app.vpane = gtk_vpaned_new();
  gtk_paned_pack1(GTK_PANED(app.vpane), app.hbox, TRUE, TRUE);
  gtk_paned_pack2(GTK_PANED(app.vpane), wid, TRUE, TRUE);
  gtk_paned_set_position(GTK_PANED(app.vpane),
                         (cfg.output_text_hidden
                          ? cfg.window_height
                          : cfg.vpane_position));
  gtk_widget_show(app.vpane);
  gtk_box_pack_start(GTK_BOX(main_window_vbox), app.vpane, TRUE, TRUE, 0);

  app.toolbar = add_hbox(main_window_vbox, FALSE, 0, FALSE, 0);
  wid = create_command_line();
  gtk_box_pack_start(GTK_BOX(app.toolbar), wid, TRUE, TRUE, 0);

  /* Toolbar Buttons */
  create_toolbar();

  /* Staus Bar */
  app.status_bar = add_label(main_window_vbox, _("Ready"), 0.5, FALSE, 0);

  /* Hidden entry field for selection.. Have to pack it to prevent a crash */
  app.selection = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(app.hbox), app.selection, TRUE, TRUE, 0); 
  
  load_bookmarks();

  gtk_widget_show(app.main_window);
}

void
recreate_main_window()
{
  FileView *old_curr_view, *old_other_view;
  gfloat curr_scroll_pos, other_scroll_pos;
  GList *curr_selection, *other_selection;
  GtkAdjustment *vadj;

  /* Save some info on the current view so we can reset it when done */
  old_curr_view = curr_view;
  old_other_view = other_view;
  curr_scroll_pos = gtk_clist_get_vadjustment(GTK_CLIST(curr_view->clist))->value;
  other_scroll_pos = gtk_clist_get_vadjustment(GTK_CLIST(other_view->clist))->value;
  curr_selection = g_list_copy(GTK_CLIST(curr_view->clist)->selection);
  other_selection = g_list_copy(other_view->old_selection);

  /* Destroy the widgets */
  gtk_widget_destroy(app.left_view_box);
  gtk_widget_destroy(app.right_view_box);
  gtk_widget_destroy(app.button_col_sw);
  gtk_widget_destroy(app.toolbar_button_box);
  
  /* Menus */
  create_menus();

  /* Left Panel */
  app.left_view_box = create_file_view(&app.left_view);
  gtk_box_pack_start(GTK_BOX(app.hbox), app.left_view_box, TRUE, TRUE, 0);
  if (cfg.view_type == RIGHT_PANE)
    gtk_widget_hide(app.left_view_box);

  /* Button Column */
  if (cfg.buttons != NULL)
    create_button_column();

  /* Right Panel */
  app.right_view_box = create_file_view(&app.right_view);
  gtk_box_pack_start(GTK_BOX(app.hbox), app.right_view_box, TRUE, TRUE, 0);
  if (cfg.view_type == LEFT_PANE)
    gtk_widget_hide(app.right_view_box);

  /* Toolbar buttons */
  create_toolbar();

  app.output_font = gdk_font_load(cfg.output_font);
  if (app.output_font != NULL)
  {
    GtkStyle *style = gtk_style_copy(gtk_widget_get_style(app.command_line));
    style->font = app.output_font;
    gtk_widget_set_style(GTK_COMBO(app.command_line)->entry, style);
  }

  load_bookmarks();

  sort_list(&app.right_view, name_sort, GTK_SORT_ASCENDING, 0);
  sort_list(&app.left_view, name_sort, GTK_SORT_ASCENDING, 0);
  change_dir(&app.right_view, app.right_view.dir);
  change_dir(&app.left_view, app.left_view.dir);

  /* Restore the saved state */
  clist_select_rows(old_other_view->clist, other_selection);
  clist_select_rows(old_curr_view->clist, curr_selection);
  g_list_free(other_selection);
  g_list_free(curr_selection);
  vadj = gtk_clist_get_vadjustment(GTK_CLIST(old_curr_view->clist));
  gtk_adjustment_set_value(GTK_ADJUSTMENT(vadj), curr_scroll_pos);
  vadj = gtk_clist_get_vadjustment(GTK_CLIST(old_other_view->clist));
  gtk_adjustment_set_value(GTK_ADJUSTMENT(vadj), other_scroll_pos);
}


