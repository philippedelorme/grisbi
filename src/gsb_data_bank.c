/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2006 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2006 Benjamin Drieu (bdrieu@april.org)	      */
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
 * \file gsb_data_bank.c
 * work with the bank structure, no GUI here
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_bank.h"
#include "gsb_data_account.h"
#include "utils_str.h"
/*END_INCLUDE*/


/**
 * \struct 
 * Describe a bank 
 */
typedef struct
{
    /* WARNING : cannot set guint for bank_number because a bug before (and after ?)
     * 0.6 sometimes set -1 to bank_number and big pb after that if we set guint here */
    gint bank_number;
    gchar *bank_name;
    gchar *bank_code;

    gchar *bank_address;
    gchar *bank_tel;
    gchar *bank_mail;
    gchar *bank_web;
    gchar *bank_note;

    gchar *correspondent_name;
    gchar *correspondent_tel;
    gchar *correspondent_mail;
    gchar *correspondent_fax;
} struct_bank;

/*START_STATIC*/
static GSList *gsb_data_bank_get_name_list ( void );
static gint gsb_data_bank_get_number_by_name ( const gchar *name );
static gpointer gsb_data_bank_get_structure ( gint bank_number );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/** contains the g_slist of struct_bank */
static GSList *bank_list;

/** a pointer to the last bank used (to increase the speed) */
static struct_bank *bank_buffer;


/**
 * set the banks global variables to NULL, usually when we init all the global variables
 *
 * \param none
 *
 * \return FALSE
 * */
gboolean gsb_data_bank_init_variables ( void )
{
    bank_list = NULL;
    bank_buffer = NULL;

    return FALSE;
}


/**
 * find and return the structure of the bank asked
 *
 * \param bank_number number of bank
 *
 * \return the adr of the struct of the bank (NULL if doesn't exit)
 * */
gpointer gsb_data_bank_get_structure ( gint bank_number )
{
    GSList *tmp;

    if (!bank_number)
	return NULL;

    /* before checking all the banks, we check the buffer */
    if ( bank_buffer
	 &&
	 bank_buffer -> bank_number == bank_number )
	return bank_buffer;

    tmp = bank_list;

    while ( tmp )
    {
	struct_bank *bank;

	bank = tmp -> data;

	if ( bank -> bank_number == bank_number )
	{
	    bank_buffer = bank;
	    return bank;
	}
	tmp = tmp -> next;
    }
    return NULL;
}

/**
 * return the number of the bank given in param
 *
 * \param bank_ptr a pointer to the struct of the bank
 *
 * \return the number of the bank, 0 if problem
 * */
gint gsb_data_bank_get_no_bank ( gpointer bank_ptr )
{
    struct_bank *bank;

    if ( !bank_ptr )
	return 0;

    bank = bank_ptr;
    bank_buffer = bank;
    return bank -> bank_number;
}


/**
 * give the g_slist of bank structure
 *
 * \param none
 *
 * \return the g_slist of banks structure
 * */
GSList *gsb_data_bank_get_bank_list ( void )
{
    return bank_list;
}



/**
 * find and return the last number of bank
 * 
 * \param none
 * 
 * \return last number of bank
 * */
gint gsb_data_bank_max_number ( void )
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = bank_list;

    while ( tmp )
    {
	struct_bank *bank;

	bank = tmp -> data;

	if ( bank -> bank_number > number_tmp )
	    number_tmp = bank -> bank_number;

	tmp = tmp -> next;
    }
    return number_tmp;
}


/**
 * create a new bank, give it a number, append it to the list
 * and return the number
 *
 * \param name the name of the bank (can be freed after, it's a copy) or NULL
 *
 * \return the number of the new bank
 * */
gint gsb_data_bank_new ( const gchar *name )
{
    struct_bank *bank;

    bank = calloc ( 1, sizeof ( struct_bank ));
    bank -> bank_number = gsb_data_bank_max_number () + 1;

    if (name)
	bank -> bank_name = my_strdup (name);

    bank_list = g_slist_append ( bank_list, bank );

    return bank -> bank_number;
}


/**
 * remove a bank
 *
 * \param bank_number the bank we want to remove
 *
 * \return TRUE ok
 * */
gboolean gsb_data_bank_remove ( gint bank_number )
{
    struct_bank *bank;
    GSList *list_tmp;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return FALSE;

    bank_list = g_slist_remove ( bank_list,
				 bank );

    /* remove the bank from the buffers */
    if ( bank_buffer == bank )
	bank_buffer = NULL;
    g_free (bank);

    /* remove that bank of the accounts */
    list_tmp = gsb_data_account_get_list_accounts ();

    while (list_tmp)
    {
	gint account_number = gsb_data_account_get_no_account (list_tmp -> data);

	if ( gsb_data_account_get_bank (account_number) == bank_number )
	    gsb_data_account_set_bank ( account_number, 0 );
	list_tmp = list_tmp -> next;
    }

    return TRUE;
}


/**
 * set a new number for the bank
 * normally used only while loading the file because
 * the number are given automaticly
 *
 * \param bank_number the number of the bank
 * \param new_no_bank the new number of the bank
 *
 * \return the new number or 0 if the bank doen't exist
 * */
gint gsb_data_bank_set_new_number ( gint bank_number,
				    gint new_no_bank )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure (bank_number);

    if (!bank)
	return 0;

    bank -> bank_number = new_no_bank;
    return new_no_bank;
}


/**
 * return the name of the bank
 *
 * \param bank_number the number of the bank
 *
 * \return the name of the bank or NULL if problem
 * */
gchar *gsb_data_bank_get_name ( gint bank_number )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return NULL;

    return bank -> bank_name;
}


/**
 * set the name of the bank
 * the value is dupplicate in memory (so parameter can be freed after)
 *
 * \param bank_number the number of the bank
 * \param name the name of the bank
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_bank_set_name ( gint bank_number,
				  const gchar *name )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return FALSE;

    /* we free the last name */
    if ( bank -> bank_name )
	free (bank -> bank_name);

    /* and copy the new one */
    bank -> bank_name = my_strdup (name);

    return TRUE;
}

/**
 * return a g_slist of names of all the banks
 * it's not a copy of the gchar..., so don't free them
 *
 * \param none
 *
 * \return a g_slist of gchar *
 * */
GSList *gsb_data_bank_get_name_list ( void )
{
    GSList *return_list;
    GSList *list_tmp;

    return_list = NULL;
    list_tmp= bank_list;

    while ( list_tmp )
    {
	struct_bank *bank;

	bank = list_tmp -> data;

	return_list = g_slist_append ( return_list,
				       bank -> bank_name );
	list_tmp = list_tmp -> next;
    }
    return return_list;
}


/**
 * return the  of the bank
 *
 * \param bank_number the number of the bank
 *
 * \return the bank_code of the bank or NULL if problem
 * */
gchar *gsb_data_bank_get_code ( gint bank_number )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return NULL;

    return bank -> bank_code;
}


/**
 * set the  of the bank
 * the value is dupplicate in memory
 *
 * \param bank_number the number of the bank
 * \param  the  of the bank
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_bank_set_code ( gint bank_number,
				  const gchar *bank_code )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return FALSE;

    /* we free the last  */
    if ( bank -> bank_code )
	free (bank -> bank_code);

    /* and copy the new one */
    bank -> bank_code = my_strdup (bank_code);

    return TRUE;
}



/**
 * return the bank_address of the bank
 *
 * \param bank_number the number of the bank
 *
 * \return the bank_address of the bank or NULL if problem
 * */
gchar *gsb_data_bank_get_bank_address ( gint bank_number )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return NULL;

    return bank -> bank_address;
}


/**
 * set the bank_address of the bank
 * the value is dupplicate in memory (so parameter can be freed after)
 *
 * \param bank_number the number of the bank
 * \param bank_address the bank_address of the bank
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_bank_set_bank_address ( gint bank_number,
					  const gchar *bank_address )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return FALSE;

    /* we free the last name */
    if ( bank -> bank_address )
	free (bank -> bank_address);

    /* and copy the new one */
    bank -> bank_address = my_strdup (bank_address);

    return TRUE;
}

/**
 * return the bank_tel of the bank
 *
 * \param bank_number the number of the bank
 *
 * \return the bank_tel of the bank or NULL if problem
 * */
gchar *gsb_data_bank_get_bank_tel ( gint bank_number )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return NULL;

    return bank -> bank_tel;
}


/**
 * set the bank_tel of the bank
 * the value is dupplicate in memory (so parameter can be freed after)
 *
 * \param bank_number the number of the bank
 * \param bank_tel the bank_tel of the bank
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_bank_set_bank_tel ( gint bank_number,
				      const gchar *bank_tel )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return FALSE;

    /* we free the last name */
    if ( bank -> bank_tel )
	free (bank -> bank_tel);

    /* and copy the new one */
    bank -> bank_tel = my_strdup (bank_tel);

    return TRUE;
}


/**
 * return the bank_mail of the bank
 *
 * \param bank_number the number of the bank
 *
 * \return the bank_mail of the bank or NULL if problem
 * */
gchar *gsb_data_bank_get_bank_mail ( gint bank_number )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return NULL;

    return bank -> bank_mail;
}


/**
 * set the bank_mail of the bank
 * the value is dupplicate in memory (so parameter can be freed after)
 *
 * \param bank_number the number of the bank
 * \param bank_mail the bank_mail of the bank
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_bank_set_bank_mail ( gint bank_number,
				       const gchar *bank_mail )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return FALSE;

    /* we free the last name */
    if ( bank -> bank_mail )
	free (bank -> bank_mail);

    /* and copy the new one */
    bank -> bank_mail = my_strdup (bank_mail);

    return TRUE;
}


/**
 * return the bank_web of the bank
 *
 * \param bank_number the number of the bank
 *
 * \return the bank_web of the bank or NULL if problem
 * */
gchar *gsb_data_bank_get_bank_web ( gint bank_number )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return NULL;

    return bank -> bank_web;
}


/**
 * set the bank_web of the bank
 * the value is dupplicate in memory (so parameter can be freed after)
 *
 * \param bank_number the number of the bank
 * \param bank_web the bank_web of the bank
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_bank_set_bank_web ( gint bank_number,
				      const gchar *bank_web )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return FALSE;

    /* we free the last name */
    if ( bank -> bank_web )
	free (bank -> bank_web);

    /* and copy the new one */
    bank -> bank_web = my_strdup (bank_web);

    return TRUE;
}


/**
 * return the bank_note of the bank
 *
 * \param bank_number the number of the bank
 *
 * \return the bank_note of the bank or NULL if problem
 * */
gchar *gsb_data_bank_get_bank_note ( gint bank_number )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return NULL;

    return bank -> bank_note;
}


/**
 * set the bank_note of the bank
 * the value is dupplicate in memory (so parameter can be freed after)
 *
 * \param bank_number the number of the bank
 * \param bank_note the bank_note of the bank
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_bank_set_bank_note ( gint bank_number,
				       const gchar *bank_note )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return FALSE;

    /* we free the last name */
    if ( bank -> bank_note )
	free (bank -> bank_note);

    /* and copy the new one */
    bank -> bank_note = my_strdup (bank_note);

    return TRUE;
}


/**
 * return the correspondent_name of the bank
 *
 * \param bank_number the number of the bank
 *
 * \return the correspondent_name of the bank or NULL if problem
 * */
gchar *gsb_data_bank_get_correspondent_name ( gint bank_number )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return NULL;

    return bank -> correspondent_name;
}


/**
 * set the correspondent_name of the bank
 * the value is dupplicate in memory (so parameter can be freed after)
 *
 * \param bank_number the number of the bank
 * \param correspondent_name the correspondent_name of the bank
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_bank_set_correspondent_name ( gint bank_number,
						const gchar *correspondent_name )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return FALSE;

    /* we free the last name */
    if ( bank -> correspondent_name )
	free (bank -> correspondent_name);

    /* and copy the new one */
    bank -> correspondent_name = my_strdup (correspondent_name);

    return TRUE;
}


/**
 * return the correspondent_tel of the bank
 *
 * \param bank_number the number of the bank
 *
 * \return the correspondent_tel of the bank or NULL if problem
 * */
gchar *gsb_data_bank_get_correspondent_tel ( gint bank_number )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return NULL;

    return bank -> correspondent_tel;
}


/**
 * set the correspondent_tel of the bank
 * the value is dupplicate in memory (so parameter can be freed after)
 *
 * \param bank_number the number of the bank
 * \param correspondent_tel the correspondent_tel of the bank
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_bank_set_correspondent_tel ( gint bank_number,
					       const gchar *correspondent_tel )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return FALSE;

    /* we free the last name */
    if ( bank -> correspondent_tel )
	free (bank -> correspondent_tel);

    /* and copy the new one */
    bank -> correspondent_tel = my_strdup (correspondent_tel);

    return TRUE;
}


/**
 * return the correspondent_mail of the bank
 *
 * \param bank_number the number of the bank
 *
 * \return the correspondent_mail of the bank or NULL if problem
 * */
gchar *gsb_data_bank_get_correspondent_mail ( gint bank_number )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return NULL;

    return bank -> correspondent_mail;
}


/**
 * set the correspondent_mail of the bank
 * the value is dupplicate in memory (so parameter can be freed after)
 *
 * \param bank_number the number of the bank
 * \param correspondent_mail the correspondent_mail of the bank
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_bank_set_correspondent_mail ( gint bank_number,
						const gchar *correspondent_mail )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return FALSE;

    /* we free the last name */
    if ( bank -> correspondent_mail )
	free (bank -> correspondent_mail);

    /* and copy the new one */
    bank -> correspondent_mail = my_strdup (correspondent_mail);

    return TRUE;
}


/**
 * return the correspondent_fax of the bank
 *
 * \param bank_number the number of the bank
 *
 * \return the correspondent_fax of the bank or NULL if problem
 * */
gchar *gsb_data_bank_get_correspondent_fax ( gint bank_number )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return NULL;

    return bank -> correspondent_fax;
}


/**
 * set the correspondent_fax of the bank
 * the value is dupplicate in memory (so parameter can be freed after)
 *
 * \param bank_number the number of the bank
 * \param correspondent_fax the correspondent_fax of the bank
 *
 * \return TRUE if ok or FALSE if problem
 * */
gboolean gsb_data_bank_set_correspondent_fax ( gint bank_number,
					       const gchar *correspondent_fax )
{
    struct_bank *bank;

    bank = gsb_data_bank_get_structure ( bank_number );

    if (!bank)
	return FALSE;

    /* we free the last name */
    if ( bank -> correspondent_fax )
	free (bank -> correspondent_fax);

    /* and copy the new one */
    bank -> correspondent_fax = my_strdup (correspondent_fax);

    return TRUE;
}



/**
 * return the number of the bank wich has the name in param
 * create it if necessary
 *
 * \param name the name of the bank
 *
 * \return the number of the bank or 0 if doesn't exist
 * */
gint gsb_data_bank_get_number_by_name ( const gchar *name )
{
    GSList *list_tmp;

    if (!name)
	return FALSE;

    list_tmp = bank_list;

    while (list_tmp)
    {
	struct_bank *bank;

	bank = list_tmp -> data;

	if (!strcmp (bank -> bank_name,
		     name ))
	    return (bank -> bank_number);
	list_tmp = list_tmp -> next;
    }

    return FALSE;
}

