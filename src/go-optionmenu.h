/* 
 * go-optionmenu.h
 *
 * Copyright (C) 2002-2005 Andreas J. Guelzow <aguelzow@taliesin.ca>
 *
 * based extensively on:
 *
 * GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * Modified by the GTK+ Team and others 1997-2000.  See the GTK AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA.
 */

#ifndef _GO_OPTIONMENU_H_
#define _GO_OPTIONMENU_H_


#include <gdk/gdk.h>
#include <gtk/gtkbutton.h>

G_BEGIN_DECLS

#define GO_TYPE_OPTION_MENU              (go_option_menu_get_type ())
#define GO_OPTION_MENU(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GO_TYPE_OPTION_MENU, GOOptionMenu))
#define GO_OPTION_MENU_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), GO_TYPE_OPTION_MENU, GOOptionMenuClass))
#define GO_IS_OPTION_MENU(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GO_TYPE_OPTION_MENU))
#define GO_IS_OPTION_MENU_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GO_TYPE_OPTION_MENU))
#define GO_OPTION_MENU_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), GO_TYPE_OPTION_MENU, GOOptionMenuClass))


/* START_INCLUDE_H */
/* END_INCLUDE_H */

typedef struct _GOOptionMenu       GOOptionMenu;
typedef struct _GOOptionMenuClass  GOOptionMenuClass;

struct _GOOptionMenu
{
	GtkButton button;
	
	GtkWidget *menu;
	GtkWidget *select_menu;
	GtkWidget *menu_item;
	GtkWidget *old_menu_item;

	GtkWidget *last_signaled_menu_item;

	GSList *selection;
	gboolean new_selection;
	
	guint16 width;
	guint16 height;
};

struct _GOOptionMenuClass
{
  GtkButtonClass parent_class;

  void (*changed) (GOOptionMenu *option_menu);

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};


#define CHILD_LEFT_SPACING        4
#define CHILD_RIGHT_SPACING       1
#define CHILD_TOP_SPACING         1
#define CHILD_BOTTOM_SPACING      1

typedef struct _GOOptionMenuProps GOOptionMenuProps;

struct _GOOptionMenuProps
{
  gboolean interior_focus;
  GtkRequisition indicator_size;
  GtkBorder indicator_spacing;
  gint focus_width;
  gint focus_pad;
};


/* START_DECLARATION */
GtkWidget * go_option_menu_get_history (GOOptionMenu *option_menu);
GType go_option_menu_get_type (void);
GtkWidget* go_option_menu_new (void);
void go_option_menu_set_history (GOOptionMenu *option_menu, GSList *selection);
void go_option_menu_set_menu (GOOptionMenu *option_menu,
			       GtkWidget *menu);
/* END_DECLARATION */


G_END_DECLS

#endif /* _GO_OPTIONMENU_H_ */
