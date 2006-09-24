#ifndef _GSB_AUTOMEM_H
#define _GSB_AUTOMEM_H (1)


/* START_INCLUDE_H */
#include "gsb_automem.h"
/* END_INCLUDE_H */

/* FIXME : doesn"t work with mk_include */
#include "utils_buttons.h"

/* START_DECLARATION */
GtkWidget *gsb_automem_checkbutton_new ( const gchar *label,
					 gboolean *value,
					 GCallback hook,
					 gpointer data );
void gsb_automem_checkbutton_set_value ( GtkWidget *checkbutton,
					 gboolean *value );
GtkWidget *gsb_automem_entry_new ( gchar **value,
				   GCallback hook,
				   gpointer data );
void gsb_automem_entry_set_value ( GtkWidget *entry,
				   gchar **value );
GtkWidget *gsb_automem_imagefile_button_new ( GsbButtonStyle style,
					      const gchar * name, const gchar * filename,
					      GCallback callback, gpointer data );
GtkWidget *gsb_automem_radiobutton_new_with_title ( GtkWidget *parent,
						    const gchar *title,
						    const gchar *choice1, const gchar *choice2,
						    gboolean *value,
						    GCallback hook, gpointer data );
GtkWidget *gsb_automem_spin_button_new ( gint *value, 
					 GCallback hook,
					 gpointer data );
void gsb_automem_spin_button_set_value ( GtkWidget *spin,
					 gint *value );
GtkWidget *gsb_automem_stock_button_menu_new ( GsbButtonStyle style,
					       const gchar * stock_id, const gchar * name, 
					       GCallback callback, gpointer data );
GtkWidget *gsb_automem_stock_button_new ( GsbButtonStyle style,
					  const gchar * stock_id, const gchar * name, 
					  GCallback callback, gpointer data );
GtkWidget *gsb_automem_textview_new ( gchar **value,
				      GCallback hook,
				      gpointer data );
/* END_DECLARATION */
#endif