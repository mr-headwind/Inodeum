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
**  Screen appearance setup
**
** Author:	Anthony Buckley
**
** History
**	23-Jun-2017	Initial code
**
*/


/* Defines */



/* Includes */

#include <stdio.h>
#include <gtk/gtk.h>  
#include <gdk/gdkkeysyms.h> 


/* Prototypes */

void set_css();


/* Globals */

static const char *debug_hdr = "DEBUG-css.c ";

static const gchar *css_data = 
	"@define-color DARK_BLUE rgba(0%,0%,50%,1.0); "
	"@define-color METAL_GREY rgba(55,83,103,1.0); "
	"GtkButton, GtkEntry, GtkLabel { font-family: Sans; font-size: 9px; }"
	"GtkLabel#data_1 { color: @DARK_BLUE; }"
	"GtkLabel#data_2 { color: #800000; font-family: Sans; font-size: 8px; }"
	"GtkLabel#data_3 { color: #400080; font-family: Sans; font-size: 7px; }"
	"GtkLabel#title_1 { font-family: Sans; font-size: 15px; font-weight: bold; }"
	"GtkLabel#title_2 { font-family: Serif; font-size: 15px; font-style: italic; color: #fa8072; }"
	"GtkLabel#title_3 { font-family: Sans; font-size: 9px; color: @DARK_BLUE;}"
	"GtkLabel#title_4 { font-family: Sans; font-size: 9px; font-weight: bold; }"
	"GtkLabel#title_5 { font-family: Sans; font-size: 9px; color: #e00b40;}"
	"GtkLabel#status { font-family: Sans; font-size: 9px; color: #b8860b; font-style: italic; }"
	"GtkEntry#ent_1 { color: @DARK_BLUE; }"
	"GtkRadioButton#rad_1 { color: @DARK_BLUE; font-family: Sans; font-size: 9px; }"
	"GtkRadioButton > GtkLabel { color: @DARK_BLUE; font-family: Sans; font-size: 8px; }"
	"GtkFrame { background-color: #e6e6fa; border-radius: 5px}"
	"GtkFrame > GtkLabel { color: #800000; font-weight: 500; }"
	"GtkDrawingArea#draw_1 { background-color: #ffffff;}"
	"GtkComboBoxText * { color: @METAL_GREY; font-family: Sans; font-size: 9px; }"
	"GtkProgressBar#pbar_1 { color: @DARK_BLUE; font-family: Sans; font-size: 7px; }"
	//"#button_1 * { background-color: #ffe4e1; border-color: #800000; }"
	"#button_1 * { color: #708090; font-weight: bold; }"
	"GtkNotebook * { font-family: Sans; font-size: 8px; }"
	"GtkTextView { font-family: Sans; font-size: 9px; }"
	"GtkTextView#txtview_1 { font-family: Sans; font-size: 8px; }"
	"GtkLinkButton { font-family: Sans; font-size: 9px; color: @DARK_BLUE; }";


// These don't work
//"GtkLabel#title_deco1 { font: Comic Sans 15; font-weight: 500; color: #fa8072 }"
//"GtkLabel#title_deco2 { font-family: Comic Sans; font-size: 15px; font-style: italic; color: #fa8072 }"


/* Set up provider data and apply */

void set_css()
{
    GError *err = NULL;

    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);

    gtk_style_context_add_provider_for_screen(screen,
    					      GTK_STYLE_PROVIDER(provider),
    					      GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(provider),
				    css_data,
				    -1,
				    &err);
				    //NULL);

    if (err != NULL)
    {
    	printf("%s set_css  ****css error  %s\n", debug_hdr, err->message); fflush(stdout);
    	g_clear_error (&err);
    }

    g_object_unref(provider);

    return;
}
