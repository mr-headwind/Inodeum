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
#include <main.h>
#include <isp.h>
#include <defs.h>


/* Defines */

#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))


/* Prototypes */

void OnOverview(GtkWidget*, gpointer);
gboolean OnExpose(GtkWidget *, cairo_t *, gpointer);
void OnService(GtkWidget*, gpointer);
void OnMonitor(GtkWidget*, gpointer);
void OnHistory(GtkWidget*, gpointer);
void OnLog(GtkWidget*, gpointer);
void OnAbout(GtkWidget*, gpointer);
void OnUserLogin(GtkWidget*, gpointer);
void OnResetPW(GtkWidget*, gpointer);
void OnViewLog(GtkWidget*, gpointer);
void OnQuit(GtkWidget*, gpointer);

void OnOK(GtkWidget*, gpointer);


extern void free_window_reg();
extern void close_open_ui();
extern int is_ui_reg(char *, int);
extern int about_main(GtkWidget *);
extern int ssl_service_details(IspData *, MainUi *);
extern void user_login_main(IspData *, GtkWidget *);
extern int delete_user_creds(IspData *, MainUi *);
extern void display_overview(IspData *isp_data, MainUi *m_ui);
extern void log_msg(char*, char*, char*, GtkWidget*);
extern void show_panel(GtkWidget *, MainUi *);
extern char * log_name();
extern GtkWidget* view_file_main(char  *);


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
    printf("%s History not available yet\n", debug_hdr); fflush(stdout);

    return;
}  


/* Callback - Log file */

void OnLog(GtkWidget *btn, gpointer user_data)
{  
    MainUi *m_ui;
    IspData *isp_data;

    /* Get details */
    m_ui = (MainUi *) user_data;
    isp_data = g_object_get_data (G_OBJECT(m_ui->window), "isp_data");

    /* Display log file contents */
    printf("%s Log not available yet\n", debug_hdr); fflush(stdout);

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


/* Callback - Reset Night Vision (only applies when switched on) */

gboolean OnExpose(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{  
    MainUi *m_ui;
    GtkAllocation allocation;

    /* Get data */
printf("%s OnExpose 1\n", debug_hdr); fflush(stdout);
    m_ui = (MainUi *) user_data;

    /* Initial */
    GdkWindow *window = gtk_widget_get_window (widget);
    //GdkWindow *window = gtk_widget_get_window (m_ui->graph_area);
    //cairo_t *cr;

/*
*/
gtk_widget_get_allocation (m_ui->window, &allocation);
printf("%s OnExpose 3 win  x %d y %d w %d h %d\n", debug_hdr,
    allocation.x, allocation.y, allocation.width, allocation.height); fflush(stdout);
gtk_widget_get_allocation (m_ui->sum_cntr, &allocation);
printf("%s OnExpose 3 sum  x %d y %d w %d h %d\n", debug_hdr,
    allocation.x, allocation.y, allocation.width, allocation.height); fflush(stdout);
gtk_widget_get_allocation (m_ui->oview_cntr, &allocation);
printf("%s OnExpose 3 oview  x %d y %d w %d h %d\n", debug_hdr,
    allocation.x, allocation.y, allocation.width, allocation.height); fflush(stdout);

    gtk_widget_get_allocation (widget, &allocation);

cairo_set_source_rgba (cr, 0.57, 0.24, 0.24, 0.7);
cairo_rectangle (cr, 0, 0, allocation.width, allocation.height);
cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
cairo_paint (cr);


    //gtk_widget_get_allocation (m_ui->graph_area, &allocation);
    //cr = gdk_cairo_create (window);

    /* Redraw to original */
    //gtk_widget_draw (m_ui->window, cr);

    //cairo_rectangle (cr, 0, 0, allocation.width, allocation.height);
    //cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
//cairo_move_to (cr, allocation.x, allocation.y);
cairo_move_to (cr, 0, 0);
cairo_rectangle (cr, 0, 0, allocation.width, allocation.height);
cairo_line_to (cr, allocation.width, allocation.height);
printf("%s OnExpose 2  x %d y %d w %d h %d\n", debug_hdr,
    allocation.x, allocation.y, allocation.width, allocation.height); fflush(stdout);
cairo_set_line_width (cr, 5.0);
cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
cairo_stroke (cr);
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
    const double M_PI = 3.14159265;
    double xc = 80.0;
    double yc = 55.0;
    double radius = 55.0;
    double angle1 = 45.0  * (M_PI/180.0);  /* angles are specified */
    double angle2 = 180.0 * (M_PI/180.0);  /* in radians           */

    cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
    cairo_set_line_width (cr, 2.0);
    cairo_arc (cr, xc, yc, radius, angle1, angle2);
    cairo_fill (cr);
    cairo_stroke (cr);

    /* draw helping lines */
    cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
    cairo_set_line_width (cr, 2.0);

    //cairo_arc (cr, xc, yc, 10.0, 0, 2*M_PI);
    //cairo_fill (cr);

    cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
    cairo_arc (cr, xc, yc, radius, angle1, angle1);
    cairo_line_to (cr, xc, yc);
    cairo_arc (cr, xc, yc, radius, angle2, angle2);
    cairo_line_to (cr, xc, yc);
    cairo_stroke (cr);

printf("%s OnExpose 9\n", debug_hdr); fflush(stdout);
    /* Draw */
    //cairo_paint (cr);
    //cairo_destroy (cr);

    return TRUE;
}



/* Callback - Quit */

void OnQuit(GtkWidget *window, gpointer user_data)
{  
    /* Close any open windows */
    close_open_ui();
    free_window_reg();

    /* Main quit */
    gtk_main_quit();

    return;
}  


/* CALLBACK other functions */


