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


/* Includes */

#include <stdlib.h>  
#include <string.h>  
#include <libgen.h>  
#include <gtk/gtk.h>  
#include <gdk/gdkkeysyms.h>  
#include <glib.h>  
#include <gnome-keyring.h>  
#include <gnome-keyring-memory.h>  
#include <isp.h>
#include <defs.h>
#include <main.h>


/* Defines */


/* Types */

typedef struct _user_login_ui
{
    GtkWidget *window;
    GtkWidget *parent_win;
    GtkWidget *uname_lbl;
    GtkWidget *uname_ent;
    GtkWidget *pw_lbl;
    GtkWidget *pw_ent;
    GtkWidget *secure_opt;
    GtkWidget *ok_btn;
    GtkWidget *cancel_btn;
    GtkWidget *ctrl_grid;
    GtkWidget *user_cntr;
    GtkWidget *btn_hbox;
    GtkWidget *main_vbox;
    int close_handler;
} UserLoginUi;


/* Prototypes */

void user_login_main(IspData *, GtkWidget *);
void initial(IspData *);
void user_ui(IspData *, UserLoginUi *);
UserLoginUi * new_user_ui();
void user_control(UserLoginUi *);
int check_user_creds(IspData *, MainUi *);
int store_user_creds(IspData *, MainUi *);
int delete_user_creds(IspData *, MainUi *);
void keyring_error(GnomeKeyringResult, char *, char *, GnomeKeyringItemInfo *, 
		   GnomeKeyringAttributeList *, GtkWidget *);

void OnUserOK(GtkWidget*, gpointer);
void OnUserCancel(GtkWidget*, gpointer);
gboolean OnUserDelete(GtkWidget*, GdkEvent *, gpointer);
void close_login_ui(GtkWidget *, UserLoginUi *);

extern void log_msg(char*, char*, char*, GtkWidget*);
extern void create_entry(GtkWidget **, char *, int, int, GtkWidget **, PangoFontDescription **);
extern void create_label(char *, int, int, GtkWidget **, PangoFontDescription **);
extern void register_window(GtkWidget *);
extern void deregister_window(GtkWidget *);
extern void OnQuit(GtkWidget*, gpointer);
extern int ssl_service_details(IspData *, MainUi *);
extern void disable_login(MainUi *);
extern void display_overview(IspData *isp_data, MainUi *m_ui);


/* Globals */

static const char *debug_hdr = "DEBUG-user_login_ui.c ";
static const char *keyring = "login";


/* Display and maintenance of user preferences */

void user_login_main(IspData *isp_data, GtkWidget *parent_win)
{
    UserLoginUi *ui;

    /* Initial */
    initial(isp_data);
    ui = new_user_ui();
    ui->parent_win = parent_win;

    /* Create the interface */
    user_ui(isp_data, ui);

    /* Register the window */
    register_window(ui->window);

    return;
}


/* Initial work */

void initial(IspData *isp_data)
{
    if (isp_data->uname != NULL)
        free(isp_data->uname);

    if (isp_data->pw != NULL)
        gnome_keyring_memory_free (isp_data->pw);
        //free(isp_data->pw);

    return;
}


/* Create new screen data variable */

UserLoginUi * new_user_ui()
{
    UserLoginUi *ui = (UserLoginUi *) malloc(sizeof(UserLoginUi));
    memset(ui, 0, sizeof(UserLoginUi));

    return ui;
}


/* Create the user interface and set the CallBacks */

void user_ui(IspData *isp_data, UserLoginUi *u_ui)
{  
    /* Set up the UI window */
    u_ui->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);  
    gtk_window_set_title(GTK_WINDOW(u_ui->window), USER_UI);
    //gtk_window_set_position(GTK_WINDOW(u_ui->window), GTK_WIN_POS_NONE);
    gtk_window_set_default_size(GTK_WINDOW(u_ui->window), 200, 100);
    gtk_container_set_border_width(GTK_CONTAINER(u_ui->window), 10);
    g_object_set_data (G_OBJECT (u_ui->window), "ui", u_ui);
    g_object_set_data (G_OBJECT (u_ui->window), "isp", isp_data);

    /* Main view */
    u_ui->main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(GTK_WIDGET (u_ui->main_vbox), GTK_ALIGN_START);

    /* Main update or view grid */
    user_control(u_ui);

    /* Box container for action buttons */
    u_ui->btn_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_halign(GTK_WIDGET (u_ui->btn_hbox), GTK_ALIGN_CENTER);

    /* Cancel button */
    u_ui->cancel_btn = gtk_button_new_with_label("  Cancel  ");
    g_signal_connect_swapped(u_ui->cancel_btn, "clicked", G_CALLBACK(OnUserCancel), u_ui->window);
    gtk_box_pack_end (GTK_BOX (u_ui->btn_hbox), u_ui->cancel_btn, FALSE, FALSE, 0);

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
    gtk_window_set_transient_for (GTK_WINDOW(u_ui->window), GTK_WINDOW(u_ui->parent_win));
    gtk_window_set_position(GTK_WINDOW(u_ui->window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_widget_show_all(u_ui->window);
    gtk_window_set_modal (GTK_WINDOW(u_ui->window), TRUE);

    return;
}


/* Control container for user details */

void user_control(UserLoginUi *u_ui)
{  
    GtkWidget *label;  
    PangoFontDescription *pf;

    /* Font and layout setup */
    pf = pango_font_description_from_string ("Sans 9");

    /* Main container */
    u_ui->user_cntr = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
 
    /* Create a grid */
    u_ui->ctrl_grid = gtk_grid_new();
    gtk_widget_set_name(u_ui->ctrl_grid, "ctrl_grid");
    gtk_grid_set_row_spacing(GTK_GRID (u_ui->ctrl_grid), 3);
    gtk_grid_set_column_spacing(GTK_GRID (u_ui->ctrl_grid), 3);
    gtk_container_set_border_width (GTK_CONTAINER (u_ui->ctrl_grid), 3);

    /* Add user and password fields with labels */
    create_label("Username", 1, 1, &(u_ui->ctrl_grid), &pf);
    create_entry(&(u_ui->uname_ent), "uname", 2, 1, &(u_ui->ctrl_grid), &pf);

    create_label("Password", 1, 2, &(u_ui->ctrl_grid), &pf);
    create_entry(&(u_ui->pw_ent), "pw", 2, 2, &(u_ui->ctrl_grid), &pf);
    gtk_entry_set_visibility (GTK_ENTRY (u_ui->pw_ent), FALSE);

    gtk_box_pack_start (GTK_BOX (u_ui->user_cntr), u_ui->ctrl_grid, FALSE, FALSE, 0);

    /* Credential storage option */
    u_ui->secure_opt = gtk_check_button_new_with_label ("Securely store your login details?");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (u_ui->secure_opt), FALSE); 
    gtk_box_pack_start (GTK_BOX (u_ui->user_cntr), u_ui->secure_opt, FALSE, FALSE, 0);

    /* Free font */
    pango_font_description_free (pf);

    return;
}


/* Check Gnome (login) keyring for stored user credentials */

int check_user_creds(IspData *isp_data, MainUi *m_ui)
{
    GList *item_ids, *l;
    GnomeKeyringResult res;
    GnomeKeyringItemInfo *info;
    GnomeKeyringAttributeList *attrs;
    GnomeKeyringAttribute *attr;
    int i, fnd;
    guint32 id;
    char *display_name, *tmp;

    /* List all the keyring items */
    res = gnome_keyring_list_item_ids_sync (keyring, &item_ids);

    if (res != GNOME_KEYRING_RESULT_OK)
    {
	keyring_error(res, "(list ids)", NULL, NULL, NULL, m_ui->window);
    	return FALSE;
    }

    /* Examine each item until the desired one is found */
    fnd = FALSE;

    for(l = item_ids; l != NULL && fnd == FALSE; l = l->next)
    {
	/* Get the keyring item info */
    	id = GPOINTER_TO_UINT (l->data);

    	res = gnome_keyring_item_get_info_sync (keyring, id, &info);

	if (res != GNOME_KEYRING_RESULT_OK)
	{
	    keyring_error(res, "(item info)", NULL, NULL, NULL, m_ui->window);
	    break;
	}

	/* Get the display name for comparison */
	display_name = gnome_keyring_item_info_get_display_name (info);

	if (strncmp(display_name, TITLE, strlen(TITLE)) == 0)
	{
	    /* Found it, get the attributes */
	    res = gnome_keyring_item_get_attributes_sync (keyring, id, &attrs);

	    if (res != GNOME_KEYRING_RESULT_OK)
	    {
		keyring_error(res, "(item attributes list)", display_name, info, NULL, m_ui->window);
		break;
	    }

	    /* Get the username and password */
	    for(i = 0; i < attrs->len; i++)
	    {
	    	attr = &gnome_keyring_attribute_list_index(attrs, i);

	    	if (attr->type == GNOME_KEYRING_ATTRIBUTE_TYPE_STRING)
		    if (strcmp(attr->name, "username") == 0)
		    	break;
	    }

	    if (i >= attrs->len)
	    {
		keyring_error(res, "(username attribute)", display_name, info, attrs, m_ui->window);
		break;
	    }

	    isp_data->uname = (char *) malloc(strlen(attr->value.string) + 1);
	    strcpy(isp_data->uname, (char *) attr->value.string);

	    tmp = gnome_keyring_item_info_get_secret (info);
	    isp_data->pw = gnome_keyring_memory_alloc((gulong) strlen(tmp) + 1);
	    isp_data->pw = gnome_keyring_memory_strdup(tmp);

	    free(tmp);
	    gnome_keyring_attribute_list_free (attrs);
	    fnd = TRUE;
	}

	gnome_keyring_item_info_free (info);
	free(display_name);
    }

    /* Clean up */
    g_list_free(item_ids);

    return fnd;
}


/* Store user credentials securely in the Gnome keyring */

int store_user_creds(IspData *isp_data, MainUi *m_ui)
{  
    GnomeKeyringResult res;
    GnomeKeyringAttributeList *attrs;
    guint32 id;
    char *display_name;

    display_name = (char *) malloc(strlen(TITLE) + strlen(isp_data->uname) + 3);
    sprintf(display_name, "%s: %s", TITLE, isp_data->uname);

    attrs = gnome_keyring_attribute_list_new ();
    gnome_keyring_attribute_list_append_string (attrs, "username", isp_data->uname);
    gnome_keyring_attribute_list_append_string (attrs, "application", TITLE);

    res = gnome_keyring_item_create_sync (keyring, GNOME_KEYRING_ITEM_GENERIC_SECRET, display_name,
					  attrs, isp_data->pw, TRUE, &id);

    if (res != GNOME_KEYRING_RESULT_OK)
    {
	keyring_error(res, "(item create)", display_name, NULL, attrs, m_ui->window);
	return FALSE;
    }

    free(display_name);
    gnome_keyring_attribute_list_free (attrs);

    return TRUE;
}


/* Remove user credentials */

int delete_user_creds(IspData *isp_data, MainUi *m_ui)
{
    GList *item_ids, *l;
    GnomeKeyringResult res;
    GnomeKeyringItemInfo *info;
    int fnd;
    guint32 id;
    char *display_name;

    /* List all the keyring items */
    res = gnome_keyring_list_item_ids_sync (keyring, &item_ids);

    if (res != GNOME_KEYRING_RESULT_OK)
    {
	keyring_error(res, "(list ids)", NULL, NULL, NULL, m_ui->window);
    	return FALSE;
    }

    /* Examine each item until the desired one is found */
    fnd = FALSE;

    for(l = item_ids; l != NULL && fnd == FALSE; l = l->next)
    {
	/* Get the keyring item info */
    	id = GPOINTER_TO_UINT (l->data);

    	res = gnome_keyring_item_get_info_sync (keyring, id, &info);

	if (res != GNOME_KEYRING_RESULT_OK)
	{
	    keyring_error(res, "(item info)", NULL, NULL, NULL, m_ui->window);
	    break;
	}

	/* Get the display name for comparison */
	display_name = gnome_keyring_item_info_get_display_name (info);

	if (strncmp(display_name, TITLE, strlen(TITLE)) == 0)
	{
	    /* Found it, delete it */
	    res = gnome_keyring_item_delete_sync (keyring, id);

	    if (res != GNOME_KEYRING_RESULT_OK)
	    {
		keyring_error(res, "(item delete)", display_name, info, NULL, m_ui->window);
		break;
	    }

	    fnd = TRUE;
	}

	gnome_keyring_item_info_free (info);
	free(display_name);
    }

    /* Clean up */
    g_list_free(item_ids);

    return fnd;
}


/* General Keyring error reporting function */

void keyring_error(GnomeKeyringResult res, 
		   char *txt, 
		   char *display_name,
		   GnomeKeyringItemInfo *info, 
		   GnomeKeyringAttributeList *attrs,
		   GtkWidget *window)
{
    char s[30];

    sprintf(s, "%d %s", res, txt);
    log_msg("ERR0027", s, "ERR0027", window);

    if (display_name != NULL)
    	free(display_name);

    if (info != NULL)
	gnome_keyring_item_info_free (info);

    if (attrs != NULL)
	gnome_keyring_attribute_list_free (attrs);

    return;
}


/* Callback - Get Login details and save credentials if required and close */

void OnUserOK(GtkWidget *btn, gpointer user_data)
{
    const gchar *uname, *pw;
    int len, r;
    UserLoginUi *u_ui;
    MainUi *m_ui;
    IspData *isp_data;

    /* Get data */
    u_ui = (UserLoginUi *) user_data;
    isp_data = (IspData *) g_object_get_data (G_OBJECT (u_ui->window), "isp");

    /* Read and store details */
    uname = gtk_entry_get_text (GTK_ENTRY (u_ui->uname_ent));
    len = gtk_entry_get_text_length (GTK_ENTRY (u_ui->uname_ent));

    if (len == 0)
    {
	log_msg("ERR0037", NULL, "ERR0037", u_ui->window);
    	return;
    }

    isp_data->uname = (char *) malloc(len + 1);
    strcpy(isp_data->uname, uname);

    pw = gtk_entry_get_text (GTK_ENTRY (u_ui->pw_ent));
    len = gtk_entry_get_text_length (GTK_ENTRY (u_ui->pw_ent));

    if (len == 0)
    {
	log_msg("ERR0038", NULL, "ERR0038", u_ui->window);
	free(isp_data->uname);
    	return;
    }

    //isp_data->pw = (char *) malloc(len + 1);
    //strcpy(isp_data->pw, pw);
    isp_data->pw = gnome_keyring_memory_alloc((gulong) len + 1);
    isp_data->pw = gnome_keyring_memory_strdup(pw);

    /* Check if save requested */
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (u_ui->secure_opt)) == TRUE)
    	store_user_creds(isp_data, m_ui);

    /* Initiate a service request, close if failure, return to login if auth error */
    m_ui = (MainUi *) g_object_get_data (G_OBJECT (u_ui->parent_win), "ui");

    r = ssl_service_details(isp_data, m_ui);

    if (r == TRUE)
    {
    	disable_login(m_ui);
    	display_overview(isp_data, m_ui);
    }

    else if (r == -1)
	return;

    /* Close the window, free the screen data and block any secondary close signal */
    close_login_ui(u_ui->window, u_ui);

    return;
}


// Callback for window close
// Destroy the window and de-register the window 
// Check for changes

void OnUserCancel(GtkWidget *window, gpointer user_data)
{ 
    GtkWidget *dialog;
    UserLoginUi *ui;
    gint response;

    /* Get data */
    ui = (UserLoginUi *) g_object_get_data (G_OBJECT (window), "ui");

    /* Confirm */
    dialog = gtk_message_dialog_new (GTK_WINDOW (window),
				     GTK_DIALOG_MODAL,
				     GTK_MESSAGE_QUESTION,
				     GTK_BUTTONS_OK_CANCEL,
				     "Confirm Cancel?");

    response = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    if (response == GTK_RESPONSE_CANCEL)
	return;

    /* Close the window, free the screen data and block any secondary close signal */
    close_login_ui(window, ui);

    /* Quit Inodeum */
    OnQuit(ui->parent_win, NULL);

    return;
}


/* Window delete event */

gboolean OnUserDelete(GtkWidget *window, GdkEvent *ev, gpointer user_data)
{
    OnUserCancel(window, user_data);

    return TRUE;
}


/* Common close */

void close_login_ui(GtkWidget *window, UserLoginUi *ui)
{ 
    g_signal_handler_block (window, ui->close_handler);

    deregister_window(window);
    gtk_window_close(GTK_WINDOW(window));

    free(ui);

    return;
}
