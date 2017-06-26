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
void OnService(GtkWidget*, gpointer);
void OnMonitor(GtkWidget*, gpointer);
void OnHistory(GtkWidget*, gpointer);
void OnLog(GtkWidget*, gpointer);
void OnAbout(GtkWidget*, gpointer);
void OnUserLogin(GtkWidget*, gpointer);
void OnResetPW(GtkWidget*, gpointer);
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
extern void show_panel(GtkWidget *, GtkWidget *);


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
    //IspData *isp_data;

    /* Get details */
    m_ui = (MainUi *) user_data;
    //isp_data = g_object_get_data (G_OBJECT(m_ui->window), "isp_data");

    /* Display usage overview details */
    show_panel(m_ui->oview_cntr, m_ui->curr_panel);
    //display_overview(isp_data, m_ui);

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
    printf("%s Network monitor not available yet\n", debug_hdr); fflush(stdout);

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
    show_panel(m_ui->about_cntr, m_ui->curr_panel);

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


