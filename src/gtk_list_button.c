/* ************************************************************************** */
/* Button with an icon and a text, with no particular size request.	      */
/*                                                                            */
/*     Copyright (C)    1995-1997 Peter Mattis, Spencer Kimball and           */
/*			          Josh MacDonald			      */
/*     	                2004      Benjamin Drieu (bdrieu@april.org)	      */
/* 			http://www.grisbi.org				      */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */


#include "gtk_list_button.h"

#define CHILD_SPACING     1

#include "xpm/book-closed.xpm"
#include "xpm/book-open.xpm"


/* Function protypes */
static void gtk_list_button_class_init ( GtkListButtonClass * );
static void gtk_list_button_init ( GtkListButton * );
static void gtk_list_button_size_request (GtkWidget *, GtkRequisition *);
gboolean gtk_list_button_clicked ( GtkButton * );
gboolean gtk_list_button_destroy ( GtkListButton * );



/** Contain a group of buttons, to store the one selected and thus,
  deselect is when another one is clicked.  Somewhat ugly, but I
  don't expect this to be widely used, so... */
GtkListButton * groups[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


/* Functions */

guint gtk_list_button_get_type ( void )
{
    static guint gtk_list_button_type = 0;

    if ( !gtk_list_button_type )
    {
	static const GtkTypeInfo gtk_list_button_info = {
	    "GtkListButton",
	    sizeof (GtkListButton),
	    sizeof (GtkListButtonClass),
	    (GtkClassInitFunc) gtk_list_button_class_init,
	    (GtkObjectInitFunc) gtk_list_button_init,
	    NULL,
	    NULL,
	    (GtkClassInitFunc) NULL
	};

	gtk_list_button_type = gtk_type_unique ( gtk_button_get_type(),
						 &gtk_list_button_info );
    }

    return ( gtk_list_button_type );

}



static void gtk_list_button_class_init ( GtkListButtonClass *klass )
{
    GtkWidgetClass *widget_class;

    widget_class = (GtkWidgetClass*) klass;
    widget_class->size_request = gtk_list_button_size_request;
}



static void gtk_list_button_init ( GtkListButton *list_button )
{
    list_button -> hbox = gtk_hbox_new ( FALSE, 6 );

    list_button -> closed_icon = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) book_closed_xpm ) );
    gtk_widget_set_usize ( list_button -> closed_icon, 24, 24 );

    list_button -> box = gtk_vbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( list_button -> hbox ), list_button -> box,
			 FALSE, FALSE, 0 );

    gtk_container_add ( GTK_CONTAINER(list_button -> box), list_button->closed_icon);

    list_button -> open_icon = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) book_open_xpm ));
    gtk_widget_set_usize ( list_button -> open_icon, 24, 24 );

    list_button -> label = gtk_label_new ("") ;
    gtk_label_set_line_wrap ( GTK_LABEL(list_button->label), TRUE );
    gtk_box_pack_start ( GTK_BOX(list_button -> hbox), list_button -> label,
			 FALSE, FALSE, 0 );

    gtk_container_add ( GTK_CONTAINER(list_button), list_button -> hbox );
    gtk_button_set_relief ( GTK_BUTTON (list_button), GTK_RELIEF_NONE );

    g_signal_connect ( GTK_BUTTON(list_button), "clicked", 
		       G_CALLBACK ( gtk_list_button_clicked ), NULL );
    g_signal_connect ( GTK_WIDGET(list_button), "destroy", 
		       G_CALLBACK ( gtk_list_button_destroy ), NULL );

    gtk_widget_show ( list_button -> hbox );
    gtk_widget_show ( list_button -> box );
    gtk_widget_show ( list_button -> closed_icon );
    gtk_widget_show ( list_button -> label );
}



GtkWidget *gtk_list_button_new ( gchar * text, int group )
{
    GtkListButton *list_button;

    list_button = g_object_new (GTK_TYPE_LIST_BUTTON, NULL);

    gtk_list_button_set_name ( list_button, text );
    list_button -> group = group;

    return ( GTK_WIDGET ( list_button ) );
}



static void gtk_list_button_size_request (GtkWidget      *widget,
					  GtkRequisition *requisition)
{
    GtkButton *button = GTK_BUTTON (widget);
    gint focus_width;
    gint focus_pad;

    gtk_widget_style_get (GTK_WIDGET (widget),
			  "focus-line-width", &focus_width,
			  "focus-padding", &focus_pad,
			  NULL);

    requisition->width = (GTK_CONTAINER (widget)->border_width + CHILD_SPACING +
			  GTK_WIDGET (widget)->style->xthickness) * 2;
    requisition->height = (GTK_CONTAINER (widget)->border_width + CHILD_SPACING +
			   GTK_WIDGET (widget)->style->ythickness) * 2;

    if (GTK_BIN (button)->child && GTK_WIDGET_VISIBLE (GTK_BIN (button)->child))
    {
	GtkRequisition child_requisition;

	gtk_widget_size_request (GTK_BIN (button)->child, &child_requisition);

	requisition->width += child_requisition.width;
	requisition->height += child_requisition.height;
    }

    /*   requisition->width += 2 * (focus_width + focus_pad); */
    requisition->width = -1;
    requisition->height += 2 * (focus_width + focus_pad);
}



void gtk_list_button_set_name ( GtkListButton * list_button, gchar * name )
{
    if ( name )
	gtk_label_set_text ( GTK_LABEL (list_button -> label), name );
}



gboolean gtk_list_button_clicked ( GtkButton *button )
{
    GtkListButton * listbutton;

    listbutton = groups [ GTK_LIST_BUTTON(button) -> group ];
    if ( listbutton && GTK_IS_WIDGET(listbutton) )
    {
	gtk_widget_hide ( listbutton -> open_icon );
	gtk_widget_show ( listbutton -> closed_icon );
	g_object_ref ( listbutton -> open_icon );
	gtk_container_add ( GTK_CONTAINER (listbutton -> box), 
			    listbutton -> closed_icon );
	gtk_container_remove ( GTK_CONTAINER (listbutton -> box), 
			       listbutton -> open_icon );
    }

    listbutton = GTK_LIST_BUTTON(button);

    gtk_widget_hide ( listbutton -> closed_icon );
    gtk_widget_show ( listbutton -> open_icon );
    g_object_ref ( listbutton -> closed_icon );
    gtk_container_add ( GTK_CONTAINER (listbutton -> box), 
			listbutton -> open_icon );
    gtk_container_remove ( GTK_CONTAINER (listbutton -> box), 
			   listbutton -> closed_icon );

    groups [ GTK_LIST_BUTTON(button) -> group ] = listbutton;

    return FALSE;
}


gboolean gtk_list_button_destroy ( GtkListButton * listbutton )
{
    if ( listbutton == groups [ listbutton -> group ] )
    {
	groups [ listbutton -> group ] = NULL;
    }  
    return ( FALSE );
}
