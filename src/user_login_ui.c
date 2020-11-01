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
**	19-Oct-2020	Changes to convert from gnome keyring to gnome libsecret
**
*/


/* Includes */

#include <unistd.h>  
#include <stdlib.h>  
#include <string.h>  
#include <libgen.h>  
#include <errno.h>  
#include <gtk/gtk.h>  
#include <gdk/gdkkeysyms.h>  
#include <glib.h>  
#include <libsecret/secret.h>  
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
int load_isp_uname(IspData *, MainUi *);
int load_isp_pw(IspData *, MainUi *);
int store_isp_uname(IspData *, MainUi *);
int store_isp_pw(IspData *, MainUi *);
int clear_isp_uname(IspData *, MainUi *);
int clear_isp_pw(IspData *, MainUi *);
int create_secret(const gchar *, IspData *, MainUi *);
int check_keyring_old(IspData *, MainUi *);
const SecretSchema * app_schema_1 (void);
const SecretSchema * app_schema_2 (void);

void OnUserOK(GtkWidget*, gpointer);
void OnUserCancel(GtkWidget*, gpointer);
gboolean OnUserDelete(GtkWidget*, GdkEvent *, gpointer);
void close_login_ui(GtkWidget *, UserLoginUi *);

extern void log_msg(char*, char*, char*, GtkWidget*);
extern void create_entry(GtkWidget **, char *, GtkWidget *, int, int);
extern void create_label(GtkWidget **, char *, char *, GtkWidget *, int, int, int, int);
extern void register_window(GtkWidget *);
extern void deregister_window(GtkWidget *);
extern void OnQuit(GtkWidget*, gpointer);
extern int ssl_service_details(IspData *, MainUi *);
extern void disable_login(MainUi *);
extern void load_overview(IspData *isp_data, MainUi *m_ui);
extern void show_panel(GtkWidget *, MainUi *);
extern void start_usage_mon(IspData *, MainUi *);
extern int refresh_thread(MainUi *);
extern void add_main_loop(MainUi *);
extern void set_connect_btns(MainUi *, int);
extern void set_css();


/* Globals */

static const char *debug_hdr = "DEBUG-user_login_ui.c ";
static const SecretSchema *SEC_SCHEMA_1;
static const SecretSchema *SEC_SCHEMA_2;


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
        secret_password_free(isp_data->uname);

    if (isp_data->pw != NULL)
        secret_password_free (isp_data->pw);

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
    set_css();
    gtk_widget_show_all(u_ui->window);
    gtk_window_set_modal (GTK_WINDOW(u_ui->window), TRUE);

    return;
}


/* Control container for user details */

void user_control(UserLoginUi *u_ui)
{  
    GtkWidget *lbl, *lbl2;  

    /* Main container */
    u_ui->user_cntr = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
 
    /* Create a grid */
    u_ui->ctrl_grid = gtk_grid_new();
    gtk_widget_set_name(u_ui->ctrl_grid, "ctrl_grid");
    gtk_grid_set_row_spacing(GTK_GRID (u_ui->ctrl_grid), 3);
    gtk_grid_set_column_spacing(GTK_GRID (u_ui->ctrl_grid), 3);
    gtk_container_set_border_width (GTK_CONTAINER (u_ui->ctrl_grid), 3);

    /* Add user and password fields with labels */
    create_label(&lbl, "uname", "Username", u_ui->ctrl_grid, 1, 1, 1, 1);
    create_entry(&(u_ui->uname_ent), "uname", u_ui->ctrl_grid, 2, 1);

    create_label(&lbl2, "pwd", "Password", u_ui->ctrl_grid, 1, 2, 1, 1);
    create_entry(&(u_ui->pw_ent), "pw", u_ui->ctrl_grid, 2, 2);
    gtk_entry_set_visibility (GTK_ENTRY (u_ui->pw_ent), FALSE);

    gtk_box_pack_start (GTK_BOX (u_ui->user_cntr), u_ui->ctrl_grid, FALSE, FALSE, 0);

    /* Credential storage option */
    u_ui->secure_opt = gtk_check_button_new_with_label ("Securely store your login details?");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (u_ui->secure_opt), FALSE); 
    gtk_box_pack_start (GTK_BOX (u_ui->user_cntr), u_ui->secure_opt, FALSE, FALSE, 0);

    return;
}


/* Check Gnome (login) keyring (secret) for stored user credentials */

int check_user_creds(IspData *isp_data, MainUi *m_ui)
{
    /* Initial */
    m_ui->user_cd = FALSE;

    /* Load secret schemas */
    SEC_SCHEMA_1 = app_schema_1();		// Username
    SEC_SCHEMA_2 = app_schema_2();		// Password

    /* Get the isp username and password */
    if (load_isp_uname(isp_data, m_ui) == TRUE)
    {
	if (load_isp_pw(isp_data, m_ui) == TRUE)
	{
	    m_ui->user_cd = TRUE;
	    return TRUE;
	}
    }

    /* Not found, check for former old style gnome keyring and convert */
    log_msg("INF0016", "found", NULL, NULL);

    if (check_keyring_old(isp_data, m_ui) == FALSE)
    {
	log_msg("INF0015", "found", NULL, NULL);
    	return FALSE;
    }
    else
    {
	m_ui->user_cd = TRUE;
	return TRUE;
    }
}


/* Store user credentials securely in the Gnome keyring */

int store_user_creds(IspData *isp_data, MainUi *m_ui)
{  
    char *logname;
    GError *error = NULL;

    /* Load secret schemas if necessary */
    if (! SEC_SCHEMA_1)
    {
	SEC_SCHEMA_1 = app_schema_1();		// Username
	SEC_SCHEMA_2 = app_schema_2();		// Password
    }

    /* Save the isp username and password */
    if (store_isp_uname(isp_data, m_ui) == FALSE)
    	return FALSE;

    if (store_isp_pw(isp_data, m_ui) == FALSE)
	return FALSE;

    log_msg("INF0015", "stored", NULL, NULL);
    return TRUE;
}


/* Remove user credentials */

int delete_user_creds(IspData *isp_data, MainUi *m_ui)
{
    char *logname;
    GError *error = NULL;

    /* Clear the stored isp username and password */
    if (clear_isp_uname(isp_data, m_ui) == FALSE)
    	return FALSE;

    if (clear_isp_pw(isp_data, m_ui) == FALSE)
	return FALSE;

    log_msg("INF0015", "removed", NULL, NULL);
    return TRUE;
}


/* Get Isp username */

int load_isp_uname(IspData *isp_data, MainUi *m_ui)
{
    int r;
    char *logname;
    GError *error = NULL;

    /* Initial */
    r = TRUE;

    /* Get the login username */
    logname = strdup(getlogin());

    /* The attributes used to lookup the username should conform to the schema */
    isp_data->uname = secret_password_lookup_sync (SEC_SCHEMA_1, NULL, &error,
						   "logname", logname, "application", TITLE, NULL);

    /* Information only - may be first usage or user may opt not to save */
    if (error != NULL)
    {
	r = FALSE;
	sprintf(app_msg_extra, "%s\n", strerror(errno));
	g_error_free (error);
    } 
    else if (isp_data->uname == NULL)
    {
	r = FALSE;
    } 

    if (r == FALSE)
	log_msg("INF0012", logname, NULL, NULL);

    free(logname);

    return r;
}


/* Get Isp password */

int load_isp_pw(IspData *isp_data, MainUi *m_ui)
{
    int r;
    GError *error = NULL;

    /* Initial */
    r = TRUE;

    /* The attributes used to lookup the password should conform to the schema */
    isp_data->pw = secret_password_lookup_nonpageable_sync (SEC_SCHEMA_2, NULL, &error,
							    "username", isp_data->uname, "application", TITLE, NULL);

    if (error != NULL)
    {
	r = FALSE;
	sprintf(app_msg_extra, "%s\n", strerror(errno));
	g_error_free (error);
    } 
    else if (isp_data->pw == NULL)
    {
	r = FALSE;
    } 

    if (r == FALSE)
	log_msg("ERR0049", isp_data->uname, "ERR0049", m_ui->window);

    return r;
}


/* Store Isp username */

int store_isp_uname(IspData *isp_data, MainUi *m_ui)
{
    int r;
    char *logname;
    GError *error = NULL;

    /* Initial */
    r = TRUE;

    /* Get the login username */
    logname = strdup(getlogin());

    /* The attributes used to store the username should conform to the schema */
    secret_password_store_sync (SEC_SCHEMA_1, SECRET_COLLECTION_DEFAULT,
				"ISP_Username", isp_data->uname, NULL, &error,
				"logname", logname, "application", TITLE, NULL);

    if (error != NULL)
    {
	r = FALSE;
	sprintf(app_msg_extra, "%s\n", strerror(errno));
	g_error_free (error);
	log_msg("ERR0050", logname, "ERR0050", m_ui->window);
    } 

    free(logname);

    return r;
}


/* Store Isp password */

int store_isp_pw(IspData *isp_data, MainUi *m_ui)
{
    int r;
    GError *error = NULL;

    /* Initial */
    r = TRUE;

    /* The attributes used to store the password should conform to the schema */
    secret_password_store_sync (SEC_SCHEMA_2, SECRET_COLLECTION_DEFAULT,
				"ISP_Password", isp_data->pw, NULL, &error,
				"username", isp_data->uname, "application", TITLE, NULL);

    if (error != NULL)
    {
	r = FALSE;
	sprintf(app_msg_extra, "%s\n", strerror(errno));
	g_error_free (error);
	log_msg("ERR0050", isp_data->uname, "ERR0050", m_ui->window);
    } 

    return r;
}


/* Clear stored Isp username */

int clear_isp_uname(IspData *isp_data, MainUi *m_ui)
{
    int r;
    char *logname;
    GError *error = NULL;
    gboolean removed;

    /* Initial */
    r = TRUE;

    /* Get the login username */
    logname = strdup(getlogin());

    /* The attributes used to clear the username should conform to the schema */
    removed = secret_password_clear_sync (SEC_SCHEMA_1, NULL, &error,
					  "logname", logname, "application", TITLE, NULL);

    if (error != NULL)
    {
	r = FALSE;
	sprintf(app_msg_extra, "%s\n", strerror(errno));
	g_error_free (error);
	log_msg("ERR0028", logname, "ERR0051", m_ui->window);
    } 
    else if (removed == FALSE)
    {
	r = FALSE;
	log_msg("ERR0028", logname, "ERR0051", m_ui->window);
    }

    free(logname);

    return r;
}


/* Clear stored Isp password */

int clear_isp_pw(IspData *isp_data, MainUi *m_ui)
{
    int r;
    GError *error = NULL;
    gboolean removed;

    /* Initial */
    r = TRUE;

    /* The attributes used to clear the password should conform to the schema */
    removed = secret_password_clear_sync (SEC_SCHEMA_2, NULL, &error,
					  "username", isp_data->uname, "application", TITLE, NULL);

    if (error != NULL)
    {
	r = FALSE;
	sprintf(app_msg_extra, "%s\n", strerror(errno));
	g_error_free (error);
	log_msg("ERR0028", isp_data->uname, "ERR0051", m_ui->window);
    } 
    else if (removed == FALSE)
    {
	r = FALSE;
	log_msg("ERR0028", isp_data->uname, "ERR0051", m_ui->window);
    }

    return r;
}


/* Create a temportary secret */

int create_secret(const gchar *pw, IspData *isp_data, MainUi *m_ui)
{
    SecretValue *temp_pw;
    gsize len;

    /* Create a temporary secret and steal it to a password */
    len = (gsize) (strlen(pw) + 1);
    temp_pw = secret_value_new(pw, (gssize) len, "text/plain");
    isp_data->pw = secret_value_unref_to_password(temp_pw, &len);

    return TRUE;
}


/* Check for and convert an old style gnome keyring */

int check_keyring_old(IspData *isp_data, MainUi *m_ui)
{
    GHashTable *passwd_attrs, *secret_attrs;
    GHashTableIter iter;
    GList *items, *l;
    gpointer key, value;
    GError *error = NULL;
    int fnd;
    gboolean removed;

    /* Old schema */
    fnd = FALSE;

    static const SecretSchema SEC_SCHEMA_OLD = 
    {
	"org.freedesktop.Secret.Generic", SECRET_SCHEMA_NONE,
	{
	    { "application", SECRET_SCHEMA_ATTRIBUTE_STRING },
	    { "username", SECRET_SCHEMA_ATTRIBUTE_STRING },
	    { "NULL", 0 },
	}
    };

    /* Search for attributes */
    passwd_attrs = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    g_hash_table_insert (passwd_attrs, g_strdup ("application"), g_strdup (TITLE));

    items = secret_password_searchv_sync (&SEC_SCHEMA_OLD, passwd_attrs, SECRET_SEARCH_ALL, NULL, &error);
    g_hash_table_unref (passwd_attrs);

    if (error != NULL)
    {
	sprintf(app_msg_extra, "%s\n", strerror(errno));
	g_error_free (error);
	log_msg("ERR0051", "Failed to search items", "ERR0051", m_ui->window);
	return FALSE;
    }

    for(l = items; l != NULL; l = l->next)
    {
    	SecretRetrievable *retrievable = SECRET_RETRIEVABLE (l->data);
    	SecretValue *sec_val = secret_retrievable_retrieve_secret_sync (retrievable, NULL, &error);

	if (error != NULL)
	{
	    sprintf(app_msg_extra, "%s\n", strerror(errno));
	    g_error_free (error);
	    log_msg("ERR0051", "Failed to retrieve secret", "ERR0051", m_ui->window);
	    break;
	}

	isp_data->pw = (gchar *) secret_value_get_text (sec_val);

	secret_attrs = secret_retrievable_get_attributes (retrievable);
	g_hash_table_iter_init (&iter, secret_attrs);

	while (g_hash_table_iter_next (&iter, (void **)&key, (void **)&value))
	{
	    if (strcmp (key, "xdg:schema") != 0)
	    {
		if (strcmp (key, "username") == 0)
		{
		    isp_data->uname = strdup((char *) value);
		    log_msg("INF0013", NULL, NULL, NULL);
		    store_user_creds(isp_data, m_ui);
		    check_user_creds(isp_data, m_ui);
		    log_msg("INF0017", "removed", NULL, NULL);
		    //delete_user_creds(isp_data, m_ui);

		    removed = secret_password_clear_sync (&SEC_SCHEMA_OLD, NULL, &error,
					  "application", TITLE, "username", isp_data->uname, NULL);

		    if (! removed)
		    {
			sprintf(app_msg_extra, "%s\n", strerror(errno));
			g_error_free (error);
			log_msg("ERR0051", "Failed to remove old keyring", "ERR0051", m_ui->window);
			break;
		    }

		    fnd = TRUE;
		    break;
		}
	    }
	}

	g_hash_table_unref (secret_attrs);
    	secret_value_unref(sec_val);
    }

    g_list_free_full (items, g_object_unref);

    return fnd;
}


/* Define the Inodeum username schema */

const SecretSchema * app_schema_1 (void)
{
    static const SecretSchema inodeum_schema_1 = 
    {
	"org.inodeum.keyring.Username", SECRET_SCHEMA_NONE,
	{
	    { "logname", SECRET_SCHEMA_ATTRIBUTE_STRING },
	    { "application", SECRET_SCHEMA_ATTRIBUTE_STRING },
	    { "NULL", 0 },
	}
    };

    return &inodeum_schema_1;
}


/* Define the Inodeum password schema */

const SecretSchema * app_schema_2 (void)
{
    static const SecretSchema inodeum_schema_2 = 
    {
	"org.inodeum.keyring.Password", SECRET_SCHEMA_NONE,
	{
	    { "username", SECRET_SCHEMA_ATTRIBUTE_STRING },
	    { "application", SECRET_SCHEMA_ATTRIBUTE_STRING },
	    { "NULL", 0 },
	}
    };

    return &inodeum_schema_2;
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
    m_ui = (MainUi *) g_object_get_data (G_OBJECT (u_ui->parent_win), "ui");

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

    /* If save requested, store and reload pw. If no save, create a temporary secret. */
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (u_ui->secure_opt)) == TRUE)
    {
	isp_data->pw = strdup(pw);
    	store_user_creds(isp_data, m_ui);
    	load_isp_pw(isp_data, m_ui);
    }
    else
    {
	create_secret(pw, isp_data, m_ui);
    }

    /* Initiate a service request, close if failure, return to login if auth error */
    r = ssl_service_details(isp_data, m_ui);

    if (r == TRUE)
    {
    	start_usage_mon(isp_data, m_ui);
    }
    else if (r == -1)
    {
	log_msg("ERR0026", NULL, "ERR0026", m_ui->window);
	return;
    }
    else
    {
	log_msg("ERR0020", NULL, "ERR0020", m_ui->window);
	return;
    }

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
    OnQuit(ui->parent_win, ui->parent_win);

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
