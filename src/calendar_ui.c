/*
**  Copyright (C) 2016 Anthony Buckley
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
** Description:	Show a calendar to allow date selection
**
** Author:	Anthony Buckley
**
** History
**	26-Feb-2018	Initial code
**
*/



/* Defines */


/* Includes */
#include <stdlib.h>  
#include <string.h>  
#include <libgen.h>  
#include <gtk/gtk.h>  
#include <stdio.h>
#include <defs.h>


/* Types */

typedef struct _calendar_ui
{
    GtkWidget *window;
    GtkWidget *parent_win;
    GtkWidget *ok_btn;
    GtkWidget *cancel_btn;
    GtkWidget *btn_hbox;
    GtkWidget *main_vbox;
    GtkWidget *calendar;
    GtkWidget *dt_fld;
    const gchar *dt_txt;
    guint dd, mm, yyyy;
} CalUi;


/* Prototypes */

void calendar_main(GtkWidget *, GtkWidget *);
int cal_init(GtkWidget *, CalUi *);
CalUi * new_cal_ui();
void calendar_ui(CalUi *);
void OnCalSelect(GtkWidget*, gpointer);
void OnCalClose(GtkWidget*, gpointer);

extern void register_window(GtkWidget *);
extern void deregister_window(GtkWidget *);


/* Globals */

static const char *debug_hdr = "DEBUG-calendar_ui.c ";


/* Display calendar for date selection */

void calendar_main(GtkWidget *dt_fld, GtkWidget *parent_win)
{
    GtkWidget *calendar_win;  
    CalUi *c_ui;

    if (dt_fld == NULL)
    	return;

    c_ui = new_cal_ui();
    cal_init(dt_fld, c_ui);

    /* Create the interface and register */
    c_ui->parent_win = parent_win;
    calendar_ui(c_ui);
    register_window(c_ui->window);

    return;
}

 
/* Initial processing - get passed date if any */

int cal_init(GtkWidget *dt_fld, CalUi *c_ui)
{
    c_ui->dt_txt = gtk_entry_get_text (GTK_ENTRY(dt_fld));
    c_ui->dt_fld = dt_fld;

    return TRUE;
}


/* Create new screen data variable */

CalUi * new_cal_ui()
{
    CalUi *c_ui = (CalUi *) malloc(sizeof(CalUi));
    memset(c_ui, 0, sizeof(CalUi));

    return c_ui;
}


/* Create the user interface and set the CallBacks */

void calendar_ui(CalUi *c_ui)
{  
    int close_hndlr_id;

    /* Set up the UI window */
    c_ui->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);  
    gtk_window_set_title(GTK_WINDOW(c_ui->window), CALENDAR_UI);
    gtk_window_set_position(GTK_WINDOW(c_ui->window), GTK_WIN_POS_NONE);
    gtk_container_set_border_width(GTK_CONTAINER(c_ui->window), 10);
    gtk_window_set_resizable (GTK_WINDOW (c_ui->window), FALSE);

    /* Main view */
    c_ui->main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    /* Calendar */
    c_ui->calendar = gtk_calendar_new();
    g_signal_connect(c_ui->calendar, "day-selected-double-click", G_CALLBACK(OnCalSelect), c_ui);
    
    /* Buttons */
    c_ui->ok_btn = gtk_button_new_with_label("  OK  ");
    g_signal_connect(c_ui->ok_btn, "clicked", G_CALLBACK(OnCalSelect), c_ui);
    c_ui->cancel_btn = gtk_button_new_with_label("Cancel");
    g_signal_connect_swapped(c_ui->cancel_btn, "clicked", G_CALLBACK(OnCalClose), c_ui->window);

    c_ui->btn_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start (GTK_BOX (c_ui->btn_hbox), c_ui->ok_btn, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (c_ui->btn_hbox), c_ui->cancel_btn, FALSE, FALSE, 0);
    gtk_widget_set_halign (c_ui->btn_hbox, GTK_ALIGN_CENTER);

    /* Combine everything onto the window */
    gtk_box_pack_start (GTK_BOX (c_ui->main_vbox), c_ui->calendar, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (c_ui->main_vbox), c_ui->btn_hbox, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(c_ui->window), c_ui->main_vbox);  

    /* Exit when window closed */
    close_hndlr_id = g_signal_connect(c_ui->window, "destroy", G_CALLBACK(OnCalClose), c_ui->window);  
    g_object_set_data (G_OBJECT (c_ui->window), "close_hndlr_id", GINT_TO_POINTER (close_hndlr_id));

    /* Show window */
    gtk_window_set_transient_for (GTK_WINDOW(c_ui->window), GTK_WINDOW(c_ui->parent_win));
    //gtk_window_set_position(GTK_WINDOW(c_ui->window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_widget_show_all(c_ui->window);
    gtk_window_set_modal (GTK_WINDOW(c_ui->window), TRUE);


    return;
}


/* Set the date field and close the window */

void OnCalSelect(GtkWidget *btn, gpointer user_data)
{ 
    CalUi *c_ui;
    char s[11];

    c_ui = (CalUi *) user_data;

    gtk_calendar_get_date (GTK_CALENDAR (c_ui->calendar), &(c_ui->yyyy), &(c_ui->mm), &(c_ui->dd));
    sprintf(s, "%02u-%02u-%02u", c_ui->yyyy, c_ui->mm, c_ui->dd);
    gtk_entry_set_text (GTK_ENTRY(c_ui->dt_fld), s);

    OnCalClose(c_ui->window, btn);

    return;
}


// Callback for window close
// Destroy the window and de-register the window 

void OnCalClose(GtkWidget *window, gpointer user_data)
{ 
    int close_hndlr_id;

    close_hndlr_id = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (window), "close_hndlr_id"));
    g_signal_handler_block (window, close_hndlr_id);
    deregister_window(window);
    gtk_window_close(GTK_WINDOW(window));

    return;
}
