/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	     2005 Benjamin Drieu (bdrieu@april.org)	      */
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


/**
 * \file gsb_status.c
 * Various routines that implement the status bar.
 */

#include "include.h"

/*START_INCLUDE*/
#include "gsb_status.h"
#include "main.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_status_pulse (  );
static void gsb_status_remove_progress (  );
static void gsb_status_set_progress ( gdouble ratio, gdouble max );
static void gsb_status_show_progress ();
/*END_STATIC*/

/*START_EXTERN*/
extern gint max;
extern GtkWidget *window;
/*END_EXTERN*/

/** Status bar displayed in the bottom of Grisbi window.  */
GtkWidget *main_statusbar = NULL;

/** Optional progress bar in main status bar.  */
GtkWidget * progress_bar = NULL;

guint context_id;
guint message_id = -1;
int timer_id;


/**
 * Create and return a new GtkStatusBar to hold various status
 * informations.
 *
 * \return	A newly allocated GtkStatusBar.
 */
GtkWidget * gsb_new_statusbar ()
{
    main_statusbar = gtk_statusbar_new ();
    context_id = gtk_statusbar_get_context_id ( main_statusbar, "Grisbi" );
    message_id = -1;

    gtk_widget_show_all ( main_statusbar );

    return main_statusbar;
}



/**
 *
 *
 */
void gsb_status_message ( gchar * message )
{
    if ( ! main_statusbar || ! GTK_IS_STATUSBAR ( main_statusbar ) )
	return NULL;

    gsb_status_clear ();
    message_id = gtk_statusbar_push ( main_statusbar, context_id, message );
}



/**
 *
 *
 */
void gsb_status_clear (  )
{
    if ( ! main_statusbar || ! GTK_IS_STATUSBAR ( main_statusbar ) )
	return NULL;

    if ( message_id >= 0 )
    {
	gtk_statusbar_remove ( main_statusbar, context_id, message_id );
	message_id = -1;
    }
}



/**
 *
 *
 */
void gsb_status_show_progress ()
{
    if ( progress_bar )
    {
	return;
    }

    progress_bar = gtk_progress_bar_new();
    gtk_box_pack_start ( GTK_BOX ( main_statusbar ), progress_bar, FALSE, FALSE, 6 );
    gtk_widget_show ( progress_bar );
}



/**
 *
 *
 */
void gsb_status_set_progress ( gdouble ratio, gdouble max )
{
    if ( ! progress_bar )
    {
	return;
    }

    gtk_progress_bar_set_fraction ( progress_bar, ratio / max );
}



/**
 *
 *
 */
void gsb_status_remove_progress (  )
{
    if ( ! progress_bar )
    {
	return;
    }

    gtk_widget_destroy ( progress_bar );
    progress_bar = NULL;
}



/**
 *
 *
 */
gboolean gsb_status_pulse (  )
{
    if ( ! progress_bar )
    {
	return;
    }

    gtk_progress_bar_pulse ( progress_bar );
    while ( gtk_events_pending () ) gtk_main_iteration ( );

    /* As this is a timeout function, return TRUE so that it
     * continues to get called */
    return TRUE;
}



void gsb_status_start_activity (  )
{
    gsb_status_show_progress ();
    timer_id = g_timeout_add ( 100, gsb_status_pulse, NULL );
}



void gsb_status_stop_activity (  )
{
    g_source_remove ( timer_id );
    gsb_status_remove_progress ();
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
