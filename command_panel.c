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
#include <glob.h>
#include <string.h>
#include <stdlib.h>  /* for getenv */
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include "emelfm.h"

/*
 * OUTPUT TEXT FUNCTIONS
 */

/* Use this to print messages to the output text field */
void
status_message(gchar *msg)
{
  gtk_text_insert(GTK_TEXT(app.output_text), app.output_font, NULL, NULL,
                  msg, -1);
  if (cfg.output_text_hidden)
    show_hide_output_text(NULL);
}

void
status_errno()
{
  status_message(g_strerror(errno));
  status_message("\n");
}

void
show_hide_output_text(GtkWidget *widget)
{
  gint pos;

  if (cfg.output_text_hidden)
  {
    pos = cfg.vpane_position;
    if (widget != NULL) /* user clicked the show/hide button */
      gtk_widget_grab_focus(GTK_COMBO(app.command_line)->entry);
  }
  else
  {
    cfg.vpane_position = GTK_PANED(app.vpane)->child1->allocation.height;
    pos = app.main_window->allocation.height;
    if (widget != NULL) /* user clicked the show/hide button */
      gtk_widget_grab_focus(curr_view->clist);
  }

  gtk_paned_set_position(GTK_PANED(app.vpane), pos);
}

static void
output_text_adjusted_cb(GtkWidget *widget, GtkAllocation *alloc)
{
  if (app.output_text->allocation.height < 10)
    cfg.output_text_hidden = TRUE;
  else
    cfg.output_text_hidden = FALSE;
}

static void
output_text_clicked_cb(GtkWidget *widget, GdkEventButton *event)
{
  if (event->button == 1)
    gtk_widget_grab_focus(GTK_COMBO(app.command_line)->entry);
  else if (event->button == 3)
    show_hide_output_text(NULL);
}

static void
show_key_bindings()
{
  GList *tmp;

  for (tmp = cfg.key_bindings; tmp != NULL; tmp = tmp->next)
  {
    KeyBinding *kb = tmp->data;

    if (strncmp(kb->action, "SYSTEM_", 7) == 0)
      status_message(kb->action+7);
    else if (strncmp(kb->action, "INTERFACE:", 10) == 0)
      status_message(kb->action+10);
    else if (strncmp(kb->action, "PLUGIN:", 7) == 0)
      status_message(kb->action+7);
    else
      status_message(kb->action);

    status_message("\t\t");
    if (kb->state & GDK_CONTROL_MASK)
      status_message(_("Ctrl+"));
    if (kb->state & GDK_MOD1_MASK)
      status_message(_("Alt+"));
    if (kb->state & GDK_SHIFT_MASK)
      status_message(_("Shift+"));

    status_message(kb->key_name);
    status_message("\n");
  }

  if (cfg.use_vi_keys)
  {
    status_message(
_( "\n"
"Vi Keys\n"
"~~~~~~~\n"
"Updir      h        Find       /\n"
"Open       l        Tag        t\n"
"Up         k        Top        g\n"
"Down       j        Bottom     G\n"
"\n"));
  }
}

/*
 * COMMAND LINE FUNCTIONS
 */
static void
command_entered(GtkWidget *entry)
{
  gchar *command, *s, *command_copy;
  GList *tmp;

  command = gtk_entry_get_text(GTK_ENTRY(entry));
  if (command[strlen(command)-1] == '&')
    file_exec(command);
  else
  {
    command_copy = s = g_strdup(command);
    if ((s = strchr(command_copy, ' ')) != NULL)
      *s = '\0';

    if (STREQ(command_copy, "cd"))
    {
      if (strlen(command) > 2)
      {
        change_dir(curr_view, s+1);
        gtk_widget_grab_focus(entry);
      }
      else
      {
        gchar *home;
        if ((home = getenv("HOME")) != NULL)
        {
          change_dir(curr_view, home);
          gtk_widget_grab_focus(entry);
        }
      }
    }
    else if (STREQ(command_copy, "x") && strlen(command) > 1)
    {
      exec_in_xterm(s+1);
      gtk_widget_grab_focus(entry);
    }
    else if (STREQ(command_copy, "clear"))
    {
      gint n = gtk_text_get_length(GTK_TEXT(app.output_text));
      gtk_text_backward_delete(GTK_TEXT(app.output_text), n);
    }
    else if (STREQ(command_copy, "keys"))
    {
      show_key_bindings();
    }
    else
      exec_and_capture_output_threaded(command);
    g_free(command_copy);
  }

  tmp = string_glist_find(cfg.command_history, command);
  if (tmp != NULL) /* found the command in history */
  {
    cfg.command_history = g_list_remove_link(cfg.command_history, tmp);
    cfg.command_history = g_list_append(cfg.command_history, g_strdup(tmp->data));
    free_glist_data(&tmp);
  }
  else /* didn't find the command in history */
  {
    cfg.command_history = g_list_append(cfg.command_history, g_strdup(command));
    if (g_list_length(cfg.command_history) > cfg.command_history_max_length)
    {
      tmp = g_list_nth(cfg.command_history, 0);
      cfg.command_history = g_list_remove_link(cfg.command_history, tmp);
      free_glist_data(&tmp);
    }
  }

  gtk_combo_set_popdown_strings(GTK_COMBO(app.command_line),
                                cfg.command_history);
  gtk_entry_set_text(GTK_ENTRY(entry), "");
}

static void
do_tab_completion()
{
  gchar *word, *old_command;
  GString *new_command;
  gchar pattern[NAME_MAX];
  glob_t matches;
  gint i;

  old_command = g_strdup(gtk_entry_get_text(GTK_ENTRY(
                         GTK_COMBO(app.command_line)->entry)));

  /* extract the word we're trying to complete */
  if ((word = strrchr(old_command, ' ')) == NULL)
    word = old_command;
  else
    *word++ = '\0';

  /* Glob it */
  g_snprintf(pattern, sizeof(pattern), "%s*", word);
  if (glob(pattern, GLOB_MARK, NULL, &matches) != 0)
  {
    g_free(old_command);
    gtk_widget_grab_focus(GTK_COMBO(app.command_line)->entry);
    return;
  }

  if (matches.gl_pathc == 1) /* found an exact match */
  {
    if (old_command == word)
      new_command = g_string_new(matches.gl_pathv[0]);
    else
    {
      new_command = g_string_new(old_command);
      g_string_sprintfa(new_command, " %s", matches.gl_pathv[0]);
    }
    if (new_command->str[new_command->len-1] != '/')
      g_string_append_c(new_command, ' ');
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(app.command_line)->entry),
                       new_command->str);
    g_string_free(new_command, TRUE);
  }
  else /* there were multiple possible matches */
  {
    for (i = 0; i < matches.gl_pathc; i++)
    {
      status_message(matches.gl_pathv[i]);
      status_message("\n");
    }

    /* find the largest common string within the matches*/
    for (i = 0; i < NAME_MAX; i++)
    {
      gint j;
      for (j = 0; j < matches.gl_pathc; j++)
      {
        if (matches.gl_pathv[0][i] != matches.gl_pathv[j][i])
          break;
      }
      if (j != matches.gl_pathc)
        break;
    }
    matches.gl_pathv[0][i] = '\0';

    if (old_command == word)
      new_command = g_string_new(matches.gl_pathv[0]);
    else
    {
      new_command = g_string_new(old_command);
      g_string_sprintfa(new_command, " %s", matches.gl_pathv[0]);
    }
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(app.command_line)->entry),
                       new_command->str);
    g_string_free(new_command, TRUE);
  }

  g_free(old_command);
  globfree(&matches);

  gtk_widget_grab_focus(GTK_COMBO(app.command_line)->entry);
}

static void
command_line_key_press_cb(GtkWidget  *entry, GdkEventKey  *event)
{
  GtkAdjustment *adj;

  switch (event->keyval)
  {
  case GDK_Return:
    if ((event->state & GDK_MOD1_MASK) || (event->state & GDK_CONTROL_MASK))
    {
      FileInfo *info;
      GList *tmp, *base;

      gtk_signal_emit_stop_by_name(GTK_OBJECT(entry), "key_press_event");

      disable_refresh();
      base = tmp = get_selection(curr_view);
      for (; tmp != NULL; tmp = tmp->next)
      {
        info = tmp->data;
        gtk_entry_append_text(GTK_ENTRY(entry), info->filename);
        gtk_entry_append_text(GTK_ENTRY(entry), " ");
      }
      g_list_free(base);
      reenable_refresh();
    }
    break;

  case 'o':
    if ((event->state & GDK_MOD1_MASK) || (event->state & GDK_CONTROL_MASK))
    {
      gtk_entry_append_text(GTK_ENTRY(entry), other_view->dir);
      gtk_entry_append_text(GTK_ENTRY(entry), " ");
    }
    break;

  case 'l':
    if ((event->state & GDK_MOD1_MASK) || (event->state & GDK_CONTROL_MASK))
    {
      FileInfo *info;
      GList *tmp, *base;

      gtk_signal_emit_stop_by_name(GTK_OBJECT(entry), "key_press_event");

      disable_refresh();
      base = tmp = get_selection(other_view);
      for (; tmp != NULL; tmp = tmp->next)
      {
        info = tmp->data;
        gtk_entry_append_text(GTK_ENTRY(entry), other_view->dir);
        gtk_entry_append_text(GTK_ENTRY(entry), "/");
        gtk_entry_append_text(GTK_ENTRY(entry), info->filename);
        gtk_entry_append_text(GTK_ENTRY(entry), " ");
      }
      g_list_free(base);
      reenable_refresh();
    }
    break;

  case 'd':
    if ((event->state & GDK_MOD1_MASK) || (event->state & GDK_CONTROL_MASK))
    {
      gtk_entry_append_text(GTK_ENTRY(entry), curr_view->dir);
      gtk_entry_append_text(GTK_ENTRY(entry), " ");
    }
    break;

  case 'z':
    if ((event->state & GDK_MOD1_MASK) || (event->state & GDK_CONTROL_MASK))
    {
      gtk_widget_grab_focus(curr_view->clist);
      if (!cfg.output_text_hidden)
        show_hide_output_text(NULL);
    }
    break;
    
  case GDK_Tab:
    gtk_signal_emit_stop_by_name(GTK_OBJECT(entry), "key_press_event");
    do_tab_completion();
    break;

  case GDK_Page_Up:
    adj = GTK_TEXT(app.output_text)->vadj;
    if (adj != NULL)
    {
      gint value = adj->value - (adj->page_size / 2);
      if (value < 0)
        value = 0;
      gtk_adjustment_set_value(adj, value);
    }
    break;

  case GDK_Page_Down:
    adj = GTK_TEXT(app.output_text)->vadj;
    if (adj != NULL)
    {
      gint end = adj->upper - adj->lower - adj->page_size;
      gint value = adj->value + (adj->page_size / 2);
      if (value > end)
        value = end;
      gtk_adjustment_set_value(adj, value);
    }
    break;

  default:
    break;
  }
}

GtkWidget *
create_command_line()
{
  GtkWidget *box, *wid;
  struct passwd *pw_buf;
  gchar hostname[32];
  gchar label_text[MAX_LEN];
  GList *tmp = NULL;

  box = gtk_hbox_new(FALSE, 0);
  wid = add_button(box, "^", FALSE, 1, show_hide_output_text, NULL);
  GTK_WIDGET_UNSET_FLAGS(wid, GTK_CAN_FOCUS);
  gtk_tooltips_set_tip(GTK_TOOLTIPS(app.tooltips), wid, _("Show/Hide output area"), NULL);
  
  if (((pw_buf = getpwuid(getuid())) != NULL)
      && (!gethostname(hostname, sizeof(hostname))))
  {
    gchar *s;
    if ((s = strchr(hostname, '.')) != NULL)
      *s = '\0';
    g_snprintf(label_text, sizeof(label_text), "%s@%s %c",
               pw_buf->pw_name, hostname, (getuid() == 0) ? '#' : '$');
    add_label(box, label_text, 0, FALSE, 2);
  }
  else
  {
    add_label(box, _("Command: "), 0, FALSE, 2);
  }

  app.command_line = gtk_combo_new();
  gtk_box_pack_start(GTK_BOX(box), app.command_line, TRUE, TRUE, 0);
  gtk_combo_disable_activate(GTK_COMBO(app.command_line));
  gtk_combo_set_use_arrows_always(GTK_COMBO(app.command_line), TRUE);
  gtk_combo_set_case_sensitive(GTK_COMBO(app.command_line), TRUE);
  tmp = g_list_append(tmp, "");
  gtk_combo_set_popdown_strings(GTK_COMBO(app.command_line), tmp);
  g_list_free(tmp);

  if (app.output_font != NULL)
  {
    GtkStyle *style = gtk_style_copy(gtk_widget_get_style(GTK_COMBO(app.command_line)->entry));
    style->font = app.output_font;
    gtk_widget_set_style(GTK_COMBO(app.command_line)->entry, style);
  }
  gtk_signal_connect(GTK_OBJECT(GTK_COMBO(app.command_line)->entry), 
                     "activate", GTK_SIGNAL_FUNC(command_entered), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_COMBO(app.command_line)->entry),
                     "key_press_event", 
                     GTK_SIGNAL_FUNC(command_line_key_press_cb), NULL);
  gtk_widget_show(app.command_line);
  gtk_widget_show(box);

  return box;
}

GtkWidget *
create_output_window()
{
  GtkWidget *sw;

  sw = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_show(sw);

  app.output_text = gtk_text_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(sw), app.output_text);
  gtk_text_set_editable(GTK_TEXT(app.output_text), FALSE);
  gtk_text_set_line_wrap(GTK_TEXT(app.output_text), TRUE); 
  GTK_WIDGET_UNSET_FLAGS(app.output_text, GTK_CAN_FOCUS);
  gtk_signal_connect(GTK_OBJECT(app.output_text), "size-allocate",
                     GTK_SIGNAL_FUNC(output_text_adjusted_cb), NULL);
  gtk_signal_connect(GTK_OBJECT(app.output_text), "button_press_event",
                     GTK_SIGNAL_FUNC(output_text_clicked_cb), NULL);
  gtk_widget_show(app.output_text);
  app.output_font = gdk_font_load(cfg.output_font);

  return sw;
}

