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
**	20-Jun-2017	Initial code
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

void overview_panel(MainUi *m_ui);
char * format_usg(char *, char *);
char * format_dt(char *, time_t *, struct tm **);
char * format_remdays(time_t, double *);
void create_charts(ServUsage *, IspData *, MainUi *);

extern void create_label(GtkWidget **, char *, char *, GtkWidget *, int, int, int, int);
extern int val_str2dbl(char *, double *, char *, GtkWidget *);
extern time_t strdt2tmt(char *, char *, char *, char *, char *, char *);
extern double difftime_days(time_t, time_t);
extern ServUsage * get_service_usage();
extern gboolean OnOvExpose (GtkWidget*, cairo_t *, gpointer);
extern PieChart * pie_chart_create(char *, double, int, const GdkRGBA *, int, int);
extern int pie_slice_create(PieChart *, char *, double, const GdkRGBA *, const GdkRGBA *, int);
extern BarChart * bar_chart_create(char *, const GdkRGBA *, int, int, Axis *, Axis *);
extern Bar * bar_create(BarChart *);
extern int bar_segment_create(BarChart *, Bar *, char *, const GdkRGBA *, const GdkRGBA *, int, double);
extern void free_pie_chart(PieChart *);
extern void free_bar_chart(BarChart *);
extern time_t date_tm_add(struct tm *, char *, int);
extern int get_user_pref(char *, char **);



/* Globals */

static const char *debug_hdr = "DEBUG-overview.c ";



/* Widgets for service overview details */

void overview_panel(MainUi *m_ui)
{  
    int i, j;

    /* Create main container grid */
    m_ui->oview_cntr = gtk_grid_new();
    gtk_widget_set_name(m_ui->oview_cntr, "oview_panel");
    gtk_grid_set_row_spacing(GTK_GRID (m_ui->oview_cntr), 2);
    gtk_grid_set_column_spacing(GTK_GRID (m_ui->oview_cntr), 2);
    gtk_container_set_border_width (GTK_CONTAINER (m_ui->oview_cntr), 2);
    gtk_widget_set_margin_top (m_ui->oview_cntr, 5);
    gtk_widget_set_margin_left (m_ui->oview_cntr, 15);

    /* Create summary container grid */
    m_ui->sum_cntr = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID (m_ui->sum_cntr), 2);
    gtk_grid_set_column_spacing(GTK_GRID (m_ui->sum_cntr), 2);
    gtk_container_set_border_width (GTK_CONTAINER (m_ui->sum_cntr), 2);
    gtk_widget_set_margin_top (m_ui->sum_cntr, 5);
    gtk_widget_set_margin_left (m_ui->sum_cntr, 15);

    /* Title labels */
    i = j = 0;
    create_label(&(m_ui->quota_lbl), "quota_lbl", NULL, m_ui->sum_cntr, i, j, 1, 1);

    j++;
    create_label(&(m_ui->next_dt_lbl), "next_dt_lbl", NULL, m_ui->sum_cntr, i, j, 1, 1);

    j++;
    create_label(&(m_ui->rem_days_lbl), "rem_days_lbl", NULL, m_ui->sum_cntr, i, j, 1, 1);

    j++;
    create_label(&(m_ui->usage_lbl), "usage_lbl", NULL, m_ui->sum_cntr, i, j, 1, 1);

    /* Data labels */
    j = 0;
    i++;
    create_label(&(m_ui->quota), "data_1", NULL, m_ui->sum_cntr, i, j, 1, 1);
    gtk_widget_set_margin_left (m_ui->quota, 15);

    j++;
    create_label(&(m_ui->rollover_dt), "data_1", NULL, m_ui->sum_cntr, i, j, 1, 1);
    gtk_widget_set_margin_left (m_ui->rollover_dt, 15);

    j++;
    create_label(&(m_ui->rem_days), "data_1", NULL, m_ui->sum_cntr, i, j, 1, 1);
    gtk_widget_set_margin_left (m_ui->rem_days, 15);

    j++;
    create_label(&(m_ui->usage), "data_1", NULL, m_ui->sum_cntr, i, j, 1, 1);
    gtk_widget_set_margin_left (m_ui->usage, 15);

    /* Add summary to main overview container */
    gtk_grid_attach(GTK_GRID (m_ui->oview_cntr), m_ui->sum_cntr, 0, 0, 1, 1);

    /* Create drawing area for graphs and add to main overview container */
    m_ui->graph_area = gtk_drawing_area_new();
    gtk_widget_set_margin_top (m_ui->graph_area, 10);
    gtk_widget_set_size_request (m_ui->graph_area, 250, 160);
    gtk_widget_set_halign (m_ui->graph_area, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (m_ui->graph_area, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID (m_ui->oview_cntr), m_ui->graph_area, 0, 1, 1, 1);

    g_signal_connect (m_ui->graph_area, "draw", G_CALLBACK (OnOvExpose), m_ui);

    /* Add to the panel stack */
    gtk_stack_add_named (GTK_STACK (m_ui->panel_stk), m_ui->oview_cntr, "oview_panel");

    return;
}


/* Load usage details */

void load_overview(IspData *isp_data, MainUi *m_ui)
{  
    char *s;
    time_t time_rovr;
    struct tm *dtm;
    time_t tm_t;
    ServUsage *srv_usg;

    /* Show summary details in text */
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

    /* Show */
    gtk_widget_show_all(m_ui->window);

    /* Set up usage graphs */
    create_charts(srv_usg, isp_data, m_ui);

    return;
}


/* Format a usage value - eg. quota, total usage */

char * format_usg(char *amt, char *unit)
{  
    int i, j;
    double dbl, div, qnt, tmp;
    char *s;
    const char *abbrev[] = {"GB", "MB", "KB", "Bytes"};
    const double divsr = 1000;

    /* If unit is not bytes or the value is not numeric, just return as is */
    if ((strncmp(unit, "byte", 4) != 0) || (val_str2dbl(amt, &dbl, NULL, NULL) == FALSE))
    {
	s = (char *) malloc(strlen(amt) + strlen(unit) + 2);
	sprintf(s, "%s %s", amt, unit);
	return s;
    }

    /* Its a number, format the amount into a GB, MB or KB string */
    if (dbl == 0)
    {
	s = (char *) malloc(8);
	sprintf(s, "0 Bytes");
	return s;
    }

    qnt = 0;
    i = 0;

    for(div = (double) 1000000000; div >= divsr; div /= divsr)
    {
    	qnt = dbl / div;

    	if (qnt >= 1)
	    break;

	i++;
    }

    if (div < divsr)
    	qnt = dbl;

    /* Need to determine significant digits */
    j = 1;
    tmp = qnt;

    while(tmp > 1)
    {
    	tmp = tmp / 10;
    	j++;
    }

    s = (char *) malloc(j + 5);
    sprintf(s, "%0.2f %s", qnt, abbrev[i]);

    return s;
}


/* Return a date in yyyy-mm-dd as dd-mmm-yyyy format along with its actual time and time components */

char * format_dt(char *dt, time_t *time_rovr, struct tm **dtm)
{  
    char yyyy[5];
    char mm[3];
    char dd[3];
    char *s;
    time_t tm_t;

    /* Get a numeric time */
    strncpy(yyyy, dt, 4);
    yyyy[4] = '\0';

    mm[0] = *(dt + 5);
    mm[1] = *(dt + 6);
    mm[2] = '\0';

    dd[0] = *(dt + 8);
    dd[1] = *(dt + 9);
    dd[2] = '\0';

    tm_t = strdt2tmt(yyyy, mm, dd, "1", "0", "0");
    *dtm = localtime(&tm_t);

    /* Set the new date */
    s = (char *) malloc(12);
    strftime(s, 12, "%d-%b-%Y", *dtm);
    *time_rovr = tm_t;

    return s;
}


/* Return days remaining this period */

char * format_remdays(time_t time_rovr, double *ndays)
{  
    char *s;

    s = (char *) malloc(5);
    *ndays = difftime_days(time_rovr, time(NULL));
    sprintf(s, "%0.2f", *ndays);

    return s;
}


/* Create usage charts objects, drawing is handled in the 'draw' (OnOvExpose) event */

void create_charts(ServUsage *srv_usg, IspData *isp_data, MainUi *m_ui)
{  
    int lbl, lgd;
    double total, quota;
    char *p;
    Bar *bar;

    /* Charts need to be recreated after refresh */
    if (m_ui->pie_chart != NULL)
    	free_pie_chart(m_ui->pie_chart);

    if (m_ui->bar_chart != NULL)
    	free_bar_chart(m_ui->bar_chart);

    /* Pie Chart and slices (quota still available or excess) */
    val_str2dbl(srv_usg->total_bytes, &total, NULL, NULL);
    val_str2dbl(srv_usg->quota, &quota, NULL, NULL);

    get_user_pref(OV_PIE_LBL, &p);
    lbl = atoi(p);
    get_user_pref(OV_PIE_LGD, &p);
    lgd = atoi(p);
    //m_ui->pie_chart = pie_chart_create(NULL, 0, FALSE, NULL, 0);
    //m_ui->pie_chart = pie_chart_create(NULL, 0, TRUE, NULL, 0);
    //m_ui->pie_chart = pie_chart_create("Quota Distribution", 0, FALSE, &DARK_BLUE, 9, TRUE);
    m_ui->pie_chart = pie_chart_create("Quota Distribution", 0, lgd, &DARK_BLUE, 9, lbl);

    if (total > quota)
    {
	pie_slice_create(m_ui->pie_chart, "Quota", quota, &LIGHT_BLUE, &DARK_MAROON, 10);
	pie_slice_create(m_ui->pie_chart, "Overdrawn", (total - quota), &LIGHT_RED, &DARK_MAROON, 10);
    }
    else
    {
	pie_slice_create(m_ui->pie_chart, "Used", total, &MID_YELLOW, &DARK_MAROON, 10);
	pie_slice_create(m_ui->pie_chart, "Remaining", (quota - total), &LIGHT_BLUE, &DARK_MAROON, 10);
    }

    /* Quota interval bar chart */
    /*
    */
printf("%s quota %0.4f  rem %0.4f\n", debug_hdr, m_ui->days_quota, m_ui->days_rem); fflush(stdout);
    get_user_pref(OV_BAR_LBL, &p);
    lbl = atoi(p);
    m_ui->bar_chart = bar_chart_create("Quota Rollover", &DARK_BLUE, 9, lbl, NULL, NULL);
    //m_ui->bar_chart = bar_chart_create("Rollover Days (%)", &DARK_BLUE, 10, TRUE, NULL, NULL);
    bar = bar_create(m_ui->bar_chart);
    bar_segment_create(m_ui->bar_chart, bar, NULL, &MID_YELLOW, &DARK_MAROON, 9, 
    		       m_ui->days_quota - m_ui->days_rem);
    bar_segment_create(m_ui->bar_chart, bar, NULL, &WHITE, &DARK_MAROON, 9, m_ui->days_rem);

    return;
}
