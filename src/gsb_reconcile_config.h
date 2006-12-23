#ifndef _GSB_RECONCILE_CONFIG_H
#define _GSB_RECONCILE_CONFIG_H (1)


enum reconciliation_columns {
    RECONCILIATION_NAME_COLUMN = 0,
    RECONCILIATION_VISIBLE_COLUMN,
    RECONCILIATION_SORT_COLUMN,
    RECONCILIATION_SPLIT_NEUTRAL_COLUMN,
    RECONCILIATION_ACCOUNT_COLUMN,
    RECONCILIATION_TYPE_COLUMN,
    NUM_RECONCILIATION_COLUMNS,
};


/* START_INCLUDE_H */
#include "gsb_reconcile_config.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
void fill_reconciliation_tree ();
GtkWidget * tab_display_reconciliation ( void );
/* END_DECLARATION */
#endif