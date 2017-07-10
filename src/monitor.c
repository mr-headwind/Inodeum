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
** Description:	Show application version, license, credits etc.
**
** Author:	Anthony Buckley
**
** History
**	10-Jul-2017	Initial code
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
#include <defs.h>
#include <version.h>


/* Types */


/* Prototypes */

void monitor_panel(MainUi *m_ui);
GtkWidget * monitor_log(MainUi *m_ui);

extern char * log_name();


/* Globals */

static const char *debug_hdr = "DEBUG-monitor.c ";



/* Application 'Monitor' current items display panel */

void monitor_panel(MainUi *m_ui)
{  
    /* Log file */
    m_ui->log_cntr = monitor_log(m_ui);

    /* Network information */
    // TODO
    
    /* Combine everything for display */
    m_ui->mon_cntr = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_name(m_ui->mon_cntr, "monitor_panel");
    gtk_widget_set_margin_top (m_ui->mon_cntr, 10);
    gtk_widget_set_margin_left (m_ui->mon_cntr, 5);

    gtk_box_pack_start (GTK_BOX (m_ui->mon_cntr), m_ui->log_cntr, FALSE, FALSE, 0);

    /* Add to the panel stack */
    gtk_stack_add_named (GTK_STACK (m_ui->panel_stk), m_ui->mon_cntr, "monitor_panel");

    return;
}


/* Log file details and a viewing option */

GtkWidget * monitor_log(MainUi *m_ui)
{  
    char *fn;
    GtkWidget *frame;
    GtkWidget *log_grid;
    GtkWidget *label_fn;
    GtkWidget *view_btn;

    /* Containers */
    frame = gtk_frame_new("Log File");
    log_grid = gtk_grid_new();

    /* Name and location */
    fn = log_name();
    label_fn = gtk_label_new(fn);
    gtk_widget_set_name(label_fn, "data_1");
    gtk_widget_set_margin_start(GTK_WIDGET (label_fn), 10);
    gtk_widget_set_halign (label_fn, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID (log_grid), label_fn, 0, 0, 1, 1);

    /* View button */
    view_btn = gtk_button_new_with_label("View");
    gtk_grid_attach(GTK_GRID (log_grid), view_btn, 0, 1, 1, 1);

    /* Pack */
    gtk_container_add(GTK_CONTAINER (frame), log_grid);

    return frame;
}
