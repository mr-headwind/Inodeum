/*
**  Copyright (C) 2017 Anthony Buckley
** 
**  This file is part of Inodeum.
** 
**  Inodeum is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**  
**  Inodeum is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**  
**  You should have received a copy of the GNU General Public License
**  along with Inodeum.  If not, see <http://www.gnu.org/licenses/>.
*/



/*
** Description:	Show current service overview.
**
** Author:	Anthony Buckley
**
** History
**	11-Dec-2017	Initial code
**
*/



/* Defines */


/* Includes */

#include <stdlib.h>  
#include <string.h>  
#include <libgen.h>  
#include <stdio.h>
#include <gtk/gtk.h>  
#include <main.h>
#include <isp.h>
#include <defs.h>
#include <version.h>


/* Types */


/* Prototypes */

void history_panel(MainUi *);
void init_history(MainUi *);
void load_history(IspData *, MainUi *);
void chart_total(ServUsage *, MainUi *);
void create_hist_graph(ServUsage *, IspData *, MainUi *);
void reset_history(MainUi *);

extern void OnHistFind(GtkWidget *, gpointer); 
extern gboolean OnHistExpose (GtkWidget*, cairo_t *, gpointer);
extern void create_label(GtkWidget **, char *, char *, GtkWidget *, int, int, int, int);
extern void create_entry(GtkWidget **, char *, GtkWidget *, int, int);
extern void create_cbox(GtkWidget **, char *, const char *[], int, int, GtkWidget *, int, int);
extern ServUsage * get_service_usage();
extern int get_hist_service_usage(IspData *, MainUi *);
extern char * format_usg(char *, char *);
extern int long_chars(long);


/* Globals */

static const char *debug_hdr = "DEBUG-history.c ";



/* Widgets for service overview details */

void history_panel(MainUi *m_ui)
{  
    GtkWidget *sum_grid;
    GtkWidget *lbl;
    GtkWidget *frame;
    const char *usg_cats[] = { "Total", "Metered up", "Metered down", "Unmetered up", "Unmetered down" };
    const int usg_cat_max = 5;

    /* Create main container grid */
    m_ui->hist_cntr = gtk_grid_new();
    gtk_widget_set_name(m_ui->hist_cntr, "hist_panel");
    gtk_grid_set_row_spacing(GTK_GRID (m_ui->hist_cntr), 2);
    gtk_grid_set_column_spacing(GTK_GRID (m_ui->hist_cntr), 2);
    gtk_container_set_border_width (GTK_CONTAINER (m_ui->hist_cntr), 2);
    gtk_widget_set_margin_top (m_ui->hist_cntr, 5);
    gtk_widget_set_margin_left (m_ui->hist_cntr, 15);

    /* Create drawing area for line graph */
    m_ui->hist_graph_area = gtk_drawing_area_new();
    gtk_widget_set_margin_top (m_ui->hist_graph_area, 10);
    gtk_widget_set_size_request (m_ui->hist_graph_area, 250, 160);
    gtk_widget_set_halign (m_ui->hist_graph_area, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (m_ui->hist_graph_area, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID (m_ui->hist_cntr), m_ui->hist_graph_area, 0, 0, 1, 1);

    g_signal_connect (m_ui->hist_graph_area, "draw", G_CALLBACK (OnHistExpose), m_ui);

    /* Summary total data */
    create_label(&(m_ui->hist_total), "data_2", "Total Usage: ", m_ui->hist_cntr, 0, 1, 1, 1);
    gtk_widget_set_margin_bottom (m_ui->hist_total, 10);
    gtk_widget_set_halign(GTK_WIDGET (m_ui->hist_total), GTK_ALIGN_CENTER);

    /* Create search container grid */
    m_ui->hist_search_cntr = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID (m_ui->hist_search_cntr), 2);
    gtk_grid_set_column_spacing(GTK_GRID (m_ui->hist_search_cntr), 2);
    gtk_container_set_border_width (GTK_CONTAINER (m_ui->hist_search_cntr), 2);
    gtk_widget_set_margin_top (m_ui->hist_search_cntr, 1);
    gtk_widget_set_margin_left (m_ui->hist_search_cntr, 10);
    gtk_widget_set_valign(GTK_WIDGET (m_ui->hist_search_cntr), GTK_ALIGN_CENTER);

    /* Search argument widgets */
    create_label(&(m_ui->from_dt_lbl), "from_dt_lbl", "Date From ", m_ui->hist_search_cntr, 0, 0, 1, 1);
    create_entry(&(m_ui->hist_from_dt), "ent_1", m_ui->hist_search_cntr, 1, 0);
    gtk_entry_set_width_chars (GTK_ENTRY(m_ui->hist_from_dt), 12);
    gtk_entry_set_max_width_chars (GTK_ENTRY(m_ui->hist_from_dt), 10);
    gtk_entry_set_max_length (GTK_ENTRY(m_ui->hist_from_dt), 10);

    m_ui->fr_btn = gtk_button_new_with_label("...");
    gtk_grid_attach(GTK_GRID (m_ui->hist_search_cntr), m_ui->fr_btn, 2, 0, 1, 1);

    create_label(&(m_ui->to_dt_lbl), "to_dt_lbl", "Date To ", m_ui->hist_search_cntr, 0, 1, 1, 1);
    create_entry(&(m_ui->hist_to_dt), "ent_1", m_ui->hist_search_cntr, 1, 1);
    gtk_entry_set_width_chars (GTK_ENTRY(m_ui->hist_to_dt), 12);
    gtk_entry_set_max_width_chars (GTK_ENTRY(m_ui->hist_to_dt), 10);
    gtk_entry_set_max_length (GTK_ENTRY(m_ui->hist_to_dt), 10);

    m_ui->to_btn = gtk_button_new_with_label("...");
    gtk_grid_attach(GTK_GRID (m_ui->hist_search_cntr), m_ui->to_btn, 2, 1, 1, 1);

    create_label(&(m_ui->cat_lbl), "cat_lbl", "Category", m_ui->hist_search_cntr, 0, 2, 1, 1);
    create_cbox(&(m_ui->usgcat_cbox), "usg_cat", usg_cats, usg_cat_max, 0, m_ui->hist_search_cntr, 1, 2);

    m_ui->hist_search_btn = gtk_button_new_with_label("Find");
    gtk_widget_set_name ( m_ui->hist_search_btn, "button_1");
    gtk_grid_attach(GTK_GRID (m_ui->hist_search_cntr), m_ui->hist_search_btn, 1, 3, 1, 1);
    gtk_widget_set_margin_top (m_ui->hist_search_btn, 2);
    g_signal_connect (m_ui->hist_search_btn, "clicked", G_CALLBACK (OnHistFind), m_ui);

    frame = gtk_frame_new (NULL);
    gtk_container_add(GTK_CONTAINER (frame), m_ui->hist_search_cntr);

    /* Set up calendar popup */

    /* Add summary to history container */
    gtk_grid_attach(GTK_GRID (m_ui->hist_cntr), frame, 0, 2, 1, 1);

    /* Add to the panel stack */
    gtk_stack_add_named (GTK_STACK (m_ui->panel_stk), m_ui->hist_cntr, "hist_panel");

    return;
}


/* Initialise date fields */

void init_history(MainUi *m_ui)
{  
    gtk_entry_set_text (GTK_ENTRY(m_ui->hist_from_dt), NULL);
    gtk_entry_set_text (GTK_ENTRY(m_ui->hist_to_dt), NULL);

    return;
}


/* Load usage details */

void load_history(IspData *isp_data, MainUi *m_ui)
{  
    const gchar *txt;
    ServUsage *srv_usg;

    /* Populate text if search fields are empty */
    txt = gtk_entry_get_text (GTK_ENTRY(m_ui->hist_from_dt));

    if (txt != NULL)
    	return;

    /* Get the service usage class */
    srv_usg = get_service_usage();

    /* Show current search values */
    gtk_entry_set_text (GTK_ENTRY(m_ui->hist_from_dt), srv_usg->hist_from_dt);
    gtk_entry_set_text (GTK_ENTRY(m_ui->hist_to_dt), srv_usg->hist_to_dt);
    gtk_combo_box_set_active (GTK_COMBO_BOX(m_ui->usgcat_cbox), srv_usg->last_cat_idx);

    /* Set total bytes */
    chart_total(srv_usg, m_ui)

    /* Show */
    gtk_widget_show_all(m_ui->window);

    /* Set up usage graphs */
    create_hist_graph(srv_usg, m_ui);

    return;
}


/* Reset history for new search criteria */

void reset_history(MainUi *m_ui)
{  
    int cat_idx;
    const gchar *dt_fr, *dt_to;
    IspData *isp_data;
    ServUsage *srv_usg;

    /* Initial */
    srv_usg = get_service_usage();
    isp_data = (IspData *) g_object_get_data (G_OBJECT(m_ui->window), "isp_data");

    /* Date changes force a new Isp query */
    dt_fr = gtk_entry_get_text (GTK_ENTRY(m_ui->hist_from_dt));
    dt_to = gtk_entry_get_text (GTK_ENTRY(m_ui->hist_to_dt));
    cat_idx = gtk_combo_box_get_active (GTK_COMBO_BOX(m_ui->usgcat_cbox));

    if ((strcmp(dt_fr, srv_usg->hist_from_dt) != 0) || (strcmp(dt_to, srv_usg->hist_to_dt) != 0))
    {
    	strcpy(srv_usg->hist_from_dt, dt_fr);
	strcpy(srv_usg->hist_to_dt, dt_to);
    	get_hist_service_usage(isp_data, m_ui);
    }
    else if (srv_usg->last_cat_idx != cat_idx)
    {
    	srv_usg->last_cat_idx = cat_idx;
    }
    else
    {
    	return;
    }

    /* Set total bytes */
    chart_total(srv_usg, m_ui)

    /* Set up usage graphs */
    create_hist_graph(srv_usg, m_ui);

    return;
}


/* Show the total bytes for the period */

void chart_total(ServUsage *srv_usg, MainUi *m_ui)
{  
    long ll;
    int i;
    char *s, *s2, *amt;

    ll = srv_usg->hist_usg_arr[srv_usg->hist_days - 1][srv_usg->last_cat_idx];
    i = long_chars(ll);
    amt = (char *) malloc(ll + 1);
    sprintf(amt, "%ld", ll);

    s = format_usg(amt, srv_usg->unit);
    s2 = (char *) malloc(strlen(s) + 14);
    sprintf(s2, "Total Usage: %s", s);
    gtk_label_set_text (GTK_LABEL (m_ui->usage), s2);

    free(s);
    free(s2);
    free(amt);

    return;
}


/* Create usage history chart objects, drawing is handled in the 'draw' (OnHistExpose) event */

void create_hist_graph(ServUsage *srv_usg, IspData *isp_data, MainUi *m_ui)
{  
    int i;

    /* Reset any existing graph */
    if (m_ui->hist_usg_graph != NULL)
    	free_line_graph(m_ui->hist_usg_graph);

    /* Build the list of graph points - use actual values: they are adjusted on drawing */
    /* Day forms the X axis and data usage forms the Y axis */
    for(i = 0; i < srv_usg->hist_days; i++)
    	line_graph_add_point(m_ui->hist_usg_graph, (double) i, srv_usg->hist_usg_arr[i][srv_usg->last_cat_idx]); 

    /* History line graph */
    m_ui->hist_usg_graph = line_graph_create(
    		NULL, NULL, 0,
		"Day", 0, srv_usg->hist_days, 1, 0,
		&DARK_BLUE, 9, &DARK_BLUE, 8,
		"MB", 0, srv_usg->hist_usg_arr[srv_usg_hist_days - 1][srv_usg->last_cat_idx], 100, 2,
		&DARK_BLUE, 9, &DARK_BLUE, 8);

    return;
}