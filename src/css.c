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
	"GtkButton, GtkEntry, GtkLabel { font: Sans 9; }"
	"GtkLabel#data_label { color: @DARK_BLUE; }"
	"GtkLabel#title_deco1 { font: Comic Sans 15; font-weight: 500; color: #fa8072 }"
	"GtkLabel#title_deco2 { font-family: Comic Sans; font-size: 15px; font-style: italic; color: #fa8072 }"
	"GtkLabel#title_deco3 { font: Serif 15; font-style: italic; color: #fa8072; }"
	"GtkLabel#title_label1 { font: Sans 15; font-weight: bold; }"
	"GtkLabel#title_label2 { font: Sans 9; color: @DARK_BLUE;}"
	"GtkNotebook * { font: Sans 8; }"
	"GtkTextView { font: Sans 9; }"
	"GtkTextView#text_view_8 { font: Sans 8; }"
	"GtkLinkButton { font: Sans 9; color: @DARK_BLUE; }";



/* Set up provider data and apply */

void set_css()
{
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);

    gtk_style_context_add_provider_for_screen(screen,
    					      GTK_STYLE_PROVIDER(provider),
    					      GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(provider),
				    css_data,
				    -1,
				    NULL);

    g_object_unref(provider);

    return;
}
