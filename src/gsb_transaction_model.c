/* ************************************************************************** */
/*                                                                            */
/*     copyright (c)	2000-2007 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2007 Benjamin Drieu (bdrieu@april.org) 	      */
/*			http://www.grisbi.org   			      */
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
 * \file gsb_transaction_model.c
 * contains convenient function to easily work with the models (sort and filter)
 * of the transactions list
 */

#include "include.h"


/*START_INCLUDE*/
#include "gsb_transaction_model.h"
#include "erreur.h"
#include "gsb_data_transaction.h"
#include "gsb_transactions_list.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GtkTreePath *gsb_transaction_model_get_model_path ( gint transaction_number,
						    gint line_in_transaction );
static gint gsb_transaction_model_get_transaction_from_model_path ( GtkTreePath *path_model );
/*END_STATIC*/


/**
 * return the path of a line of the transaction, in the tree view
 * this is the path on the sorted model, ie what we see after been sorted and filtered
 *
 * \param transaction_number
 * \param line_in_transaction the line of the transaction (0, 1, 2 or 3)
 *
 * \return a newly allocated GtkTreePath
 * */
GtkTreePath *gsb_transactions_list_get_sorted_path ( gint transaction_number,
						     gint line_in_transaction )
{
    GtkTreePath *path_model;
    GtkTreePath *path_filtered;
    GtkTreePath *path_sorted;

    if ( !transaction_number || !gsb_transactions_list_get_tree_view()
	 || line_in_transaction < 0 || line_in_transaction >= TRANSACTION_LIST_ROWS_NB )
	return NULL;

    path_model = gsb_transaction_model_get_model_path (transaction_number, line_in_transaction);
    path_filtered = gtk_tree_model_filter_convert_child_path_to_path ( GTK_TREE_MODEL_FILTER (gsb_transactions_list_get_filter ()),
								       path_model );
    path_sorted = gtk_tree_model_sort_convert_child_path_to_path ( GTK_TREE_MODEL_SORT (gsb_transactions_list_get_sortable ()),
								   path_filtered );
    gtk_tree_path_free (path_model);
    gtk_tree_path_free (path_filtered);
    return path_sorted;
}


/**
 * return the number of transaction on the sorted path
 * this is the path on the sorted model, ie what we see after been sorted and filtered
 * 
 * \param path_sorted the path in the sorted model
 * 
 * \return the transaction number or -1 (white line) if problem
 * */
gint gsb_transaction_model_get_transaction_from_sorted_path ( GtkTreePath *path_sorted )
{
    gpointer transaction;
    GtkTreeIter iter;
    GtkTreePath *path_model;

    /* if no path, return -1, wich is the white line */
    if ( !path_sorted )
	return -1;

    path_model = gsb_transaction_model_get_model_path_from_sorted_path (path_sorted);

    if (!path_model)
	return -1;

    gtk_tree_model_get_iter ( GTK_TREE_MODEL (gsb_transactions_list_get_store()),
			      &iter,
			      path_model );
    gtk_tree_model_get ( GTK_TREE_MODEL (gsb_transactions_list_get_store()),
			 &iter,
			 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction,
			 -1 );

    gtk_tree_path_free (path_model);

    return gsb_data_transaction_get_transaction_number (transaction);
}



/** 
 * return the path of a line of the transaction, in the model
 * this is the path on the original model, ie not sorted and not filtered
 * 
 * \param transaction_number
 * \param line_in_transaction the line of the transaction (0, 1, 2 or 3)
 * 
 * \return a newly allocated GtkTreePath
 * */
GtkTreePath *gsb_transaction_model_get_model_path ( gint transaction_number,
						    gint line_in_transaction )
{
    GtkTreePath *path;
    GtkTreeIter *iter;

    if ( !transaction_number || !gsb_transactions_list_get_tree_view()
	 || line_in_transaction < 0 || line_in_transaction >= TRANSACTION_LIST_ROWS_NB )
	return NULL;

    iter = gsb_transaction_model_get_iter_from_transaction ( transaction_number,
							     line_in_transaction );
    path = gtk_tree_model_get_path ( GTK_TREE_MODEL (gsb_transactions_list_get_store()),
				     iter );
    gtk_tree_iter_free (iter);
    return path;
}



/**
 * return the number of transaction on the model path
 * this is the path on the original model, ie not sorted and not filtered
 * 
 * \param path_model the path in the original model
 * 
 * \return the transaction number
 * */
gint gsb_transaction_model_get_transaction_from_model_path ( GtkTreePath *path_model )
{
    gpointer transaction;
    GtkTreeIter iter;
    GtkTreeModel *model;

    /* if no path, return -1, wich is the white line */
    if ( !path_model )
	return -1;

    model = GTK_TREE_MODEL (gsb_transactions_list_get_store());

    gtk_tree_model_get_iter ( model,
			      &iter,
			      path_model );
    gtk_tree_model_get ( model,
			 &iter,
			 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction,
			 -1 );

    return gsb_data_transaction_get_transaction_number (transaction);
}



/**
 * return the path in the original model from the path in the sorted model
 * 
 * \param path_sorted
 * 
 * \return a newly allocated GtkTreePath or NULL
 * */
GtkTreePath *gsb_transaction_model_get_model_path_from_sorted_path ( GtkTreePath *path_sorted )
{
    GtkTreePath *path_filter;
    GtkTreePath *path_model = NULL;

    if ( !path_sorted )
	return NULL;

    path_filter = gtk_tree_model_sort_convert_path_to_child_path ( GTK_TREE_MODEL_SORT (gsb_transactions_list_get_sortable()),
								   path_sorted );

    if ( path_filter )
    {
	path_model = gtk_tree_model_filter_convert_path_to_child_path ( GTK_TREE_MODEL_FILTER (gsb_transactions_list_get_filter()),
								  path_filter );
	gtk_tree_path_free (path_filter);
    }

    return path_model;
}



/**
 * return the iter of the transaction in the original model
 * this is the iter on the original model, ie not sorted and not filtered
 * 
 * \param transaction_number
 * \param line_in_transaction can be 0, 1, 2 ou 3
 *
 * \return a newly allocated GtkTreeIter or NULL
 * */
GtkTreeIter *gsb_transaction_model_get_iter_from_transaction ( gint transaction_number,
							       gint line_in_transaction )
{
    GtkTreeIter iter;
    GtkTreeModel *model;

    model = GTK_TREE_MODEL (gsb_transactions_list_get_store());

    if ( !model
	 ||
	 !transaction_number)
	return NULL;

    /* we go through the list in the model untill we find the transaction */
    if ( gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( model ),
					 &iter ))
    {
	do
	{
	    gpointer transaction_pointer;
	    gint line;
	    GtkTreeIter iter_child;

	    gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
				 &iter,
				 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction_pointer,
				 TRANSACTION_COL_NB_TRANSACTION_LINE, &line,
				 -1 );

	    if ( line == line_in_transaction
		 &&
		 gsb_data_transaction_get_transaction_number (transaction_pointer) == transaction_number )
		return ( gtk_tree_iter_copy (&iter));

	    /* gtk_tree_iter_next doesn't go in the children, so if the current transaction
	     * has children, we have to look for the transaction here, and go down into the children */
	    if ( gtk_tree_model_iter_children ( GTK_TREE_MODEL (model),
						&iter_child, &iter))
	    {
		/* ok so iter_child is on a breakdown child, we go to see all the breakdowns */
		do
		{
		    /* we don't mind the line for a breakdown child, because only one */
		    gtk_tree_model_get ( GTK_TREE_MODEL ( model ),
					 &iter_child,
					 TRANSACTION_COL_NB_TRANSACTION_ADDRESS, &transaction_pointer,
					 -1 );
		    if ( gsb_data_transaction_get_transaction_number (transaction_pointer) == transaction_number )
			return ( gtk_tree_iter_copy (&iter_child));
		}
		while (gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),&iter_child ));
	    }
	}
	while (gtk_tree_model_iter_next ( GTK_TREE_MODEL ( model ),&iter ));

    }
    return NULL;
}



/**
 * convert an iter in the sorted_model to an iter in the original tree model
 * (sorted_model -> filtered_model -> tree_model)
 *
 * \param tree_iter a pointer to an iter which will be set to the row in the tree_model
 * \param sorted_iter the iter on the row in the sorted_model
 * 
 * \return
 * */
void gsb_transaction_model_convert_sorted_iter_to_model_iter ( GtkTreeIter *tree_iter,
							       GtkTreeIter *sorted_iter )
{
    GtkTreeIter iter_in_filter;

    devel_debug ( "gsb_transaction_model_convert_sorted_iter_to_model_iter" );

    gtk_tree_model_sort_convert_iter_to_child_iter ( GTK_TREE_MODEL_SORT (gsb_transactions_list_get_sortable ()),
						     &iter_in_filter,
						     sorted_iter );
    gtk_tree_model_filter_convert_iter_to_child_iter ( GTK_TREE_MODEL_FILTER (gsb_transactions_list_get_filter ()),
						       tree_iter,
						       &iter_in_filter );
}

