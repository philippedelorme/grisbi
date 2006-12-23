#ifndef _GSB_COMBO_BOX_H
#define _GSB_COMBO_BOX_H (1)


/* START_INCLUDE_H */
#include "gsb_combo_box.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gint gsb_combo_box_get_index ( GtkWidget *combo_box );
GtkWidget *gsb_combo_box_new_with_index ( gchar **string,
					  GCallback func );
gint gsb_combo_box_set_index ( GtkWidget *combo_box,
			       gint index );
/* END_DECLARATION */
#endif