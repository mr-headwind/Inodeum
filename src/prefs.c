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
** Description: Preferences user interface and management.
**
** Author:	Anthony Buckley
**
** History
**	8-May-2017	Initial code
**
*/


/* Includes */

#include <gtk/gtk.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <main.h>


/* Defines */


/* Types */


/* Prototypes */

int get_user_pref(char *, char **);

extern void set_panel_btn(GtkWidget *, char *, GtkWidget *, int, int, int, int);
extern void OnResetPW(GtkWidget*, gpointer);


/* Globals */

static const char *debug_hdr = "DEBUG-prefs.c ";



/* Create widgets for the preference panel */

void pref_panel(MainUi *m_ui)
{  
    /* Create preference container grid */
    m_ui->pref_cntr = gtk_grid_new();
    gtk_widget_set_name(m_ui->pref_cntr, "pref_panel");
    gtk_grid_set_row_spacing(GTK_GRID (m_ui->pref_cntr), 2);
    gtk_grid_set_column_spacing(GTK_GRID (m_ui->pref_cntr), 2);
    gtk_container_set_border_width (GTK_CONTAINER (m_ui->pref_cntr), 2);
    gtk_widget_set_margin_top (m_ui->pref_cntr, 5);
    gtk_widget_set_margin_left (m_ui->pref_cntr, 15);

    /* Delete saved password */
    m_ui->reset_pw_btn = gtk_button_new_with_label("Delete Saved Password");
    set_panel_btn(m_ui->reset_pw_btn, "reset_pw_btn",  m_ui->pref_cntr, 0, 0, 1, 1);
    g_signal_connect (m_ui->reset_pw_btn, "clicked", G_CALLBACK (OnResetPW), m_ui);
    gtk_widget_show (m_ui->reset_pw_btn);

    /* Display perecentage on usage pie chart */

    /* Pie chart labels or legend */

    /* Display perecentage on bar chart */

    /* Add to the panel stack */
    gtk_stack_add_named (GTK_STACK (m_ui->panel_stk), m_ui->pref_cntr, "pref_panel");

    return;
}


/* Return a pointer to a user preference value for a key or NULL */

int get_user_pref(char *key, char **val)
{
    int i;

printf("%s USER PREFERENCES Not Implemented yet\n", debug_hdr);
    *val = NULL;
    i = 0;

    return i;
}
