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


/* Prototypes */

GtkWidget* calendar_main(char  *);
GtkWidget* calendar_ui(char *);
void OnCalClose(GtkWidget*, gpointer);

extern void register_window(GtkWidget *);
extern void deregister_window(GtkWidget *);


/* Globals */

static const char *debug_hdr = "DEBUG-calendar_ui.c ";


/* Display file contents */

GtkWidget* calendar_main(char *dt_in)
{
    char sel_dt;
    GtkWidget *calendar_window;  

    /* Check and open the file */
    if (dt_in == NULL)
    	cal_init(sel_dt);
    else
    	sel_dt = dt_in;

    /* Create the interface */
    calendar_window = calendar_ui(sel_dt);
    gtk_widget_show_all(calendar_window);

    /* Register the window */
    register_window(calendar_window);

    return;
}

 
/* Initial processing - get current date */

int cal_init(char *sel_dt)
{

    return TRUE;
}


/* Create the user interface and set the CallBacks */

GtkWidget* calendar_ui(char *sel_dt)
{  
    GtkWidget *calendar_window;  
    GtkWidget *mbox, *bbox, *lbox;  
    GtkWidget *label_t, *label_f;  
    GtkWidget *close_btn;  
    int rc;
    int close_hndlr_id;

    /* Set up the UI window */
    calendar_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);  
    gtk_window_set_title(GTK_WINDOW(calendar_window), VIEW_FILE_UI);
    gtk_window_set_position(GTK_WINDOW(calendar_window), GTK_WIN_POS_NONE);
    gtk_window_set_default_size(GTK_WINDOW(calendar_window), 400, 450);
    gtk_container_set_border_width(GTK_CONTAINER(calendar_window), 10);

    /* Main view */
    mbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    /* Label for file name */
    font_desc = pango_font_description_from_string ("Sans 9");

    label_t = gtk_label_new("Filename:");
    pango_font_description_set_weight(font_desc, PANGO_WEIGHT_BOLD);
    gtk_widget_override_font (GTK_WIDGET (label_t), font_desc);

    label_f = gtk_label_new(fn);
    pango_font_description_set_weight(font_desc, PANGO_WEIGHT_NORMAL);
    gtk_widget_override_color(label_f, GTK_STATE_FLAG_NORMAL, &DARK_BLUE);
    gtk_widget_override_font (GTK_WIDGET (label_f), font_desc);

    pango_font_description_free (font_desc);

    lbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start (GTK_BOX (lbox), label_t, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (lbox), label_f, FALSE, FALSE, 0);
    
    /* Scrolled window for TextView */
    scrollwin = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrollwin),
    				   GTK_POLICY_AUTOMATIC,
    				   GTK_POLICY_AUTOMATIC);

    /* Text area for file contents */
    txt_view = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(scrollwin), txt_view);
    txt_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (txt_view));
    gtk_widget_set_size_request (scrollwin, 500, 400);
    gtk_text_buffer_set_text (txt_buffer, "", -1);
    gtk_text_view_set_editable (GTK_TEXT_VIEW (txt_view), FALSE);

    /* Populate the text area */
    rc = TRUE;

    while(rc != EOF)
    {
	rc = read_file(buffer, sizeof(buffer));
	gtk_text_buffer_get_end_iter (txt_buffer, &iter);
	gtk_text_buffer_insert (txt_buffer, &iter, buffer, -1);
	gtk_text_iter_forward_to_end (&iter);
    }
    
    /* Close button */
    close_btn = gtk_button_new_with_label("  Close  ");
    g_signal_connect_swapped(close_btn, "clicked", G_CALLBACK(OnCalClose), calendar_window);
    bbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_end (GTK_BOX (bbox), close_btn, FALSE, FALSE, 0);
    gtk_widget_set_halign(GTK_WIDGET (bbox), GTK_ALIGN_CENTER);

    /* Combine everything onto the window */
    gtk_box_pack_start (GTK_BOX (mbox), lbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (mbox), bbox, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(calendar_window), mbox);  

    /* Exit when window closed */
    close_hndlr_id = g_signal_connect(calendar_window, "destroy", G_CALLBACK(OnCalClose), calendar_window);  
    g_object_set_data (G_OBJECT (calendar_window), "close_hndlr_id", GINT_TO_POINTER (close_hndlr_id));

    return calendar_window;
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
