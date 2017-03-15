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
** Description:
**  Error and Message Reference functions
**  Logging functions
**  Session management
**  Window management
**  General usage functions
**
** Author:	Anthony Buckley
**
** History
**	09-Jan-2017	Initial code
**
*/


/* Defines */

#define ERR_FILE
#define MAX_SETTING 50


/* Includes */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <gtk/gtk.h>
#include <defs.h>


/* Prototypes */

int check_app_dir();
int reset_log();
void log_msg(char*, char*, char*, GtkWidget*);
void app_msg(char*, char *, GtkWidget*);
void info_dialog(GtkWidget *, char *, char *);
void get_msg(char*, char*, char*);
void close_log();
void register_window(GtkWidget *);
void deregister_window(GtkWidget *);
int is_ui_reg(char *, int);
void free_window_reg();
void close_open_ui();

/* ??? */
gint query_dialog(GtkWidget *, char *, char *);
void string_trim(char*);
int close_ui(char *);
char * log_name();
char * app_dir_path();
char * home_dir();
void strlower(char *, char *);
void dttm_stamp(char *, size_t);
int check_dir(char *);
int make_dir(char *);
int val_str2numb(char *, int *, char *, GtkWidget *);
int check_errno(char *);
int64_t msec_time();
void print_bits(size_t const, void const * const);
GtkWidget * find_parent(GtkWidget *);
GtkWidget * find_widget_by_name(GtkWidget *, char *);
GtkWidget * find_widget_by_parent(GtkWidget *, char *);
GList * ctrl_widget_list(GtkWidget *, GtkWidget *);
void cur_date_str(char *, int, char *);


/* Globals */

static const char *app_messages[][2] = 
{ 
    { "MSG0001", "Session started. "},
    { "MSG0002", "Session ends. "},
    { "ERR0001", "Failed to create log file: %s "},
    { "ERR0002", "Failed to read $HOME variable. "},
    { "ERR0003", "Failed to create Application directory: %s "},
    { "ERR0004", "Failed to get ip for Host: %s "},
    { "ERR0005", "Failed to resolve Host: %s "},
    { "ERR0006", "Failed to create socket. "},
    { "ERR0007", "%s is not a valid address family. "},
    { "ERR0008", "Failed to create socket. "},
    { "ERR0009", "Socket failed to connect. "},
    { "ERR0010", "Failed to send query. "},
    { "ERR0011", "Error receiving data. "},
    { "ERR0012", "SSLv23_method failed. "},
    { "ERR0013", "Unable to create a new SSL context structure. "},
    { "ERR0014", "Unable to load certificate location. "},
    { "ERR0015", "Unable to make SSL connection. "},
    { "ERR0016", "Unable to set Hostname and Port. "},
    { "ERR0017", "Unable to fetch ssl connection object. "},
    { "ERR0018", "Unable to select cipher. "},
    { "ERR0019", "Unable to set SNI Hostname extension. "},
    { "ERR0020", "Error: Connection failed. "},
    { "ERR0021", "Error: Server did not issue a certificate. "},
    { "ERR0022", "Error: Unable to verify certificate. "},
    { "ERR0023", "Failed to send full query to server. "},
    { "ERR0024", "Error: Unable send query to server. "},
    { "ERR0030", "Error: XML tag %s not found. "},
    { "ERR0031", "Error: XML tag attribute %s not found. "},
    { "ERR0032", "Error: XML tag value not found. "},
    { "ERR0033", "Error: No items found in returned XML for %s. "},
    { "ERR0034", "Error: User Default Service - %s - not found. "},
    { "ERR9999", "Error - Unknown error message given. "}			// NB - MUST be last
};

static const int Msg_Count = 32;
static char *Home;
static char *logfile = NULL;
static char *app_dir;
static FILE *lf = NULL;
static int app_dir_len;
static const char *debug_hdr = "DEBUG-utility.c ";
static GList *open_ui_list_head = NULL;
static GList *open_ui_list = NULL;


/* Process additional application messages and error conditions */

void app_msg(char *msg_id, char *opt_str, GtkWidget *window)
{
    char msg[512];
    int i;

    /* Lookup the error */
    get_msg(msg, msg_id, opt_str);
    strcat(msg, " \n\%s");

    /* Display the error */
    info_dialog(window, msg, app_msg_extra);

    /* Reset global error details */
    app_msg_extra[0] = '\0';

    return;
}


/* Add a message to the log file and optionally display a popup */

void log_msg(char *msg_id, char *opt_str, char *sys_msg_id, GtkWidget *window)
{
    char msg[512];
    char date_str[50];

    /* Lookup the error */
    get_msg(msg, msg_id, opt_str);

    /* Log the message */
    cur_date_str(date_str, sizeof(date_str), "%d-%b-%Y %I:%M:%S %p");

    /* This may before anything has been set up (chicken & egg !). Use stderr if required */
    if (lf == NULL)
    	lf = stderr;

    fprintf(lf, "%s - %s\n", date_str, msg);

    if (strlen(app_msg_extra) > 0)
	fprintf(lf, "\t%s\n", app_msg_extra);

    fflush(lf);

    /* Reset global error details */
    app_msg_extra[0] = '\0';

    /* Optional display */
    if (sys_msg_id && window && logfile)
    {
    	sprintf(app_msg_extra, "\nLog file (%s) may contain more details.", logfile);
    	app_msg(sys_msg_id, opt_str, window);
    }

    return;
}


/* General prupose information dialog */

void info_dialog(GtkWidget *window, char *msg, char *opt)
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new (GTK_WINDOW (window),
				     GTK_DIALOG_MODAL,
				     GTK_MESSAGE_ERROR,
				     GTK_BUTTONS_CLOSE,
				     msg,
				     opt);

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    return;
}


/* General prupose query dialog */

gint query_dialog(GtkWidget *window, char *msg, char *opt)
{
    GtkWidget *dialog;
    gint res;

    GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

    dialog = gtk_message_dialog_new (GTK_WINDOW (window),
				     flags,
				     GTK_MESSAGE_QUESTION,
				     GTK_BUTTONS_YES_NO,
				     msg,
				     opt);

    res = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    return res;
}


/* Reset the log file */

int reset_log()
{
    logfile = (char *) malloc(strlen(Home) + (strlen(TITLE) * 2) + 10);
    sprintf(logfile, "%s/.%s/%s.log", Home, TITLE, TITLE);

    if ((lf = fopen(logfile, "w")) == (FILE *) NULL)
    {
	log_msg("MSG0001", logfile, NULL, NULL);
	free(logfile);
	return FALSE;
    }
    else
    {
    	g_print("%s: See Log file - %s for all details.\n", TITLE, logfile);
    }

    return TRUE;
}


/* Close the log file and free any memory */

void close_log()
{
    fclose(lf);
    free(logfile);
    free(app_dir);

    return;
}


/* Return the logfile name */

char * log_name()
{
    return logfile;
}


/* Return the application directory path */

char * app_dir_path()
{
    return app_dir;
}


/* Return the Home directory path */

char * home_dir()
{
    return Home;
}


/* Set up application directory(s) for the user if necessary */

int check_app_dir()
{
    struct stat fileStat;
    int err;

    if ((Home = getenv("HOME")) == NULL)
    {
    	log_msg("ERR0002", NULL, NULL, NULL);
    	return FALSE;
    }

    app_dir = (char *) malloc(strlen(Home) + strlen(TITLE) + 5);
    sprintf(app_dir, "%s/.%s", Home, TITLE);
    app_dir_len = strlen(app_dir);

    if ((err = stat(app_dir, &fileStat)) < 0)
    {
	if ((err = mkdir(app_dir, 0700)) != 0)
	{
	    log_msg("ERR0003", app_dir, NULL, NULL);
	    free(app_dir);
	    return FALSE;
	}
    }

    return TRUE;
}


/* Error lookup and optional string argument substitution */

void get_msg(char *s, char *msg_id, char *opt_str)
{
    int i;
    char *p, *p2;

    /* Find message */
    for(i = 0; i < Msg_Count; i++)
    {
    	if ((strcmp(msg_id, app_messages[i][0])) == 0)
	    break;
    }

    if (i >= Msg_Count)
    	i--;

    /* Check substitution. If none, show message as is with any '%s' blanked out. */
    p = (char *) app_messages[i][1];
    p2 = strstr(p, "%s");

    if ((! opt_str) || (strlen(opt_str) == 0) || (p2 == NULL))
    {
	sprintf(s, "(%s) %s", app_messages[i][0], app_messages[i][1]);

	if (p2 != NULL)
	{
	    p2 = strstr(s, "%s");
	    *p2++ = ' ';
	    *p2 = ' ';
	}

    	return;
    }

    /* Add substitution string */
    *s = '\0';
    sprintf(s, "(%s) ", app_messages[i][0]);

    for(s = (s + strlen(app_messages[i][0]) + 3); p < p2; p++)
    	*s++ = *p;

    *s = '\0';

    strcat(s, opt_str);
    strcat(s, p2 + 2);

    return;
}


/* Remove leading and trailing spaces from a string */

void string_trim(char *s)
{
    int i;
    char *p;

    /* Trailing */
    for(i = strlen(s) - 1; i >= 0; i--)
    {
	if (isspace(s[i]))
	    s[i] = '\0';
	else
	    break;
    }

    /* Empty - all spaces */
    if (*s == '\0')
    	return;

    /* Leading */
    p = s;

    while(isspace(*p))
    {
    	p++;
    }

    while(*p != '\0')
    {
    	*s++ = *p++;
    }

    *s = '\0';

    return;
}


/* Regiser the window as open */

void register_window(GtkWidget *window)
{
    open_ui_list = g_list_append (open_ui_list_head, window);

    if (open_ui_list_head == NULL)
    	open_ui_list_head = open_ui_list;

    return;
}


/* De-register the window as closed */

void deregister_window(GtkWidget *window)
{
    open_ui_list_head = g_list_remove (open_ui_list_head, window);

    return;
}


/* Check if a window title is registered (open) and present it to the user if reguired */

int is_ui_reg(char *s, int present)
{
    GtkWidget *window;
    const gchar *title;

    open_ui_list = g_list_first(open_ui_list_head);

    while(open_ui_list != NULL)
    {
	window = GTK_WIDGET (open_ui_list->data);
	title = gtk_window_get_title(GTK_WINDOW (window));

	if (strcmp(s, title) == 0)
	{
	    if (present == TRUE)
	    {
	    	gtk_window_present (GTK_WINDOW (window));
	    }

	    return TRUE;
	}

	open_ui_list = g_list_next(open_ui_list);
    }

    return FALSE;
}


/* Close any open windows */

void close_open_ui()
{
    GtkWidget *window;

    open_ui_list = g_list_first(open_ui_list_head);

    while(open_ui_list != NULL)
    {
	window = GTK_WIDGET (open_ui_list->data);
	gtk_window_close(GTK_WINDOW (window));
	open_ui_list = g_list_next(open_ui_list);
    }

    return;
}


/* Close a window */

int close_ui(char *s)
{
    GtkWidget *window;
    const gchar *title;

    open_ui_list = g_list_first(open_ui_list_head);

    while(open_ui_list != NULL)
    {
	window = GTK_WIDGET (open_ui_list->data);
	title = gtk_window_get_title(GTK_WINDOW (window));

	if (strcmp(s, title) == 0)
	{
	    gtk_window_close(GTK_WINDOW (window));
	    return TRUE;
	}

	open_ui_list = g_list_next(open_ui_list);
    }

    return FALSE;
}


/* Free the window register */

void free_window_reg()
{
    g_list_free (open_ui_list_head);

    return;
}


/* Convert a string to lowercase */

void strlower(char *s1, char *s2)
{
    for(; *s1 != '\0'; s1++, s2++)
    	*s2 = tolower(*s1);

    *s2 = *s1;

    return;
}


/* Return a date and time stamp */

void dttm_stamp(char *s, size_t max)
{
    size_t sz;
    struct tm *tm;
    time_t current_time;

    *s = '\0';
    current_time = time(NULL);
    tm = localtime(&current_time);
    sz = strftime(s, max, "%d%m%Y_%H%M%S", tm);

    return;
}


/* Check directory exists */

int check_dir(char *s)
{
    struct stat fileStat;
    int err;

    if ((err = stat(s, &fileStat)) < 0)
	return FALSE;

    if ((fileStat.st_mode & S_IFMT) == S_IFDIR)
	return TRUE;
    else
	return FALSE;
}


/* Create a directory */

int make_dir(char *s)
{
    int err;

    if ((err = mkdir(s, 0700)) != 0)
    {
	log_msg("SYS9002", s, NULL, NULL);
	return FALSE;
    }

    return TRUE;
}


/* Convert a string to a number and validate */

int val_str2numb(char *s, int *numb, char *subst, GtkWidget *window)
{
    int i;
    char *end;

    if (strlen(s) > 0)
    {
	errno = 0;
	i = strtol(s, &end, 10);

	if (errno != 0)
	{
	    app_msg("APP0002", subst, window);
	    return FALSE;
	}
	else if (*end)
	{
	    app_msg("APP0002", subst, window);
	    return FALSE;
	}
    }
    else
    {
    	i = 0;
    }

    *numb = i;

    return TRUE;
}


/* Check and print error message */

int check_errno(char *s)
{
    int err;

    if (errno != 0)
    {
	printf("%s %s - error: (%d) %s\n", debug_hdr, s, errno, strerror(errno));
	return errno;
    }

    return 0;
}


/* Return the current time in milliseconds */

int64_t msec_time()
{
    int64_t msecs;
    struct timespec t;

    clock_gettime(CLOCK_REALTIME_COARSE, &t);
    msecs = t.tv_sec * INT64_C(1000) + t.tv_nsec / 1000000;

    return msecs;
}


/* Show binary representation of value (useful debug) */

void print_bits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for(i = size - 1; i >= 0; i--)
    {
        for(j = 7; j >= 0; j--)
        {
            byte = b[i] & (1<<j);
            byte >>= j;
            printf("%u", byte);
        }
    }

    printf("\n");
    fflush(stdout);
}


/* Return the parent of a widget */

GtkWidget * find_parent(GtkWidget *init_widget)
{
    GtkWidget *parent_contnr;

    parent_contnr = gtk_widget_get_parent(init_widget);

    if (! GTK_IS_CONTAINER(parent_contnr))
    {
	log_msg("SYS9011", "From initial widget", NULL, NULL);
    	return NULL;
    }

    return parent_contnr;
}


/* Search for a child widget using the widget name */

GtkWidget * find_widget_by_name(GtkWidget *parent_contnr, char *nm)
{
    GtkWidget *widget;
    const gchar *widget_name;

    if (! GTK_IS_CONTAINER(parent_contnr))
    {
	log_msg("SYS9011", "By name", NULL, NULL);
    	return NULL;
    }

    GList *child_widgets = gtk_container_get_children(GTK_CONTAINER (parent_contnr));

    child_widgets = g_list_first(child_widgets);

    while (child_widgets != NULL)
    {
	widget = child_widgets->data;
	widget_name = gtk_widget_get_name (widget);

	if (widget_name != NULL)
	{
	    if (strcmp(widget_name, nm) == 0)
	    {
		g_list_free (child_widgets);
		return widget;
	    }
	}

	child_widgets = g_list_next(child_widgets);
    }

    g_list_free (child_widgets);

    return NULL;
}


/* Search for a widget using the parent of an initiating widget */

GtkWidget * find_widget_by_parent(GtkWidget *init_widget, char *nm)
{
    GtkWidget *widget;
    GtkWidget *parent_contnr;
    const gchar *widget_name;

    parent_contnr = gtk_widget_get_parent(init_widget);

    if (! GTK_IS_CONTAINER(parent_contnr))
    {
	log_msg("SYS9011", "By parent", NULL, NULL);
    	return NULL;
    }

    GList *child_widgets = gtk_container_get_children(GTK_CONTAINER (parent_contnr));

    child_widgets = g_list_first(child_widgets);

    while (child_widgets != NULL)
    {
	widget = child_widgets->data;
	widget_name = gtk_widget_get_name (widget);

	if (strcmp(widget_name, nm) == 0)
	{
	    g_list_free (child_widgets);
	    return widget;
	}

	child_widgets = g_list_next(child_widgets);
    }

    g_list_free (child_widgets);

    return NULL;
}


/* Find all the control widgets in a container */

GList * ctrl_widget_list(GtkWidget *contr, GtkWidget *window)
{
    GtkWidget *widget;
    GList *ctl_list = NULL;
    GList *tmp_list = NULL;

    if (! GTK_IS_CONTAINER(contr))
    {
	log_msg("SYS9011", "Get Next Control", "SYS9011", window);
	return NULL;
    }

    GList *child_widgets = gtk_container_get_children(GTK_CONTAINER (contr));
    child_widgets = g_list_first(child_widgets);

    while (child_widgets != NULL)
    {
	widget = child_widgets->data;

	if ((GTK_IS_RANGE (widget)) || (GTK_IS_COMBO_BOX_TEXT (widget)) || (GTK_IS_RADIO_BUTTON (widget)))
	{
	    ctl_list = g_list_prepend(ctl_list, widget);
	}
	else if (GTK_IS_CONTAINER (widget))
	{
	    tmp_list = ctrl_widget_list(widget, window);
	    ctl_list = g_list_concat(ctl_list, tmp_list);
	}

	child_widgets = g_list_next(child_widgets);
    }

    return ctl_list;
}


/* Get a string for the current time */

void cur_date_str(char *date_str, int s_sz, char *fmt)
{
    struct tm *tm;
    time_t current_time;
    size_t sz;

    *date_str = '\0';
    current_time = time(NULL);
    tm = localtime(&current_time);
    sz = strftime(date_str, s_sz, fmt, tm);

    return;
}
