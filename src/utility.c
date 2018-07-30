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
**  Message (Errors, Warnings, Informaton) and Logging utilities
**  Application utilities
**  String utilities
**  Session and Window management utilities
**  General usage utilities
**  File utilities
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
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <gtk/gtk.h>
#include <defs.h>


/* Prototypes */

int check_app_dir();
int reset_log();
void log_msg(char*, char*, char*, GtkWidget*);
void app_msg(char*, char *, GtkWidget*);
void log_status_msg(char *, char *, char *, char *, GtkWidget *);
void info_dialog(GtkWidget *, char *, char *);
void get_msg(char*, char*, char*);
void close_log();
void register_window(GtkWidget *);
void deregister_window(GtkWidget *);
int is_ui_reg(char *, int);
void free_window_reg();
void close_open_ui();
int val_str2dbl(char *, double *, char *, GtkWidget *);
void string_trim(char*);
char * log_name();
char * app_dir_path();
char * home_dir();
void set_sz_abbrev(char *s);
int check_errno();
void strlower(char *, char *);
int stat_file(char *, struct stat *);
long read_file_all(char *, char *);
FILE * open_file(char *, char *);
int read_file(FILE *, char *, int);
GtkWidget * find_widget_by_data(GtkWidget *, char *, const gchar *, char *);

extern void cur_date_str(char *, int, char *);


/* Globals */

static const char *app_messages[][2] = 
{ 
    { "MSG0001", "Session started. "},
    { "MSG0002", "Session ends. "},
    { "MSG0003", "%s "},
    { "MSG0004", "Warning: Inconsistent 'Unit' encountered - %s. "},
    { "MSG0005", "The Service Plan has changed significantly. Please restart Inodeum. "},
    { "INF0001", "Connection error (see log file) %s "},
    { "INF0002", "Service query error (see log file) %s "},
    { "INF0003", "Connecting... %s "},
    { "INF0004", "Handshaking... %s "},
    { "INF0005", "Retrieving usage details... %s "},
    { "INF0006", "Default service error (see log file) %s "},
    { "INF0007", "XML parse error (see log file) %s "},
    { "INF0008", "HTML error (see log file) %s "},
    { "INF0009", "Username / Password invalid (see log file) %s "},
    { "INF0010", "Client error (see log file) %s "},
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
    { "ERR0025", "HTML - %s - response status found. "},
    { "ERR0026", "Username / Password invalid "},
    { "ERR0027", "Unable to access Keyring. Error: %s. "},
    { "ERR0028", "Could not remove user credentials. "},
    { "ERR0029", "Non-numeric character (%s) entered. "},
    { "ERR0030", "Error: XML tag %s not found. "},
    { "ERR0031", "Error: XML tag attribute %s not found. "},
    { "ERR0032", "Error: XML tag value not found. "},
    { "ERR0033", "Error: No items found in returned XML for %s. "},
    { "ERR0034", "Error: User Default Service - %s - not found. "},
    { "ERR0035", "Warning: Unexpected Tag attribute found - %s. "},
    { "ERR0036", "Warning: One or more Tag attributes not found. "},
    { "ERR0037", "You must enter your ISP login Username. "},
    { "ERR0038", "You must enter your ISP login password. "},
    { "ERR0039", "Invalid XML format found: %s. "},
    { "ERR0040", "Unable to convert %s to a number. "},
    { "ERR0041", "File %s does not exist or cannot be read. "},
    { "ERR0042", "File error %s "},
    { "ERR0043", "Failed to create file: %s "},
    { "ERR0044", "Failed to create data refresh timer. "},
    { "ERR0045", "Date format error. "},
    { "ERR0046", "Calendar date field cannot be NULL. "},
    { "ERR0047", "Error determining Network devices. "},
    { "ERR0048", "Error starting network speed monitor thread. "},
    { "ERR9998", "Error: %s. "},
    { "ERR9999", "Error - Unknown error message given. "}			// NB - MUST be last
};

static const int Msg_Count = 65;
static char *Home;
static char *logfile = NULL;
static char *app_dir;
static FILE *lf = NULL;
static int app_dir_len;
static const char *debug_hdr = "DEBUG-utility.c ";
static GList *open_ui_list_head = NULL;
static GList *open_ui_list = NULL;
static char msg[512];



/*****  MESSAGE (ERRORS, WARNINGS, INFORMATON) AND lOGGING UTILITIES *****/



/* Process additional application messages and error conditions */

void app_msg(char *msg_id, char *opt_str, GtkWidget *window)
{
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
	fprintf(lf, "%s\n", app_msg_extra);

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


/* Add a message to the log file and display text in the info status area */

void log_status_msg(char *msg_id, char *opt_str, char *inf_id, char *opt_inf, GtkWidget *status_info)
{
    /* Log file */
    log_msg(msg_id, opt_str, NULL, NULL);

    /* Lookup the message */
    get_msg(msg, inf_id, opt_inf);

    gtk_label_set_text (GTK_LABEL (status_info), msg + strlen(inf_id) + 2);

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



/*****  APPLICATION UTILITIES *****/



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



/*****  STRING UTILITIES *****/



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


/* Convert a string to lowercase */

void strlower(char *s1, char *s2)
{
    for(; *s1 != '\0'; s1++, s2++)
    	*s2 = tolower(*s1);

    *s2 = *s1;

    return;
}


/* Convert a string to a (double) number and validate */

int val_str2dbl(char *s, double *numb, char *subst, GtkWidget *window)
{
    double dbl;
    char *end;

    if (strlen(s) > 0)
    {
	errno = 0;
	//l = strtol(s, &end, 10);
	dbl = strtod(s, &end);

	if (errno != 0)
	{
	    if (subst != NULL)
		app_msg("ERR0040", subst, window);
	    return FALSE;
	}
	else if (*end)
	{
	    if (subst != NULL)
		app_msg("ERR0040", subst, window);
	    return FALSE;
	}
    }
    else
    {
    	dbl = 0;
    }

    *numb = dbl;

    return TRUE;
}



/*****  SESSION AND WINDOW MANAGEMENT UTILITIES *****/



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


/* Free the window register */

void free_window_reg()
{
    g_list_free (open_ui_list_head);

    return;
}



/*****  GENERAL USAGE UTILITIES *****/



/* Abbreviate (a sizing) text */

void set_sz_abbrev(char *s)
{
    int len;
    double dv;
    char abbr[6];

    len = strlen(s);

    if (len < 4)	// Bytes
    {
	strcpy(abbr, "Bytes");
	dv = 1.0;
	s = (char *) realloc(s, len + 6);
    }
    else if (len < 7)	// Kilobytes
    {
    	strcpy(abbr, "KB");
    	dv = 1000.0;
    }
    else if (len < 10)	// Megabytes
    {
    	strcpy(abbr, "MB");
    	dv = 1000000.0;
    }
    else		// Gigabytes
    {
    	strcpy(abbr, "GB");
    	dv = 1000000000.0;
    }

    sprintf(s, "%0.2f %s", atof(s)/dv, abbr);

    return;
}


/* Check and print error message */

int check_errno()
{
    int i, max;
    char err[20];
    char *p;

    if (errno != 0)
    {
	max = sizeof(app_msg_extra) - 1;
	p = strerror(errno);
	printf("Error: %d  %s\n", errno, p);


	for(i = 0; i < max && p[i] != '\0'; i++)
	    app_msg_extra[i] = p[i];

	app_msg_extra[i] = '\0';

	sprintf(err, "%d", errno);
	log_msg("ERR9998", err, NULL, NULL);
	return errno;
    }

    return 0;
}



/*****  FILE UTILITIES *****/



/* Stat a file */

int stat_file(char *fn, struct stat *buf)
{
    int status;

    status = stat(fn, buf);

    if (status < 0)
    {
    	check_errno();
	return FALSE;
    }
    
    return TRUE;
}


/* Read an entire file */

long read_file_all(char *fn, char *s)
{
    long sz;
    FILE *fp;
    struct stat buf;

    /* Get file size */
    if (stat_file(fn, &buf) == FALSE)
    	return -1;

    sz = (long) buf.st_size;
    s = (char *) malloc(sz + 1);

    /* Open */
    fp = fopen(fn, "r");

    if (! fp)
    	return -1;
    
    /* Read */
    if (fgets(s, sz, fp) == NULL)
    {
    	fclose(fp);
    	return -1;
    };

    /* Close */
    fclose(fp);

    return sz;
}


/* Open a file */

FILE * open_file(char *fn, char *op)
{
    FILE *fd;

    if ((fd = fopen(fn, op)) == (FILE *) NULL)
    {
    	check_errno();
	return (FILE *) NULL;
    }

    return fd;
}


/* Read a file */

int read_file(FILE *fd, char *buf, int sz_len)
{
    int i, max;
    char c;

    i = 0;
    max = sz_len - 1;
    buf[0] = '\0';
    
    while((c = fgetc(fd)) != EOF)
    {
    	buf[i++] = c;

    	if (i >= max)
	    break;
    }

    buf[i] = '\0';

    if (c == EOF)
    {
	fclose(fd);
	return (int) c;
    }
    else
    {
	return i;
    }
}


/* Search for a child widget using object data for the widget */

GtkWidget * find_widget_by_data(GtkWidget *parent_contnr, char *nm, const gchar *data_key, char *data_val)
{
    GtkWidget *widget;
    const gchar *widget_name;
    char *val;

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
		val = g_object_get_data(G_OBJECT (widget), data_key);

		if (val != NULL)
		{
		    if (strcmp(val, data_val) == 0)
		    {
			g_list_free (child_widgets);
			return widget;
		    }
		}
	    }
	}

	child_widgets = g_list_next(child_widgets);
    }

    g_list_free (child_widgets);

    return NULL;
}
