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
** Description: Functions for drawing Cairo charts
**
** Author:	Anthony Buckley
**
** History
**	28-Jul-2017	Initial code
**
*/


/* Includes */

#include <gtk/gtk.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <cairo/cairo.h>
#include <cairo_chart.h>


/* Defines */


/* Types */


/* Prototypes */

PieChart * pie_chart_init(char *, double, int);
int pie_slice_create(PieChart *, char *, double, const GdkRGBA *);
void free_pie_chart(PieChart *);
void free_slices(gpointer);
int draw_pie_chart(cairo_t *, PieChart *, GtkAllocation *);


/* Globals */

static const char *debug_hdr = "DEBUG-cairo_chart.c ";



/* Create and initialise a new pie chart */

// Some rules for creation:-
// . A title is optional (may be NULL). If used, keep as short as possible.
// . Total value is optional (zero). Code will work it out anyway, but might be a useful error check.
// . Legend should be TRUE or FALSE.

PieChart * pie_chart_init(char *title, double total_val, int legend)
{
    PieChart *pc;

    if (legend < 0 || legend > 1)
    	return NULL;

    pc = (PieChart *) malloc(sizeof(PieChart));
    memset(pc, 0, sizeof(PieChart));

    if (title != NULL)
    {
    	pc->chart_title = malloc(strlen(title) + 1);
    	strcpy(pc->chart_title, title);
    }

    //pc->cr = cr;
    pc->total_value = total_val;
    pc->legend = legend;

    return pc;
}


/* Create and initialise a new pie slice */

int pie_slice_create(PieChart *pc, char *desc, double val, const GdkRGBA *colour)
{
    PieSlice *ps;

    ps = (PieSlice *) malloc(sizeof(PieSlice));
    memset(ps, 0, sizeof(PieSlice));

    if (desc != NULL)
    {
    	ps->desc = malloc(strlen(desc) + 1);
    	strcpy(ps->desc, desc);
    }

    ps->slice_value = val;
    ps->colour = colour;
    pc->pie_slices = g_list_append (pc->pie_slices, ps);

    return TRUE;
}


/* Free all pie chart resources */

void free_pie_chart(PieChart *pc)
{
    if (pc->chart_title)
    	free(pc->chart_title);

    g_list_free_full (pc->pie_slices, (GDestroyNotify) free_slices);
    free(pc);

    return;
}


/* Free a pie chart slice */

void free_slices(gpointer data)
{  
    PieSlice *ps;

    ps = (PieSlice *) data;
    
    if (ps->desc)
	free(ps->desc);

    free(ps);

    return;
}


/* Draw a pie chart */

int draw_pie_chart(cairo_t *cr, PieChart *pc, GtkAllocation *allocation)
{
    int r;
    double xc, yc, radius, total_amt, tmp;
    double angle_from, angle_to;
    GList *l;
    PieSlice *ps;
    const GdkRGBA *rgba;

    /* Initial */
    cairo_move_to (cr, 0, 0);
    r = TRUE;

    /* Calculate or verify the total amount */
    total_amt = 0;

    for(l = pc->pie_slices; l != NULL; l = l->next)
    {
    	ps = (PieSlice *) l->data;
    	total_amt += ps->slice_value;
    }

    if (pc->total_value != 0)
    	if (pc->total_value != total_amt)
	    r = -1;

    /* Set pie centre and radius leaving a buffer at sides (25%) */
    xc = (double) allocation->height / 2;
    yc = (double) allocation->width / 2;
    radius = yc * 0.75;
    angle_from = 0.0;

    /* Loop through the slices and draw each */
    for(l = pc->pie_slices; l != NULL; l = l->next)
    {
    	ps = (PieSlice *) l->data;
    	rgba = ps->colour;
    	tmp = (ps->slice_value / total_amt) * 360.0;
    	//angle_to = (tmp * (M_PI / 180.0));
    	angle_to = angle_from + (tmp * (M_PI / 180.0));
    	cairo_set_source_rgba (cr, rgba->red, rgba->green, rgba->blue, rgba->alpha);
    	cairo_set_line_width (cr, 2.0);
    	cairo_arc (cr, xc, yc, radius, angle_from, angle_to);
	cairo_line_to (cr, xc, yc);
	cairo_fill (cr);
	cairo_stroke (cr);
    	angle_from = angle_to;
    }

    return r;
}
