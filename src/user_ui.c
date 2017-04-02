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
** Description: Functions to get user credentials from the Gnome keyring or,
**		if not present, a user details interface.
**
** Author:	Anthony Buckley
**
** History
**	02-Apr-2017	Initial code
**
*/



/* Defines */


/* Types */

typedef struct _user_ui
{
    GtkWidget *window;
    GtkWidget *uname_lbl;
    GtkWidget *uname_ent;
    GtkWidget *pw_lbl;
    GtkWidget *pw_ent;
    GtkWidget *ok_btn;
    GtkWidget *close_btn;
    GtkWidget *user_cntr;
    GtkWidget *btn_hbox;
    GtkWidget *main_vbox;
    int close_handler;
} UserUi;


/* Includes */
#include <stdlib.h>  
#include <string.h>  
#include <libgen.h>  
#include <gtk/gtk.h>  
#include <gdk/gdkkeysyms.h>  
#include <isp.h>
#include <defs.h>


/* Prototypes */

int user_main(IspData *, GtkWidget *);
void user_ui(IspData *, UserUi *);
UserUi * new_user_ui();
void user_control(UserUi *);
int check_user_creds(IspData *);

void OnUserOK(GtkRange*, gpointer);
void OnUserClose(GtkWidget*, gpointer);
gboolean OnUserDelete(GtkWidget*, GdkEvent *, gpointer);

extern void log_msg(char*, char*, char*, GtkWidget*);


/* Globals */

static const char *debug_hdr = "DEBUG-user_ui.c ";


/* Display and maintenance of user preferences */

int user_main(IspData *isp_data, GtkWidget *window)
{
    UserUi *ui;

    /* Initial */
    ui = new_user_ui();

    /* Create the interface */
    user_ui(isp_data, ui);

    /* Register the window */
    register_window(ui->window);

    return TRUE;
}


/* Create new screen data variable */

UserUi * new_user_ui()
{
    UserUi *ui = (UserUi *) malloc(sizeof(UserUi));
    memset(ui, 0, sizeof(UserUi));

    return ui;
}


/* Create the user interface and set the CallBacks */

void user_ui(IspData *isp_data, UserUi *u_ui)
{  
    /* Set up the UI window */
    u_ui->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);  
    gtk_window_set_title(GTK_WINDOW(u_ui->window), USER_UI);
    gtk_window_set_position(GTK_WINDOW(u_ui->window), GTK_WIN_POS_NONE);
    gtk_window_set_default_size(GTK_WINDOW(u_ui->window), 200, 100);
    gtk_container_set_border_width(GTK_CONTAINER(u_ui->window), 10);
    g_object_set_data (G_OBJECT (u_ui->window), "ui", u_ui);

    /* Main view */
    u_ui->vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(GTK_WIDGET (u_ui->vbox), GTK_ALIGN_START);

    /* Main update or view grid */
    user_control(u_ui);  LOGNAME var

    /* Box container for action buttons */
    u_ui->btn_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_halign(GTK_WIDGET (u_ui->btn_hbox), GTK_ALIGN_CENTER);

    /* Close button */
    u_ui->close_btn = gtk_button_new_with_label("  Close  ");
    g_signal_connect_swapped(u_ui->close_btn, "clicked", G_CALLBACK(OnUserClose), u_ui->window);
    gtk_box_pack_end (GTK_BOX (u_ui->btn_hbox), u_ui->close_btn, FALSE, FALSE, 0);

    /* OK button */
    u_ui->ok_btn = gtk_button_new_with_label("  OK  ");
    g_signal_connect(u_ui->ok_btn, "clicked", G_CALLBACK(OnUserOK), (gpointer) u_ui);
    gtk_box_pack_end (GTK_BOX (u_ui->btn_hbox), u_ui->ok_btn, FALSE, FALSE, 0);

    /* Combine everything onto the window */
    gtk_box_pack_start (GTK_BOX (u_ui->main_vbox), u_ui->user_cntr, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (u_ui->main_vbox), u_ui->btn_hbox, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(u_ui->window), u_ui->main_vbox);

    /* Exit when window closed */
    u_ui->close_handler = g_signal_connect(u_ui->window, "delete-event", G_CALLBACK(OnUserDelete), NULL);

    /* Show window */
    gtk_widget_show_all(u_ui->window);

    return;
}


/* Main view */

void create_main_view(IspData *isp_data, MainUi *m_ui)
{  
    /* New container for main view */
    m_ui->ctrl_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);

    /* Grid for Username & Password */
    user_details(isp_data, m_ui);

    /* Return data area */
    xml_recv_area(isp_data, m_ui);

    /* Buttons */
    ctrl_btns(m_ui);

    /* Combine everything onto the main view */
    gtk_box_pack_start (GTK_BOX (m_ui->ctrl_box), m_ui->cntl_grid, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (m_ui->ctrl_box), m_ui->scrollwin, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (m_ui->ctrl_box), m_ui->ok_btn, TRUE, TRUE, 0);

    return;
}


/* Entry for username and password */

void user_details(IspData *isp_data, MainUi *m_ui)
{  
    GtkWidget *label;
    PangoFontDescription *font_desc;

    /* Initial */
    font_desc = pango_font_description_from_string ("Sans 9");

    /* Create a grid */
    m_ui->cntl_grid = gtk_grid_new();
    gtk_widget_set_name(m_ui->cntl_grid, "ctrl_grid");
    gtk_grid_set_row_spacing(GTK_GRID (m_ui->cntl_grid), 3);
    gtk_grid_set_column_spacing(GTK_GRID (m_ui->cntl_grid), 3);
    gtk_container_set_border_width (GTK_CONTAINER (m_ui->cntl_grid), 3);

    /* Add user and password fields with labels */
    create_label("Username", 1, 1, &(m_ui->cntl_grid), &font_desc);
    create_entry(&(m_ui->uname_ent), "uname", 2, 1, &(m_ui->cntl_grid), &font_desc);

    create_label("Password", 1, 2, &(m_ui->cntl_grid), &font_desc);
    create_entry(&(m_ui->pw_ent), "pw", 2, 2, &(m_ui->cntl_grid), &font_desc);
    gtk_entry_set_visibility (GTK_ENTRY (m_ui->pw_ent), FALSE);

    /* Clean up */
    pango_font_description_free (font_desc);

    return;
}


/* Create standard entry */

void create_entry(GtkWidget **ent, char *nm, 
		    int col, int row, GtkWidget **cntr, 
		    PangoFontDescription **pf)
{  
    GtkWidget *lbl;

    pango_font_description_set_weight(*pf, PANGO_WEIGHT_NORMAL);
    *ent = gtk_entry_new();  
    gtk_widget_set_name(*ent, nm);
    gtk_entry_set_max_length (GTK_ENTRY (*ent), 32);
    gtk_entry_set_width_chars (GTK_ENTRY (*ent), 15);
    gtk_widget_override_font (*ent, *pf);

    gtk_widget_set_valign(GTK_WIDGET (*ent), GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID (*cntr), *ent, col, row, 1, 1);

    return;
}


/* Create standard label */

void create_label(char *lbl_txt, 
		    int col, int row, GtkWidget **cntr, 
		    PangoFontDescription **pf)
{  
    GtkWidget *lbl;

    pango_font_description_set_weight(*pf, PANGO_WEIGHT_BOLD);
    lbl = gtk_label_new(lbl_txt);  
    gtk_widget_override_font (lbl, *pf);

    gtk_widget_set_valign(GTK_WIDGET (lbl), GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID (*cntr), lbl, col, row, 1, 1);

    return;
}


/* OK button */

void ctrl_btns(MainUi *m_ui)
{  
    PangoFontDescription *font_desc;

    /* Initial */
    font_desc = pango_font_description_from_string ("Sans 9");
    pango_font_description_set_weight(font_desc, PANGO_WEIGHT_NORMAL);

    m_ui->ok_btn = gtk_button_new_with_label("   OK   ");  
    gtk_widget_set_name(m_ui->ok_btn, "ok");
    gtk_widget_override_font (m_ui->ok_btn, font_desc);
    gtk_widget_set_halign(GTK_WIDGET (m_ui->ok_btn), GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top (GTK_WIDGET (m_ui->ok_btn), 10);

    /* Callbacks */
    g_signal_connect (m_ui->ok_btn, "clicked", G_CALLBACK (OnOK), m_ui);

    /* Clean up */
    pango_font_description_free (font_desc);

    return;
}


/* Check Gnome keyring for stored user credentials */

int check_user_creds(IspData *isp_data);
{  
    printf("%s Gnome keyring storage not yet available\n", debug_hdr);
    return FALSE;

    return TRUE;
}
