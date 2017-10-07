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
** Description:	Show application version, license, credits etc.
**
** Author:	Anthony Buckley
**
** History
**	10-Jun-2017	Initial code
**
*/



/* Defines */

#define DESC_MAX 2


/* Includes */

#include <stdlib.h>  
#include <string.h>  
#include <libgen.h>  
#include <stdio.h>
#include <gtk/gtk.h>  
#include <main.h>
#include <defs.h>
#include <version.h>


/* Types */


/* Prototypes */

void about_panel(MainUi *m_ui);
GtkWidget * about_hdr(MainUi *);
GtkWidget * about_misc(MainUi *);
GtkWidget * about_tabnb(MainUi *);
GtkWidget * new_page(int);
void add_lic_link(GtkTextBuffer **, GtkWidget **);


/* Globals */

static const char *debug_hdr = "DEBUG-about.c ";

static const char *about_text[][2] =
{
    { "License", "Copyright (C) 2017  Anthony Buckley\n\n"
    		 "This program comes with ABSOLUTELY NO\n"
    		 "WARRANTY. See the GNU General Public\n" 
		 "License, version 3 or later for details.\n" },
    { "Credits", "Tony Buckley\t tony.buckley000@gmail.com\n" }
};
static const int txt_max = 2;

static const char *license_url[] =
{
    "http://www.gnu.org/licenses/gpl.html",
    "http://www.gnu.org/licenses/gpl-3.0.html"
};
static const int url_max = 2;

const char *desc[] = 
    {
	"An Internet usage monitor, plus more,",
	"for Internode Linux users."
    };



/* Application 'About' Information display panel */

void about_panel(MainUi *m_ui)
{  
    /* Header */
    m_ui->hdr_box = about_hdr(m_ui);

    /* General information */
    m_ui->misc_box = about_misc(m_ui);

    /* License and Credits */
    m_ui->tab_nb = about_tabnb(m_ui);
    
    /* Combine everything for display */
    m_ui->about_cntr = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_name(m_ui->about_cntr, "about_panel");
    gtk_widget_set_margin_top (m_ui->about_cntr, 10);
    gtk_widget_set_margin_left (m_ui->about_cntr, 5);

    gtk_box_pack_start (GTK_BOX (m_ui->about_cntr), m_ui->hdr_box, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (m_ui->about_cntr), m_ui->misc_box, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (m_ui->about_cntr), m_ui->tab_nb, FALSE, FALSE, 0);

    /* Add to the panel stack */
    gtk_stack_add_named (GTK_STACK (m_ui->panel_stk), m_ui->about_cntr, "about_panel");

    return;
}


/* Header information - icon, application, version */

GtkWidget * about_hdr(MainUi *m_ui)
{  
    GtkWidget *hdr_box;
    GtkWidget *label_t, *label_v;
    char *app_icon;
    char *s;

    /* Title and version */
    label_t = gtk_label_new(TITLE);
    gtk_widget_set_name(label_t, "title_2");

    s = (char *) malloc(strlen(VERSION) + 12);
    sprintf(s, "(Version: %s)", VERSION);
    label_v = gtk_label_new(s);
    gtk_widget_set_name(label_v, "title_3");
    gtk_widget_set_margin_start(GTK_WIDGET (label_v), 30);
    free(s);
    gtk_widget_set_halign (label_v, GTK_ALIGN_END);
    gtk_widget_set_valign (label_v, GTK_ALIGN_END);

    /* Icon */
    app_icon = g_strconcat (PACKAGE_DATA_DIR, "/pixmaps/", TITLE, "/inodeum.png",NULL);
    m_ui->app_icon = gtk_image_new_from_file(app_icon);
    g_free(app_icon);
    gtk_widget_set_margin_end(GTK_WIDGET (m_ui->app_icon), 15);

    /* Pack */
    hdr_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign (hdr_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (hdr_box, GTK_ALIGN_CENTER);

    gtk_box_pack_start (GTK_BOX (hdr_box), m_ui->app_icon, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hdr_box), label_t, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hdr_box), label_v, FALSE, FALSE, 0);

    return hdr_box;
}


/* General information - decsription, homepage */

GtkWidget * about_misc(MainUi *m_ui)
{  
    int i;
    char *s;
    GtkWidget *misc_box;
    GtkWidget *label_t[DESC_MAX];

    /* Description */
    misc_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    gtk_widget_set_halign (misc_box, GTK_ALIGN_START);
    gtk_widget_set_margin_top(GTK_WIDGET (misc_box), 5);
    gtk_widget_set_margin_start(GTK_WIDGET (misc_box), 5);

    for(i = 0; i < DESC_MAX; i++)
    {
	label_t[i] = gtk_label_new(desc[i]);
	gtk_widget_set_halign (label_t[i], GTK_ALIGN_START);
	gtk_box_pack_start (GTK_BOX (misc_box), label_t[i], FALSE, FALSE, 0);
    }

    /* Web page */
    s = (char *) malloc(strlen(TITLE) + 9);
    sprintf(s, "%s Web Page", TITLE);
    m_ui->home_page = gtk_link_button_new_with_label (APP_URI, s);
    free(s);
    gtk_widget_set_halign (m_ui->home_page, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(GTK_WIDGET (m_ui->home_page), 5);
    gtk_widget_set_margin_bottom(GTK_WIDGET (m_ui->home_page), 5);
    gtk_box_pack_start (GTK_BOX (misc_box), m_ui->home_page, FALSE, FALSE, 0);

    return misc_box;
}


/* License and Credits */

GtkWidget * about_tabnb(MainUi *m_ui)
{  
    int i;
    GtkWidget *tab_nb;

    /* Setup */
    tab_nb = gtk_notebook_new();
    gtk_notebook_set_scrollable (GTK_NOTEBOOK(tab_nb), FALSE);

    /* Tab pages */
    for(i = 0; i < txt_max; i++)
    {
	if (gtk_notebook_append_page (GTK_NOTEBOOK (tab_nb), 
				      new_page(i), 
				      gtk_label_new (about_text[i][0])) == -1)
	    return NULL;
    }

    return tab_nb;
}


/* New tabbed notebook page */

GtkWidget * new_page(int i)
{  
    GtkWidget *scroll_win;
    GtkWidget *txt_view;  
    GtkTextBuffer *txt_buffer;  

    /* TextView */
    txt_view = gtk_text_view_new();
    gtk_widget_set_name (txt_view, "txtview_1");
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (txt_view), GTK_WRAP_WORD);
    txt_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (txt_view));
    gtk_text_buffer_set_text (txt_buffer, about_text[i][1], -1);

    /* Page specific additions */
    switch(i)
    {
    	case 0:
	    add_lic_link(&txt_buffer, &txt_view);
	    break;

	default:
	    break;
    };

    gtk_text_view_set_editable (GTK_TEXT_VIEW (txt_view), FALSE);

    /* Scrolled window for TextView */
    scroll_win = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scroll_win),
    				   GTK_POLICY_EXTERNAL,
    				   GTK_POLICY_EXTERNAL);
    gtk_widget_set_size_request (scroll_win, 200, 120);
    gtk_container_add(GTK_CONTAINER(scroll_win), txt_view);
    gtk_container_set_border_width(GTK_CONTAINER(scroll_win), 3);

    return scroll_win;
}


/* Add links to the License page */

void add_lic_link(GtkTextBuffer **txt_buffer, GtkWidget **txt_view)
{  
    int i;
    GtkTextChildAnchor *anchor_lnk;
    GtkTextIter iter;

    for(i = 0; i < url_max; i++)
    {
	GtkWidget *lic_url = gtk_link_button_new (license_url[i]);
	gtk_text_buffer_get_end_iter (*txt_buffer, &iter);
	anchor_lnk = gtk_text_buffer_create_child_anchor (*txt_buffer, &iter);
	gtk_text_view_add_child_at_anchor (GTK_TEXT_VIEW (*txt_view), lic_url, anchor_lnk);
	gtk_text_iter_forward_to_end (&iter);
	gtk_text_buffer_insert (*txt_buffer, &iter, "\n", -1);
    }

    return;
}
