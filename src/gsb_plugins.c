/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2006-2006 Benjamin Drieu (bdrieu@april.org)	      */
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

#include "include.h"

/*START_INCLUDE*/
#include "gsb_plugins.h"
#include "dialog.h"
#include "gsb_file_config.h"
#include "include.h"
#include "gsb_plugins.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

/*START_STATIC*/
/*END_STATIC*/


/** List of registered plugins.  It should contain gsb_plugin structures. */
GSList * plugins = NULL;


/**
 *
 *
 */
void gsb_plugins_scan_dir ( const char *dirname )
{
    GDir * plugin_dir;
    gchar * filename, * plugin_name;

    plugin_dir = g_dir_open ( dirname, 0, NULL );
    if ( ! plugin_dir )
	return;
    
    while ( ( filename = g_dir_read_name ( plugin_dir ) ) != NULL )
    {
	gchar * complete_filename;
	gsb_plugin * plugin = g_malloc0 ( sizeof ( gsb_plugin ) );

	if ( strncmp ( filename + ( strlen ( filename ) - 3 ), ".so", 3 ) )
	    continue;

	complete_filename = g_strconcat ( PLUGINS_DIR, C_DIRECTORY_SEPARATOR,
					  filename, NULL );

	if ( ! ( plugin -> handle = 
		 g_module_open (complete_filename, G_MODULE_BIND_MASK ) ) )
	{
	    dialogue_error ( g_strdup_printf ( "Couldn't load module %s: %s", filename,
					       g_module_error() ) );
	    g_free ( plugin );
	    continue;
	}
	
	if ( ! g_module_symbol ( plugin -> handle, "plugin_name",
				 (gpointer) &plugin_name ) )
	{
	    dialogue_error ( g_strdup_printf ( "Plugin %s has no register symbol", 
					       filename ) );
	    g_free ( plugin );
	    continue;
	}
	plugin -> name = plugin_name;

	if ( ! g_module_symbol ( plugin -> handle, "plugin_register",
				 (gpointer)  &( plugin -> plugin_register ) ) )
	{
	    dialogue_error ( g_strdup_printf ( "Plugin %s has no register symbol", 
					       filename ) );
	    g_free ( plugin );
	    continue;
	}

	plugin -> plugin_register ();

	if ( ! g_module_symbol ( plugin -> handle, "plugin_run",
				 (gpointer) &( plugin -> plugin_run ) ) )
	{
	    dialogue_error ( g_strdup_printf ( "Plugin %s has no run symbol", 
					       filename ) );
	    g_free ( plugin );
	    continue;
	}

	plugins = g_slist_append ( plugins, plugin );

    }

    g_dir_close ( plugin_dir );
}



/**
 *
 *
 *
 */
gsb_plugin * gsb_find_plugin ( gchar * plugin_name )
{
    GSList * tmp = plugins;

    g_return_val_if_fail ( plugin_name, NULL );
    
    while ( tmp )
    {
	gsb_plugin * plugin = (gsb_plugin *) tmp -> data;

	if ( ! strcmp ( plugin_name, plugin -> name ) )
	{
	    return plugin;
	}

	tmp = tmp -> next;
    }

    return NULL;
}



/**
 * Get activated plugins.
 *
 * \return A newly-allocated string representing activated plugins.
 */
gchar * gsb_plugin_get_list ()
{
    gchar * list = NULL;
    GSList * tmp = plugins;

    while ( tmp )
    {
	gsb_plugin * plugin = (gsb_plugin *) tmp -> data;

	if ( ! list )
	{
	    list = g_strconcat ( _("with plugins "), plugin -> name, NULL );
	}
	else
	{
	    list = g_strconcat ( list, ", ", plugin -> name, NULL );
	}

	tmp = tmp -> next;
    }

    return list;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */