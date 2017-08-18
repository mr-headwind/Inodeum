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
#include <defs.h>


/* Defines */


/* Types */


/* Prototypes */

PieChart * pie_chart_init(char *, double, int, const GdkRGBA *, int);
int pie_slice_create(PieChart *, char *, double, const GdkRGBA *, const GdkRGBA *, int);
void free_pie_chart(PieChart *);
void free_slices(gpointer);
int draw_pie_chart(cairo_t *, PieChart *, GtkAllocation *);
int pie_chart_title(cairo_t *, PieChart *, GtkAllocation *, GtkAlign, GtkAlign);
void pc_drawing(cairo_t *, PieChart *, double, double, double, double);
void ps_labels(cairo_t *, PieChart *, double, double, double, double);
void text_coords(cairo_t *, char *, double, double, double, double, double, double, double *, double *);
int legend_space(GList *, double, double);
void pc_legend(cairo_t *, PieChart *, double, double, double);


/* Globals */

static const char *debug_hdr = "DEBUG-cairo_chart.c ";



/* Create and initialise a new pie chart */

// Rules for creation:-
// . Title is optional (NULL).
// . Text colour is optional (NULL).
// . Total value is optional (0) as the code will work it out anyway (may be a useful error check).
// . Legend is either TRUE or FALSE.
// . Text size is optional (0) and will default to 12.

PieChart * pie_chart_init(char *title, double total_val, int legend, const GdkRGBA *txt_colour, int txt_sz)
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

	if (txt_colour != NULL)
	    pc->txt_colour = txt_colour;
	else
	    pc->txt_colour = &BLACK;

	if (txt_sz > 0)
	    pc->txt_sz = txt_sz;
	else
	    pc->txt_sz = 12;
    }

    pc->total_value = total_val;
    pc->legend = legend;

    return pc;
}


/* Create and initialise a new pie slice */

// Rules for creation:-
// . Description is optional (NULL).
// . Text colour is optional (NULL), but will default to BLACK is a description is present.
// . Text size is optional (0) and will default to 12.

int pie_slice_create(PieChart *pc, char *desc, double val, 
		     const GdkRGBA *colour, const GdkRGBA *txt_colour, int txt_sz)
{
    PieSlice *ps;

    ps = (PieSlice *) malloc(sizeof(PieSlice));
    memset(ps, 0, sizeof(PieSlice));

    if (desc != NULL)
    {
    	ps->desc = malloc(strlen(desc) + 1);
    	strcpy(ps->desc, desc);

	if (txt_colour != NULL)
	    ps->txt_colour = txt_colour;
	else
	    ps->txt_colour = &BLACK;

	if (txt_sz > 0)
	    ps->txt_sz = txt_sz;
	else
	    ps->txt_sz = 12;
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


/* Write a pie chart title if present */

int pie_chart_title(cairo_t *cr, PieChart *pc, GtkAllocation *allocation, GtkAlign h_align, GtkAlign v_align)
{
    double xc, yc;
    const GdkRGBA *rgba;
    cairo_text_extents_t ext;

    /* Ignore if no title */
    if (pc->chart_title == NULL)
    	return FALSE;

    /* Appearance */
    rgba = pc->txt_colour;
    cairo_set_source_rgba (cr, rgba->red, rgba->green, rgba->blue, rgba->alpha);
    cairo_set_font_size (cr, (double) pc->txt_sz);

    /* Determine space to be consumed by text */
    cairo_text_extents (cr, pc->chart_title, &ext);

    /* Set alignment */
    switch (h_align)
    {
    	case GTK_ALIGN_START:
	    xc = 0;
	    break;

    	case GTK_ALIGN_CENTER:
	    xc = ((double) allocation->width / 2.0) - (ext.width / 2.0);
	    break;

    	case GTK_ALIGN_END:
	    xc = (double) allocation->width - ext.width;
	    break;

	default:
	    xc = 0;
    }

    switch (v_align)
    {
    	case GTK_ALIGN_START:
	    yc = ext.height;
	    break;

    	case GTK_ALIGN_CENTER:
	    yc = ((double) allocation->height / 2.0) - (ext.height / 2.0);
	    break;

    	case GTK_ALIGN_END:
	    yc = (double) allocation->height - ext.height;
	    break;

	default:
	    yc = ext.height;
    }

    /* Set Title */
    cairo_move_to (cr, xc, yc);
    cairo_show_text (cr, pc->chart_title);
    cairo_fill (cr);

    return TRUE;
}


/* Draw a pie chart */

int draw_pie_chart(cairo_t *cr, PieChart *pc, GtkAllocation *allocation)
{
    int r, tf;
    double xc, yc, radius, total_amt;
    GList *l;
    PieSlice *ps;

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

    /* Set pie centre and radius leaving a buffer at sides */
    xc = (double) allocation->width / 2.5;
    yc = (double) allocation->height / 2.0;
    radius = (double) (allocation->width / 2.0) * 0.7;

    /* Draw the pie chart */
    pc_drawing(cr, pc, xc, yc, radius, total_amt);

    /* Check if there is sufficient space for a legend */
    if (pc->legend == TRUE)
	tf = legend_space(pc->pie_slices, (double) allocation->width, yc - radius);
    else
    	tf = FALSE;

    /* Labels or legend */
    if ( l == FALSE)
    	ps_labels(cr, pc, xc, yc, radius, total_amt);
    else
    	pc_legend(cr, pc, xc, yc, radius);

    return r;
}


/* Loop through the slices and draw each */

void pc_drawing(cairo_t *cr, PieChart *pc, double xc, double yc, double radius, double total_amt)
{
    double angle_from, angle_to, tmp;
    GList *l;
    PieSlice *ps;
    const GdkRGBA *rgba;

    /* Start point */
    angle_from = M_PI * 3 / 2;

    /* Slices */
    for(l = pc->pie_slices; l != NULL; l = l->next)
    {
    	ps = (PieSlice *) l->data;

    	/* Convert the value to degrees and then degrees to radians */
    	tmp = (ps->slice_value / total_amt) * 360.0;
    	angle_to = angle_from + (tmp * (M_PI / 180.0));			
printf("%s tmp %0.4f total %0.4f val %0.4f\n", debug_hdr, tmp, total_amt, ps->slice_value);fflush(stdout);
printf("%s angle to %0.4f angle fr %0.4f\n", debug_hdr, angle_to, angle_from);fflush(stdout);

    	/* Draw */
    	rgba = ps->colour;
    	cairo_set_source_rgba (cr, rgba->red, rgba->green, rgba->blue, rgba->alpha);
    	cairo_set_line_width (cr, 2.0);
    	cairo_move_to (cr, xc, yc);
    	cairo_arc (cr, xc, yc, radius, angle_from, angle_to);
	cairo_line_to (cr, xc, yc);
	cairo_fill (cr);
	cairo_stroke (cr);
    	angle_from = angle_to;
    }

    return;
}


/* Loop through the slices and draw each */

void ps_labels(cairo_t *cr, PieChart *pc, double xc, double yc, double radius, double total_amt)
{
    double angle_from, angle_to, tmp;
    double desc_x, desc_y;
    GList *l;
    PieSlice *ps;
    const GdkRGBA *rgba;

    /* Loop through the slices and set text if present */
    angle_from = M_PI * 3 / 2;

    /* Slices and text */
    for(l = pc->pie_slices; l != NULL; l = l->next)
    {
    	ps = (PieSlice *) l->data;

    	if (ps->desc == NULL)
	    continue;

	cairo_set_font_size (cr, (double) ps->txt_sz);
    	rgba = ps->txt_colour;
    	tmp = (ps->slice_value / total_amt) * 360.0;
    	angle_to = angle_from + (tmp * (M_PI / 180.0));

	text_coords(cr, ps->desc, angle_from, angle_to, xc, yc, radius, 0.5, &desc_x, &desc_y);

    	cairo_set_source_rgba (cr, rgba->red, rgba->green, rgba->blue, rgba->alpha);
    	cairo_move_to (cr, desc_x, desc_y);
    	cairo_show_text (cr, ps->desc);
	cairo_fill (cr);
    	angle_from = angle_to;
    }

    return;
}


// Text coordinates for a pie chart
// Cosine is relationship of 'adjacent' to hypentuse
// Sine is relationship of 'opposite' to hypentuse
// Apply an arbitrary 'fudge' factor for position along the radius (use 1.0 if none is desired) 

void text_coords(cairo_t *cr, char *desc, 
		 double angle_from, double angle_to, double xc, double yc,
		 double hyp, double adj, 
		 double *coord_x, double *coord_y)
{
    double desc_angle;

    desc_angle = (angle_from + angle_to) / 2.0;
    *coord_x = (cos (desc_angle) * hyp * adj) + xc;
    *coord_y = (sin (desc_angle) * hyp * adj) + yc;

    return;
}


/* Check if there is sufficient room for a legend */

int legend_space(GList *pie_slices, double w, double h)
{
    int r;
    GList *l;
    PieSlice *ps;

    r = FALSE;
    cairo_set_font_size (cr, 9.0);

    for(l = pie_slices; l != NULL; l = l->next)
    {
    	ps = (PieSlice *) l->data;

    	if (ps->desc == NULL)
	    return r;
    }

    return r;
}


/* Loop through the slices and draw each */

void pc_legend(cairo_t *cr, PieChart *pc, double xc, double yc, double radius)
{

    return;
}
