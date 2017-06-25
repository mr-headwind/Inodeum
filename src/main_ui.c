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
** Description: Functions to create the main user interface for the application.
**
** Author:	Anthony Buckley
**
** History
**	09-Jan-2017	Initial code
**
*/



/* Defines */
#define MAIN_UI


/* Includes */
#include <stdlib.h>  
#include <string.h>  
#include <libgen.h>  
#include <gtk/gtk.h>  
#include <gdk/gdkkeysyms.h>  
#include <main.h>
#include <isp.h>
#include <defs.h>


/* Prototypes */

void main_ui(IspData *, MainUi *);
void create_menu(IspData *, MainUi *);
void create_main_view(IspData *, MainUi *);
void usage_btns(MainUi *);
void set_panel_btn(GtkWidget *, char *, GtkWidget *, int, int, int, int);
void overview_panel(MainUi *);
void service_panel(MainUi *);
void monitor_panel(MainUi *);
void history_panel(MainUi *);
void log_panel(MainUi *);
void about_panel(MainUi *);
void create_label(GtkWidget **, char *, char *, GtkWidget *, int, int, int, int);
GtkWidget * debug_cntr(GtkWidget *);

void create_entry(GtkWidget **, char *, int, int, GtkWidget **);
void disable_login(MainUi *);

extern void log_msg(char*, char*, char*, GtkWidget*);
extern void user_login_main(IspData *, GtkWidget *);
extern int check_user_creds(IspData *, MainUi *);
extern int ssl_service_details(IspData *, MainUi *);
extern void display_overview(IspData *, MainUi *);
extern void set_css();

extern void OnOverview(GtkWidget*, gpointer);
extern void OnService(GtkWidget*, gpointer);
extern void OnMonitor(GtkWidget*, gpointer);
extern void OnHistory(GtkWidget*, gpointer);
extern void OnLog(GtkWidget*, gpointer);
extern void OnAbout(GtkWidget*, gpointer);
extern void OnUserLogin(GtkWidget*, gpointer);
extern void OnResetPW(GtkWidget*, gpointer);
extern void OnQuit(GtkWidget*, gpointer);

extern void OnOK(GtkRange*, gpointer);


/* Globals */

static const char *debug_hdr = "DEBUG-main_ui.c ";


/* Create the user interface and set the CallBacks */

void main_ui(IspData *isp_data, MainUi *m_ui)
{  
    int login_req, r;

    /* Set up the UI window */
    m_ui->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);  
    g_object_set_data (G_OBJECT (m_ui->window), "isp_data", isp_data);
    g_object_set_data (G_OBJECT (m_ui->window), "ui", m_ui);
    gtk_window_set_title(GTK_WINDOW(m_ui->window), TITLE);
    gtk_window_set_position(GTK_WINDOW(m_ui->window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(m_ui->window), 200, 200);
    gtk_container_set_border_width(GTK_CONTAINER(m_ui->window), 10);

    /* Main view */
    m_ui->mbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    gtk_widget_set_halign(GTK_WIDGET (m_ui->mbox), GTK_ALIGN_START);

    /* MENU */
    create_menu(isp_data, m_ui);

    /* CONTROL PANEL */
    create_main_view(isp_data, m_ui);

    /* INFORMATION AREA AT BOTTOM OF WINDOW */
    m_ui->status_info = gtk_label_new(NULL);
    gtk_widget_set_margin_top(GTK_WIDGET (m_ui->status_info), 5);
    gtk_label_set_text(GTK_LABEL (m_ui->status_info), " ");
    gtk_widget_set_halign(GTK_WIDGET (m_ui->status_info), GTK_ALIGN_START);

    /* Combine everything onto the window */
    gtk_box_pack_start (GTK_BOX (m_ui->mbox), m_ui->menu_bar, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (m_ui->mbox), m_ui->ctrl_box, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (m_ui->mbox), m_ui->status_info, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(m_ui->window), m_ui->mbox);  

    /* Exit when window closed */
    g_signal_connect(m_ui->window, "destroy", G_CALLBACK(OnQuit), m_ui->window);  

    /* Show window */
    set_css();
    gtk_widget_show_all(m_ui->window);

    /* Check user credentials from the gnome keyring */
    login_req = FALSE;

    if (check_user_creds(isp_data, m_ui) == FALSE)
    {
	/* Get user credentials and service request via user entry interface */
	login_req = TRUE;
    }
    else
    {
	/* Initiate a service request */
	r = ssl_service_details(isp_data, m_ui);
	
	if (r == -1)
	    login_req = TRUE;

	else if (r == FALSE)
	    return;
    }

    /* User login or display usage details */
    if (login_req == TRUE)
    {
    	user_login_main(isp_data, m_ui->window);
    }
    else
    {
    	disable_login(m_ui);
    	display_overview(isp_data, m_ui);
    }

    return;
}


/*
** Menu function for application.
**
**  File       Service	     Help
**   - Exit    	-Login        - About
*/

void create_menu(IspData *isp_data, MainUi *m_ui)
{
    GtkAccelGroup *accel_group = NULL;

    /* Create menubar */
    m_ui->menu_bar = gtk_menu_bar_new();


    /* FILE MENU */
    m_ui->file_menu = gtk_menu_new();

    /* File menu items */
    m_ui->file_exit = gtk_menu_item_new_with_mnemonic ("E_xit");

    /* Add to menu */
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->file_menu), m_ui->file_exit);

    /* Callbacks */
    g_signal_connect_swapped (m_ui->file_exit, "activate", G_CALLBACK (OnQuit), m_ui->window); 

    /* Show menu items */
    gtk_widget_show (m_ui->file_exit);


    /* SERVICE MENU */
    m_ui->service_menu = gtk_menu_new();

    /* Service menu items */
    m_ui->user_login = gtk_menu_item_new_with_mnemonic ("User Login...");
    m_ui->reset_pw = gtk_menu_item_new_with_mnemonic ("Delete saved password");

    /* Add to menu */
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->service_menu), m_ui->user_login);
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->service_menu), m_ui->reset_pw);

    /* Callbacks */
    g_signal_connect (m_ui->user_login, "activate", G_CALLBACK (OnUserLogin), m_ui); 
    g_signal_connect (m_ui->reset_pw, "activate", G_CALLBACK (OnResetPW), m_ui); 

    /* Show menu items */
    gtk_widget_show (m_ui->user_login);
    gtk_widget_show (m_ui->reset_pw);


    /* HELP MENU */
    m_ui->help_menu = gtk_menu_new();

    /* Option menu items */
    m_ui->help_about = gtk_menu_item_new_with_mnemonic ("About...");

    /* Add to menu */
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->help_menu), m_ui->help_about);

    /* Callbacks */
    g_signal_connect (m_ui->help_about, "activate", G_CALLBACK (OnAbout), m_ui);

    /* Show menu items */
    gtk_widget_show (m_ui->help_about);


    /* File header menu */
    m_ui->file_hdr = gtk_menu_item_new_with_mnemonic ("_File");
    gtk_widget_show (m_ui->file_hdr);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (m_ui->file_hdr), m_ui->file_menu);
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->menu_bar), m_ui->file_hdr);

    /* Service header menu */
    m_ui->service_hdr = gtk_menu_item_new_with_mnemonic ("_Service");
    gtk_widget_show (m_ui->service_hdr);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (m_ui->service_hdr), m_ui->service_menu);
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->menu_bar), m_ui->service_hdr);

    /* Help header menu */
    m_ui->help_hdr = gtk_menu_item_new_with_mnemonic ("_Help");
    gtk_widget_show (m_ui->help_hdr);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (m_ui->help_hdr), m_ui->help_menu);
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->menu_bar), m_ui->help_hdr);


    /* Accelerators */
    accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW (m_ui->window), accel_group);

    gtk_widget_add_accelerator(m_ui->file_exit, "activate", accel_group, GDK_KEY_q,
    			       GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE); 

    return;
}  


/* Main view */

void create_main_view(IspData *isp_data, MainUi *m_ui)
{  
    /* New container for main view */
    m_ui->ctrl_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);

    /* Usage button panel */
    usage_btns(m_ui);

    /* Scrolled window to attach the different panels to */
    m_ui->scrollwin = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request (m_ui->scrollwin, 300, 200);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (m_ui->scrollwin),
    				   GTK_POLICY_AUTOMATIC,
    				   GTK_POLICY_AUTOMATIC);

    /* Usage panels */
    overview_panel(m_ui);
    service_panel(m_ui);
    monitor_panel(m_ui);
    history_panel(m_ui);
    log_panel(m_ui);
    about_panel(m_ui);

    /* Combine everything onto the main view */
    gtk_box_pack_start (GTK_BOX (m_ui->ctrl_box), m_ui->btn_panel, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (m_ui->ctrl_box), m_ui->scrollwin, TRUE, TRUE, 0);

    return;
}


/* Usage monitor button (menu) panel */

void usage_btns(MainUi *m_ui)
{  
    int i, j;

    /* Create grid to contain the usage monitor function buttons */
    m_ui->btn_panel = gtk_grid_new();
    gtk_widget_set_name(m_ui->btn_panel, "btn_panel");
    gtk_grid_set_row_spacing(GTK_GRID (m_ui->btn_panel), 2);
    gtk_grid_set_column_spacing(GTK_GRID (m_ui->btn_panel), 2);
    gtk_container_set_border_width (GTK_CONTAINER (m_ui->btn_panel), 2);
    gtk_widget_set_hexpand (m_ui->btn_panel, TRUE);
    gtk_widget_set_vexpand (m_ui->btn_panel, TRUE);

    /* Create buttons */
    i = j = 0;
    m_ui->overview_btn = gtk_button_new_with_label("Overview");  
    set_panel_btn(m_ui->overview_btn, "overview_btn",  m_ui->btn_panel, i, j, 1, 1);

    i++;
    m_ui->service_btn = gtk_button_new_with_label("Service");  
    set_panel_btn(m_ui->service_btn, "service_btn",  m_ui->btn_panel, i, j, 1, 1);

    i++;
    m_ui->monitor_btn = gtk_button_new_with_label("Monitor");  
    set_panel_btn(m_ui->monitor_btn, "monitor_btn",  m_ui->btn_panel, i, j, 1, 1);

    i = 0;
    j++;
    m_ui->history_btn = gtk_button_new_with_label("History");  
    set_panel_btn(m_ui->history_btn, "history_btn",  m_ui->btn_panel, i, j, 1, 1);

    i++;
    m_ui->log_btn = gtk_button_new_with_label("Log");  
    set_panel_btn(m_ui->log_btn, "log_btn",  m_ui->btn_panel, i, j, 1, 1);

    i++;
    m_ui->about_btn = gtk_button_new_with_label("About");  
    set_panel_btn(m_ui->about_btn, "about_btn",  m_ui->btn_panel, i, j, 1, 1);

    /* Callbacks */
    g_signal_connect (m_ui->overview_btn, "clicked", G_CALLBACK (OnOverview), m_ui);
    g_signal_connect (m_ui->service_btn, "clicked", G_CALLBACK (OnService), m_ui);
    g_signal_connect (m_ui->monitor_btn, "clicked", G_CALLBACK (OnMonitor), m_ui);
    g_signal_connect (m_ui->history_btn, "clicked", G_CALLBACK (OnHistory), m_ui);
    g_signal_connect (m_ui->log_btn, "clicked", G_CALLBACK (OnLog), m_ui);
    g_signal_connect (m_ui->about_btn, "clicked", G_CALLBACK (OnAbout), m_ui);

    return;
}


/* Set a panel button in the grid */

void set_panel_btn(GtkWidget *btn, char *nm, GtkWidget *cntr,
		   int col, int row, int c_spn, int r_spn) 
{

    gtk_widget_set_name(btn, nm);
    gtk_widget_set_vexpand (btn, TRUE);
    gtk_widget_set_hexpand (btn, TRUE);
    gtk_grid_attach(GTK_GRID (cntr), btn, col, row, c_spn, r_spn);

    return;
}


/* Create widgets for the overview panel */

void overview_panel(MainUi *m_ui)
{  
    int i, j;

    /* Create container grid */
    m_ui->oview_cntr = gtk_grid_new();
    g_object_set_data (G_OBJECT (m_ui->oview_cntr), "loaded", GINT_TO_POINTER (FALSE)); // ??
    gtk_widget_set_name(m_ui->oview_cntr, "oview_panel");
    gtk_grid_set_row_spacing(GTK_GRID (m_ui->oview_cntr), 2);
    gtk_grid_set_column_spacing(GTK_GRID (m_ui->oview_cntr), 2);
    gtk_container_set_border_width (GTK_CONTAINER (m_ui->oview_cntr), 2);
    gtk_widget_set_margin_top (m_ui->oview_cntr, 15);
    gtk_widget_set_margin_left (m_ui->oview_cntr, 15);

    /* Title labels */
    i = j = 0;
    create_label(&(m_ui->quota_lbl), "quota_lbl", NULL, m_ui->oview_cntr, i, j, 1, 1);

    j++;
    create_label(&(m_ui->next_dt_lbl), "next_dt_lbl", NULL, m_ui->oview_cntr, i, j, 1, 1);

    j++;
    create_label(&(m_ui->rem_days_lbl), "rem_days_lbl", NULL, m_ui->oview_cntr, i, j, 1, 1);

    j++;
    create_label(&(m_ui->usage_lbl), "usage_lbl", NULL, m_ui->oview_cntr, i, j, 1, 1);

    /* Data labels */
    j = 0;
    i++;
    create_label(&(m_ui->quota), "data_label", NULL, m_ui->oview_cntr, i, j, 1, 1);
    gtk_widget_set_margin_left (m_ui->quota, 15);

    j++;
    create_label(&(m_ui->rollover_dt), "data_label", NULL, m_ui->oview_cntr, i, j, 1, 1);
    gtk_widget_set_margin_left (m_ui->rollover_dt, 15);

    j++;
    create_label(&(m_ui->rem_days), "data_label", NULL, m_ui->oview_cntr, i, j, 1, 1);
    gtk_widget_set_margin_left (m_ui->rem_days, 15);

    j++;
    create_label(&(m_ui->usage), "data_label", NULL, m_ui->oview_cntr, i, j, 1, 1);
    gtk_widget_set_margin_left (m_ui->usage, 15);

    /*
    m_ui->txt_view = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(m_ui->scrollwin), m_ui->txt_view);
    gtk_widget_set_name(m_ui->txt_view, "xml");
    gtk_text_view_set_editable (GTK_TEXT_VIEW (m_ui->txt_view), FALSE);
    */

    return;
}


/* Create widgets for the service panel */

void service_panel(MainUi *m_ui)
{  

    return;
}


/* Create widgets for the monitor panel */

void monitor_panel(MainUi *m_ui)
{  

    return;
}


/* Create widgets for the history panel */

void history_panel(MainUi *m_ui)
{  

    return;
}


/* Create widgets for the log panel */

void log_panel(MainUi *m_ui)
{  

    return;
}


/* Create widgets for the about panel */

void about_panel(MainUi *m_ui)
{  

    return;
}


/* Create standard label */

void create_label(GtkWidget **lbl, char *nm, char *txt, GtkWidget *cntr, 
		  int col, int row, int c_spn, int r_spn)
{  
    *lbl = gtk_label_new(txt);  
    gtk_widget_set_name(*lbl, nm);

    gtk_widget_set_halign(*lbl, GTK_ALIGN_START);
    gtk_widget_set_valign(*lbl, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top (*lbl, 5);
    gtk_grid_attach(GTK_GRID (cntr), *lbl, col, row, c_spn, r_spn);

    return;
}


/* Create standard entry */

void create_entry(GtkWidget **ent, char *nm, 
		    int col, int row, GtkWidget **cntr) 
{  
    GtkWidget *lbl;

    *ent = gtk_entry_new();  
    gtk_widget_set_name(*ent, nm);
    gtk_entry_set_max_length (GTK_ENTRY (*ent), 32);
    gtk_entry_set_width_chars (GTK_ENTRY (*ent), 15);

    gtk_widget_set_valign(GTK_WIDGET (*ent), GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID (*cntr), *ent, col, row, 1, 1);

    return;
}


/* The manual login menu should be disabled once logged in */

void disable_login(MainUi *m_ui)
{  
    gtk_widget_set_sensitive (m_ui->user_login, FALSE);

    return;
}


/* Debug widget container */

GtkWidget * debug_cntr(GtkWidget *cntr)
{
    const gchar *widget_name;
    GtkWidget *widget;
    GtkWidgetPath *w_path;

    if (! GTK_IS_CONTAINER(cntr))
    {
	log_msg("SYS9011", "btn children", NULL, NULL);
    	return NULL;
    }

    widget_name = gtk_widget_get_name (cntr);
    printf("%s widget structure for %s\n", debug_hdr, widget_name);

    GList *child_widgets = gtk_container_get_children(GTK_CONTAINER (cntr));
    //printf("%s \tno of children %d\n", debug_hdr, g_list_length(child_widgets));

    child_widgets = g_list_first(child_widgets);

    while (child_widgets != NULL)
    {
	widget = child_widgets->data;
	widget_name = gtk_widget_get_name (widget);
	printf("%s \tname %s\n", debug_hdr, widget_name);

	w_path = gtk_container_get_path_for_child (GTK_CONTAINER (cntr), widget);
	printf("%s \tpath %s\n", debug_hdr, gtk_widget_path_to_string (w_path));

	if (GTK_IS_CONTAINER(widget))
	    debug_cntr(widget);

	if (GTK_IS_LABEL (widget))
	    break;

	child_widgets = g_list_next(child_widgets);
    }

    g_list_free (child_widgets);

    return widget;
}
