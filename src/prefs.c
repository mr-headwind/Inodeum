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
GtkWidget * reset_pw(MainUi *);
GtkWidget * pie_chart_prefs(MainUi *);
GtkWidget * bar_chart_prefs(MainUi *);

extern void set_panel_btn(GtkWidget *, char *, GtkWidget *, int, int, int, int);
extern void create_label(GtkWidget **, char *, char *, GtkWidget *, int, int, int, int);
extern void create_radio(GtkWidget **, GtkWidget *, char *, char *, GtkWidget *, int, int, int, int, int);
extern void OnResetPW(GtkWidget*, gpointer);


/* Globals */

static const char *debug_hdr = "DEBUG-prefs.c ";



/* Create widgets for the preference panel */

void pref_panel(MainUi *m_ui)
{  
    /* Create preference container */
    m_ui->pref_cntr = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_name(m_ui->pref_cntr, "pref_panel");
    gtk_widget_set_margin_top (m_ui->pref_cntr, 10);
    gtk_widget_set_margin_left (m_ui->pref_cntr, 5);

    /* Delete saved password */
    m_ui->pw_cntr = reset_pw(m_ui);
    gtk_box_pack_start (GTK_BOX (m_ui->pref_cntr), m_ui->pw_cntr, FALSE, FALSE, 0);

    /* Overview charts */
    m_ui->pie_chart_cntr = pie_chart_prefs(m_ui);
    gtk_box_pack_start (GTK_BOX (m_ui->pref_cntr), m_ui->pie_chart_cntr, FALSE, FALSE, 0);

    /* Add to the panel stack */
    gtk_stack_add_named (GTK_STACK (m_ui->panel_stk), m_ui->pref_cntr, "pref_panel");

    return;
}


/* Delete saved password on keyring */

GtkWidget * reset_pw(MainUi *m_ui)
{
    GtkWidget *frame;

    /* Containers */
    frame = gtk_frame_new("Reset Password");

    /* Reset button */
    m_ui->reset_pw_btn = gtk_button_new_with_label("Delete Saved Password");
    gtk_widget_set_halign(m_ui->reset_pw_btn, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(m_ui->reset_pw_btn, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(m_ui->reset_pw_btn, 5);
    gtk_widget_set_margin_bottom(m_ui->reset_pw_btn, 5);
    gtk_container_add(GTK_CONTAINER (frame), m_ui->reset_pw_btn);
    g_signal_connect (m_ui->reset_pw_btn, "clicked", G_CALLBACK (OnResetPW), m_ui);
    gtk_widget_show (m_ui->reset_pw_btn);

    return frame;
}


/* Preferences for overview panel pie chart */

GtkWidget * pie_chart_prefs(MainUi *m_ui)
{
    GtkWidget *frame;
    GtkWidget *grid;
    GtkWidget *lbl;
    GtkWidget *radio, *radio_grp;

    /* Containers */
    frame = gtk_frame_new("Overview Pie Chart");
    grid = gtk_grid_new();

    /* Label */
    create_label(&(lbl), "usg_lbl", "Display Text", grid, 0, 0, 1, 1);
    gtk_widget_set_halign(lbl, GTK_ALIGN_END);
    gtk_widget_set_margin_start(lbl, 15);
    gtk_widget_set_margin_end(lbl, 10);

    /* Set label and percentage options */
    create_radio(&radio, NULL, "Labels", "rad_1", grid, FALSE, 1, 0, 1,1);
    radio_grp = radio;
    gtk_widget_set_margin_top (radio, 5);
    create_radio(&radio, radio_grp, "Percentage", "rad_1", grid, FALSE, 1, 1, 1,1);
    create_radio(&radio, radio_grp, "Both", "rad_1", grid, TRUE, 1, 2, 1,1);

    /* Label */
    create_label(&(lbl), "typ_lbl", "Labels", grid, 0, 3, 1, 1);
    gtk_widget_set_halign(lbl, GTK_ALIGN_END);
    gtk_widget_set_margin_start(lbl, 15);css.c
    gtk_widget_set_margin_end(lbl, 10);

    /* Set legend options */
    create_radio(&radio, NULL, "On Chart", "rad_1", grid, FALSE, 1, 3, 1,1);
    radio_grp = radio;
    gtk_widget_set_margin_top (radio, 5);
    create_radio(&radio, radio_grp, "Legend", "rad_1", grid, FALSE, 2, 3, 1,1);
    gtk_widget_set_margin_top (radio, 5);

    gtk_container_add(GTK_CONTAINER (frame), grid);

    return frame;
}


/* Preferences for overview panel bar chart */

GtkWidget * bar_chart_prefs(MainUi *m_ui)
{
    GtkWidget *frame;
    GtkWidget *grid;
    GtkWidget *lbl;
    GtkWidget *radio, *radio_grp;

    /* Containers */
    frame = gtk_frame_new("Overview Pie Chart");
    grid = gtk_grid_new();

    /* Label */
    create_label(&(lbl), "usg_lbl", "Display Text", grid, 0, 0, 1, 1);
    gtk_widget_set_halign(lbl, GTK_ALIGN_END);
    gtk_widget_set_margin_start(lbl, 15);
    gtk_widget_set_margin_end(lbl, 10);

    /* Set label and percentage options */
    create_radio(&radio, NULL, "Labels", "rad_1", grid, FALSE, 1, 0, 1,1);
    radio_grp = radio;
    gtk_widget_set_margin_top (radio, 5);
    create_radio(&radio, radio_grp, "Percentage", "rad_1", grid, FALSE, 1, 1, 1,1);
    create_radio(&radio, radio_grp, "Both", "rad_1", grid, TRUE, 1, 2, 1,1);

    /* Label */
    create_label(&(lbl), "typ_lbl", "Labels", grid, 0, 3, 1, 1);
    gtk_widget_set_halign(lbl, GTK_ALIGN_END);
    gtk_widget_set_margin_start(lbl, 15);css.c
    gtk_widget_set_margin_end(lbl, 10);

    /* Set legend options */
    create_radio(&radio, NULL, "On Chart", "rad_1", grid, FALSE, 1, 3, 1,1);
    radio_grp = radio;
    gtk_widget_set_margin_top (radio, 5);
    create_radio(&radio, radio_grp, "Legend", "rad_1", grid, FALSE, 2, 3, 1,1);
    gtk_widget_set_margin_top (radio, 5);

    gtk_container_add(GTK_CONTAINER (frame), grid);

    /* Containers */
    frame = gtk_frame_new("Overview Bar Chart");
    grid = gtk_grid_new();

    /* Label */
    create_label(&(lbl), "usg_lbl", "Display Text", grid, 0, 0, 1, 1);
    gtk_widget_set_halign(lbl, GTK_ALIGN_END);
    gtk_widget_set_margin_start(lbl, 15);
    gtk_widget_set_margin_end(lbl, 10);

    /* Set label and percentage options */
    create_radio(&radio, NULL, "Labels", "rad_1", grid, FALSE, 1, 0, 1,1);
    radio_grp = radio;
    gtk_widget_set_margin_top (radio, 5);
    create_radio(&radio, radio_grp, "Percentage", "rad_1", grid, TRUE, 1, 1, 1,1);
    create_radio(&radio, radio_grp, "Both", "rad_1", grid, FALSE, 1, 2, 1,1);

    gtk_container_add(GTK_CONTAINER (frame), grid);

    return frame;
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
