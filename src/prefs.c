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
#include <defs.h>


/* Defines */

#define PREF_KEY_SZ 10


/* Types */

typedef struct _user_pref
{
    char key[10];
    char val[10];
} UserPref;


/* Prototypes */

void pref_panel(MainUi *);
int get_user_pref(char *, char **);
int set_user_pref(char *, char *);
int add_user_pref(char *, char *);
int read_user_prefs(GtkWidget *);
int write_user_prefs(GtkWidget *);
void set_default_prefs();
void free_prefs();
GtkWidget * reset_pw(MainUi *);
GtkWidget * std_prefs(MainUi *);

extern void set_panel_btn(GtkWidget *, char *, GtkWidget *, int, int, int, int);
extern void create_label(GtkWidget **, char *, char *, GtkWidget *, int, int, int, int);
extern void create_entry(GtkWidget **, char *, GtkWidget *, int, int);
extern void create_radio(GtkWidget **, GtkWidget *, char *, char *, GtkWidget *, 
			 int, int, int, int, int,
			 char *, char *);
extern void log_msg(char*, char*, char*, GtkWidget*);
extern char * app_dir_path();
extern void string_trim(char*);
extern void OnResetPW(GtkWidget*, gpointer);
extern void OnPrefSave(GtkWidget*, gpointer);
extern void OnPrefPieLbl(GtkToggleButton*, gpointer);
extern void OnPrefPieLgd(GtkToggleButton*, gpointer);
extern void OnPrefBarLbl(GtkToggleButton*, gpointer);


/* Globals */

static const char *debug_hdr = "DEBUG-prefs.c ";
static GList *pref_list = NULL;



/* Create widgets for the preference panel */

void pref_panel(MainUi *m_ui)
{
    GtkWidget *pw_cntr;
    GtkWidget *std_cntr;

    /* Create preference container */
    m_ui->pref_cntr = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_name(m_ui->pref_cntr, "pref_panel");
    gtk_widget_set_margin_top (m_ui->pref_cntr, 5);
    gtk_widget_set_margin_left (m_ui->pref_cntr, 5);

    /* Delete saved password */
    pw_cntr = reset_pw(m_ui);
    gtk_box_pack_start (GTK_BOX (m_ui->pref_cntr), pw_cntr, FALSE, FALSE, 0);

    /* Standard preferences (charts, refresh) */
    std_cntr = std_prefs(m_ui);
    gtk_box_pack_start (GTK_BOX (m_ui->pref_cntr), std_cntr, FALSE, FALSE, 0);

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


/* Standard preferences (charts, refresh) */

GtkWidget * std_prefs(MainUi *m_ui)
{
    char *p;
    int i, idx[3];
    GtkWidget *frame;
    GtkWidget *grid, *tbox;
    GtkWidget *lbl;
    GtkWidget *save_btn;
    GtkWidget *radio, *radio_grp;

    /* Containers */
    frame = gtk_frame_new("General Preferences");
    grid = gtk_grid_new();

    /* Label */
    create_label(&(lbl), "title_4", "Usage pie", grid, 0, 0, 1, 1);
    gtk_widget_set_margin_start(lbl, 5);

    /* Set label and percentage options */
    get_user_pref(OV_PIE_LBL, &p);
    memset(&idx, 0, sizeof(idx));
    i = atoi(p);
    idx[i] = TRUE;

    create_radio(&radio, NULL, "Label", "rad_1", grid, idx[0], 0, 1, 1, 1, "idx", "0");
    g_signal_connect (radio, "toggled", G_CALLBACK (OnPrefPieLbl), m_ui);
    radio_grp = radio;
    gtk_widget_set_halign(radio, GTK_ALIGN_END);
    gtk_widget_set_valign(radio, GTK_ALIGN_CENTER);

    create_radio(&radio, radio_grp, "Percent", "rad_1", grid, idx[1], 1, 1, 1, 1, "idx", "1");
    g_signal_connect (radio, "toggled", G_CALLBACK (OnPrefPieLbl), m_ui);
    gtk_widget_set_halign(radio, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(radio, GTK_ALIGN_CENTER);

    create_radio(&radio, radio_grp, "Both", "rad_1", grid, idx[2], 2, 1, 1, 1, "idx", "2");
    g_signal_connect (radio, "toggled", G_CALLBACK (OnPrefPieLbl), m_ui);
    gtk_widget_set_halign(radio, GTK_ALIGN_START);
    gtk_widget_set_valign(radio, GTK_ALIGN_CENTER);

    /* Label */
    create_label(&(lbl), "typ_lbl", "Description", grid, 0, 2, 1, 1);
    gtk_widget_set_margin_start(lbl, 5);
    gtk_widget_set_margin_top (lbl, 0);

    /* Set legend options */
    get_user_pref(OV_PIE_LGD, &p);
    memset(&idx, 0, sizeof(idx));
    i = atoi(p);
    idx[i] = TRUE;

    create_radio(&radio, NULL, "On Chart", "rad_1", grid, idx[0], 1, 2, 1, 1, "idx", "0");
    g_signal_connect (radio, "toggled", G_CALLBACK (OnPrefPieLgd), m_ui);
    radio_grp = radio;
    gtk_widget_set_margin_top (radio, 5);
    gtk_widget_set_margin_start (radio, 10);
    gtk_widget_set_halign(radio, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(radio, GTK_ALIGN_CENTER);

    create_radio(&radio, radio_grp, "Legend", "rad_1", grid, idx[1], 2, 2, 1, 1, "idx", "1");
    g_signal_connect (radio, "toggled", G_CALLBACK (OnPrefPieLgd), m_ui);
    gtk_widget_set_halign(radio, GTK_ALIGN_START);
    gtk_widget_set_valign(radio, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top (radio, 5);
    gtk_widget_set_margin_end (radio, 10);

    /* Label */
    create_label(&(lbl), "title_4", "Rollover bar", grid, 0, 3, 1, 1);
    gtk_widget_set_margin_start(lbl, 5);

    /* Set label and percentage options */
    get_user_pref(OV_BAR_LBL, &p);
    memset(&idx, 0, sizeof(idx));
    i = atoi(p);
    idx[i] = TRUE;

    create_radio(&radio, NULL, "Label", "rad_1", grid, idx[0], 0, 4, 1, 1, "idx", "0");
    g_signal_connect (radio, "toggled", G_CALLBACK (OnPrefBarLbl), m_ui);
    radio_grp = radio;
    gtk_widget_set_halign(radio, GTK_ALIGN_END);
    gtk_widget_set_valign(radio, GTK_ALIGN_CENTER);

    create_radio(&radio, radio_grp, "Percent", "rad_1", grid, idx[1], 1, 4, 1, 1, "idx", "1");
    g_signal_connect (radio, "toggled", G_CALLBACK (OnPrefBarLbl), m_ui);
    gtk_widget_set_halign(radio, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(radio, GTK_ALIGN_CENTER);

    create_radio(&radio, radio_grp, "Both", "rad_1", grid, idx[2], 2, 4, 1, 1, "idx", "2");
    g_signal_connect (radio, "toggled", G_CALLBACK (OnPrefBarLbl), m_ui);
    gtk_widget_set_halign(radio, GTK_ALIGN_START);
    gtk_widget_set_valign(radio, GTK_ALIGN_CENTER);

    /* Label */
    create_label(&(lbl), "title_4", "Refresh every", grid, 0, 5, 1, 1);
    gtk_widget_set_margin_start(lbl, 5);

    /* Set refresh interval */
    get_user_pref(REFRESH_TM, &p);
    m_ui->refresh_tm = gtk_entry_new();  
    gtk_widget_set_name(m_ui->refresh_tm, "data_1");
    gtk_entry_set_text (GTK_ENTRY (m_ui->refresh_tm), p);
    gtk_widget_set_hexpand (m_ui->refresh_tm, FALSE);
    gtk_entry_set_max_length (GTK_ENTRY (m_ui->refresh_tm), 3);
    gtk_entry_set_width_chars (GTK_ENTRY (m_ui->refresh_tm), 3);
    gtk_widget_set_margin_start(m_ui->refresh_tm, 8);
    gtk_widget_set_valign(GTK_WIDGET (m_ui->refresh_tm), GTK_ALIGN_CENTER);
    gtk_widget_set_halign(GTK_WIDGET (m_ui->refresh_tm), GTK_ALIGN_CENTER);

    //g_signal_connect (ent, "toggled", G_CALLBACK (OnPrefRefresh), m_ui);

    /* Label */
    lbl = gtk_label_new("(mins.)");  
    gtk_widget_set_name(lbl, "title_4");

    tbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    gtk_box_pack_start (GTK_BOX (tbox), m_ui->refresh_tm, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (tbox), lbl, FALSE, FALSE, 0);
    gtk_widget_set_valign(GTK_WIDGET (tbox), GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top (tbox, 5);
    gtk_grid_attach(GTK_GRID (grid), tbox, 1, 5, 1, 1);

    /* Save button */
    save_btn = gtk_button_new_with_label("Save");
    gtk_widget_set_margin_top(save_btn, 10);
    gtk_widget_set_margin_bottom(save_btn, 3);
    gtk_widget_set_valign(save_btn, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID (grid), save_btn, 1, 6, 1, 1);
    g_signal_connect (save_btn, "clicked", G_CALLBACK (OnPrefSave), m_ui);

    gtk_container_add(GTK_CONTAINER (frame), grid);

    return frame;
}


/* Return a pointer to a user preference value for a key or NULL */

int get_user_pref(char *key, char **val)
{
    int i;
    UserPref *user_pref;
    GList *l;

    *val = NULL;
    i = 0;

    for(l = pref_list; l != NULL; l = l->next)
    {
    	user_pref = (UserPref *) l->data;

    	if (strcmp(user_pref->key, key) == 0)
    	{
	    *val = user_pref->val;
	    break;
    	}

	i++;
    }

    return i;
}


/* Set a user preference */

int set_user_pref(char *key, char *val)
{
    int i;
    UserPref *user_pref;
    GList *l;

    i = 0;

    for(l = pref_list; l != NULL; l = l->next)
    {
    	user_pref = (UserPref *) l->data;

    	if (strcmp(user_pref->key, key) == 0)
    	{
	    strcpy(user_pref->val, val);
	    break;
    	}

	i++;
    }

    return i;
}


/* Add a user preference */

int add_user_pref(char *key, char *val)
{
    GList *l;

    UserPref *user_pref = (UserPref *) malloc(sizeof(UserPref));
    strcpy(user_pref->key, key);
    strcpy(user_pref->val, val);

    l = g_list_append(pref_list, (gpointer) user_pref);
    pref_list = l;

    return TRUE;
}


/* Read the user preferences file */

int read_user_prefs(GtkWidget *window)
{
    FILE *fd = NULL;
    struct stat fileStat;
    char buf[256];
    char *pref_fn;
    char *app_dir;
    char *p, *p2;
    int app_dir_len;
    int err;
    UserPref *user_pref;

    /* Get the full path for the preferences file */
    app_dir = app_dir_path();
    app_dir_len = strlen(app_dir);
    pref_fn = (char *) malloc(app_dir_len + 19);
    sprintf(pref_fn, "%s/%s", app_dir, USER_PREFS);

    /* If no preferences exist, create a default set */
    err = stat(pref_fn, &fileStat);

    if ((err < 0) || (fileStat.st_size == 0))
    {
    	set_default_prefs();
	free(pref_fn);
	return TRUE;
    }
    
    /* Read existing user preferences */
    if ((fd = fopen(pref_fn, "r")) == (FILE *) NULL)
    {
	free(pref_fn);
	return FALSE;
    }
    
    /* Store the preferences */
    while ((fgets(buf, sizeof(buf), fd)) != NULL)
    {
	/* Check and save key */
	if ((p = strchr(buf, '|')) == NULL)
	{
	    free(pref_fn);
	    sprintf(app_msg_extra, "%s", buf);
	    log_msg("ERR0042", "Invalid user preference key format", "ERR0042", window);
	    return FALSE;
	}

	if ((p - buf) > (PREF_KEY_SZ - 1))
	{
	    free(pref_fn);
	    sprintf(app_msg_extra, "%s", buf);
	    log_msg("ERR0042", "Invalid user preference key size", "ERR0042", window);
	    return FALSE;
	}

	/* Check and save value */
	if ((p2 = strchr((p), '\n')) == NULL)
	{
	    free(pref_fn);
	    sprintf(app_msg_extra, "%s", buf);
	    log_msg("ERR0042", "Invalid user preference value", "ERR0042", window);
	    return FALSE;
	}

	/* Create a preference entry */
	UserPref *user_pref = (UserPref *) malloc(sizeof(UserPref));
	strncpy(user_pref->key, buf, p - buf);
	user_pref->key[p - buf] = '\0';

	p++;
	*p2 = '\0';

	strcpy(user_pref->val, p);
	    
	pref_list = g_list_prepend(pref_list, user_pref);
    }

    /* Still may need set up some default preferences */
    pref_list = g_list_reverse(pref_list);
    set_default_prefs();

    /* Close off */
    fclose(fd);
    free(pref_fn);

    return TRUE;
}


/* Write the user preferences file */

int write_user_prefs(GtkWidget *window)
{
    FILE *fd = NULL;
    char buf[256];
    char *pref_fn;
    char *app_dir;
    int app_dir_len;
    UserPref *user_pref;
    GList *l;

    /* Get the full path for the preferecnes file */
    app_dir = app_dir_path();
    app_dir_len = strlen(app_dir);
    pref_fn = (char *) malloc(app_dir_len + 19);
    sprintf(pref_fn, "%s/user_preferences", app_dir);

    /* New or overwrite file */
    if ((fd = fopen(pref_fn, "w")) == (FILE *) NULL)
    {
	free(pref_fn);
	return FALSE;
    }

    /* Write new values */
    for(l = pref_list; l != NULL; l = l->next)
    {
    	user_pref = (UserPref *) l->data;

    	if (user_pref->val != NULL)
    	{
	    sprintf(buf, "%s|%s\n", user_pref->key, user_pref->val);
	    
	    if ((fputs(buf, fd)) == EOF)
	    {
		free(pref_fn);
		log_msg("ERR0043", pref_fn, "ERR0043", window);
		return FALSE;
	    }
    	}
    }

    /* Close off */
    fclose(fd);
    free(pref_fn);

    return TRUE;
}


/* Set up default user preferences. All preferences may not be present */

void set_default_prefs()
{
    char *p;

    /* Overview pie chart labels */
    get_user_pref(OV_PIE_LBL, &p);

    if (p == NULL)
	add_user_pref(OV_PIE_LBL, "2");

    /* Overview pie chart labels or legend */
    get_user_pref(OV_PIE_LGD, &p);

    if (p == NULL)
	add_user_pref(OV_PIE_LGD, "0");

    /* Overview bar chart labels */
    get_user_pref(OV_BAR_LBL, &p);

    if (p == NULL)
	add_user_pref(OV_BAR_LBL, "1");

    /* Overview bar chart labels */
    get_user_pref(REFRESH_TM, &p);

    if (p == NULL)
	add_user_pref(REFRESH_TM, "30");

    return;
}


/* Free the user preferences */

void free_prefs()
{
    UserPref *user_pref;

    pref_list = g_list_first(pref_list);

    while(pref_list != NULL)
    {
    	user_pref = (UserPref *) pref_list->data;
    	free(user_pref);
	pref_list = g_list_next(pref_list);
    }

    g_list_free(pref_list);

    return;
}
