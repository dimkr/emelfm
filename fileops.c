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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include "emelfm.h"

/*
 * This file contains functions used to handle different file operations
 * like copying, deleting, sym linking, etc.. most functions do this by 
 * forking and running the standard unix commands for the appropriate operation
 */

/* we need to install this sigchild handler so that we don't get zombies */
static void
received_sigchild()
{
  waitpid(-1, NULL, WNOHANG);
}

void
set_sigchild_handler()
{
  struct sigaction new_action;

  new_action.sa_handler = received_sigchild;
  sigemptyset(&new_action.sa_mask);
  new_action.sa_flags = SA_RESTART;
  sigaction(SIGCHLD, &new_action, NULL);
}

static void start_reader_thread(void *ptr);

/* simply executes the argument array passed. the sigchild handler set up
 * during startup will handle the process when it has ended so that we dont
 * get zombies
 */
static gint
execute(gchar **args)
{
  gint pid;

  if ( (pid = fork()) == 0 )
  {
    close(0);
    close(1);
    close(2);
    execvp(args[0], args);
    exit(127);
  }

  return pid;
}
    
/* copy src to dest.. return when the operation is done */
gint
file_copy(gchar *src, gchar *dest)
{
  gchar *args[5];

  args[0] = "cp";
  args[1] = "-rf";
  args[2] = src;
  args[3] = dest;
  args[4] = NULL;

  return exec_and_capture_output(args);
}

/* delete file.. return when the operation is done */
gint
file_delete(gchar *file)
{
  gchar *args[4];
  
  args[0] = "rm";
  args[1] = "-rf";
  args[2] = file;
  args[3] = NULL;

  return exec_and_capture_output(args); 
}

/* move src to dest.. return when the operation is done */
gint
file_move(gchar *src, gchar *dest)
{
  gchar *args[5];

  args[0] = "mv";
  args[1] = "-f";
  args[2] = src;
  args[3] = dest;
  args[4] = NULL;

  return exec_and_capture_output(args);
}

gint
file_chmod(gchar *path, gchar *mode, gboolean recurse_dirs)
{
  gchar *args[5];
  gint i = 0;

  args[i++] = "chmod";
  if (recurse_dirs)
    args[i++] = "-R";
  args[i++] = mode;
  args[i++] = path;
  args[i++] = NULL;
  
  return exec_and_capture_output(args);
}

gint
file_chown(gchar *path,
           uid_t owner_id,
           gid_t group_id,
           gboolean recurse_dirs)
{
  gchar *args[5];
  gchar mode[14];
  gint i = 0;

  g_snprintf(mode, sizeof(mode), "%d:%d", (int) owner_id, (int) group_id);

  args[i++] = "chown";
  if (recurse_dirs)
    args[i++] = "-R";
  args[i++] = mode;
  args[i++] = path;
  args[i++] = NULL;

  return exec_and_capture_output(args);
}
  
/* symlink src to dest */
gint
file_symlink(gchar *src, gchar *dest)
{
  gchar *args[5];
  gint i = 0;

  args[i++] = "ln";
  args[i++] = "-s";
  args[i++] = src;
  args[i++] = dest;
  args[i++] = NULL;

  return exec_and_capture_output(args);
}

gint
file_mount(gchar *mount_point)
{
  gchar *args[3];

  args[0] = "mount";
  args[1] = mount_point;
  args[2] = NULL;

  return exec_and_capture_output(args);
}

gint
file_umount(gchar *mount_point)
{
  gchar *args[3];

  args[0] = "umount";
  args[1] = mount_point;
  args[2] = NULL;

  return exec_and_capture_output(args);
}

gint
file_mkdir(gchar *path)
{
  return mkdir(path, 0777);
}

/* execute command.. do not wait for it to exit */
gint
file_exec(gchar *command)
{
  gchar *args[4];
  pid_t pid;

  args[0] = "sh";
  args[1] = "-c";
  args[2] = command;
  args[3] = NULL;

  pid = execute(args);
  return pid;
}

gint
op_with_ow_check(gchar *src, gchar *dest, gint (*op_func)(gchar *, gchar *))
{
  gint overwrite_button;
  gchar *s;

  if ((s = strrchr(dest, '/')) != NULL)
    create_confirm_overwrite_dialog(s+1, &overwrite_button);
  else
    create_confirm_overwrite_dialog(dest, &overwrite_button);

  gtk_main();
  switch (overwrite_button)
  {
    case YES:
      op_func(src, dest);
      return YES;
    case YES_TO_ALL:
      op_func(src, dest);
      return YES_TO_ALL;
    case NO:
      return NO;
    case CANCEL:
      return CANCEL;
    default:
      fprintf(stderr, _("Something went wrong in op_with_ow_check\n"));
      break;
  }

  /* This can never happen */
  return -1;
}

gint
exec_in_xterm(gchar *command)
{
  gchar *args[6];

  args[0] = cfg.xterm_command;
  args[1] = "-e";
  args[2] = "sh";
  args[3] = "-c";
  args[4] = command;
  args[5] = NULL;

  return execute(args);
}

gint
exec_and_capture_output(gchar **args)
{
  gint file_pipes[2];
  gint pid;
  gchar buf[1024];
  gint nread;

  if (pipe(file_pipes) != 0)
  {
    status_errno();
    return -1;
  }

  if ((pid = fork()) == -1)
  {
    status_errno();
    return -1;
  }

  if (pid == 0)
  {
    close(0);
    close(1);
    close(2);
    dup(file_pipes[0]);
    dup(file_pipes[1]);
    dup(file_pipes[1]);
    close(file_pipes[0]);
    close(file_pipes[1]);

    execvp(args[0], args);
    exit(127);
  }
  else
  {
    close(file_pipes[1]);
    while ((nread = read(file_pipes[0], buf, sizeof(buf)-1)) > 0)
    {
      buf[nread] = '\0';
      gtk_text_insert(GTK_TEXT(app.output_text), app.output_font, NULL, NULL,
                      buf, -1);
      if (cfg.output_text_hidden)
        show_hide_output_text(NULL);
    }
    close(file_pipes[0]);
  }

  return 0;
}

gint
exec_and_capture_output_threaded(gchar *command)
{
  gint file_pipes[2];
  gint pid;
  gchar *args[4];
  pthread_t reader_thread;


  if (pipe(file_pipes) != 0)
  {
    status_errno();
    return -1;
  }

  if ((pid = fork()) == -1)
  {
    status_errno();
    return -1;
  }

  if (pid == 0)
  {
    close(0);
    close(1);
    close(2);
    dup(file_pipes[0]);
    dup(file_pipes[1]);
    dup(file_pipes[1]);
    close(file_pipes[0]);
    close(file_pipes[1]);

    args[0] = "sh";
    args[1] = "-c";
    args[2] = command;
    args[3] = NULL;
    execvp(args[0], args);
    exit(127);
  }
  else
  {
    close(file_pipes[1]);
    pthread_create(&reader_thread, NULL, (void *)&start_reader_thread,
                   g_memdup(&file_pipes[0], sizeof(gint)));
  }

  return pid;
}

static void
start_reader_thread(void *ptr)
{
  gchar buf[1024];
  gint nread;
  gint *file_pipe = (gint *)ptr;

  while ((nread = read(*file_pipe, buf, sizeof(buf)-1)) > 0)
  {
    buf[nread] = '\0';
    gdk_threads_enter();
    gtk_text_insert(GTK_TEXT(app.output_text), app.output_font, NULL, NULL,
                    buf, -1);
    gdk_threads_leave();
  }
  gdk_threads_enter();
  gtk_text_insert(GTK_TEXT(app.output_text), app.output_font, NULL, NULL,
                  _("----end-of-output----\n"), -1);
  gdk_threads_leave();
  close(*file_pipe);
  g_free(file_pipe);
  pthread_exit(0);
}

