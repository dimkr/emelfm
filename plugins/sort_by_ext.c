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
#include "../emelfm.h"

static gint
ext_sort(GtkCList *clist, gconstpointer row1, gconstpointer row2)
{
  FileInfo *info1, *info2;
  gchar *s1, *s2;
  gint i;

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

  if ((s1 = strrchr(info1->filename, '.')) == NULL)
  {
    /* info1 has NO extension.. check info2 */
    if ((s2 = strrchr(info2->filename, '.')) != NULL)
      return -1;
    else
      return strcmp(info1->filename, info2->filename);
  }

  /* info1 HAS an extension.. check info2 */
  if ((s2 = strrchr(info2->filename, '.')) == NULL)
    return 1;

  /* Both have extensions.. see if they are the same */
  if ((i = strcmp(s1, s2)) == 0)
    return strcmp(info1->filename, info2->filename);
  else
    return strcmp(s1, s2);
}

static void
sort_by_ext()
{
  sort_list(curr_view, ext_sort, GTK_SORT_ASCENDING, 0);
}

gint
init_plugin(Plugin *p)
{
  p->name = "Sort By Extension";
  p->description = "Sort the current File List by filename extension.";
  p->plugin_cb = sort_by_ext;

  return TRUE;
}


