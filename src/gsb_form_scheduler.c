/* ************************************************************************** */
/*                                                                            */
/*                                gsb_form_scheduler                          */
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
 * \file gsb_form_scheduler.c
 * work with the form of the scheduler
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_form_scheduler.h"
#include "erreur.h"
#include "comptes_traitements.h"
#include "gsb_calendar_entry.h"
#include "gsb_combo_box.h"
#include "gsb_data_scheduled.h"
#include "utils_dates.h"
#include "gsb_form.h"
#include "gsb_form_widget.h"
#include "gtk_combofix.h"
#include "utils_str.h"
#include "gsb_form_scheduler.h"
#include "gsb_data_form.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_form_scheduler_button_press_event ( GtkWidget *entry,
						 GdkEventButton *ev,
						 gint *ptr_origin );
static gboolean gsb_form_scheduler_change_account ( GtkWidget *button,
					     gpointer null );
static gboolean gsb_form_scheduler_entry_lose_focus ( GtkWidget *entry,
					       GdkEventFocus *ev,
					       gint *ptr_origin );
static void gsb_form_scheduler_free_content_list ( GSList *content_list );
static gboolean gsb_form_scheduler_frequency_button_changed ( GtkWidget *combo_box,
						       gpointer null );
static gint gsb_form_scheduler_get_auto ( void );
static GSList *gsb_form_scheduler_get_content_list ( void );
static gint gsb_form_scheduler_get_frequency ( void );
static gint gsb_form_scheduler_get_frequency_user ( void );
static gint gsb_form_scheduler_get_frequency_user_button ( void );
static GDate *gsb_form_scheduler_get_limit_date ( void );
static gboolean gsb_form_scheduler_set_account ( gint account_number );
static gboolean gsb_form_scheduler_set_auto ( gboolean automatic );
static void gsb_form_scheduler_set_content_list ( GSList *content_list );
static gboolean gsb_form_scheduler_set_frequency ( gint frequency );
static gboolean gsb_form_scheduler_set_frequency_user ( gint user_freq );
static gboolean gsb_form_scheduler_set_frequency_user_button ( gboolean automatic );
static gboolean gsb_form_scheduler_set_limit_date ( GDate *date );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *formulaire;
extern GtkTreeSelection * selection;
extern GtkTooltips *tooltips_general_grisbi;
/*END_EXTERN*/

/**
 * \struct
 * Associate an element number for the scheduler part of the form
 * with the pointer to its widget */
typedef struct
{
    gint element_number;
    GtkWidget *element_widget;
} scheduled_element;

/**
 * \struct
 * associate an element number and a string value
 * used to keed the content of the form when changing the account
 * because the form is redrawed */
typedef struct {
    gint element_number;
    gchar *element_string;
    /* if we decide to save too the values of buttons,
     * can set here a gint element_int */
} content_element;

/** contains the list of the scheduled elements, ie list of link
 * between an element number and the pointer of its widget
 * for now, this list if filled at the opening of grisbi and never erased */
static GSList *scheduled_element_list = NULL;

/**
 * used when change the account with the button, have only the new account,
 * so can use that variable wich will be set to the new account at the end
 * of the callback when change the button (gsb_form_scheduler_change_account)
 * */
static gint last_account_number = 0;



/**
 * create the scheduled part : that widgets are created at the begining
 * and normally never destroyed, they are showed only for
 * scheduled transactions
 *
 * \param table a GtkTable with the dimension SCHEDULED_HEIGHT*SCHEDULED_WIDTH to be filled
 *
 * \return FALSE
 * */
gboolean gsb_form_scheduler_create ( GtkWidget *table )
{
    gint row, column;
    scheduled_element *element;

    if (!table)
	return FALSE;

    /* just in case... be sure that not created */
    if (scheduled_element_list)
	return FALSE;

    /* check the dimensions, 
     * if problem give a warning message but continue the program with changing the values */
    g_object_get ( G_OBJECT (table),
		   "n-columns", &column,
		   "n-rows", &row,
		   NULL );
    if ( column != SCHEDULED_WIDTH
	 ||
	 row != SCHEDULED_HEIGHT )
    {
	warning_debug ( _("gsb_form_scheduler_create is called with a bad table,\nthe number of rows or columns is not good.\nThe function will resize the table to the correct values but should check that warning."));
	gtk_table_resize ( GTK_TABLE (table),
			   SCHEDULED_HEIGHT, 
			   SCHEDULED_WIDTH );
    }

    /* ok, now fill the form 
     * we play with height and width, but for now it's fix : 6 columns and 1 line */
    for ( row=0 ; row < SCHEDULED_HEIGHT ; row++ )
	for ( column=0 ; column < SCHEDULED_WIDTH ; column++ )
	{
	    gint element_number;
	    GtkWidget *widget = NULL;
	    gchar *text_auto [] = { _("Manual"), _("Automatic"), NULL };
	    gchar *text_frequency [] = { _("Once"), _("Weekly"), _("Monthly"), _("two months"),  _("trimester"), _("Yearly"), _("Custom"), NULL };
	    gchar *text_frequency_user [] = { _("Days"), _("Weeks"), _("Months"), _("Years"), NULL };

	    element_number = row*SCHEDULED_WIDTH + column;

	    switch ( element_number )
	    {
		case SCHEDULED_FORM_ACCOUNT:
		    widget = gtk_combo_box_new ();
		    gsb_account_create_name_tree_model ( widget,
							 NULL,
							 FALSE );
		    gtk_combo_box_set_active ( GTK_COMBO_BOX (widget),
					       0 );
		    g_signal_connect ( G_OBJECT (widget),
				       "changed",
				       G_CALLBACK (gsb_form_scheduler_change_account),
				       NULL );
		    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
					   widget,
					   _("Choose the account"),
					   _("Choose the account") );
		    break;

		case SCHEDULED_FORM_AUTO:
		    widget = gsb_combo_box_new_with_index ( text_auto,
							    NULL );
		    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
					   widget,
					   _("Automatic/manual scheduled transaction"),
					   _("Automatic/manual scheduled transaction") );
		    break;

		case SCHEDULED_FORM_FREQUENCY_BUTTON:
		    widget = gsb_combo_box_new_with_index ( text_frequency,
							    G_CALLBACK (gsb_form_scheduler_frequency_button_changed));
		    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
					   widget,
					   _("Frequency"),
					   _("Frequency"));
		    break;

		case SCHEDULED_FORM_LIMIT_DATE:
		    widget = gsb_calendar_entry_new ();
		    g_signal_connect ( G_OBJECT (widget),
				       "button-press-event",
				       G_CALLBACK (gsb_form_scheduler_button_press_event),
				       GINT_TO_POINTER (element_number));
		    g_signal_connect ( G_OBJECT (widget),
				       "focus-in-event",
				       G_CALLBACK (gsb_form_entry_get_focus),
				       GINT_TO_POINTER (element_number));
		    g_signal_connect_after ( GTK_OBJECT (widget),
					     "focus-out-event",
					     G_CALLBACK (gsb_form_scheduler_entry_lose_focus),
					     GINT_TO_POINTER (element_number));
		    break;

		case SCHEDULED_FORM_FREQUENCY_USER_ENTRY:
		    widget = gtk_entry_new ();
		    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
					   widget,
					   _("Custom frequency"),
					   _("Custom frequency") );

		    break;

		case SCHEDULED_FORM_FREQUENCY_USER_BUTTON:
		    widget = gsb_combo_box_new_with_index ( text_frequency_user,
							    NULL );
		    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
					   widget,
					   _("Custom frequency"),
					   _("Custom frequency") );
		    break;
	    }

	    if (!widget)
		continue;

	    /* save the element */
	    element = g_malloc (sizeof (scheduled_element));
	    element -> element_number = element_number;
	    element -> element_widget = widget;
	    scheduled_element_list = g_slist_append ( scheduled_element_list,
						      element );

	    /* set the key signal */
	    g_signal_connect ( GTK_OBJECT (widget),
			       "key-press-event",
			       G_CALLBACK (gsb_form_key_press_event),
			       GINT_TO_POINTER (SCHEDULED_FORM_ACCOUNT));

	    /* set in the form */
	    gtk_table_attach ( GTK_TABLE (table),
			       widget,
			       column, column+1,
			       row, row+1,
			       GTK_EXPAND | GTK_FILL,
			       GTK_EXPAND | GTK_FILL,
			       0, 0);
	}
    return FALSE;
}


/**
 * destroy all the elements in scheduled_element_list
 * and free the list
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_form_scheduler_free_list ( void )
{
    GSList *list_tmp;

    devel_debug ("gsb_form_scheduler_free_list");

    if (!scheduled_element_list)
	return FALSE;

    list_tmp = scheduled_element_list;

    while (list_tmp)
    {
	scheduled_element *element;

	element = list_tmp -> data;
	if (element -> element_widget
	    &&
	    GTK_IS_WIDGET (element -> element_widget))
	    gtk_widget_destroy (element -> element_widget);
	g_free (element);

	list_tmp = list_tmp -> next;
    }
    g_slist_free (scheduled_element_list);
    scheduled_element_list =NULL;

    return FALSE;
}


/**
 * callback called when changing the account from the form's button
 * re-fill the form but keep the values
 *
 * \param button
 * \param null
 *
 * \return FALSE
 * */
gboolean gsb_form_scheduler_change_account ( GtkWidget *button,
					     gpointer null )
{
    gint save_transaction;
    gint save_execute;
    GSList *content_list;
    gboolean is_breakdown;
    GtkWidget *category_entry;
    gint new_account_number;

    devel_debug ("gsb_form_scheduler_change_account");

    new_account_number = gsb_form_get_account_number ();

    /* need to check first if breakdown (see later) */
    category_entry = gsb_form_widget_get_widget (TRANSACTION_FORM_CATEGORY);
    if (category_entry
	&&
	gsb_form_widget_check_empty (GTK_COMBOFIX (category_entry) -> entry)
	&&
	!strcmp (gtk_combofix_get_text (GTK_COMBOFIX (category_entry)), _("Breakdown of transaction")))
	/* ok it's a breakdown */
	is_breakdown = TRUE;
    else
	is_breakdown = FALSE;
	
    /* problem here : when change account, the form can be changed, with new or less widgets
     * so we fill again de form
     * but il the user fill the form and want to change after the account, it's annoying because
     * filling again the form will lose all the data
     * so first save the data and after filling the form, set back the data
     * may still a problem : if for example we set a note, go to an account without notes, and
     * go back to an account with a note, the first content of the note will be lost but it should
     * be very rare to do that and i think very difficult to code something to keep that... */
    save_transaction = GPOINTER_TO_INT (g_object_get_data ( G_OBJECT ( formulaire ),
							    "transaction_number_in_form" ));
    save_execute = GPOINTER_TO_INT (g_object_get_data ( G_OBJECT (formulaire),
							"execute_scheduled"));
    content_list = gsb_form_scheduler_get_content_list ();

    gsb_form_fill_from_account (-1);

    /* a problem now, fill_from_account will clean the form,
     * and make unsensitive some part of the form (method of payment...)
     * and make sensitive some other part wich could be unsensitive (for breakdown for example)
     * so we call gsb_form_set_sensitive, but 2 args, breakdown or child.
     * cannot be a child because child cannot access to the account button, so just to check
     * if it's a breakdown (done before)
     */
    gsb_form_change_sensitive_buttons (TRUE);
    gsb_form_set_sensitive (is_breakdown, FALSE);

    gsb_form_scheduler_set_content_list (content_list);
    gsb_form_scheduler_free_content_list (content_list);

    g_object_set_data ( G_OBJECT ( formulaire ),
			"transaction_number_in_form",
			GINT_TO_POINTER (save_transaction));
    g_object_set_data ( G_OBJECT ( formulaire ),
			"execute_scheduled",
			GINT_TO_POINTER (save_execute));

    last_account_number = new_account_number;
    return FALSE;
}


/**
 * get the content in the form, and set it in a list composed
 * of each element with their content (list of struct content_element
 *
 * \param 
 *
 * \return a newly allocated list
 * */
GSList *gsb_form_scheduler_get_content_list ( void )
{
    GSList *content_list = NULL;
    GSList *tmp_list;

    tmp_list = gsb_form_widget_get_list ();

    while (tmp_list)
    {
	struct_element *element;

	element = tmp_list -> data;

	if (GTK_WIDGET_VISIBLE (element -> element_widget))
	{
	    switch (element -> element_number)
	    {
		/* first, check the entries */
		case TRANSACTION_FORM_DATE:
		case TRANSACTION_FORM_VALUE_DATE:
		case TRANSACTION_FORM_DEBIT:
		case TRANSACTION_FORM_CREDIT:
		case TRANSACTION_FORM_NOTES:
		case TRANSACTION_FORM_BANK:
		case TRANSACTION_FORM_VOUCHER:
		    if (!gsb_form_widget_check_empty (element -> element_widget))
		    {
			content_element *element_save;

			element_save = g_malloc0 (sizeof (content_element));
			element_save -> element_number = element -> element_number;
			element_save -> element_string = my_strdup (gtk_entry_get_text (GTK_ENTRY (element -> element_widget)));
			content_list = g_slist_append ( content_list,
							element_save );
		    }
		    break;

		    /* next, check the combobox */
		case TRANSACTION_FORM_EXERCICE:
		case TRANSACTION_FORM_TYPE:
		case TRANSACTION_FORM_DEVISE:
		case TRANSACTION_FORM_CONTRA:
		    /* for now, i do nothig here because complex, change with the buttons, and :
		     * exercice usually is set automaticly with the date
		     * the type and contra should change with the change of bank
		     * the devise will change with the bank
		     * */
		    break;

		    /* check the combofix */
		case TRANSACTION_FORM_PARTY:
		case TRANSACTION_FORM_CATEGORY:
		case TRANSACTION_FORM_BUDGET:
		    if (!gsb_form_widget_check_empty (GTK_COMBOFIX (element -> element_widget) -> entry))
		    {
			content_element *element_save;

			element_save = g_malloc0 (sizeof (content_element));
			element_save -> element_number = element -> element_number;
			element_save -> element_string = my_strdup (gtk_combofix_get_text (GTK_COMBOFIX (element -> element_widget)));
			content_list = g_slist_append ( content_list,
							element );
		    }
		    break;
	    }
	}
	tmp_list = tmp_list -> next;
    }
    return content_list;
}


/**
 * set the content in the form according to the content of the list
 * of struct content_element given in param
 *
 * \param content_list the list of struct content_element
 *
 * \return
 * */
void gsb_form_scheduler_set_content_list ( GSList *content_list )
{
    GSList *list_tmp;

    list_tmp = content_list;

    /* see each saved element */
    while (list_tmp)
    {
	content_element *element;
	GSList *form_list;

	element = list_tmp -> data;

	/* normally, cannot happen */
	if (!element)
	    continue;

	/* we look for that element in the form */
	form_list = gsb_form_widget_get_list ();

	while (form_list)
	{
	    struct_element *form_element;

	    form_element = form_list -> data;

	    if (!GTK_WIDGET_VISIBLE (form_element -> element_widget))
		continue;

	    switch (form_element -> element_number)
	    {
		/* first, the entries */
		case TRANSACTION_FORM_DATE:
		case TRANSACTION_FORM_VALUE_DATE:
		case TRANSACTION_FORM_DEBIT:
		case TRANSACTION_FORM_CREDIT:
		case TRANSACTION_FORM_NOTES:
		case TRANSACTION_FORM_BANK:
		case TRANSACTION_FORM_VOUCHER:
		    gsb_form_entry_get_focus (form_element -> element_widget);
		    gtk_entry_set_text ( GTK_ENTRY (form_element -> element_widget),
					 element -> element_string );
		    break;

		    /* next, the combobox */
		case TRANSACTION_FORM_EXERCICE:
		case TRANSACTION_FORM_TYPE:
		case TRANSACTION_FORM_DEVISE:
		case TRANSACTION_FORM_CONTRA:
		    /* for now, i do nothig here because complex, change with the buttons, and :
		     * exercice usually is set automaticly with the date
		     * the type and contra should change with the change of bank
		     * the devise will change with the bank
		     * */
		    break;

		    /* check the combofix */
		case TRANSACTION_FORM_PARTY:
		case TRANSACTION_FORM_CATEGORY:
		case TRANSACTION_FORM_BUDGET:
		    gsb_form_entry_get_focus (form_element -> element_widget);
		    gtk_combofix_set_text ( GTK_COMBOFIX ( form_element -> element_widget ),
					    element -> element_string );
		    break;
	    }
	    form_list = form_list -> next;
	}
	list_tmp = list_tmp -> next;
    }
}


/**
 * free a content list created by gsb_form_scheduler_get_content_list
 *
 * \param content_list
 *
 * \return
 * */
void gsb_form_scheduler_free_content_list ( GSList *content_list )
{
    GSList *list_tmp;

    list_tmp = content_list;

    while (list_tmp)
    {
	content_element *element;

	element = list_tmp -> data;

	/* normally, cannot happen */
	if (!element)
	    continue;
	
	if (element -> element_string)
	    g_free (element -> element_string);
	g_free (element);

	list_tmp = list_tmp -> next;
    }
    g_slist_free (content_list);
}

/**
 * clean the scheduled form part
 * and set the default values
 *
 * \param account number
 *
 * \return FALSE
 * */
gboolean gsb_form_scheduler_clean ( void )
{
    gint column;
    GtkWidget *widget;

    devel_debug ("gsb_form_scheduler_clean");

    /* set to NULL the execute flag */
    g_object_set_data ( G_OBJECT (formulaire),
			"execute_scheduled", NULL );

    /* first we show it, becaus hidden when execute a scheduled transaction */
    if (gsb_form_get_origin () == ORIGIN_VALUE_SCHEDULED)
	gtk_widget_show (gsb_form_get_scheduler_part ());

    /* clean the scheduled widget */
    for ( column = 0 ; column < SCHEDULED_FORM_MAX_WIDGETS ; column++ )
    {
	widget = gsb_form_scheduler_get_element_widget (column);

	/* some widgets can be set unsensitive because of the children of breakdowns,
	 * so resensitive all to be sure */
	if (widget)
	{
	    switch (column)
	    {
		case SCHEDULED_FORM_ACCOUNT:
		case SCHEDULED_FORM_AUTO:
		case SCHEDULED_FORM_FREQUENCY_BUTTON:
		case SCHEDULED_FORM_FREQUENCY_USER_BUTTON:
		    gtk_combo_box_set_active ( GTK_COMBO_BOX (widget),
					       0 );
		    gtk_widget_set_sensitive ( widget,
					       FALSE );
		    break;

		case SCHEDULED_FORM_LIMIT_DATE:
		    gsb_form_widget_set_empty ( widget,
						TRUE );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 _("Limit date") );
		    gtk_widget_set_sensitive ( widget,
					       TRUE );
		    break;

		case SCHEDULED_FORM_FREQUENCY_USER_ENTRY:
		    gsb_form_widget_set_empty ( widget,
						TRUE );
		    gtk_entry_set_text ( GTK_ENTRY ( widget ),
					 _("User frequency") );
		    gtk_widget_set_sensitive ( widget,
					       TRUE );
		    break;
	    }
	}
    }
    return FALSE;
}

/**
 * sensitive the buttons of the scheduler part of the form
 *
 * \param sensitive TRUE or FALSE
 *
 * \return FALSE
 * */
gboolean gsb_form_scheduler_sensitive_buttons ( gboolean sensitive )
{
    gint column;

    devel_debug (g_strdup_printf ( "gsb_form_scheduler_sensitive_buttons %d", sensitive));

    /* clean the scheduled widget */
    for ( column = 0 ; column < SCHEDULED_FORM_MAX_WIDGETS ; column++ )
    {
	GtkWidget *widget;

	widget = gsb_form_scheduler_get_element_widget (column);

	if (widget)
	{
	    switch (column)
	    {
		case SCHEDULED_FORM_ACCOUNT:
		case SCHEDULED_FORM_AUTO:
		case SCHEDULED_FORM_FREQUENCY_BUTTON:
		case SCHEDULED_FORM_FREQUENCY_USER_BUTTON:
		    gtk_widget_set_sensitive ( widget,
					       sensitive );
		    break;
	    }
	}
    }
    return FALSE;
}



/**
 * set the scheduler part of the form from the scheduled transaction given in param
 *
 * \param scheduled_number
 *
 * \return FALSE
 * */
gboolean gsb_form_scheduler_set ( gint scheduled_number )
{
    devel_debug ( g_strdup_printf ( "gsb_form_scheduler_set %d", scheduled_number));

    gsb_form_scheduler_sensitive_buttons (TRUE);

    /* if we are on a white breakdown line, set all as the mother */
    if (scheduled_number < -1)
	scheduled_number = gsb_data_scheduled_get_mother_scheduled_number (scheduled_number);

    gsb_form_scheduler_set_account ( gsb_data_scheduled_get_account_number (scheduled_number));
    gsb_form_scheduler_set_auto ( gsb_data_scheduled_get_automatic_scheduled (scheduled_number));
    gsb_form_scheduler_set_frequency ( gsb_data_scheduled_get_frequency (scheduled_number));
    gsb_form_scheduler_set_limit_date ( gsb_data_scheduled_get_limit_date (scheduled_number));
    gsb_form_scheduler_set_frequency_user ( gsb_data_scheduled_get_user_entry (scheduled_number));
    gsb_form_scheduler_set_frequency_user_button ( gsb_data_scheduled_get_user_interval (scheduled_number));
    return FALSE;
}


/**
 * fill the scheduled transaction given in param with the content of
 * the scheduled part of the form (ie frequency...)
 *
 * \param scheduled_number
 *
 * \return FALSE
 * */
gboolean gsb_form_scheduler_get_scheduler_part ( gint scheduled_number )
{
    if (!scheduled_number)
	return FALSE;

    /* needn't to fill the account number because set while creating the scheduled transaction */

    gsb_data_scheduled_set_automatic_scheduled ( scheduled_number,
						 gsb_form_scheduler_get_auto ());
    gsb_data_scheduled_set_frequency ( scheduled_number,
				       gsb_form_scheduler_get_frequency ());
    gsb_data_scheduled_set_limit_date ( scheduled_number,
					gsb_form_scheduler_get_limit_date ());
    gsb_data_scheduled_set_user_interval ( scheduled_number,
					   gsb_form_scheduler_get_frequency_user_button ());
    gsb_data_scheduled_set_user_entry ( scheduled_number,
					gsb_form_scheduler_get_frequency_user ());
    return FALSE;
}


/**
 * return the widget of the element_number given in param,
 * for the scheduler part of the form
 *
 * \param element_number
 *
 * \return a GtkWidget * or NULL
 * */
GtkWidget *gsb_form_scheduler_get_element_widget ( gint element_number )
{
    GSList *list_tmp;

    list_tmp = scheduled_element_list;

    while (list_tmp)
    {
	scheduled_element *element;

	element = list_tmp -> data;
	if (element -> element_number == element_number)
	    return element -> element_widget;

	list_tmp = list_tmp -> next;
    }
    return NULL;
}


/**
 * called when we press the button in an entry field in
 * the form for the scheduled part
 *
 * \param entry wich receive the signal
 * \param ev can be NULL
 * \param ptr_origin a pointer to int on the element_number
 *
 * \return FALSE
 * */
gboolean gsb_form_scheduler_button_press_event ( GtkWidget *entry,
						 GdkEventButton *ev,
						 gint *ptr_origin )
{
    gint element_number;
    GtkWidget *date_entry;

    element_number = GPOINTER_TO_INT (ptr_origin);

    /* set the form sensitive */
    gsb_form_change_sensitive_buttons (TRUE);

    /* set the current date into the date entry except if there is already something into the value date */
    date_entry = gsb_form_widget_get_widget (TRANSACTION_FORM_DATE);
    if ( gsb_form_widget_check_empty (date_entry))
    {
	gtk_entry_set_text ( GTK_ENTRY (date_entry),
			     gsb_date_today ());
	gsb_form_widget_set_empty ( date_entry,
				    FALSE );
    }
    return FALSE;
}


/**
 * called when an entry lose the focus in the scheduled part
 *
 * \param entry
 * \param ev
 * \param ptr_origin a pointer gint wich is the number of the element
 *
 * \return FALSE
 * */
gboolean gsb_form_scheduler_entry_lose_focus ( GtkWidget *entry,
					       GdkEventFocus *ev,
					       gint *ptr_origin )
{
    gchar *string;
    gint element_number;
    gint account_number;

    /* remove the selection */

    gtk_editable_select_region ( GTK_EDITABLE ( entry ),
				 0,
				 0 );
    element_number = GPOINTER_TO_INT (ptr_origin);
    account_number = gsb_form_get_account_number ();

    /* string will be filled only if the field is empty */
    string = NULL;

    switch ( element_number )
    {
	case  SCHEDULED_FORM_LIMIT_DATE:
	    if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY (entry))))
		string = _("Limit date");
	    break;

	case  SCHEDULED_FORM_FREQUENCY_USER_ENTRY:
	    if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY (entry))))
		string = _("Own frequency");
	    break;

	default :
	    break;
    }

    /* if string is not NULL, the entry is empty so set the empty field to TRUE */

    if ( string )
    {
	gtk_entry_set_text ( GTK_ENTRY ( entry ), string );
	gsb_form_widget_set_empty ( entry,
				    TRUE );
    }
    return FALSE;
}



/**
 * called when the frequency button is changed
 * show/hide the necessary widget according to its state
 *
 * \param combo_box
 *
 * \return FALSE 
 * */
gboolean gsb_form_scheduler_frequency_button_changed ( GtkWidget *combo_box,
						       gpointer null )
{
    gchar *selected_item;

    selected_item = gtk_combo_box_get_active_text ( GTK_COMBO_BOX (combo_box));
    
    if ( !strcmp ( selected_item,
		   _("Once")))
    {
	gtk_widget_hide (gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_LIMIT_DATE));
	gtk_widget_hide (gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_FREQUENCY_USER_ENTRY));
	gtk_widget_hide (gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_FREQUENCY_USER_BUTTON));
    }
    else
    {
	gtk_widget_show (gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_LIMIT_DATE));
	if ( !strcmp ( selected_item,
		       _("Custom")))
	{
	    gtk_widget_show (gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_FREQUENCY_USER_ENTRY));
	    gtk_widget_show (gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_FREQUENCY_USER_BUTTON));
	}
	else
	{
	    gtk_widget_hide (gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_FREQUENCY_USER_ENTRY));
	    gtk_widget_hide (gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_FREQUENCY_USER_BUTTON));
	}
    }
    g_free (selected_item);
    return FALSE;
}


/**
 * get the account number from the scheduled button and return it
 *
 * \param
 *
 * \return the account number or -2 if problem
 * */
gint gsb_form_scheduler_get_account ( void )
{
    gint account_number;
    GtkTreeIter iter;
    GtkWidget *button;

    button = gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_ACCOUNT);
    /* if no account button, go away... */
    if (!button)
	return -2;

    if ( !gtk_combo_box_get_active_iter ( GTK_COMBO_BOX (button),
					  &iter ))
	return -2;

    gtk_tree_model_get ( GTK_TREE_MODEL (gtk_combo_box_get_model (GTK_COMBO_BOX (button))),
			 &iter,
			 1, &account_number,
			 -1 );
    return account_number;
}

/**
 * set the account in the account button
 * this will automatickly change the content of the form
 *
 * \param account_number
 *
 * \return FALSE if not done, TRUE if ok
 * */
gboolean gsb_form_scheduler_set_account ( gint account_number )
{
    GtkTreeIter iter;
    GtkWidget *button;
    GtkTreeModel *model;

    button = gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_ACCOUNT);
    /* if no account button, go away... */
    if (!button)
	return FALSE;

    model = gtk_combo_box_get_model ( GTK_COMBO_BOX (button));

    if ( gtk_tree_model_get_iter_first (model, &iter))
    {
	do
	{
	    gint number;

	    gtk_tree_model_get ( model, &iter,
				 1, &number,
				 -1 );
	    if (number == account_number)
	    {
		gtk_combo_box_set_active_iter ( GTK_COMBO_BOX (button),
						&iter );
		return TRUE;
	    }
	}
	while (gtk_tree_model_iter_next (model, &iter));
    }
    return FALSE;
}


/**
 * get the automatic/manual mode from the form's button
 *
 * \param
 *
 * \return TRUE if automatic, FALSE if manual, -1 if nothing selected (normaly should not append)
 * */
gint gsb_form_scheduler_get_auto ( void )
{
    GtkWidget *button;

    button = gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_AUTO);
    /* if no automatic button, go away... */
    if (!button)
	return FALSE;

    return gsb_combo_box_get_index (button);
}


/**
 * set the automatic/manual mode on the form's button
 *
 * \param automatic TRUE to set automatic, FALSE to set manual
 *
 * \return TRUE if ok, FALSE else
 * */
gboolean gsb_form_scheduler_set_auto ( gboolean automatic )
{
    GtkWidget *button;

    button = gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_AUTO);
    /* if no automatic button, go away... */
    if (!button)
	return FALSE;

    return gsb_combo_box_set_index ( button,
				     automatic );
}


/**
 * get the frequency from the form's button
 *
 * \param
 *
 * \return 0 : Once, 1 : Weekly, 2 : Monthly, 3: Yearly, 4: Custom, -1 if nothing selected (normaly should not append)
 * */
gint gsb_form_scheduler_get_frequency ( void )
{
    GtkWidget *button;

    button = gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_FREQUENCY_BUTTON);
    /* if no automatic button, go away... */
    if (!button)
	return FALSE;

    return gsb_combo_box_get_index (button);
}


/**
 * set the frequency on the form's button
 *
 * \param frequency 0 : Once, 1 : Weekly, 2 : Monthly, 3: Yearly, 4: Custom
 *
 * \return TRUE if ok, FALSE else
 * */
gboolean gsb_form_scheduler_set_frequency ( gint frequency )
{
    GtkWidget *button;

    button = gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_FREQUENCY_BUTTON);
    /* if no automatic button, go away... */
    if (!button)
	return FALSE;

    return gsb_combo_box_set_index ( button,
				     frequency );
}


/**
 * get the limit date from the form's scheduled part
 *
 * \param
 *
 * \return a GDate or NULL if nothing into the entry or problem
 * */
GDate *gsb_form_scheduler_get_limit_date ( void )
{
    GtkWidget *entry;
    GDate *date;

    entry = gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_LIMIT_DATE);
    /* if no entry, go away... */
    if (!entry
	||
	gsb_form_widget_check_empty (entry)
	||
	!GTK_WIDGET_VISIBLE (entry))
	return NULL;

    date = gsb_calendar_entry_get_date (entry);
    return date;
}


/**
 * set the limit date in the form's scheduled part
 *
 * \param date a GDate or NULL if we want to free the entry
 *
 * \return TRUE if ok, FALSE else
 * */
gboolean gsb_form_scheduler_set_limit_date ( GDate *date )
{
    GtkWidget *entry;

    entry = gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_LIMIT_DATE);
    /* if no entry, go away... */
    if (!entry
	||
	!GTK_WIDGET_VISIBLE (entry))
	return FALSE;

    if (gsb_calendar_entry_set_date ( entry, date ))
	gsb_form_widget_set_empty (entry, FALSE);

    return TRUE;
}


/**
 * get the user frequency from the form's scheduled part
 *
 * \param
 *
 * \return a gint contains the user frequency
 * */
gint gsb_form_scheduler_get_frequency_user ( void )
{
    GtkWidget *entry;

    entry = gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_FREQUENCY_USER_ENTRY);
    /* if no entry, go away... */
    if (!entry
	||
	gsb_form_widget_check_empty (entry))
	return 0;

    return utils_str_atoi (gtk_entry_get_text (GTK_ENTRY (entry)));
}


/**
 * set the user frequency in the form's scheduled part
 *
 * \param user_freq the user frequency number
 *
 * \return TRUE if ok, FALSE else
 * */
gboolean gsb_form_scheduler_set_frequency_user ( gint user_freq )
{
    GtkWidget *entry;
    gchar *string;

    entry = gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_FREQUENCY_USER_ENTRY);
    /* if no entry, go away... */
    if (!entry
	||
	!GTK_WIDGET_VISIBLE (entry))
	return FALSE;

    string = utils_str_itoa (user_freq);
    if (!string)
	return FALSE;

    gsb_form_widget_set_empty (entry, FALSE);
    gtk_entry_set_text ( GTK_ENTRY (entry), string);

    return TRUE;
}


/**
 * get the user frequency from the form's button
 *
 * \param
 *
 * \return 0 : Days, 1 : Months, 2 : Years, -1 if nothing selected (normaly should not append)
 * */
gint gsb_form_scheduler_get_frequency_user_button ( void )
{
    GtkWidget *button;

    button = gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_FREQUENCY_USER_BUTTON);
    /* if no automatic button, go away... */
    if (!button)
	return FALSE;

    return gsb_combo_box_get_index (button);
}


/**
 * set the user frequency on the form's button
 *
 * \param frequency 0 : Days, 1 : Months, 2 : Years
 *
 * \return TRUE if ok, FALSE else
 * */
gboolean gsb_form_scheduler_set_frequency_user_button ( gboolean automatic )
{
    GtkWidget *button;

    button = gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_FREQUENCY_USER_BUTTON);
    /* if no automatic button, go away... */
    if (!button
	||
	!GTK_WIDGET_VISIBLE (button))
	return FALSE;

    return gsb_combo_box_set_index ( button,
				     automatic );
}

