#ifndef _GSB_DATA_CURRENCY_LINK_H
#define _GSB_DATA_CURRENCY_LINK_H (1)


/* START_INCLUDE_H */
#include "gsb_data_currency_link.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gdouble gsb_data_currency_link_get_change_rate ( gint currency_link_number );
GSList *gsb_data_currency_link_get_currency_link_list ( void );
gint gsb_data_currency_link_get_first_currency ( gint currency_link_number );
gint gsb_data_currency_link_get_invalid_link ( gint currency_link_number );
const gchar *gsb_data_currency_link_get_invalid_message ( gint currency_link_number );
gint gsb_data_currency_link_get_no_currency_link ( gpointer currency_link_ptr );
gint gsb_data_currency_link_get_second_currency ( gint currency_link_number );
gboolean gsb_data_currency_link_init_variables ( void );
gint gsb_data_currency_link_new ( gint currency_link_number );
gboolean gsb_data_currency_link_remove ( gint currency_link_number );
gint gsb_data_currency_link_search ( gint currency_1,
				     gint currency_2 );
gboolean gsb_data_currency_link_set_change_rate ( gint currency_link_number,
						  gdouble change_rate);
gboolean gsb_data_currency_link_set_first_currency ( gint currency_link_number,
						     gint first_currency );
gint gsb_data_currency_link_set_new_number ( gint currency_link_number,
					     gint new_no_currency_link );
gboolean gsb_data_currency_link_set_second_currency ( gint currency_link_number,
						      gint second_currency );
/* END_DECLARATION */
#endif