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

void OnOK(GtkWidget*, gpointer);
void OnAbout(GtkWidget*, gpointer);
void OnQuit(GtkWidget*, gpointer);


extern void free_window_reg();
extern void close_open_ui();
extern int is_ui_reg(char *, int);
extern int about_main(GtkWidget *);

/* ??? */
extern void log_msg(char*, char*, char*, GtkWidget*);
extern void app_msg(char*, char*, GtkWidget*);
extern char * log_name();
extern int val_str2numb(char *, int *, char *, GtkWidget *);
extern int set_eos(MainUi *);
extern int close_ui(char *);
extern gint query_dialog(GtkWidget *, char *, char *);


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
    g_print("***** OnOK not available yet\n");

    return;
}  


/* Callback - Show About details */

void OnAbout(GtkWidget *menu_item, gpointer user_data)
{  
    MainUi *m_ui;

    /* Check if already open */
    if (is_ui_reg(ABOUT_UI, TRUE))
    	return;

    /* Get data */
    m_ui = (MainUi *) user_data;

    /* Open */
    about_main(m_ui->window);

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


