/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2007 Benjamin Drieu (bdrieu@april.org)	      */
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
 * \file gsb_form_widget.c
 * convenient functions to work with the elements in the form
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_form_widget.h"
#include "gsb_form_transaction.h"
#include "erreur.h"
#include "gsb_calendar_entry.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_form.h"
#include "gsb_data_payee.h"
#include "gsb_form.h"
#include "gsb_fyear.h"
#include "gsb_payment_method.h"
#include "gtk_combofix.h"
#include "structures.h"
#include "traitement_variables.h"
#include "gsb_data_payment.h"
#include "include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_form_widget_can_focus ( gint element_number );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkTooltips *tooltips_general_grisbi;
/*END_EXTERN*/

/** contains a list of struct_element according to the current form */
static GSList *form_list_widgets = NULL;

/** the 2 styles, grey and normal for the entries of the form */
static GtkStyle *style_entree_formulaire[2];
#define ENCLAIR 0
#define ENGRIS 1




/**
 * return the list wich contains the widgets of the form
 *
 * \param
 *
 * \return a GSList with the pointers to the widgets of the form
 * */
GSList *gsb_form_widget_get_list ( void )
{
    return form_list_widgets;
}


/**
 * free the content of the list of the widget's form
 * so free the content of the form
 *
 * \param 
 *
 * \return FALSE
 * */
gboolean gsb_form_widget_free_list ( void )
{
    GSList *tmp_list;

    devel_debug ("gsb_form_widget_free_list");

    if (!form_list_widgets)
	return FALSE;

    tmp_list = form_list_widgets;
    while (tmp_list)
    {
	struct_element *element;

	element = tmp_list -> data;

	/* just to make sure... */
	if (element)
	{
	    /* if there is something in the combofix we destroy, the popup will
	     * be showed because destroying the gtk_entry will erase it directly,
	     * so the simpliest way to avoid that is to erase now the entry, but with
	     * gtk_combofix_set_text [cedric] (didn't succeed with another thing...) */
	    if (GTK_IS_COMBOFIX (element -> element_widget))
		gtk_combofix_set_text ( GTK_COMBOFIX (element -> element_widget),
					"" );
	    gtk_widget_destroy (element -> element_widget);
	    g_free (element);
	}
	tmp_list = tmp_list -> next;
    }
    g_slist_free (form_list_widgets);
    form_list_widgets = NULL;

    return FALSE;
}


/**
 * init the colors for the differents states of the entries
 *
 * \param
 *
 * \return FALSE
 * */
gboolean gsb_form_widget_init_entry_colors ( void )
{
    GdkColor normal_color;
    GdkColor grey_color;

    normal_color.red = COULEUR_NOIRE_RED;
    normal_color.green = COULEUR_NOIRE_GREEN;
    normal_color.blue = COULEUR_NOIRE_BLUE;
    normal_color.pixel = 0;

    grey_color.red = COULEUR_GRISE_RED;
    grey_color.green = COULEUR_GRISE_GREEN;
    grey_color.blue = COULEUR_GRISE_BLUE;
    grey_color.pixel = 0;

    style_entree_formulaire[ENCLAIR] = gtk_style_new();
    style_entree_formulaire[ENCLAIR] -> text[GTK_STATE_NORMAL] = normal_color;

    style_entree_formulaire[ENGRIS] = gtk_style_new();
    style_entree_formulaire[ENGRIS] -> text[GTK_STATE_NORMAL] = grey_color;

    g_object_ref ( style_entree_formulaire[ENCLAIR] );
    g_object_ref ( style_entree_formulaire[ENGRIS] );

    return FALSE;
}


/**
 * create and return the widget according to its element number
 *
 * \param element_number the number corresponding to the widget wanted
 * \param account_number used for method of payment and currency
 *
 * \return the widget
 * */
GtkWidget *gsb_form_widget_create ( gint element_number,
				    gint account_number )
{
    GtkWidget *widget;

    if (!element_number)
	return NULL;

    widget = NULL;

    switch (element_number)
    {
	case TRANSACTION_FORM_DATE:
	case TRANSACTION_FORM_VALUE_DATE:
	    widget = gsb_calendar_entry_new ();
	    break;

	case TRANSACTION_FORM_DEBIT:
	case TRANSACTION_FORM_CREDIT:
	case TRANSACTION_FORM_NOTES:
	case TRANSACTION_FORM_CHEQUE:
	case TRANSACTION_FORM_VOUCHER:
	case TRANSACTION_FORM_BANK:
	    widget = gtk_entry_new();
	    break;

	case TRANSACTION_FORM_EXERCICE:
	    widget = gsb_fyear_make_combobox ();
	    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				   widget,
				   _("Choose the financial year"),
				   _("Choose the financial year") );
	    break;

	case TRANSACTION_FORM_PARTY:
	    widget = gtk_combofix_new_complex ( gsb_data_payee_get_name_and_report_list());
	    gtk_combofix_set_force_text ( GTK_COMBOFIX (widget),
					  etat.combofix_force_payee );
	    gtk_combofix_set_max_items ( GTK_COMBOFIX (widget),
					 etat.combofix_max_item );
	    gtk_combofix_set_case_sensitive ( GTK_COMBOFIX (widget),
					      etat.combofix_case_sensitive );
	    gtk_combofix_set_enter_function ( GTK_COMBOFIX (widget),
					      etat.combofix_enter_select_completion );
	    /* we never mix the payee because the only case of the complex combofix is
	     * for the report and there is no sense to mix report with the payee */
	    gtk_combofix_set_mixed_sort ( GTK_COMBOFIX (widget),
					  FALSE );
	    break;

	case TRANSACTION_FORM_CATEGORY:
	    widget = gtk_combofix_new_complex ( gsb_data_category_get_name_list (TRUE, TRUE, TRUE, TRUE));
	    gtk_combofix_set_force_text ( GTK_COMBOFIX (widget),
					  etat.combofix_force_category );
	    gtk_combofix_set_max_items ( GTK_COMBOFIX (widget),
					 etat.combofix_max_item );
	    gtk_combofix_set_case_sensitive ( GTK_COMBOFIX (widget),
					      etat.combofix_case_sensitive );
	    gtk_combofix_set_enter_function ( GTK_COMBOFIX (widget),
					      etat.combofix_enter_select_completion );
	    gtk_combofix_set_mixed_sort ( GTK_COMBOFIX (widget),
					  etat.combofix_mixed_sort );
	    break;

	case TRANSACTION_FORM_FREE:
	    break;

	case TRANSACTION_FORM_BUDGET:
	    widget = gtk_combofix_new_complex ( gsb_data_budget_get_name_list (TRUE, TRUE));
	    gtk_combofix_set_force_text ( GTK_COMBOFIX (widget),
					  etat.combofix_force_category );
	    gtk_combofix_set_max_items ( GTK_COMBOFIX (widget),
					 etat.combofix_max_item );
	    gtk_combofix_set_case_sensitive ( GTK_COMBOFIX (widget),
					      etat.combofix_case_sensitive );
	    gtk_combofix_set_enter_function ( GTK_COMBOFIX (widget),
					      etat.combofix_enter_select_completion );
	    gtk_combofix_set_mixed_sort ( GTK_COMBOFIX (widget),
					  etat.combofix_mixed_sort );
	    break;

	case TRANSACTION_FORM_TYPE:
	    widget = gtk_combo_box_new ();
	    g_signal_connect ( G_OBJECT (widget),
			       "changed",
			       G_CALLBACK (gsb_payment_method_changed_callback),
			       NULL );
	    gsb_payment_method_create_combo_list ( widget,
						   GSB_PAYMENT_DEBIT,
						   account_number, 0 );
	    gtk_combo_box_set_active ( GTK_COMBO_BOX (widget),
				       0 );
	    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				   widget,
				   _("Choose the method of payment"),
				   _("Choose the method of payment") );
	    break;

	case TRANSACTION_FORM_DEVISE:
	    widget = gsb_currency_make_combobox (FALSE);
	    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				   widget,
				   _("Choose currency"),
				   _("Choose currency") );
	    gsb_currency_set_combobox_history ( widget,
						gsb_data_account_get_currency (account_number));
	    break;

	case TRANSACTION_FORM_CHANGE:
	    widget = gtk_button_new_with_label ( _("Change") );
	    gtk_button_set_relief ( GTK_BUTTON ( widget ),
				    GTK_RELIEF_NONE );
	    g_signal_connect ( GTK_OBJECT (  widget ),
			       "clicked",
			       G_CALLBACK ( click_sur_bouton_voir_change ),
			       NULL );
	    break;

	case TRANSACTION_FORM_CONTRA:
	    /* no menu at begining, appened when choose the contra-account */
	    widget = gtk_combo_box_new ();
	    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ),
				   widget,
				   _("Contra-transaction method of payment"),
				   _("Contra-transaction method of payment") );
	    break;

	case TRANSACTION_FORM_OP_NB:
	case TRANSACTION_FORM_MODE:
	    widget = gtk_label_new ( "" );
	    break;
    }

    if ( widget )
    {
	/* first, append the widget to the list */
	struct_element *element;

	element = g_malloc (sizeof (struct_element));
	element -> element_number = element_number;
	element -> element_widget = widget;
	form_list_widgets = g_slist_append ( form_list_widgets, element );

	/* set the signals */
	if ( GTK_IS_ENTRY ( widget ))
	{
	    g_signal_connect ( GTK_OBJECT ( widget ),
			       "focus-in-event",
			       G_CALLBACK ( gsb_form_entry_get_focus ),
			       GINT_TO_POINTER ( element_number ));
	    g_signal_connect ( GTK_OBJECT ( widget ),
			       "focus-out-event",
			       G_CALLBACK ( gsb_form_entry_lose_focus ),
			       GINT_TO_POINTER ( element_number ));
	    g_signal_connect ( GTK_OBJECT ( widget ),
			       "button-press-event",
			       G_CALLBACK ( gsb_form_button_press_event ),
			       GINT_TO_POINTER ( element_number ));
	    g_signal_connect ( GTK_OBJECT ( widget ),
			       "key-press-event",
			       G_CALLBACK ( gsb_form_key_press_event ),
			       GINT_TO_POINTER ( element_number ));
	}
	else
	{
	    if ( GTK_IS_COMBOFIX ( widget ))
	    {
		g_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX ( widget ) -> entry ),
				   "focus-in-event",
				   G_CALLBACK ( gsb_form_entry_get_focus ),
				   GINT_TO_POINTER ( element_number ));
		g_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX (widget ) -> entry ),
				   "focus-out-event",
				   G_CALLBACK ( gsb_form_entry_lose_focus ),
				   GINT_TO_POINTER ( element_number ));
		g_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX (widget ) -> entry ),
				   "button-press-event",
				   G_CALLBACK ( gsb_form_button_press_event ),
				   GINT_TO_POINTER ( element_number ));
		g_signal_connect ( GTK_OBJECT ( GTK_COMBOFIX (widget ) -> entry ),
				   "key-press-event",
				   G_CALLBACK ( gsb_form_key_press_event ),
				   GINT_TO_POINTER ( element_number ));
	    }
	    else
		/* neither an entry, neither a combofix */
		g_signal_connect ( GTK_OBJECT ( widget ),
				   "key-press-event",
				   G_CALLBACK ( gsb_form_key_press_event ),
				   GINT_TO_POINTER ( element_number ));
	}
    }
    return widget;
}


/**
 * return the pointer to the widget corresponding to the given element
 *
 * \param element_number
 *
 * \return the widget or NULL
 * */
GtkWidget *gsb_form_widget_get_widget ( gint element_number )
{
    GSList *list_tmp;

    if ( !element_number )
	return NULL;

    list_tmp = form_list_widgets;

    while (list_tmp)
    {
	struct_element *element;

	element = list_tmp -> data;

	if (element -> element_number == element_number)
	    return element -> element_widget;

	list_tmp = list_tmp -> next;
    }
    return NULL;
}


/**
 * get an element number and return its name
 *
 * \param element_number
 *
 * \return the name of the element or NULL if problem
 * */
gchar *gsb_form_widget_get_name ( gint element_number )
{
    switch ( element_number )
    {
	case -1:
	    /* that value shouldn't be there, it shows that a gsb_data_form_... returns
	     * an error value */
	    warning_debug ( "gsb_form_widget_get_name : a value in the form is -1 wich should not happen.\nA gsb_data_form_... function must have returned an error value..." );
	    return NULL;
	    break;

	case TRANSACTION_FORM_DATE:
	    return (N_("Date"));
	    break;

	case TRANSACTION_FORM_DEBIT:
	    return (N_("Debit"));
	    break;

	case TRANSACTION_FORM_CREDIT:
	    return (N_("Credit"));
	    break;

	case TRANSACTION_FORM_VALUE_DATE:
	    return (N_("Value date"));
	    break;

	case TRANSACTION_FORM_EXERCICE:
	    return (N_("Financial year"));
	    break;

	case TRANSACTION_FORM_PARTY:
	    return (N_("Payee"));
	    break;

	case TRANSACTION_FORM_CATEGORY:
	    return (N_("Category line"));
	    break;

	case TRANSACTION_FORM_FREE:
	    return (N_("Free"));
	    break;

	case TRANSACTION_FORM_BUDGET:
	    return (N_("Budgetary line"));
	    break;

	case TRANSACTION_FORM_NOTES:
	    return (N_("Notes"));
	    break;

	case TRANSACTION_FORM_TYPE:
	    return (N_("Method of payment"));
	    break;

	case TRANSACTION_FORM_CHEQUE:
	    return (N_("Cheque/Transfer number"));
	    break;

	case TRANSACTION_FORM_DEVISE:
	    return (N_("Currency"));
	    break;

	case TRANSACTION_FORM_CHANGE:
	    return (N_("Change"));
	    break;

	case TRANSACTION_FORM_VOUCHER:
	    return (N_("Voucher"));
	    break;

	case TRANSACTION_FORM_BANK:
	    return (N_("Bank references"));
	    break;

	case TRANSACTION_FORM_CONTRA:
	    return (N_("Contra-transaction method of payment"));
	    break;

	case TRANSACTION_FORM_OP_NB:
	    return (N_("Transaction number"));
	    break;

	case TRANSACTION_FORM_MODE:
	    return (N_("Automatic/Manual"));
	    break;
    }
    return NULL;
}


/**
 * return the number of the next valid element in the direction given in param
 *
 * \param account_number
 * \param element_number
 * \param direction GSB_LEFT, GSB_RIGHT, GSB_UP, GSB_DOWN
 *
 * \return 	the next element number
 * 		-1 if problem or not change
 * 		-2 if end of the form and need to finish it
 * */
gint gsb_form_widget_next_element ( gint account_number,
				    gint element_number,
				    gint direction )
{
    gint row;
    gint column;
    gint return_value_number = 0;
    gint form_column_number;
    gint form_row_number;

    if ( !gsb_data_form_look_for_value ( account_number,
					 element_number,
					 &row,
					 &column ))
	return -1;

    form_column_number = gsb_data_form_get_nb_columns (account_number);
    form_row_number = gsb_data_form_get_nb_rows (account_number);

    while ( !gsb_form_widget_can_focus (return_value_number)) 
    {
	switch ( direction )
	{
	    case GSB_LEFT:
		if ( !column && !row )
		{
		    /* we are at the upper left, go on the bottom right */
		    column = form_column_number;
		    row = form_row_number -1; 
		}

		if ( --column == -1 )
		{
		    column = form_column_number - 1;
		    row--;
		}
		return_value_number = gsb_data_form_get_value ( account_number,
								column,
								row );
		break;

	    case GSB_RIGHT:
		if ( column == (form_column_number - 1)
		     &&
		     row == (form_row_number - 1))
		{
		    /* we are on the bottom right, we finish the edition or
		     * go to the upper left */
		    if ( !etat.entree )
		    {
			return_value_number = -2;
			continue;
		    }
		    column = -1;
		    row = 0; 
		}

		if ( ++column == form_column_number )
		{
		    column = 0;
		    row++;
		}
		return_value_number = gsb_data_form_get_value ( account_number,
								column,
								row );
		break;

	    case GSB_UP:
		if ( !row )
		{
		    return_value_number = -1;
		    continue;
		}

		row--;
		return_value_number = gsb_data_form_get_value ( account_number,
								column,
								row );
		break;

	    case GSB_DOWN:

		if ( row == (form_row_number - 1))
		{
		    return_value_number = -1;
		    continue;
		}
		row++;
		return_value_number = gsb_data_form_get_value ( account_number,
								column,
								row );
		break;

	    default:
		return_value_number = -1;
	}
    }
    return return_value_number;
}


/**
 * check if the given element can receive focus
 *
 * \param element_number
 * 
 * \return TRUE : can receive focus, or FALSE
 * */
gboolean gsb_form_widget_can_focus ( gint element_number )
{
    GtkWidget *widget;

    /* if element_number is -1 or -2, the iteration while must
     * stop, so return TRUE */
    if ( element_number == -1
	 ||
	 element_number == -2 )
	return TRUE;

    widget = gsb_form_widget_get_widget (element_number);

    if ( !widget )
	return FALSE;

    if ( !GTK_WIDGET_VISIBLE (widget))
	return FALSE;

    if ( !GTK_WIDGET_SENSITIVE (widget))
	return FALSE;

    if ( !(GTK_IS_COMBOFIX (widget)
	   ||
	   GTK_IS_ENTRY (widget)
	   ||
	   GTK_IS_BUTTON (widget)
	   ||
	   GTK_IS_COMBO_BOX (widget)))
	return FALSE;

    return TRUE;
}


/**
 * set the focus on the given element
 *
 * \param element_number
 *
 * \return
 * */
void gsb_form_widget_set_focus ( gint element_number )
{
    GtkWidget *widget;

    devel_debug ( g_strdup_printf ( "gsb_form_widget_set_focus %d",
				    element_number ));

    widget = gsb_form_widget_get_widget (element_number);

    if ( !widget )
	return;

    if ( GTK_IS_COMBOFIX ( widget ))
	gtk_widget_grab_focus ( GTK_COMBOFIX ( widget ) -> entry );
    else
	gtk_widget_grab_focus ( widget );

    return;
}


/**
 * check if the entry given in param is empty (ie grey) or not
 *
 * \param entry must be an entry
 *
 * \return TRUE : entry is free, FALSE : not free
 * */
gboolean gsb_form_widget_check_empty ( GtkWidget *entry )
{
    if (!entry
	||
	!GTK_IS_ENTRY (entry))
	return FALSE;

    return (gtk_widget_get_style ( entry ) == style_entree_formulaire[ENGRIS] );
}


/**
 * set the style to the entry as empty or not
 *
 * \param entry must be an entry
 * \param empty TRUE or FALSE
 *
 * \return 
 * */
void gsb_form_widget_set_empty ( GtkWidget *entry,
				 gboolean empty )
{
    if (!entry
	||
	!GTK_IS_ENTRY (entry))
	return;

    if (empty)
	gtk_widget_set_style ( entry,
			       style_entree_formulaire[ENGRIS] );
    else
	gtk_widget_set_style (  entry,
				style_entree_formulaire[ENCLAIR] );
}

