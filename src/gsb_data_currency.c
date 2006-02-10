/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2006 Cédric Auger (cedric@grisbi.org)	      */
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
 * \file gsb_data_currency.c
 * work with the currency structure, no GUI here
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_currency.h"
#include "utils_dates.h"
#include "utils_str.h"
/*END_INCLUDE*/


/**
 * \struct 
 * Describe a currency 
 */
typedef struct
{
    guint currency_number;
    gchar *currency_name;
    gchar *currency_code;
    gchar *currency_code_iso4217;

    /* the next fields shouldn't be used except for old currencies,
     * they should be removed when i will change the currency function */
    gint change_to_euro;                                  /* à 1 si cette devise doit passer à l'euro */
    gint link_currency;       /* si = 1 : 1 nom_devise = "change" devise_en_rapport */
    gint contra_currency_number; /* the contra currency if linked with another currency */
    GDate *change_date;                     /*   dernière mise à jour du change, NULL si aucun change */
    gdouble change_rate;                                          /* taux de change */
} struct_currency;

/*START_STATIC*/
static GSList *gsb_data_currency_get_name_list ( void );
static gpointer gsb_data_currency_get_structure ( gint currency_number );
static gint gsb_data_currency_max_number ( void );
static gboolean gsb_data_currency_set_default_currency ( gint currency_number );
/*END_STATIC*/

/*START_EXTERN*/
extern     gchar * buffer ;
/*END_EXTERN*/

/** contains the g_slist of struct_currency */
static GSList *currency_list;

/** a pointer to the last currency used (to increase the speed) */
static struct_currency *currency_buffer;

/** the number of the default currency */
static gint default_currency_number;

/**
 * set the currencies global variables to NULL, usually when we init all the global variables
 *
 * \param none
 *
 * \return FALSE
 * */
gboolean gsb_data_currency_init_variables ( void )
{
    currency_list = NULL;
    currency_buffer = NULL;
    default_currency_number = 0;

    return FALSE;
}


/**
 * find and return the structure of the currency asked
 *
 * \param currency_number number of currency
 *
 * \return the adr of the struct of the currency (NULL if doesn't exit)
 * */
gpointer gsb_data_currency_get_structure ( gint currency_number )
{
    GSList *tmp;

    if (!currency_number)
	return NULL;

    /* before checking all the currencies, we check the buffer */

    if ( currency_buffer
	 &&
	 currency_buffer -> currency_number == currency_number )
	return currency_buffer;

    tmp = currency_list;

    while ( tmp )
    {
	struct_currency *currency;

	currency = tmp -> data;

	if ( currency -> currency_number == currency_number )
	{
	    currency_buffer = currency;
	    return currency;
	}
	tmp = tmp -> next;
    }
    return NULL;
}

/**
 * return the number of the currency given in param
 *
 * \param currency_ptr a pointer to the struct of the currency
 *
 * \return the number of the currency, 0 if problem
 * */
gint gsb_data_currency_get_no_currency ( gpointer currency_ptr )
{
    struct_currency *currency;

    if ( !currency_ptr )
	return 0;

    currency = currency_ptr;
    currency_buffer = currency;
    return currency -> currency_number;
}


/**
 * give the g_slist of currency structure
 * usefull when want to check all currencies
 *
 * \param none
 *
 * \return the g_slist of currencies structure
 * */
GSList *gsb_data_currency_get_currency_list ( void )
{
    return currency_list;
}



/**
 * find and return the last number of currency
 * 
 * \param none
 * 
 * \return last number of currency
 * */
gint gsb_data_currency_max_number ( void )
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = currency_list;

    while ( tmp )
    {
	struct_currency *currency;

	currency = tmp -> data;

	if ( currency -> currency_number > number_tmp )
	    number_tmp = currency -> currency_number;

	tmp = tmp -> next;
    }
    return number_tmp;
}


/**
 * create a new currency, give him a number, append it to the list
 * and return the number
 *
 * \param name the name of the currency (can be freed after, it's a copy) or NULL
 *
 * \return the number of the new currency
 * */
gint gsb_data_currency_new ( const gchar *name )
{
    struct_currency *currency;

    currency = calloc ( 1, sizeof ( struct_currency ));
    currency -> currency_number = gsb_data_currency_max_number () + 1;

    if (name)
	currency -> currency_name = my_strdup (name);
    else 
	currency -> currency_name = NULL;

    currency_list = g_slist_append ( currency_list, currency );

    return currency -> currency_number;
}


/**
 * remove a currency
 * set all the currencies of transaction which are this one to 0
 * update combofix and mark file as modified
 *
 * \param currency_number the currency we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_currency_remove ( gint currency_number )
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return FALSE;

    currency_list = g_slist_remove ( currency_list,
				     currency );

    /* remove the currency from the buffers */

    if ( currency_buffer == currency )
	currency_buffer = NULL;
    g_free (currency);

    return TRUE;
}


/**
 * set a new number for the currency
 * normally used only while loading the file because
 * the number are given automaticly
 *
 * \param currency_number the number of the currency
 * \param new_no_currency the new number of the currency
 *
 * \return the new number or 0 if the currency doen't exist
 * */
gint gsb_data_currency_set_new_number ( gint currency_number,
					gint new_no_currency )
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return 0;

    currency -> currency_number = new_no_currency;
    return new_no_currency;
}


/**
 * give the default currency number
 *
 * \param
 *
 * \return the default currency number
 * */
gint gsb_data_currency_get_default_currency (void)
{
    return default_currency_number;
}

/**
 * set the default currency number
 * if the currency corresponding to the number doesn't exists, return FALSE and do nothing
 *
 * \param currency_number
 *
 * \return TRUE ok, FALSE the currency doesn't exist
 * */
gboolean gsb_data_currency_set_default_currency ( gint currency_number )
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return FALSE;

    default_currency_number = currency_number;
    return TRUE;
}



/**
 * return the name of the currency
 *
 * \param currency_number the number of the currency
 *
 * \return the name of the currency or NULL if problem
 * */
gchar *gsb_data_currency_get_name ( gint currency_number )
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return NULL;

    return currency -> currency_name;
}


/**
 * set the name of the currency
 * the value is dupplicate in memory
 *
 * \param currency_number the number of the currency
 * \param name the name of the currency
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_currency_set_name ( gint currency_number,
				      const gchar *name )
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return FALSE;

    /* we free the last name */
    if ( currency -> currency_name )
	free (currency -> currency_name);

    /* and copy the new one */
    if (name)
	currency -> currency_name = my_strdup (name);
    else
	currency -> currency_name = NULL;

    return TRUE;
}

/**
 * return a g_slist of names of all the currencies
 * it's not a copy of the gchar...
 *
 * \param none
 *
 * \return a g_slist of gchar *
 * */
GSList *gsb_data_currency_get_name_list ( void )
{
    GSList *return_list;
    GSList *tmp_list;

    return_list = NULL;
    tmp_list= currency_list;

    while ( tmp_list )
    {
	struct_currency *currency;

	currency = tmp_list -> data;

	return_list = g_slist_append ( return_list,
				       currency -> currency_name );
	tmp_list = tmp_list -> next;
    }
    return return_list;
}


/**
 * return the  of the currency
 *
 * \param currency_number the number of the currency
 *
 * \return the currency_code of the currency or NULL if problem
 * */
gchar *gsb_data_currency_get_code ( gint currency_number )
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return NULL;

    return currency -> currency_code;
}


/**
 * set the  of the currency
 * the value is dupplicate in memory
 *
 * \param currency_number the number of the currency
 * \param  the  of the currency
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_currency_set_code ( gint currency_number,
				      const gchar *currency_code )
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return FALSE;

    /* we free the last  */
    if ( currency -> currency_code )
	free (currency -> currency_code);

    /* and copy the new one */
    if (currency_code)
	currency -> currency_code = my_strdup (currency_code);
    else
	currency -> currency_code = NULL;

    return TRUE;
}


/**
 * return the  of the currency
 *
 * \param currency_number the number of the currency
 *
 * \return the code_iso4217 of the currency or NULL if problem
 * */
gchar *gsb_data_currency_get_code_iso4217 ( gint currency_number )
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return NULL;

    return currency -> currency_code_iso4217;
}


/**
 * set the  of the currency
 * the value is dupplicate in memory
 *
 * \param currency_number the number of the currency
 * \param  the code_iso4217 of the currency
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_currency_set_code_iso4217 ( gint currency_number,
					      const gchar *currency_code_iso4217 )
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return FALSE;

    /* we free the last  */
    if ( currency -> currency_code_iso4217 )
	free (currency ->currency_code_iso4217 );

    /* and copy the new one */
    if (currency_code_iso4217)
	currency -> currency_code_iso4217 = my_strdup (currency_code_iso4217);
    else
	currency -> currency_code_iso4217 = NULL;

    return TRUE;
}


/**
 * return the change_to_euro of the currency
 *
 * \param currency_number the number of the currency
 *
 * \return the change_to_euro of the currency or -1 if problem
 * */
gint gsb_data_currency_get_change_to_euro ( gint currency_number )
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return -1;

    return currency -> change_to_euro;
}


/**
 * set the change_to_euro of the currency
 * the value is dupplicate in memory
 *
 * \param currency_number the number of the currency
 * \param  the change_to_euro of the currency
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_currency_set_change_to_euro ( gint currency_number,
						gint change_to_euro)
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return FALSE;

    currency -> change_to_euro = change_to_euro;

    return TRUE;
}


/**
 * return the link_currency of the currency
 *
 * \param currency_number the number of the currency
 *
 * \return the link_currency of the currency or -1 if problem
 * */
gint gsb_data_currency_get_link_currency ( gint currency_number )
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return -1;

    return currency -> link_currency;
}


/**
 * set the link_currency of the currency
 * the value is dupplicate in memory
 *
 * \param currency_number the number of the currency
 * \param  the link_currency of the currency
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_currency_set_link_currency ( gint currency_number,
					       gint link_currency)
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return FALSE;

    currency -> link_currency = link_currency;

    return TRUE;
}


/**
 * return the contra_currency_number of the currency
 *
 * \param currency_number the number of the currency
 *
 * \return the contra_currency_number of the currency or 0 if problem
 * */
gint gsb_data_currency_get_contra_currency_number ( gint currency_number )
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return 0;

    return currency -> contra_currency_number;
}


/**
 * set the contra_currency_number of the currency
 * the value is dupplicate in memory
 *
 * \param currency_number the number of the currency
 * \param  the contra_currency_number of the currency
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_currency_set_contra_currency_number ( gint currency_number,
							gint contra_currency_number)
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return FALSE;

    currency -> contra_currency_number = contra_currency_number;

    return TRUE;
}


/**
 * return the change_date of the currency
 *
 * \param currency_number the number of the currency
 *
 * \return the change_date of the currency or NULL if problem
 * */
GDate *gsb_data_currency_get_change_date ( gint currency_number )
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return NULL;

    return currency -> change_date;
}


/**
 * set the change_date of the currency
 * the value is dupplicate in memory
 *
 * \param currency_number the number of the currency
 * \param  the change_date of the currency
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_currency_set_change_date ( gint currency_number,
					     GDate *change_date )
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return FALSE;

    currency -> change_date = gsb_date_copy (change_date);

    return TRUE;
}


/**
 * return the change_rate of the currency
 *
 * \param currency_number the number of the currency
 *
 * \return the change_rate of the currency or 0 if problem
 * */
gdouble gsb_data_currency_get_change_rate ( gint currency_number )
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return 0.0;

    return currency -> change_rate;
}


/**
 * set the change_rate of the currency
 * the value is dupplicate in memory
 *
 * \param currency_number the number of the currency
 * \param  the  of the currency
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_currency_set_change_rate ( gint currency_number,
					     gdouble change_rate)
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return FALSE;

    currency -> change_rate = change_rate;

    return TRUE;
}

/**
 * return the number of the currency wich has the name in param
 * create it if necessary
 *
 * \param name the name of the currency
 *
 * \return the number of the currency or 0 if doesn't exist
 * */
gint gsb_data_currency_get_number_by_name ( const gchar *name )
{
    GSList *list_tmp;

    if (!name)
	return FALSE;

    list_tmp = currency_list;

    while (list_tmp)
    {
	struct_currency *currency;

	currency = list_tmp -> data;

	if (!strcmp (currency -> currency_name,
		     name ))
	    return (currency -> currency_number);
	list_tmp = list_tmp -> next;
    }

    return FALSE;
}

/**
 * return the number of the currency wich has the code iso4217 in param
 * create it if necessary
 *
 * \param code the iso code of the currency
 *
 * \return the number of the currency or 0 if doesn't exist
 * */
gint gsb_data_currency_get_number_by_code_iso4217 ( const gchar *code )
{
    GSList *list_tmp;

    if (!code)
	return FALSE;

    list_tmp = currency_list;

    while (list_tmp)
    {
	struct_currency *currency;

	currency = list_tmp -> data;

	if (!strcmp (currency -> currency_code_iso4217,
		     code ))
	    return (currency -> currency_number);
	list_tmp = list_tmp -> next;
    }

    return FALSE;
}


/**
 * return the code, and if the currency has no code, return the isocode
 *
 * \param currency_number
 *
 * \return the code, isocode or NULL
 * */
gchar *gsb_data_currency_get_code_or_isocode ( gint currency_number )
{
    struct_currency *currency;

    currency = gsb_data_currency_get_structure ( currency_number );

    if (!currency)
	return NULL;

    if (currency -> currency_code)
	return currency -> currency_code;

    return currency -> currency_code_iso4217;
}