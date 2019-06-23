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

#define SD_W 1600
#define SD_H 900
#define SD_SZ 3


/* Includes */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>  
#include <gdk/gdk.h> 


/* Prototypes */

void set_css();
void check_screen_res();
void get_screen_res(GdkRectangle *);
void css_adjust_font_sz();


/* Globals */

static const char *debug_hdr = "DEBUG-css.c ";

/*  16.04
*/

//static const gchar *css_data = 
static gchar *css_data = 
	"@define-color DARK_BLUE rgba(0%,0%,50%,1.0); "
	"@define-color METAL_GREY rgba(55,83,103,1.0); "
	"GtkButton, GtkEntry, GtkLabel { font-family: Sans; font-size: 12px; }"
	"GtkLabel#data_1 { color: @DARK_BLUE; }"
	"GtkLabel#data_2 { color: #800000; font-family: Sans; font-size: 11px; }"
	"GtkLabel#data_3 { color: #400080; font-family: Sans; font-size: 10px; }"
	"GtkLabel#title_1 { font-family: Sans; font-size: 18px; font-weight: bold; }"
	"GtkLabel#title_2 { font-family: Serif; font-size: 18px; font-style: italic; color: #fa8072; }"
	"GtkLabel#title_3 { font-family: Sans; font-size: 12px; color: @DARK_BLUE;}"
	"GtkLabel#title_4 { font-family: Sans; font-size: 12px; font-weight: bold; }"
	"GtkLabel#title_5 { font-family: Sans; font-size: 12px; color: #e00b40;}"
	"GtkLabel#status { font-family: Sans; font-size: 12px; color: #b8860b; font-style: italic; }"
	"GtkEntry#ent_1 { color: @DARK_BLUE; }"
	"GtkRadioButton#rad_1 { color: @DARK_BLUE; font-family: Sans; font-size: 12px; }"
	"GtkRadioButton > GtkLabel { color: @DARK_BLUE; font-family: Sans; font-size: 12px; }"
	"GtkFrame { background-color: #e6e6fa; border-radius: 8px}"
	"GtkFrame > GtkLabel { color: #800000; font-weight: 500; }"
	"GtkComboboxText * { color: @METAL_GREY; font-family: Sans; font-size: 12px; }"
	"GtkProgressBar#pbar_1 { color: @DARK_BLUE; font-family: Sans; font-size: 10px; }"
	"#button_1 * { color: #708090; font-weight: bold; }"
	"GtkNotebook * { font-family: Sans; font-size: 11px; }"
	"GtkTextView { font-family: Sans; font-size: 12px; }"
	"GtkTextView#txtview_1 { font-family: Sans; font-size: 11px; }"
	"GtkLinkButton { font-family: Sans; font-size: 12px; color: @DARK_BLUE; }";

/*  18.04     !@#$%^ Pain! At 18.04 the selectors became proper selector names, not the Gtk name

static const gchar *css_data = 
	"@define-color DARK_BLUE rgba(0%,0%,50%,1.0); "
	"@define-color METAL_GREY rgba(55,83,103,1.0); "
	"button, entry, label { font-family: Sans; font-size: 12px; }"
	"label#data_1 { color: @DARK_BLUE; }"
	"label#data_2 { color: #800000; font-family: Sans; font-size: 11px; }"
	"label#data_3 { color: #400080; font-family: Sans; font-size: 10px; }"
	"label#title_1 { font-family: Sans; font-size: 18px; font-weight: bold; }"
	"label#title_2 { font-family: Serif; font-size: 18px; font-style: italic; color: #fa8072; }"
	"label#title_3 { font-family: Sans; font-size: 12px; color: @DARK_BLUE;}"
	"label#title_4 { font-family: Sans; font-size: 12px; font-weight: bold; }"
	"label#title_5 { font-family: Sans; font-size: 12px; color: #e00b40;}"
	"label#status { font-family: Sans; font-size: 12px; color: #b8860b; font-style: italic; }"
	"entry#ent_1 { color: @DARK_BLUE; }"
	"radiobutton#rad_1 { color: @DARK_BLUE; font-family: Sans; font-size: 12px; }"
	"radiobutton > label { color: @DARK_BLUE; font-family: Sans; font-size: 12px; }"
	"frame { background-color: #e6e6fa; border-radius: 8px}"
	"frame > label { color: #800000; font-weight: 500; }"
	"combobox * { color: @METAL_GREY; font-family: Sans; font-size: 13px; }"
	"progressbar#pbar_1 { color: @DARK_BLUE; font-family: Sans; font-size: 10px; }"
	"#button_1 * { color: #708090; font-weight: bold; }"
	"notebook * { font-family: Sans; font-size: 11px; }"
	"textview { font-family: Sans; font-size: 12px; }"
	"textview#txtview_1 { font-family: Sans; font-size: 11px; }"
	"button.link { font-family: Sans; font-size: 12px; color: @DARK_BLUE; }";
*/


// These don't work
//"GtkLabel#title_deco1 { font: Comic Sans 15; font-weight: 500; color: #fa8072 }"
//"GtkLabel#title_deco2 { font-family: Comic Sans; font-size: 15px; font-style: italic; color: #fa8072 }"


/* Set up provider data and apply */

void set_css()
{
    GError *err = NULL;

    check_screen_res();

    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);

    gtk_style_context_add_provider_for_screen(screen,
    					      GTK_STYLE_PROVIDER(provider),
    					      GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(provider),
				    (const char *) css_data,
				    -1,
				    &err);

    if (err != NULL)
    {
    	printf("%s set_css  ****css error  %s\n", debug_hdr, err->message); fflush(stdout);
    	g_clear_error (&err);
    }

    g_object_unref(provider);

    return;
}


/* Check the screen resolution and need to adjust the font size */

void check_screen_res()
{
    GdkRectangle workarea = {0};

    get_screen_res(&workarea); 
    printf ("%s get_screen_res W: %u x H:%u\n", debug_hdr, workarea.width, workarea.height);
	
    // Default font size suits Full HD resolution, but lesser res needs needs lesser font size to stop
    // Inodeum looking too large. If approaching FHD, keep the default.
    // SD_W and SD_H are not really standard def, but serve as a good cut-off point.
    if (workarea.width > SD_W || workarea.height > SD_H)
    	return;

    css_adjust_font_sz();

    return;
}


/* Get the screen resolution and apply the appropriate font */

void get_screen_res(GdkRectangle *workarea)
{
    gdouble res;
    GdkScreen *scr;

    /* 16.04
    */

    if ((scr = gdk_screen_get_default ()) == NULL)
    	return;
    
    gdk_screen_get_monitor_workarea (scr, 0, workarea);


    /* 18.04
    
    gdk_monitor_get_workarea (gdk_display_get_primary_monitor (gdk_display_get_default()),
			      workarea);
    */

    return;
}


/* Adjust the font size down */

void css_adjust_font_sz()
{
    int i, j, fn_len, new_fn_len;
    char *p;
    char num[4];

    /* Extract and adjust the font size */
    p = css_data;

    while ((p = strstr(p, "px")) != NULL)
    {
    	/* Determine the font size */
    	for(i = 1; *(p - i) != ' '; i++);
    	
    	j = 0;
    	i--;
    	fn_len = i;
	printf("%s fn_len is: %d\n", debug_hdr, fn_len); fflush(stdout);

	while(j < i)
	{
	    num[j] = *(p - i + j);
	    j++;
	}

	num[j] = '\0';

	/* Convert and decrease */
	i = atoi(num) - SD_SZ;
	sprintf(num, "%d", i);
	printf("%s new num is: %s\n", debug_hdr, num); fflush(stdout);

	/* Substitute new number, may have tp pad some spaces */
	new_fn_len = strlen(num);

	for(i = new_fn_len; i < fn_len; i++)
	{
	    j = p - fn_len + i;
	    printf("%s j: %d  fn_len: %d  new_fn_len: %d   i: %d\n", debug_hdr, j, fn_len, new_fn_len, i); fflush(stdout);
	    *(p - fn_len + i) = ' ';
	}

	printf("%s num: %s\n", debug_hdr, num); fflush(stdout);
	for(i = 0; i < new_fn_len; i++)
	    *(p - new_fn_len + i) = num[i];

	/* Advance to next */
	p++;
    }
	printf("%s css is: %s\n", debug_hdr, css_data); fflush(stdout);

    return;
}
