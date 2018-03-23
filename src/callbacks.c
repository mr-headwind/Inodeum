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
** Description:	Module for (Main) Callback functions
**
** Author:	Anthony Buckley
**
** History
**	10-Jan-2017	Initial code
*/


/* Includes */

#include <stdlib.h>  
#include <string.h>  
#include <libgen.h>  
#include <gtk/gtk.h>  
#include <cairo/cairo.h>
#include <math.h>
#include <ctype.h>
#include <pthread.h>
#include <main.h>
#include <isp.h>
#include <defs.h>


/* Defines */

#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))


/* Prototypes */

void OnOverview(GtkWidget*, gpointer);
void OnService(GtkWidget*, gpointer);
void OnMonitor(GtkWidget*, gpointer);
void OnHistory(GtkWidget*, gpointer);
void OnPref(GtkWidget*, gpointer);
void OnAbout(GtkWidget*, gpointer);
void OnUserLogin(GtkWidget*, gpointer);
void OnResetPW(GtkWidget*, gpointer);
void OnPrefSave(GtkWidget*, gpointer);
void OnPrefPieLbl(GtkToggleButton*, gpointer);
void OnPrefPieLgd(GtkToggleButton*, gpointer);
void OnPrefBarLbl(GtkToggleButton*, gpointer);
void OnHistFind(GtkWidget *, gpointer);
void OnCalendar(GtkWidget *, gpointer);
int OnSetRefresh(GtkWidget*, GdkEvent *, gpointer);
void OnRefreshTxt(GtkEditable *, gchar *, gint, gpointer, gpointer);
void OnViewLog(GtkWidget*, gpointer);
gboolean OnOvExpose(GtkWidget *, cairo_t *, gpointer);
gboolean OnHistExpose(GtkWidget *, cairo_t *, gpointer);
void OnQuit(GtkWidget*, gpointer);

void OnOK(GtkWidget*, gpointer);


extern void free_window_reg();
extern void close_open_ui();
extern int is_ui_reg(char *, int);
extern int about_main(GtkWidget *);
extern int ssl_service_details(IspData *, MainUi *);
extern void user_login_main(IspData *, GtkWidget *);
extern int delete_user_creds(IspData *, MainUi *);
extern void load_history(IspData *isp_data, MainUi *m_ui);
extern void reset_history(MainUi *);
extern void log_msg(char*, char*, char*, GtkWidget*);
extern void app_msg(char*, char*, GtkWidget*);
extern void show_panel(GtkWidget *, MainUi *);
extern char * log_name();
extern GtkWidget* view_file_main(char  *);
extern int calendar_main(GtkWidget *, GtkWidget *);
extern int write_user_prefs(GtkWidget *);
extern int set_user_pref(char *, char *);
extern int draw_pie_chart(cairo_t *, PieChart *, GtkAllocation *);
extern void draw_bar_chart(cairo_t *, BarChart *, GtkAllocation *);
extern int pie_chart_title(cairo_t *, PieChart *, GtkAllocation *, GtkAlign, GtkAlign);
extern int bar_chart_title(cairo_t *, BarChart *, GtkAllocation *, GtkAlign, GtkAlign);
extern void draw_line_graph(cairo_t *, LineGraph *, GtkAllocation *);
extern int chart_title(cairo_t *, CText *, GtkAllocation *, GtkAlign, GtkAlign);
extern void show_surface_info(cairo_t *, GtkAllocation *);


/* Globals */

static const char *debug_hdr = "DEBUG-callbacks.c ";


/* Callbacks */


/* Callback - Service request */

void OnOK(GtkWidget *ok_btn, gpointer user_data)
{  
    MainUi *m_ui;
    IspData *isp_data;

    /* Get details */
    m_ui = (MainUi *) user_data;
    isp_data = g_object_get_data (G_OBJECT(m_ui->window), "isp_data");

    /* Submit a service request */
    if (ssl_service_details(isp_data, m_ui) == FALSE)
    	return;

    return;
}  


/* Callback - Usage overview */

void OnOverview(GtkWidget *btn, gpointer user_data)
{  
    MainUi *m_ui;

    /* Get details */
    m_ui = (MainUi *) user_data;

    /* Display usage overview details */
    show_panel(m_ui->oview_cntr, m_ui);

    return;
}  


/* Callback - Service details */

void OnService(GtkWidget *btn, gpointer user_data)
{  
    MainUi *m_ui;
    IspData *isp_data;

    /* Get details */
    m_ui = (MainUi *) user_data;
    isp_data = g_object_get_data (G_OBJECT(m_ui->window), "isp_data");

    /* Display service plan details */
    printf("%s Service plan not available yet\n", debug_hdr); fflush(stdout);

    return;
}  


/* Callback - Network monitor */

void OnMonitor(GtkWidget *btn, gpointer user_data)
{  
    MainUi *m_ui;
    IspData *isp_data;

    /* Get details */
    m_ui = (MainUi *) user_data;
    isp_data = g_object_get_data (G_OBJECT(m_ui->window), "isp_data");

    /* Display current network information */
    printf("%s Network monitor not complete yet\n", debug_hdr); fflush(stdout);
    show_panel(m_ui->mon_cntr, m_ui);

    return;
}  


/* Callback - History usage */

void OnHistory(GtkWidget *btn, gpointer user_data)
{  
    MainUi *m_ui;
    IspData *isp_data;

    /* Get details */
    m_ui = (MainUi *) user_data;
    isp_data = g_object_get_data (G_OBJECT(m_ui->window), "isp_data");

    /* Display usage history */
    load_history(isp_data, m_ui);
    show_panel(m_ui->hist_cntr, m_ui);

    return;
}  


/* Callback - Log file */

void OnPref(GtkWidget *btn, gpointer user_data)
{  
    MainUi *m_ui;

    /* Get details */
    m_ui = (MainUi *) user_data;

    /* Display About details */
    show_panel(m_ui->pref_cntr, m_ui);

    return;
}  


/* Callback - Show About details */

void OnAbout(GtkWidget *btn, gpointer user_data)
{  
    MainUi *m_ui;

    /* Get data */
    m_ui = (MainUi *) user_data;

    /* Display About details */
    show_panel(m_ui->about_cntr, m_ui);

    return;
}  


/* Callback - User Login */

void OnUserLogin(GtkWidget *menu_item, gpointer user_data)
{  
    MainUi *m_ui;
    IspData *isp_data;

    /* Check if already open */
    if (is_ui_reg(USER_UI, TRUE))
    	return;

    /* Get data */
    m_ui = (MainUi *) user_data;
    isp_data = g_object_get_data (G_OBJECT(m_ui->window), "isp_data");

    /* Open */
    user_login_main(isp_data, m_ui->window);

    return;
}  


/* Callback - Remove a securely saved password */

void OnResetPW(GtkWidget *menu_item, gpointer user_data)
{  
    MainUi *m_ui;
    IspData *isp_data;
    GtkWidget *dialog;
    gint response;

    /* Get data */
    m_ui = (MainUi *) user_data;
    isp_data = g_object_get_data (G_OBJECT(m_ui->window), "isp_data");

    /* Confirm and delete */
    dialog = gtk_message_dialog_new (GTK_WINDOW (m_ui->window),
				     GTK_DIALOG_MODAL,
				     GTK_MESSAGE_QUESTION,
				     GTK_BUTTONS_OK_CANCEL,
				     "This will remove your securely stored login password.\n"
				     "Please confirm.");

    response = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    if (response == GTK_RESPONSE_CANCEL)
	return;

    if (delete_user_creds(isp_data, m_ui) == FALSE)
    	log_msg("ERR0028", NULL, "ERR0028", m_ui->window);

    return;
}  


/* Callback - Save user preferences */

void OnPrefSave(GtkWidget *btn, gpointer user_data)
{  
    MainUi *m_ui;

    /* Get data */
    m_ui = (MainUi *) user_data;

    /* Write user preferences to file */
    write_user_prefs(m_ui->window);

    return;
}  


/* Callback - User preference (pie chart labels) toggled */

void OnPrefPieLbl(GtkToggleButton *rad, gpointer user_data)
{  
    MainUi *m_ui;
    char *idx;

    /* Get data */
    m_ui = (MainUi *) user_data;

    /* Ignore if not active */
    if (! gtk_toggle_button_get_active(rad))
	return;

    /* Determine which radio toggled and set the preference */
    idx = (char *) g_object_get_data (G_OBJECT(rad), "idx");
    set_user_pref(OV_PIE_LBL, idx);

    return;
}  


/* Callback - User preference (label or legend) toggled */

void OnPrefPieLgd(GtkToggleButton *rad, gpointer user_data)
{  
    MainUi *m_ui;
    char *idx;

    /* Get data */
    m_ui = (MainUi *) user_data;

    /* Ignore if not active */
    if (! gtk_toggle_button_get_active(rad))
	return;

    /* Determine which radio toggled and set the preference */
    idx = (char *) g_object_get_data (G_OBJECT(rad), "idx");
    set_user_pref(OV_PIE_LGD, idx);

    return;
}  


/* Callback - User preference (bar chart labels) toggled */

void OnPrefBarLbl(GtkToggleButton *rad, gpointer user_data)
{  
    MainUi *m_ui;
    char *idx;

    /* Get data */
    m_ui = (MainUi *) user_data;

    /* Ignore if not active */
    if (! gtk_toggle_button_get_active(rad))
	return;

    /* Determine which radio toggled and set the preference */
    idx = (char *) g_object_get_data (G_OBJECT(rad), "idx");
    set_user_pref(OV_BAR_LBL, idx);

    return;
}  


/* Callback - Refined history search */

void OnHistFind(GtkWidget *btn, gpointer user_data)
{  
    MainUi *m_ui;

    /* Get data */
    m_ui = (MainUi *) user_data;

    /* Reset history for new criteria */
    reset_history(m_ui);

    return;
}


/* Callback - Refined history search */

void OnCalendar(GtkWidget *btn, gpointer user_data)
{  
    int r;
    MainUi *m_ui;
    const gchar *nm;
    GtkWidget *dt_fld;

    /* Get data */
    m_ui = (MainUi *) user_data;

    /* Set active field for date */
    nm = gtk_widget_get_name (btn);

    if (strcmp(nm, "fr_btn") == 0)
	dt_fld = m_ui->hist_from_dt;

    else if (strcmp(nm, "to_btn") == 0)
	dt_fld = m_ui->hist_to_dt;
    else
	return;

    /* Check if already open */
    if (is_ui_reg(CALENDAR_UI, TRUE))
    	return;

    /* Open a calendar date selection 'popup' */
    r = calendar_main(dt_fld, m_ui->window);

    switch(r)
    {
    	case 0:
	    app_msg("ERR0046", NULL, m_ui->window);
	    break;
    	case -1:
	    sprintf(app_msg_extra, "Unsupported date template.");
	    app_msg("ERR0045", NULL, m_ui->window);
	    break;
    	case -2:
	    sprintf(app_msg_extra, "Invalid year.");
	    app_msg("ERR0045", NULL, m_ui->window);
	    break;
    	case -3:
	    sprintf(app_msg_extra, "Invalid month.");
	    app_msg("ERR0045", NULL, m_ui->window);
	    break;
    	case -4:
	    sprintf(app_msg_extra, "Invalid 3 letter month abbreviation.");
	    app_msg("ERR0045", NULL, m_ui->window);
	    break;
    	case -5:
	    sprintf(app_msg_extra, "Invalid day.");
	    app_msg("ERR0045", NULL, m_ui->window);
	    break;
    	case 1:
    	default:
	    break;
    }

    return;
}


/* Callback - User preference (Data Refresh) set */

int OnSetRefresh(GtkWidget *refresh_tm, GdkEvent *ev, gpointer user_data)
{  
    MainUi *m_ui;
    const char *s;

    /* Get data */
    m_ui = (MainUi *) user_data;

    /* Write user preferences to file */
    s = gtk_entry_get_text (GTK_ENTRY (refresh_tm));
    set_user_pref(REFRESH_TM, (char *) s);

    return FALSE;
}  


/* Callback - User preference (Data Refresh) text checking */

void OnRefreshTxt(GtkEditable *edit, 
		 gchar *new_txt, 
		 gint new_length, 
		 gpointer pos, 
		 gpointer user_data)
{  
    int i;
    const gchar* content;
    MainUi *m_ui;

    /* Get data */
    m_ui = (MainUi *) user_data;

    /* Check numeric content only */
    for(i = 0; i < new_length; i++)
    {
    	if (! isdigit(*(new_txt + i)))
    	{
	    log_msg("ERR0029", new_txt, "ERR0029", m_ui->window);
	    break;
	}
    }

    return;
}  


/* Callback - View Log File details */

void OnViewLog(GtkWidget *view_log, gpointer user_data)
{  
    MainUi *m_ui;
    char *log_fn;

    /* Check if already open */
    if (is_ui_reg(VIEW_FILE_UI, TRUE))
    	return;

    /* Open */
    log_fn = log_name();
    m_ui = (MainUi *) user_data;

    if (view_file_main(log_fn) == NULL)
    	log_msg("ERR0041", log_fn, "ERR0041", m_ui->window);

    return;
}  


/* Callback - Cairo charts displaying usage information */

gboolean OnOvExpose(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{  
    MainUi *m_ui;
    GtkAllocation allocation, pseudo_alloc;

    /* Get user data, the drawing area and adjust if necessary */
    m_ui = (MainUi *) user_data;

    GdkWindow *window = gtk_widget_get_window (widget);
    gtk_widget_get_allocation (widget, &allocation);
    memcpy(&pseudo_alloc, &allocation, sizeof(allocation));

show_surface_info(cr, &allocation);	// Info or Debug

    /* Drawing area space needs to be split up for a pie chart and a bar chart */
    pseudo_alloc.width = (double) pseudo_alloc.width * 0.7;
    pseudo_alloc.x = 0;
    pseudo_alloc.y = 0;

    /* Do title (this does nothing if there is no title) */
    pie_chart_title(cr, m_ui->pie_chart, &pseudo_alloc, GTK_ALIGN_CENTER, GTK_ALIGN_START);

    /* Need to adjust y coordinate - if we used GTK_ALIGN_END (v_align) we would adjust the height */
    if (m_ui->pie_chart->title != NULL)
	pseudo_alloc.y += m_ui->pie_chart->title->ext.height;		// GTK_ALIGN_START
	//pseudo_alloc.height -= m_ui->pie_chart->title.ext.height;	// GTK_ALIGN_END

    /* Draw the pie chart */
    draw_pie_chart(cr, m_ui->pie_chart, &pseudo_alloc);

    /* Do title (this does nothing if there is no title) */
    pseudo_alloc.x = pseudo_alloc.width;
    pseudo_alloc.y = 0;
    pseudo_alloc.width = allocation.width - pseudo_alloc.x;
    bar_chart_title(cr, m_ui->bar_chart, &pseudo_alloc, GTK_ALIGN_CENTER, GTK_ALIGN_START);

    /* Need to adjust y coordinate - if we used GTK_ALIGN_END (v_align) we would adjust the height */
    if (m_ui->bar_chart->title != NULL)
	pseudo_alloc.y += m_ui->bar_chart->title->ext.height;		// GTK_ALIGN_START
	//pseudo_alloc.height -= m_ui->bar_chart->title.ext.height;	// GTK_ALIGN_END

    /* Draw the bar chart */
    draw_bar_chart(cr, m_ui->bar_chart, &pseudo_alloc);

/*
cairo_set_source_rgba (cr, 0.57, 0.24, 0.24, 0.7);
cairo_rectangle (cr, 0, 0, allocation.width, allocation.height);
cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
cairo_paint (cr);
*/


//cairo_move_to (cr, allocation.x, allocation.y);
/*
cairo_move_to (cr, 0, 0);
cairo_rectangle (cr, 0, 0, allocation.width, allocation.height);
cairo_line_to (cr, allocation.width, allocation.height);
printf("%s OnExpose 2  x %d y %d w %d h %d\n", debug_hdr,
    allocation.x, allocation.y, allocation.width, allocation.height); fflush(stdout);
cairo_set_line_width (cr, 5.0);
cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
cairo_stroke (cr);
*/
/*
cairo_move_to (cr, allocation.x, allocation.y);
cairo_rel_line_to (cr, allocation.width, 0);
cairo_rel_line_to (cr, 0, allocation.height);
cairo_rel_line_to (cr, -allocation.width, 0);
cairo_close_path (cr);
cairo_stroke (cr);
    return TRUE;
*/

    /* Draw arc */
    //const double M_PI = 3.14159265;
/*
    double xc = 80.0;
    double yc = 55.0;
    double radius = 55.0;
    double angle1 = 45.0  * (M_PI/180.0);  /* angles are specified **
    double angle2 = 180.0 * (M_PI/180.0);  /* in radians           **

    cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
    cairo_set_line_width (cr, 2.0);
    cairo_arc (cr, xc, yc, radius, angle1, angle2);
    cairo_line_to (cr, xc, yc);
    cairo_fill (cr);
    cairo_stroke (cr);

    cairo_set_font_size (cr, 10);
    cairo_set_source_rgba (cr, 0, 0, 0, 0.5);
    double lbl_angle = ((angle1 + angle2) / 2);
    //double label_x = yc * (1 + 0.1 * cos (lbl_angle));
    //double label_y = yc * (1 + 0.6 * sin (lbl_angle));
    //double label_x = allocation.height / 2 * (1 + 0.7 * cos (lbl_angle));
    //double label_y = allocation.height / 2 * (1 + 0.7 * sin (lbl_angle));
    double label_x = radius * (1 + 0.2 * cos (lbl_angle));
    double label_y = radius * (1 + 0.7 * sin (lbl_angle));
printf("%s OnExpose 3  angle %f cos %f sin %f\n", 
debug_hdr, lbl_angle, cos(lbl_angle), sin(lbl_angle)); fflush(stdout);
    cairo_move_to (cr, label_x, label_y);
    cairo_show_text (cr, "90%");
    cairo_fill (cr);
*/


    /* draw helping lines */
    //cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
    //cairo_set_line_width (cr, 2.0);

    //cairo_arc (cr, xc, yc, 10.0, 0, 2*M_PI);
    //cairo_fill (cr);

    /*
    cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
    cairo_arc (cr, xc, yc, radius, angle1, angle1);
    cairo_line_to (cr, xc, yc);
    cairo_arc (cr, xc, yc, radius, angle2, angle2);
    cairo_line_to (cr, xc, yc);
    cairo_stroke (cr);
    */

printf("%s OnExpose 9\n", debug_hdr); fflush(stdout);
    /* Draw */
    //cairo_paint (cr);
    //cairo_destroy (cr);

    return TRUE;
}


/* Callback - Cairo charts displaying usage history information */

gboolean OnHistExpose(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{  
    MainUi *m_ui;
    GtkAllocation allocation, pseudo_alloc;

    /* Get user data, the drawing area and adjust if necessary */
    m_ui = (MainUi *) user_data;

    // Drawing area does not require adjustment for history, 
    // but use a 'pseudo' allocation anyway to keep to a standard approach
    GdkWindow *window = gtk_widget_get_window (widget);
    gtk_widget_get_allocation (widget, &allocation);
    memcpy(&pseudo_alloc, &allocation, sizeof(allocation));
printf("%s OnHistExpose 1\n", debug_hdr); fflush(stdout);
show_surface_info(cr, &allocation);	// Info or Debug

    pseudo_alloc.x = 0;
    pseudo_alloc.y = 0;

    /* Do title (this does nothing if there is no title) */
    chart_title(cr, m_ui->hist_usg_graph->title, &pseudo_alloc, GTK_ALIGN_CENTER, GTK_ALIGN_START);

    /* Draw the history graph */
    draw_line_graph(cr, m_ui->hist_usg_graph, &pseudo_alloc);

    return TRUE;
}




/* Callback - Quit */

void OnQuit(GtkWidget *w, gpointer user_data)
{  
    GtkWidget *window;
    MainUi *m_ui;

    /* Do some clean up */
    window = (GtkWidget *) user_data;
    m_ui = g_object_get_data (G_OBJECT(window), "ui");
    pthread_cancel(m_ui->RefTmr.refresh_tid);
    g_source_remove (m_ui->RefTmr.tmr_id);

    /* Close any open windows */
    close_open_ui();
    free_window_reg();

    /* Main quit */
    gtk_main_quit();

    return;
}  


/* CALLBACK other functions */


