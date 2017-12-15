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

void history_panel(MainUi *m_ui);
void load_history(IspData *, MainUi *);
void create_hist_graph(ServUsage *, IspData *, MainUi *);

extern gboolean OnHistExpose (GtkWidget*, cairo_t *, gpointer);
extern void create_label(GtkWidget **, char *, char *, GtkWidget *, int, int, int, int);

/*
extern int val_str2dbl(char *, double *, char *, GtkWidget *);
extern time_t strdt2tmt(char *, char *, char *, char *, char *, char *);
extern double difftime_days(time_t, time_t);
extern ServUsage * get_service_usage();
extern time_t date_tm_add(struct tm *, char *, int);
extern int get_user_pref(char *, char **);
*/



/* Globals */

static const char *debug_hdr = "DEBUG-history.c ";



/* Widgets for service overview details */

void history_panel(MainUi *m_ui)
{  
    int i, j;

    /* Create main container grid */
    m_ui->hist_cntr = gtk_grid_new();
    gtk_widget_set_name(m_ui->hist_cntr, "hist_panel");
    gtk_grid_set_row_spacing(GTK_GRID (m_ui->hist_cntr), 2);
    gtk_grid_set_column_spacing(GTK_GRID (m_ui->hist_cntr), 2);
    gtk_container_set_border_width (GTK_CONTAINER (m_ui->hist_cntr), 2);
    gtk_widget_set_margin_top (m_ui->hist_cntr, 5);
    gtk_widget_set_margin_left (m_ui->hist_cntr, 15);

    /* Create summary container grid */
    m_ui->hist_ctrl_cntr = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID (m_ui->hist_ctrl_cntr), 2);
    gtk_grid_set_column_spacing(GTK_GRID (m_ui->hist_ctrl_cntr), 2);
    gtk_container_set_border_width (GTK_CONTAINER (m_ui->hist_ctrl_cntr), 2);
    gtk_widget_set_margin_top (m_ui->hist_ctrl_cntr, 5);
    gtk_widget_set_margin_left (m_ui->hist_ctrl_cntr, 15);

    /* Title labels */
    i = j = 0;
    create_label(&(m_ui->start_dt_lbl), "start_dt_lbl", NULL, m_ui->hist_ctrl_cntr, i, j, 1, 1);

    j++;
    create_label(&(m_ui->end_dt_lbl), "end_dt_lbl", NULL, m_ui->hist_ctrl_cntr, i, j, 1, 1);

    /* Date entry fields */
    /*
    j++;
    create_entry(&(m_ui->search date), "rem_days_lbl", NULL, m_ui->hist_ctrl_cntr, i, j, 1, 1);

    j++;
    create_entry(&(m_ui->search to date), "usage_lbl", NULL, m_ui->hist_ctrl_cntr, i, j, 1, 1);
    */

    /* Search button */

    /* Data labels */
    j = 0;
    i++;
    create_label(&(m_ui->hist_total), "data_1", NULL, m_ui->hist_ctrl_cntr, i, j, 1, 1);
    gtk_widget_set_margin_left (m_ui->hist_total, 15);

    /* Add summary to history container */
    gtk_grid_attach(GTK_GRID (m_ui->hist_cntr), m_ui->hist_ctrl_cntr, 0, 0, 1, 1);

    /* Create drawing area for line graph and add to history container */
    m_ui->hist_graph_area = gtk_drawing_area_new();
    gtk_widget_set_margin_top (m_ui->hist_graph_area, 10);
    gtk_widget_set_size_request (m_ui->hist_graph_area, 250, 160);
    gtk_widget_set_halign (m_ui->hist_graph_area, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (m_ui->hist_graph_area, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID (m_ui->hist_cntr), m_ui->graph_area, 0, 1, 1, 1);

    g_signal_connect (m_ui->hist_graph_area, "draw", G_CALLBACK (OnHistExpose), m_ui);

    /* Add to the panel stack */
    gtk_stack_add_named (GTK_STACK (m_ui->panel_stk), m_ui->hist_cntr, "hist_panel");

    return;
}


/* Load usage details */

void load_history(IspData *isp_data, MainUi *m_ui)
{  
    char *s;
    time_t time_rovr;
    struct tm *dtm;
    time_t tm_t;
    ServUsage *srv_usg;

    /* Show summary details in text */
    /*
    srv_usg = get_service_usage();

    s = (char *) malloc(strlen(srv_usg->plan_interval) + 7);
    sprintf(s, "%s Quota:", srv_usg->plan_interval);
    gtk_label_set_text (GTK_LABEL (m_ui->quota_lbl), s);
    free(s);

    s = format_usg(srv_usg->quota, srv_usg->unit);
    gtk_label_set_text (GTK_LABEL (m_ui->quota), s);
    free(s);

    gtk_label_set_text (GTK_LABEL (m_ui->next_dt_lbl), "Next Rollover:");
    s = format_dt(srv_usg->rollover_dt, &time_rovr, &dtm);
    gtk_label_set_text (GTK_LABEL (m_ui->rollover_dt), s);
    free(s);

    gtk_label_set_text (GTK_LABEL (m_ui->rem_days_lbl), "Days remaining:");
    s = format_remdays(time_rovr, &(m_ui->days_rem));
    gtk_label_set_text (GTK_LABEL (m_ui->rem_days), s);
    free(s);

    tm_t = date_tm_add(dtm, "month", -1);
    m_ui->days_quota = difftime_days(time_rovr, tm_t);

    gtk_label_set_text (GTK_LABEL (m_ui->usage_lbl), "Total Usage:");
    s = format_usg(srv_usg->total_bytes, srv_usg->unit);
    gtk_label_set_text (GTK_LABEL (m_ui->usage), s);
    free(s);
    */

    /* Show */
    gtk_widget_show_all(m_ui->window);

    /* Set up usage graphs */
    create_hist_graph(srv_usg, isp_data, m_ui);

    return;
}


/* Create usage history chart objects, drawing is handled in the 'draw' (OnHistExpose) event */

void create_hist_graph(ServUsage *srv_usg, IspData *isp_data, MainUi *m_ui)
{  
    /* If nothing changed, return */
    //if (m_ui->hist_graph_list != NULL)
    	return;

    /* Build the GList for the history line graph */
    //hist_graph_pts(isp_data, m_ui);

    /* Free old charts */

    /* History line graph */
    //m_ui->hist_usg_graph = line_graph_create("Quota Distribution", 0, lgd, &DARK_BLUE, 9, lbl);

    return;
}
