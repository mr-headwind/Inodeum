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
#include <unistd.h>
#include <errno.h>
#include <libgen.h>  
#include <gtk/gtk.h>  
#include <gdk/gdkkeysyms.h>  
#include <pthread.h>
#include <main.h>
#include <isp.h>
#include <defs.h>


/* Prototypes */

void main_ui(IspData *, MainUi *);
void create_menu(IspData *, MainUi *);
void create_main_view(IspData *, MainUi *);
void usage_btns(MainUi *);
void set_panel_btn(GtkWidget *, char *, GtkWidget *, int, int, int, int);
void create_label(GtkWidget **, char *, char *, GtkWidget *, int, int, int, int);
void create_label2(GtkWidget **, char *, char *, GtkWidget *);
void create_entry(GtkWidget **, char *, GtkWidget *, int, int);
void create_radio(GtkWidget **, GtkWidget *, char *, char *, GtkWidget *, int, char *, char *);
void create_cbox(GtkWidget **, char *, const char *[], int, int, GtkWidget *, int, int);
void show_panel(GtkWidget *, MainUi *); 
void disable_login(MainUi *);
void add_main_loop(MainUi *);
gboolean refresh_main_loop_fn(gpointer);
int refresh_thread(MainUi *);
void * timer_thread(void *);
GtkWidget * debug_cntr(GtkWidget *);

extern void log_msg(char*, char*, char*, GtkWidget*);
extern void user_login_main(IspData *, GtkWidget *);
extern int check_user_creds(IspData *, MainUi *);
extern int ssl_service_details(IspData *, MainUi *);
extern void overview_panel(MainUi *);
extern void load_overview(IspData *, MainUi *);
extern void serv_plan_panel(IspData *, MainUi *);
extern void history_panel(MainUi *);
extern void pref_panel(MainUi *);
extern void about_panel(MainUi *);
extern void monitor_panel(MainUi *);
extern void init_history(MainUi *);
extern void set_css();
extern int get_user_pref(char *, char **);

extern void OnOverview(GtkWidget*, gpointer);
extern void OnService(GtkWidget*, gpointer);
extern void OnMonitor(GtkWidget*, gpointer);
extern void OnHistory(GtkWidget*, gpointer);
extern void OnPref(GtkWidget*, gpointer);
extern void OnAbout(GtkWidget*, gpointer);
extern void OnUserLogin(GtkWidget*, gpointer);
extern void OnQuit(GtkWidget*, gpointer);

extern void OnOK(GtkRange*, gpointer);


/* Globals */

static const char *debug_hdr = "DEBUG-main_ui.c ";
static const int main_loop_interval = 30;
static int ret_mon;


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
    gtk_window_set_default_size(GTK_WINDOW(m_ui->window), 300, 390);
    gtk_container_set_border_width(GTK_CONTAINER(m_ui->window), 2);

    /* Main view */
    m_ui->mbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    gtk_widget_set_halign(GTK_WIDGET (m_ui->mbox), GTK_ALIGN_START);

    /* MENU */
    create_menu(isp_data, m_ui);

    /* CONTROL PANEL */
    create_main_view(isp_data, m_ui);

    /* INFORMATION AREA AT BOTTOM OF WINDOW */
    m_ui->status_info = gtk_label_new(NULL);
    gtk_widget_set_name (GTK_WIDGET (m_ui->status_info), "status");
    gtk_widget_set_margin_top(GTK_WIDGET (m_ui->status_info), 5);
    gtk_label_set_text(GTK_LABEL (m_ui->status_info), " ");
    gtk_widget_set_halign(GTK_WIDGET (m_ui->status_info), GTK_ALIGN_START);

    /* Combine everything onto the window */
    gtk_box_pack_start (GTK_BOX (m_ui->mbox), m_ui->menu_bar, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (m_ui->mbox), m_ui->ctrl_box, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (m_ui->mbox), m_ui->status_info, FALSE, FALSE, 0);

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
    	load_overview(isp_data, m_ui);
    	show_panel(m_ui->oview_cntr, m_ui);

    	if (refresh_thread(m_ui) == TRUE)
	    add_main_loop(m_ui);
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
    g_signal_connect (m_ui->file_exit, "activate", G_CALLBACK (OnQuit), m_ui->window); 

    /* Show menu items */
    gtk_widget_show (m_ui->file_exit);


    /* SERVICE MENU */
    m_ui->service_menu = gtk_menu_new();

    /* Service menu items */
    m_ui->user_login = gtk_menu_item_new_with_mnemonic ("User Login...");

    /* Add to menu */
    gtk_menu_shell_append (GTK_MENU_SHELL (m_ui->service_menu), m_ui->user_login);

    /* Callbacks */
    g_signal_connect (m_ui->user_login, "activate", G_CALLBACK (OnUserLogin), m_ui); 

    /* Show menu items */
    gtk_widget_show (m_ui->user_login);


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

    /* Stack widget to attach the different panels to */
    m_ui->panel_stk = gtk_stack_new();
    gtk_stack_set_homogeneous(GTK_STACK (m_ui->panel_stk), TRUE);
    gtk_stack_set_transition_type (GTK_STACK (m_ui->panel_stk), GTK_STACK_TRANSITION_TYPE_NONE); 

    /* Usage panels */
    overview_panel(m_ui);
    serv_plan_panel(isp_data, m_ui);
    monitor_panel(m_ui);
    history_panel(m_ui);
    pref_panel(m_ui);
    about_panel(m_ui);

    /* Combine everything onto the main view */
    gtk_box_pack_start (GTK_BOX (m_ui->ctrl_box), m_ui->btn_panel, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (m_ui->ctrl_box), m_ui->panel_stk, TRUE, TRUE, 0);

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
    m_ui->pref_btn = gtk_button_new_with_label("Preferences");  
    set_panel_btn(m_ui->pref_btn, "pref_btn",  m_ui->btn_panel, i, j, 1, 1);

    i++;
    m_ui->about_btn = gtk_button_new_with_label("About");  
    set_panel_btn(m_ui->about_btn, "about_btn",  m_ui->btn_panel, i, j, 1, 1);

    /* Callbacks */
    g_signal_connect (m_ui->overview_btn, "clicked", G_CALLBACK (OnOverview), m_ui);
    g_signal_connect (m_ui->service_btn, "clicked", G_CALLBACK (OnService), m_ui);
    g_signal_connect (m_ui->monitor_btn, "clicked", G_CALLBACK (OnMonitor), m_ui);
    g_signal_connect (m_ui->history_btn, "clicked", G_CALLBACK (OnHistory), m_ui);
    g_signal_connect (m_ui->pref_btn, "clicked", G_CALLBACK (OnPref), m_ui);
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


/* Maintain which panel is visible */

void show_panel(GtkWidget *cntr, MainUi *m_ui) 
{
    if (cntr == m_ui->curr_panel)
    	return;

    gtk_stack_set_visible_child (GTK_STACK (m_ui->panel_stk), cntr);

    m_ui->curr_panel = cntr;

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


/* Create standard label */

void create_label2(GtkWidget **lbl, char *nm, char *txt, GtkWidget *cntr) 
{  
    *lbl = gtk_label_new(txt);  
    gtk_widget_set_name(*lbl, nm);

    gtk_widget_set_halign(*lbl, GTK_ALIGN_START);
    gtk_widget_set_valign(*lbl, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top (*lbl, 5);
    gtk_widget_set_margin_start (*lbl, 10);
    gtk_box_pack_start (GTK_BOX (cntr), *lbl, FALSE, FALSE, 0);

    return;
}


/* Create standard entry */

void create_entry(GtkWidget **ent, char *nm, GtkWidget *cntr, int col, int row) 
{  
    *ent = gtk_entry_new();  
    gtk_widget_set_name(*ent, nm);

    gtk_widget_set_valign(GTK_WIDGET (*ent), GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID (cntr), *ent, col, row, 1, 1);

    return;
}


/* Create standard radio */

void create_radio(GtkWidget **rad, GtkWidget *grp, char *txt, char *nm, GtkWidget *cntr, 
		  int active, char *obj_nm, char *obj_data_str)
{  
    if (grp == NULL)
	*rad = gtk_radio_button_new_with_label (NULL, txt);
    else
	*rad = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (grp), txt);

    gtk_widget_set_name(*rad, "rad_1");
    gtk_widget_set_halign(*rad, GTK_ALIGN_START);
    gtk_widget_set_valign(*rad, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top (*rad, 0);
    gtk_widget_set_margin_start (*rad, 0);
    gtk_box_pack_start (GTK_BOX (cntr), *rad, FALSE, FALSE, 0);

    if (active == TRUE)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (*rad), TRUE);

    if (obj_nm != NULL && obj_data_str != NULL)
	g_object_set_data_full (G_OBJECT (*rad), obj_nm, g_strdup (obj_data_str), (GDestroyNotify) g_free);

    return;
}


/* Create standard combobox */

void create_cbox(GtkWidget **cbox, char *nm, const char *arr[], int max, int active, 
		 GtkWidget *cntr, int col, int row)
{  
    int i;
    char s[max];

    *cbox = gtk_combo_box_text_new();  
    gtk_widget_set_name(*cbox, nm);

    for(i = 0; i < max; i++)
    {
    	sprintf(s, "%d", i);
    	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT(*cbox), s, arr[i]);
    }

    gtk_combo_box_set_active (GTK_COMBO_BOX(*cbox), active);

    gtk_widget_set_halign(*cbox, GTK_ALIGN_START);
    gtk_widget_set_valign(*cbox, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top (*cbox, 0);
    gtk_grid_attach(GTK_GRID (cntr), *cbox, col, row, 1, 1);

    return;
}


/* The manual login menu should be disabled once logged in */

void disable_login(MainUi *m_ui)
{  
    gtk_widget_set_sensitive (m_ui->user_login, FALSE);

    if (m_ui->user_cd == FALSE)
    	gtk_widget_set_sensitive (m_ui->reset_pw_btn, FALSE);


    return;
}


/* Inject a main loop timer */

void add_main_loop(MainUi *m_ui)
{  
    m_ui->RefTmr.tmr_id = g_timeout_add_seconds(main_loop_interval, refresh_main_loop_fn, m_ui);

    return;
}


/* Main loop timer function to check for data refresh */

gboolean refresh_main_loop_fn(gpointer user_data)
{
    int r;
    MainUi *m_ui;
    IspData *isp_data;
    RefreshTmr *ref_tmr;

    /* Initial */
    m_ui = (MainUi *) user_data;
    ref_tmr = &(m_ui->RefTmr);
    isp_data = (IspData *) g_object_get_data (G_OBJECT (m_ui->window), "isp_data");

    gtk_label_set_text (GTK_LABEL (m_ui->status_info), ref_tmr->info_txt);
    gtk_widget_show (m_ui->status_info);

    if (ref_tmr->refresh_req == FALSE)
    	return TRUE;

    /* Reset usage data */
    if (ssl_service_details(isp_data, m_ui) != TRUE)
    	return FALSE;

    init_history(m_ui);
    load_overview(isp_data, m_ui);
    refresh_thread(m_ui);

    return TRUE;
}


/* Start the refresh timer thread and set the start time */

int refresh_thread(MainUi *m_ui)
{  
    int p_err;
    char *p;

    /* Initial timer setup */
    m_ui->RefTmr.refresh_req = FALSE;
    m_ui->RefTmr.start_t = time(NULL);
    get_user_pref(REFRESH_TM, &p);
    m_ui->RefTmr.ref_interval = atol(p) * 60;

    /* Start thread */
    if ((p_err = pthread_create(&(m_ui->RefTmr.refresh_tid), NULL, &timer_thread, (void *) m_ui)) != 0)
    {
	sprintf(app_msg_extra, "Error: %s", strerror(p_err));
	log_msg("ERR0044", NULL, "ERR0044", m_ui->window);
	return FALSE;
    }

    return TRUE;
}


/* Refresh timer thread */

void * timer_thread(void *arg)
{  
    int mins;
    time_t ref_t;
    MainUi *m_ui;
    RefreshTmr *ref_tmr;

    /* Initial */
    m_ui = (MainUi *) arg;
    ref_tmr = &(m_ui->RefTmr);
    ref_t = ref_tmr->start_t + ref_tmr->ref_interval;

    /* Get time */
    while(1)
    {
	ref_tmr->curr_t = time(NULL);
	mins = (int) (((ref_t - ref_tmr->curr_t) / 60.0) + 0.5);

	/* Set info text */
	if (mins == 0)
	{
	    sprintf(ref_tmr->info_txt, "Refreshing usage details...");
	    ref_tmr->refresh_req = TRUE;
	    break;
	}
	else
	{
	    if (mins == 1)
		sprintf(ref_tmr->info_txt, "Next refresh due in %d minute", mins);
	    else
		sprintf(ref_tmr->info_txt, "Next refresh due in %d minutes", mins);
	}

	sleep(15);
    }
    
    pthread_exit(&ret_mon);

/* Debug
printf("%s timer thread start\n", debug_hdr); fflush(stdout);
printf("%s timer thread start: %ld, current: %ld, interval %ld\n", 
    debug_hdr, ref_tmr->start_t, ref_tmr->curr_t, ref_tmr->ref_interval); fflush(stdout);
printf("%s timer thread exit\n", debug_hdr); fflush(stdout);
*/
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
