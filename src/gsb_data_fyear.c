/* ************************************************************************** */
/* work with the struct of fyear                                              */
/*                                                                            */
/*                                                                            */
/*     Copyright (C)	2000-2005 Cédric Auger (cedric@grisbi.org)	      */
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
 * \file gsb_data_fyear_data.c
 * work with the fyear structure, no GUI here
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_fyear.h"
#include "utils_dates.h"
#include "utils_str.h"
/*END_INCLUDE*/


/**
 * \struct 
 * Describe a fyear 
 */
typedef struct
{
    guint fyear_number;
    gchar *fyear_name;
    GDate *begining_date;
    GDate *end_date;
    gboolean showed_in_form;

} struct_fyear;


/*START_STATIC*/
static GSList *gsb_data_fyear_get_name_list ( void );
static gint gsb_data_fyear_get_number_by_name ( const gchar *name );
static gint gsb_data_fyear_get_pointer_from_name_in_glist ( struct_fyear *fyear,
						     const gchar *name );
static gpointer gsb_data_fyear_get_structure ( gint fyear_number );
static gint gsb_data_fyear_max_number ( void );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/** contains the g_slist of struct_fyear */
static GSList *fyear_list;

/** a pointer to the last fyear used (to increase the speed) */
static struct_fyear *fyear_buffer;


/**
 * set the fyears global variables to NULL,
 * usually when we init all the global variables
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_data_fyear_init_variables ( void )
{
    fyear_list = NULL;
    fyear_buffer = NULL;

    return FALSE;
}


/**
 * find and return the structure of the fyear asked
 *
 * \param fyear_number number of fyear
 *
 * \return the adr of the struct of the fyear (NULL if doesn't exit)
 * */
gpointer gsb_data_fyear_get_structure ( gint fyear_number )
{
    GSList *tmp;

    if (!fyear_number)
	return NULL;

    /* before checking all the fyears, we check the buffer */

    if ( fyear_buffer
	 &&
	 fyear_buffer -> fyear_number == fyear_number )
	return fyear_buffer;

    tmp = fyear_list;
    
    while ( tmp )
    {
	struct_fyear *fyear;

	fyear = tmp -> data;

	if ( fyear -> fyear_number == fyear_number )
	{
	    fyear_buffer = fyear;
	    return fyear;
	}

	tmp = tmp -> next;
    }
    return NULL;
}


/**
 * give the g_slist of fyears structure
 * usefull when want to check all fyears
 *
 * \param none
 *
 * \return the g_slist of fyears structure
 * */
GSList *gsb_data_fyear_get_fyears_list ( void )
{
    return fyear_list;
}

/**
 * return the number of the fyears given in param
 *
 * \param fyear_ptr a pointer to the struct of the fyear
 *
 * \return the number of the fyear, 0 if problem
 * */
gint gsb_data_fyear_get_no_fyear ( gpointer fyear_ptr )
{
    struct_fyear *fyear;
    
    if ( !fyear_ptr )
	return 0;
    
    fyear = fyear_ptr;
    fyear_buffer = fyear;
    return fyear -> fyear_number;
}


/**
 * find and return the last number of fyear
 * 
 * \param none
 * 
 * \return last number of fyear
 * */
gint gsb_data_fyear_max_number ( void )
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = fyear_list;
    
    while ( tmp )
    {
	struct_fyear *fyear;

	fyear = tmp -> data;

	if ( fyear -> fyear_number > number_tmp )
	    number_tmp = fyear -> fyear_number;

	tmp = tmp -> next;
    }
    return number_tmp;
}


/**
 * create a new fyear, give him a number, append it to the list
 * and return the number
 *
 * \param name the name of the fyear (can be freed after, it's a copy) or NULL
 *
 * \return the number of the new fyear
 * */
gint gsb_data_fyear_new ( const gchar *name )
{
    struct_fyear *fyear;

    fyear = calloc ( 1, sizeof ( struct_fyear ));
    fyear -> fyear_number = gsb_data_fyear_max_number () + 1;

    if (name)
	fyear -> fyear_name = my_strdup (name);
    else 
	fyear -> fyear_name = NULL;

    fyear_list = g_slist_append ( fyear_list, fyear );
    fyear_buffer = fyear;

    return fyear -> fyear_number;
}

/**
 * remove a fyear
 * set all the fyears of transaction which are this one to 0
 * update combofix and mark file as modified
 *
 * \param fyear_number the fyear we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_fyear_remove ( gint fyear_number )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return FALSE;
    
    fyear_list = g_slist_remove ( fyear_list,
				  fyear );
    
    /* remove the fyear from the buffers */

    if ( fyear_buffer == fyear )
	fyear_buffer = NULL;
    g_free (fyear);

    return TRUE;
}


/**
 * set a new number for the fyear
 * normally used only while loading the file because
 * the number are given automaticly
 *
 * \param fyear_number the number of the fyear
 * \param new_no_fyear the new number of the fyear
 *
 * \return the new number or 0 if the fyear doen't exist
 * */
gint gsb_data_fyear_set_new_number ( gint fyear_number,
				     gint new_no_fyear )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return 0;

    fyear -> fyear_number = new_no_fyear;
    return new_no_fyear;
}


/**
 * return the number of the fyear wich has the name in param
 *
 * \param name the name of the fyear
 * \param create TRUE if we want to create it if it doen't exist
 *
 * \return the number of the fyear or 0 if problem
 * */
gint gsb_data_fyear_get_number_by_name ( const gchar *name )
{
    GSList *list_tmp;
    gint fyear_number = 0;

    list_tmp = g_slist_find_custom ( fyear_list,
				     name,
				     (GCompareFunc) gsb_data_fyear_get_pointer_from_name_in_glist );
    
    if ( list_tmp )
    {
	struct_fyear *fyear;
	
	fyear = list_tmp -> data;
	fyear_number = fyear -> fyear_number;
    }
    return fyear_number;
}


/**
 * used with g_slist_find_custom to find a fyear in the g_list
 * by his name
 *
 * \param fyear the struct of the current fyear checked
 * \param name the name we are looking for
 *
 * \return 0 if it's the same name
 * */
gint gsb_data_fyear_get_pointer_from_name_in_glist ( struct_fyear *fyear,
						     const gchar *name )
{
    return ( g_strcasecmp ( fyear -> fyear_name,
			    name ));
}

/**
 * return the name of the fyear
 *
 * \param fyear_number the number of the fyear
 *
 * \return the name of the fyear or NULL if fail
 * */
gchar *gsb_data_fyear_get_name ( gint fyear_number )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return NULL;

    return fyear -> fyear_name;
}


/**
 * set the name of the fyear
 * the value is dupplicate in memory
 *
 * \param fyear_number the number of the fyear
 * \param name the name of the fyear
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_fyear_set_name ( gint fyear_number,
				   const gchar *name )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return FALSE;

    /* we free the last name */
    if ( fyear -> fyear_name )
	free (fyear -> fyear_name);
    
    /* and copy the new one */
    fyear -> fyear_name = my_strdup (name);

    return TRUE;
}



/**
 * return the begining date of the fyear
 *
 * \param fyear_number the number of the fyear
 *
 * \return the begining date of the fyear or NULL if fail
 * */
GDate *gsb_data_fyear_get_begining_date ( gint fyear_number )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return NULL;

    return fyear -> begining_date;
}


/**
 * set the begining date of the fyear
 * the value is dupplicate in memory
 *
 * \param fyear_number the number of the fyear
 * \param date the begining date of the fyear
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_fyear_set_begining_date ( gint fyear_number,
					    GDate *date )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return FALSE;

    /* we free the last date */
    if ( fyear -> begining_date )
	g_date_free (fyear -> begining_date);
    
    /* and copy the new one */
    fyear -> begining_date = gsb_date_copy (date);

    return TRUE;
}


/**
 * return the end date of the fyear
 *
 * \param fyear_number the number of the fyear
 *
 * \return the end date of the fyear or NULL if fail
 * */
GDate *gsb_data_fyear_get_end_date ( gint fyear_number )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return NULL;

    return fyear -> end_date;
}


/**
 * set the end date of the fyear
 * the value is dupplicate in memory
 *
 * \param fyear_number the number of the fyear
 * \param date the end date of the fyear
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_fyear_set_end_date ( gint fyear_number,
				       GDate *date )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return FALSE;

    /* we free the last date */
    if ( fyear -> end_date )
	g_date_free (fyear -> end_date);

    /* and copy the new one */
    fyear -> end_date = gsb_date_copy (date);

    return TRUE;
}



/**
 * return the showed_in_form of the fyear
 *
 * \param fyear_number the number of the fyear
 *
 * \return the showed_in_form of the fyear or NULL if fail
 * */
gboolean gsb_data_fyear_get_form_show ( gint fyear_number )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return FALSE;

    return fyear -> showed_in_form;
}


/**
 * set the showed_in_form of the fyear
 *
 * \param fyear_number the number of the fyear
 * \param showed_in_form the showed_in_form of the fyear
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_fyear_set_form_show ( gint fyear_number,
					gboolean showed_in_form )
{
    struct_fyear *fyear;

    fyear = gsb_data_fyear_get_structure ( fyear_number );

    if (!fyear)
	return FALSE;

    /* and copy the new one */
    fyear -> showed_in_form = showed_in_form;

    return TRUE;
}


/**
 * return a g_slist of names of all the fyears
 * it's not a copy of the gchar...
 *
 * \param none
 *
 * \return a g_slist of gchar *
 * */
GSList *gsb_data_fyear_get_name_list ( void )
{
    GSList *return_list;
    GSList *tmp_list;

    return_list = NULL;
    tmp_list= fyear_list;

    while ( tmp_list )
    {
	struct_fyear *fyear;

	fyear = tmp_list -> data;

	return_list = g_slist_append ( return_list,
				       fyear -> fyear_name );
	tmp_list = tmp_list -> next;
    }
    return return_list;
}


/**
 * get the financial year corresponding to the given date
 *
 * \param date
 *
 * \return the number of financial year or 0 if not on that date
 * */
gint gsb_data_fyear_get_from_date ( GDate *date )
{
    GSList *tmp_list;

    tmp_list = fyear_list;
    while (tmp_list)
    {
	struct_fyear *fyear;

	fyear = tmp_list -> data;

	if ( g_date_compare ( date, fyear -> begining_date) >= 0
	     &&
	     g_date_compare ( date, fyear -> end_date) <= 0 )
	    return fyear -> fyear_number;
	tmp_list = tmp_list -> next;
    }
    return 0;
}


/**
 * compare 2 financial years
 *
 * \param fyear_number_1
 * \param fyear_number_2
 *
 * \return -1 if fyear 1 is before 2 ; +1 if fyear 1 is after 2 ; 0 if problem
 * */
gint gsb_data_fyear_compare ( gint fyear_number_1,
			      gint fyear_number_2 )
{
    struct_fyear *fyear_1;
    struct_fyear *fyear_2;

    fyear_1 = gsb_data_fyear_get_structure (fyear_number_1);
    fyear_2 = gsb_data_fyear_get_structure (fyear_number_2);

    if (!fyear_1
	||
	!fyear_2)
	return 0;

    if (g_date_compare (fyear_1 -> begining_date, fyear_2 -> end_date) >= 0)
	return 1;
    if (g_date_compare (fyear_2 -> begining_date, fyear_1 -> end_date) >= 0)
	return -1;
    return 0;
}


