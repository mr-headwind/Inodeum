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

PieChart * pie_chart_create(char *, double, int, const GdkRGBA *, int);
int pie_slice_create(PieChart *, char *, double, const GdkRGBA *, const GdkRGBA *, int);
void free_pie_chart(PieChart *);
void free_slices(gpointer);
int draw_pie_chart(cairo_t *, PieChart *, GtkAllocation *);
int pie_chart_title(cairo_t *, PieChart *, GtkAllocation *, GtkAlign, GtkAlign);
void pc_drawing(cairo_t *, PieChart *, double, double, double, double);
void ps_labels(cairo_t *, PieChart *, double, double, double, double);
void text_coords(cairo_t *, char *, double, double, double, double, double, double, double *, double *);
int legend_space(cairo_t *, GList *, double, double, double, double);
void pc_legend(cairo_t *, GList *, double, double, double, double);
Axis * create_axis(char *, double, double, double, const GdkRGBA *, int);
void free_axis(Axis *);
void draw_axis(cairo_t *, Axis *, double, double, double, double);
BarChart * bar_chart_create(char *, const GdkRGBA *, int, int, Axis *, Axis *);
int bar_create(BarChart *, const GdkRGBA *, int);
int bar_segment_create(BarChart *, Bar *, char *, const GdkRGBA *, double);
void free_bar_chart(BarChart *);
void free_bars(gpointer);
void free_bar_segment(gpointer);
void draw_bar_chart(cairo_t *, BarChart *, GtkAllocation *);
int bar_chart_title(cairo_t *, BarChart *, GtkAllocation *, GtkAlign, GtkAlign);
void chart_title(cairo_t *, char *, const GdkRGBA *, double, GtkAllocation *, GtkAlign, GtkAlign);
void show_surface_info(cairo_t *, GtkAllocation *);


/* Globals */

static const char *debug_hdr = "DEBUG-cairo_chart.c ";
static const double buf1_y = 5.0;
static const double rect_width = 20.0;
static const double buf1_x = 5.0;
static const double buf2_x = 10.0;



/* Create and initialise a new pie chart */

// Rules for creation:-
// . Title is optional (NULL).
// . Text colour is optional (NULL).
// . Total value is optional (0) as the code will work it out anyway (may be a useful error check).
// . Legend is either TRUE or FALSE.
// . Text size is optional (0) and will default to 12.

PieChart * pie_chart_create(char *title, double total_val, int legend, const GdkRGBA *txt_colour, int txt_sz)
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

    if (val == 0)
    	return FALSE;

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
    /* Ignore if no title */
    if (pc->chart_title == NULL)
    	return FALSE;

    /* Generic title function */
    chart_title(cr, pc->chart_title, pc->txt_colour, (double) pc->txt_sz, allocation, h_align, v_align);

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
	tf = legend_space(cr, pc->pie_slices, yc, radius, (double) allocation->width, allocation->height);
    else
    	tf = FALSE;

    /* Labels or legend */
    if ( tf == FALSE)
    	ps_labels(cr, pc, xc, yc, radius, total_amt);
    else
    	pc_legend(cr, pc->pie_slices, yc, radius, (double) allocation->width, allocation->height);

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

    	if (tmp < 1.0)
	    continue;

    	angle_to = angle_from + (tmp * (M_PI / 180.0));			

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

/* Debug
printf("%s tmp %0.4f total %0.4f val %0.4f\n", debug_hdr, tmp, total_amt, ps->slice_value);fflush(stdout);
printf("%s angle to %0.4f angle fr %0.4f\n", debug_hdr, angle_to, angle_from);fflush(stdout);
*/

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

// Fairly basic rules:-
// . If any text missing, legend not possible
// . Each item consists of a coloured rectangle, text and some buffer space
// . Use 1.5 line spacing
// . Items applied across page while they fit then down, etc. while there is space

int legend_space(cairo_t *cr, GList *pie_slices, double yc, double radius, double max_w, double max_h)
{
    double w, h, buf;
    GList *l;
    PieSlice *ps;
    cairo_text_extents_t ext;

    cairo_set_font_size (cr, 9.0);
    w = 1;
    h = yc + radius + buf1_y;
    buf = rect_width + (buf1_x * 2);			// rect (20), rect:text (5), item:item (10)

    for(l = pie_slices; l != NULL; l = l->next)
    {
    	ps = (PieSlice *) l->data;

    	if (ps->desc == NULL)
	    return FALSE;

	cairo_text_extents (cr, ps->desc, &ext);
	w = w + ext.width + buf;

	if (w > max_w)
	{
	    w = 1;
	    h = h + (ext.height / 2.0);
	}

	if (h > max_h)
	    return FALSE;
    }

/* Debug
printf("%s sp 1 max_w %0.4f max_y %0.4f\n", debug_hdr, max_w, max_h); fflush(stdout);
printf("%s sp 2 w %0.4f ext_w %0.4f\n", debug_hdr, w, ext.width); fflush(stdout);
printf("%s sp 3 h %0.4f ext_h %0.4f\n", debug_hdr, h, ext.height); fflush(stdout);
printf("%s sp 4 \n", debug_hdr); fflush(stdout);
*/

    return TRUE;
}


/* Draw the pie chart legend - see function 'legend_space' for rules */

void pc_legend(cairo_t *cr, GList *pie_slices, double yc, double radius, double max_w, double max_h)
{
    double x, y;
    GList *l;
    const GdkRGBA *rgba;
    PieSlice *ps;
    cairo_text_extents_t ext;

    /* Initial */
    cairo_set_font_size (cr, 9.0);
    x = 1;
    y = yc + radius + buf1_y;

    /* Loop through slices and draw a legend for each */
    for(l = pie_slices; l != NULL; l = l->next)
    {
    	ps = (PieSlice *) l->data;

    	if (ps->desc == NULL)		// Should not happen
	    return;

    	/* Coloured rectangle */
    	rgba = ps->colour;
    	cairo_set_source_rgba (cr, rgba->red, rgba->green, rgba->blue, rgba->alpha);
    	cairo_set_line_width (cr, 1.0);
    	cairo_rectangle (cr, x, y, rect_width, ext.height);
	cairo_fill (cr);

    	/* Bit fiddly, but this puts a border on the rectangle */
    	cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
    	cairo_rectangle (cr, x, y, rect_width, ext.height);
	cairo_stroke (cr);

	/* Text description */
	cairo_text_extents (cr, ps->desc, &ext);
    	cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
	cairo_move_to (cr, x + rect_width + buf1_x, y + ext.height);
    	cairo_show_text (cr, ps->desc);
	cairo_fill (cr);

	/* Check position */
	x = x + rect_width + buf2_x + ext.width;

	if (x > max_w)
	{
	    x = 1;
	    y = y + (ext.height / 2.0);
	}

	if (y > max_h)
	    return;			// Should not happen
    }

/* Debug
printf("%s leg 1  x %0.4f y %0.4f max w %0.4f max h %0.4f\n", debug_hdr, x, y, max_w, max_h); fflush(stdout);
printf("%s leg 2\n", debug_hdr); fflush(stdout);
printf("%s leg 3  x %0.4f y %0.4f max w %0.4f max h %0.4f\n", debug_hdr, x, y, max_w, max_h); fflush(stdout);
*/

    return;
}


/* Create an Axis */

// Rules for creation:-
// . The only items not optional (NULL or zero) are the Unit (axis label) anf the Step.
//   The others may be added as required or will be derived as part of a chart.

Axis * create_axis(char *unit, double start_val, double end_val, double step, 
		   const GdkRGBA *txt_colour, int txt_sz)
{
    Axis *axis;

    if (end_val < start_val)
    	return NULL;

    if (step == 0)
    	return NULL;

    axis = (Axis *) malloc(sizeof(Axis));
    memset(axis, 0, sizeof(Axis));

    axis->x1 = -1;
    axis->y1 = -1;
    axis->x2 = -1;
    axis->y2 = -1;

    axis->unit = malloc(strlen(unit) + 1);
    strcpy(axis->unit, unit);

    if (txt_colour != NULL)
	axis->txt_colour = txt_colour;
    else
	axis->txt_colour = &BLACK;

    if (txt_sz > 0)
	axis->txt_sz = txt_sz;
    else
	axis->txt_sz = 10;

    axis->start_val = start_val;
    axis->end_val = end_val;
    axis->step = step;

    return axis;
}


/* Free all axis resources */

void free_axis(Axis *axis)
{
    if (axis->unit)
    	free(axis->unit);

    free(axis);

    return;
}


/* Draw an axis */

void draw_axis(cairo_t *cr, Axis *axis, double x1, double y1, double x2, double y2)
{
    int i, n_steps;
    double step_x, step_y, tmpx, tmpy, offset;
    char s[10];
    const int mark_width = 5;
    const int txt_buf = 3;
    cairo_text_extents_t ext;
    const GdkRGBA *rgba;

    /* Save the latest coordinates */
    axis->x1 = x1;
    axis->y1 = y1;
    axis->x2 = x2;
    axis->y2 = y2;

    /* Steps */
    n_steps = (int) (axis->end_val - axis->start_val) / axis->step;
    step_x = (x2 - x1) / n_steps;
    step_y = (y2 - y1) / n_steps;

    /* Draw axis line */
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
    cairo_move_to (cr, x1, y1);
    cairo_line_to (cr, x2, y2);

    /* Draw step marks and text */
    cairo_set_font_size (cr, 8);

    for(i = 0; i < n_steps; i++)
    {
	cairo_move_to (cr, x1 + (step_x * i), y1 + (step_y * i));
	sprintf(s, "%0.5f", (double) n_steps * axis->step);
	cairo_text_extents (cr, s, &ext);
	cairo_get_current_point (cr, &tmpx, &tmpy);

	if (step_x == 0)					// Vertical
	{
	    cairo_line_to (cr, tmpx - mark_width, tmpy);
	    cairo_move_to (cr, tmpx - ext.width - txt_buf, tmpy - (ext.height / 2));
	}
	else							// Horizontal
	{
	    cairo_line_to (cr, tmpx, tmpy + mark_width);
	    cairo_move_to (cr, tmpx - (ext.width / 2), tmpy + ext.height + txt_buf);
	}

    	cairo_show_text (cr, s);
	cairo_fill (cr);
    }

    /* Draw axis unit label text */
    if (axis->unit != NULL)
    {
	rgba = axis->txt_colour;
	cairo_set_source_rgba (cr, rgba->red, rgba->green, rgba->blue, rgba->alpha);
	cairo_set_font_size (cr, axis->txt_sz);

	if (step_x == 0)
	{
	    offset = ext.width + txt_buf;
	    cairo_move_to (cr, x1, (y1 - y2) / 2);
	    cairo_text_extents (cr, axis->unit, &ext);
	    cairo_get_current_point (cr, &tmpx, &tmpy);
	    cairo_move_to (cr, tmpx - offset - ext.width, tmpy);
	}
	else
	{
	    offset = ext.height + txt_buf;
	    cairo_move_to (cr, (x1 - x2) / 2, y1);
	    cairo_text_extents (cr, axis->unit, &ext);
	    cairo_get_current_point (cr, &tmpx, &tmpy);
	    cairo_move_to (cr, tmpx, tmpy + offset + ext.height);
	}

	cairo_show_text (cr, axis->unit);
	cairo_fill (cr);
    }

    return;
}


/* Create and initialise a new bar chart */

// Rules for creation:-
// . Everything is optional (NULL).
// . The only thing that is ulimately essential is that at least one Bar must be created
//   separately with the 'bar_create' function which adds it to the GList of bars.
// . Text size defaults to 12 and text colour defaults to BLACK if a title is present.
// . The axes are convenience items only. It isn't necessary to have any axes at all and
//   they can be separate items in their own right if desired. If present they, ( or even it)
//   will drawn and destroyed as part of the bar chart functions. Just saves having to keep
//   track and code manually.

BarChart * bar_chart_create(char *title, const GdkRGBA *txt_colour, int txt_sz, int show_perc, 
			    Axis *x_axis, Axis *y_axis)
{
    BarChart *bc;

    if (show_perc < 0 || show_perc > 1)
    	return NULL;

    bc = (BarChart *) malloc(sizeof(BarChart));
    memset(bc, 0, sizeof(BarChart));

    if (title != NULL)
    {
    	bc->chart_title = malloc(strlen(title) + 1);
    	strcpy(bc->chart_title, title);

	if (txt_colour != NULL)
	    bc->txt_colour = txt_colour;
	else
	    bc->txt_colour = &BLACK;

	if (txt_sz > 0)
	    bc->txt_sz = txt_sz;
	else
	    bc->txt_sz = 12;
    }

    bc->show_perc = show_perc;
    bc->x_axis = x_axis;
    bc->y_axis = y_axis;

    return bc;
}


/* Create and initialise a new bar chart bar */

// Rules for creation:-
// . Everything is optional (NULL).
// . The only thing that is ulimately essential is that at least one Bar Segment must be created
//   separately with the 'bar_segment_create' function which adds it to the GList of bar segments.
// . Text size defaults to 10 and text colour defaults to BLACK if a description is present.

int bar_create(BarChart *bc, const GdkRGBA *txt_colour, int txt_sz)
{
    Bar *bar;

    bar = (Bar *) malloc(sizeof(Bar));
    memset(bar, 0, sizeof(Bar));

    if (txt_colour != NULL)
	bar->txt_colour = txt_colour;
    else
	bar->txt_colour = &BLACK;

    if (txt_sz > 0)
	bar->txt_sz = txt_sz;
    else
	bar->txt_sz = 10;

    bc->bars = g_list_append (bc->bars, bar);

    return TRUE;
}


/* Create and initialise a new bar segment */

// Rules for creation:-
// . Description is optional (NULL).

int bar_segment_create(BarChart *bc, Bar *bar, char *desc, const GdkRGBA *colour, double val)
{
    BarSegment *seg;

    if (val == 0)
    	return FALSE;

    if (colour != NULL)
    	return FALSE;

    seg = (BarSegment *) malloc(sizeof(BarSegment));
    memset(seg, 0, sizeof(BarSegment));

    if (desc != NULL)
    {
    	seg->desc = malloc(strlen(desc) + 1);
    	strcpy(seg->desc, desc);
    }

    if (val < bar->bar_min_val)
    	bar->bar_min_val = val;

    if (val > bar->bar_max_val)
    	bar->bar_max_val = val;

    if (val < bc->chart_min_val)
    	bc->chart_min_val = val;

    if (val > bc->chart_max_val)
    	bc->chart_max_val = val;

    bar->bar_abs_val += abs(val);
    seg->segment_value = val;
    seg->colour = colour;
    bar->bar_segments = g_list_append (bar->bar_segments, seg);

    return TRUE;
}


/* Free all bar chart resources */

void free_bar_chart(BarChart *bc)
{
    if (bc->chart_title)
    	free(bc->chart_title);

    if (bc->x_axis != NULL)
    	free(bc->x_axis);

    if (bc->y_axis != NULL)
    	free(bc->y_axis);

    g_list_free_full (bc->bars, (GDestroyNotify) free_bars);
    free(bc);

    return;
}


/* Free a bar chart bar */

void free_bars(gpointer data)
{  
    Bar *bar;

    bar = (Bar *) data;
    
    g_list_free_full (bar->bar_segments, (GDestroyNotify) free_bar_segment);
    free(bar);

    return;
}


/* Free a bar chart bar segment */

void free_bar_segment(gpointer data)
{  
    BarSegment *bar_seg;

    bar_seg = (BarSegment *) data;
    
    if (bar_seg->desc)
	free(bar_seg->desc);

    free(bar_seg);

    return;
}


/* Write a bar chart title if present */

int bar_chart_title(cairo_t *cr, BarChart *bc, GtkAllocation *allocation, GtkAlign h_align, GtkAlign v_align)
{
    /* Ignore if no title */
    if (bc->chart_title == NULL)
    	return FALSE;

    /* Generic title function */
    chart_title(cr, bc->chart_title, bc->txt_colour, (double) bc->txt_sz, allocation, h_align, v_align);

    return TRUE;
}


/* Draw a bar chart */

void draw_bar_chart(cairo_t *cr, BarChart *bc, GtkAllocation *allocation)
{
    double x1, y1, x2, y2;
    GList *l;
    Bar *bar;

    /* Initial */
    //cairo_move_to (cr, x, 0);

    /* Draw axes if required */
    if (bc->x_axis != NULL)
    	if (bc->x_axis->x1 == -1)
	{
	    axis_coords(cr, bc->x_axis, &x1, &y1, &x2, &y2);
	    draw_axis(cr, bc->x_axis, x1, y1, x2, y2);
	}

    /* Loop thru the bars */
    for(l = bc->bars; l != NULL; l = l->next)
    {
    	bar = (Bar *) l->data;
    	//draw_bar(cr, bar, allocation);
    }

    return;
}


/* Draw a bar of a chart */

void draw_bar(cairo_t *cr, Bar *bar, GtkAllocation *allocation)
{
    GList *l;
    BarSegment *bar_seg;

    /* Initial */
    cairo_move_to (cr, 0, 0);

    /* Loop thru the bar segments and draw */
    for(l = bar->bar_segments; l != NULL; l = l->next)
    {
    	bar_seg = (BarSegment *) l->data;
    }

    return;
}


/* Write a chart title */

void chart_title(cairo_t *cr, char *title, const GdkRGBA *rgba, double sz,
		GtkAllocation *allocation, GtkAlign h_align, GtkAlign v_align)
{
    double xc, yc;
    cairo_text_extents_t ext;

    /* Appearance */
    cairo_set_source_rgba (cr, rgba->red, rgba->green, rgba->blue, rgba->alpha);
    cairo_set_font_size (cr, sz);

    /* Determine space to be consumed by text */
    cairo_text_extents (cr, title, &ext);

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
    cairo_show_text (cr, title);
    cairo_fill (cr);

    return;
}


/* Cairo surface & allocation info */

void show_surface_info(cairo_t *cr, GtkAllocation *allocation)
{
    int w, h;
    double x, y;
    cairo_surface_t *surface;

    if (allocation != NULL)
    {
	printf("%s Allocation  x %d y %d w %d h %d\n", debug_hdr,
						       allocation->x, allocation->y, 
						       allocation->width, allocation->height); fflush(stdout);
    }

    surface = cairo_get_target (cr);
    w = cairo_image_surface_get_width (surface);
    h = cairo_image_surface_get_height (surface);
    cairo_get_current_point (cr, &x, &y);

    printf("%s Context  x %0.4f y %0.4f w %d h %d\n", debug_hdr, x, y, w, h); fflush(stdout);

    return;
}
