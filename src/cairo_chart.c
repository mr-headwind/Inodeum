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
#include <cairo-xlib.h>
#include <defs.h>


/* Defines */


/* Types */


/* Prototypes */

PieChart * pie_chart_create(char *, double, int, const GdkRGBA *, int, int);
int pie_slice_create(PieChart *, char *, double, const GdkRGBA *, const GdkRGBA *, int);
void free_pie_chart(PieChart *);
void free_slices(gpointer);
int draw_pie_chart(cairo_t *, PieChart *, GtkAllocation *);
int pie_chart_title(cairo_t *, PieChart *, GtkAllocation *, GtkAlign, GtkAlign);
void pc_drawing(cairo_t *, PieChart *, double, double, double, double);
void ps_labels(cairo_t *, PieChart *, double, double, double, double);
void text_coords(cairo_t *, char *, double, double, double, double, double, double, double *, double *);
int check_legend(cairo_t *, PieChart *, double *, double *, double *, double *, double *, GtkAllocation *);
void draw_pc_legend(cairo_t *, GList *, double, double, double, double);

Axis * create_axis(char *, double, int, const GdkRGBA *, int, const GdkRGBA *, int);
void free_axis(Axis *);
int draw_axis(cairo_t *, Axis *, int, GtkAllocation *);
int axis_space_analysis(cairo_t *, Axis *, double, double, double, double, GtkAllocation *);
void axes_auto_fit(cairo_t *, Axis *, Axis *, GtkAllocation *);
void axis_step_bounds(Axis *);

BarChart * bar_chart_create(char *, const GdkRGBA *, int, int, Axis *, Axis *);
Bar * bar_create(BarChart *);
int bar_segment_create(BarChart *, Bar *, char *, const GdkRGBA *, const GdkRGBA *, int, double);
void free_bar_chart(BarChart *);
void free_bars(gpointer);
void free_bar_segment(gpointer);
int draw_bar_chart(cairo_t *, BarChart *, GtkAllocation *);
void draw_bar(cairo_t *, BarChart *, Bar *, int, int, double, double);
int bar_chart_title(cairo_t *, BarChart *, GtkAllocation *, GtkAlign, GtkAlign);

LineGraph * line_graph_create(char *, const GdkRGBA *, int, 
			      char *, double, double,
			      const GdkRGBA *, int, const GdkRGBA *, int,
			      char *, double, double,
			      const GdkRGBA *, int, const GdkRGBA *, int,
			      const GdkRGBA *);
void free_line_graph(LineGraph *);
void free_points(gpointer);
void draw_line_graph(cairo_t *, LineGraph *, GtkAllocation *);
void draw_point_line(cairo_t *, double, double, double, double);
void line_graph_add_point(LineGraph *, double, double);
void set_line_graph_bounds(LineGraph *);

int chart_title(cairo_t *, CText *, GtkAllocation *, GtkAlign, GtkAlign);
CText * label_text(int, CText *);
CText * percent_ctext(int, char *, const GdkRGBA *, int, CText *);
CText * percent_text(cairo_t *, CText *, double, double, CText *);
void draw_text_lines(cairo_t *, CText **, int, int, double, double);
double confirm_font_size(cairo_t *, char *, int, double);
CText * new_chart_text(char *, const GdkRGBA *, int);
void free_chart_text(CText *);
void get_ctext_ext(cairo_t *, CText *);
void show_surface_info(cairo_t *, GtkAllocation *);

extern int long_chars(long);
extern int double_chars(double);
char * dtos(double, int);


/* Globals */

static const char *debug_hdr = "DEBUG-cairo_chart.c ";
static const double lgd_rect_width = 20.0;
static const double lgd_buf = 5.0;
static const double r_rad = 0.7;
static const long mk_length = 5.0;
static const double axis_buf = 5.0;



/* Create and initialise a new pie chart */

// Rules for creation:-
// . Title is optional (NULL).
// . Text colour is optional (NULL).
// . Total value is optional (0) as the code will work it out anyway (may be a useful error check).
// . Legend is either TRUE or FALSE.
// . Text size is optional (0) and will default to 12.

PieChart * pie_chart_create(char *title, double total_val, int legend, 
			    const GdkRGBA *txt_colour, int txt_sz, int lbl_opt)
{
    PieChart *pc;

    if (legend < 0 || legend > 1)
    	return NULL;

    if (lbl_opt < LBL || lbl_opt > BOTH)
    	return NULL;

    pc = (PieChart *) malloc(sizeof(PieChart));
    memset(pc, 0, sizeof(PieChart));

    pc->title = new_chart_text(title, txt_colour, txt_sz);

    pc->lbl_opt = lbl_opt;
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

    ps->desc = new_chart_text(desc, txt_colour, txt_sz);
    ps->perc_txt = percent_ctext(pc->lbl_opt, "(nnnnnn%)", txt_colour, txt_sz, ps->desc);

    ps->slice_value = val;
    ps->colour = colour;
    pc->pie_slices = g_list_append (pc->pie_slices, ps);

    return TRUE;
}


/* Free all pie chart resources */

void free_pie_chart(PieChart *pc)
{
    if (pc->title != NULL)
    	free_chart_text(pc->title);

    g_list_free_full (pc->pie_slices, (GDestroyNotify) free_slices);
    free(pc);

    return;
}


/* Free a pie chart slice */

void free_slices(gpointer data)
{  
    PieSlice *ps;

    ps = (PieSlice *) data;
    
    if (ps->desc != NULL)
    	free_chart_text(ps->desc);

    if (ps->perc_txt != NULL)
    	free_chart_text(ps->perc_txt);

    free(ps);

    return;
}


/* Write a pie chart title if present (convenience if somewhat dubious function) */

int pie_chart_title(cairo_t *cr, PieChart *pc, GtkAllocation *allocation, GtkAlign h_align, GtkAlign v_align)
{
    int r;

    /* Generic title function */
    r = chart_title(cr, pc->title, allocation, h_align, v_align);

    return r;
}


/* Draw a pie chart */

int draw_pie_chart(cairo_t *cr, PieChart *pc, GtkAllocation *allocation)
{
    int r, lgd;
    double xc, yc, radius, total_amt;
    double lx, ly;
    GList *l;
    PieSlice *ps;

    /* Initial */
    cairo_move_to (cr, allocation->x, allocation->y);
    r = TRUE;
    lgd = FALSE;

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

    /* Set default pie centre and radius leaving a buffer at sides */
    xc = (double) allocation->width / 2.0;
    yc = (double) allocation->height / 2.0;
    radius = xc * r_rad;

    /* Check if there is sufficient space for a legend */
    if (pc->legend == TRUE)
	lgd = check_legend(cr, pc, 
			   &xc, &yc, &radius, &lx, &ly, 
			   allocation);

    /* Draw the pie chart */
    pc_drawing(cr, pc, xc, yc, radius, total_amt);

    /* Labels or legend */
    if (lgd == FALSE)
    	ps_labels(cr, pc, xc, yc, radius, total_amt);
    else
    	draw_pc_legend(cr, pc->pie_slices, total_amt, lx, ly, (double) allocation->width);

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
    angle_from = M_PI * 3/2;

    /* Slices */
    for(l = pc->pie_slices; l != NULL; l = l->next)
    {
    	ps = (PieSlice *) l->data;

    	/* Convert the value to degrees and then degrees to radians */
    	tmp = (ps->slice_value / total_amt) * 360.0;

    	if (tmp < 1.0)
	    continue;

    	angle_to = angle_from + (tmp * (M_PI/180.0));			

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


/* Loop through the slices and draw labels */

void ps_labels(cairo_t *cr, PieChart *pc, double xc, double yc, double radius, double total_amt)
{
    int i;
    double angle_from, angle_to, tmp;
    double desc_x, desc_y;
    CText *txt[2];
    CText *ctxt;
    GList *l;
    PieSlice *ps;
    const GdkRGBA *rgba;

    /* Loop through the slices and set text if present */
    angle_from = M_PI * 3/2;

    for(l = pc->pie_slices; l != NULL; l = l->next)
    {
    	ps = (PieSlice *) l->data;

	/* Set the description (could be null) */
	txt[0] = label_text(pc->lbl_opt, ps->desc);

	/* Set percentage (could be null) */
	txt[1] = percent_text(cr, ps->perc_txt, ps->slice_value, total_amt, ps->desc);

	/* Loop to print any description and percentage */
	desc_x = 0;

	for(i = 0; i < 2; i++)
	{
	    ctxt = txt[i];

	    if (ctxt == NULL)
	    	continue;

	    cairo_set_font_size (cr, (double) ctxt->sz);
	    rgba = ctxt->colour;
	    tmp = (ps->slice_value / total_amt) * 360.0;
	    angle_to = angle_from + (tmp * (M_PI/180.0));

	    if (desc_x == 0)
	    {
		text_coords(cr, ctxt->txt, angle_from, angle_to, xc, yc, radius, 0.5, &desc_x, &desc_y);
		angle_from = angle_to;
	    }
	    else
	    {
	    	desc_y += txt[i - 1]->ext.height;
	    	desc_x = desc_x + ((txt[i-1]->ext.width - ctxt->ext.width) / 2);
	    }
	    	
	    cairo_set_source_rgba (cr, rgba->red, rgba->green, rgba->blue, rgba->alpha);
	    cairo_move_to (cr, desc_x, desc_y);
	    cairo_show_text (cr, ctxt->txt);
	    cairo_fill (cr);
	}
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


// Check if there is sufficient room for a legend, abort if any text missing
// Multiple loops are a little inefficient, but clearer and easier for changes

int check_legend(cairo_t *cr, PieChart *pc, 
		 double *xc, double *yc, double *radius, double *lx, double *ly, 
		 GtkAllocation *allocation)
{
    int no_slices;
    double rows, rw, sw, lgd_w, lgd_h;
    double max_txt_width, max_txt_height;
    double alloc_w, alloc_h, alloc_x, alloc_y;
    GList *l;
    PieSlice *ps;
    CText *desc;
    const int min_pie_sz = 130;			// Radius plus space buffer

    /* Get size (extent) of each slice for desc & percent if applicable */
    for(l = pc->pie_slices; l != NULL; l = l->next)
    {
    	ps = (PieSlice *) l->data;

    	if (ps->desc == NULL)
	    return FALSE;

	desc = ps->desc;
	cairo_set_font_size (cr, (double) desc->sz);
	cairo_text_extents (cr, desc->txt, &(desc->ext));

	if (ps->perc_txt != NULL)
	{
	    desc = ps->perc_txt;
	    cairo_set_font_size (cr, (double) desc->sz);
	    cairo_text_extents (cr, desc->txt, &(desc->ext));
	}
    }

    /* Work out space required horizontally:- allocation height vs legend 'rows' required */
    rows = 1;
    rw = 1;
    no_slices = 0;
    max_txt_width = 0;
    max_txt_height = 0;
    alloc_w = (double) allocation->width;
    alloc_h = (double) allocation->height;
    alloc_x = (double) allocation->x;
    alloc_y = (double) allocation->y;

    for(l = pc->pie_slices; l != NULL; l = l->next)
    {
    	ps = (PieSlice *) l->data;

	no_slices++;
	desc = ps->desc;
	sw = desc->ext.width + lgd_rect_width + lgd_buf;

	if (desc->ext.height > max_txt_height)
	    max_txt_height = desc->ext.height;

	if (ps->perc_txt != NULL)
	{
	    desc = ps->perc_txt;
	    sw = sw + desc->ext.width + lgd_buf;
	}

	if (sw > max_txt_width)
	    max_txt_width = sw;

	rw += sw;

	if (rw > (double) allocation->width)
	{
	    rows++;
	    rw = sw + 1;
	}
    }

    // Check height:- alloc height must be > (row height * rows) + min. pie size
    // If sufficient space, determine legend coordinates and adjust pie coordinates
    lgd_h = (rows * (max_txt_height + 2));
    lgd_w = 0;

    if (alloc_h >= (lgd_h + min_pie_sz))
    {
	*yc = (alloc_h - lgd_h) / 2.0;
	*radius = *yc * r_rad;
	*lx = alloc_x + 1;
	*ly = alloc_y + alloc_h - lgd_h;
    	return TRUE;
    }

    // Work out space required vertically:- allocation height vs max legend width and rows
    // If sufficient space, determine legend coordinates and adjust pie coordinates

    if (alloc_w < (max_txt_width + min_pie_sz + lgd_buf))
    	return FALSE;

    lgd_w = max_txt_width + 2;
    lgd_h = (no_slices * (max_txt_height + 2));

    if (alloc_h >= lgd_h)
    {
	*xc = (alloc_w - lgd_w) / 2.0;
	*radius = *xc * r_rad;
	*lx = alloc_x + alloc_w - lgd_w;
	*ly = alloc_y + ((alloc_h - lgd_h) / 2);
    	return TRUE;
    }

    return FALSE;
}


/* Draw the pie chart legend */

void draw_pc_legend(cairo_t *cr, GList *pie_slices,
		    double total_amt, double lx, double ly, double alloc_w)
{
    double x, y, tmp;
    GList *l;
    const GdkRGBA *rgba;
    PieSlice *ps;
    CText *desc;

    /* Initial */
    x = lx;
    y = ly;

    /* Loop through slices and draw a legend for each */
    for(l = pie_slices; l != NULL; l = l->next)
    {
    	ps = (PieSlice *) l->data;
	desc = ps->desc;

	/* Check position */
	tmp = x + lgd_rect_width + lgd_buf + desc->ext.width;

	if (ps->perc_txt)
	    tmp = tmp + lgd_buf + ps->perc_txt->ext.width;

	if (tmp > alloc_w)
	{
	    x = lx;
	    y = y + desc->ext.height + lgd_buf;
	}

    	/* Coloured rectangle */
    	rgba = ps->colour;
    	cairo_set_source_rgba (cr, rgba->red, rgba->green, rgba->blue, rgba->alpha);
    	cairo_set_line_width (cr, 1.0);
    	cairo_rectangle (cr, x, y - desc->ext.height, lgd_rect_width, desc->ext.height);
	cairo_fill (cr);

    	/* Bit fiddly, but this puts a border on the rectangle */
    	cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
    	cairo_rectangle (cr, x, y - desc->ext.height, lgd_rect_width, desc->ext.height);
	cairo_stroke (cr);

	/* Text description an percentage */
	x = x + lgd_rect_width + lgd_buf;

	cairo_set_font_size (cr, desc->sz);
    	cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
	cairo_move_to (cr, x, y);
    	cairo_show_text (cr, desc->txt);
	cairo_fill (cr);

	x = x + desc->ext.width + lgd_buf;

	if (ps->perc_txt != NULL)
	{
	    desc = percent_text(cr, ps->perc_txt, ps->slice_value, total_amt, ps->desc);
	    cairo_set_font_size (cr, desc->sz);
	    cairo_move_to (cr, x, y);
	    cairo_show_text (cr, desc->txt);
	    cairo_fill (cr);
	    x = x + desc->ext.width + lgd_buf;
	}
    }

/* Debug
printf("%s leg 1  x %0.4f y %0.4f max w %0.4f max h %0.4f\n", debug_hdr, x, y, max_w, max_h); fflush(stdout);
printf("%s leg 2\n", debug_hdr); fflush(stdout);
printf("%s leg 3  x %0.4f y %0.4f max w %0.4f max h %0.4f\n", debug_hdr, x, y, max_w, max_h); fflush(stdout);
*/

    return;
}


/* Create and initialise a new bar chart */

// Rules for creation:-
// . Everything is optional (NULL).
// . The only thing that is ulimately essential is that at least one Bar must be created
//   separately with the 'bar_create' function which adds it to the GList of bars.
// . Text size defaults to 12 and text colour defaults to BLACK if a title is present.
// . The axes are convenience items only. It isn't necessary to have any axes at all and
//   they can be separate items in their own right if desired. If present they, (or it)
//   will be drawn and destroyed as part of the bar chart functions. Just saves having to keep
//   track and code manually.

BarChart * bar_chart_create(char *title, const GdkRGBA *txt_colour, int txt_sz, int lbl_opt, 
			    Axis *x_axis, Axis *y_axis)
{
    BarChart *bc;

    if (lbl_opt < LBL || lbl_opt > BOTH)
    	return NULL;

    bc = (BarChart *) malloc(sizeof(BarChart));
    memset(bc, 0, sizeof(BarChart));

    bc->title = new_chart_text(title, txt_colour, txt_sz);

    bc->lbl_opt = lbl_opt;
    bc->x_axis = x_axis;
    bc->y_axis = y_axis;

    return bc;
}


/* Create and initialise a new bar chart bar */

// Rules for creation:-
// . The only thing that is ulimately essential is that at least one Bar Segment must be created
//   separately with the 'bar_segment_create' function which adds it to the GList of bar segments.

Bar * bar_create(BarChart *bc)
{
    Bar *bar;

    bar = (Bar *) malloc(sizeof(Bar));
    memset(bar, 0, sizeof(Bar));

    bc->bars = g_list_append (bc->bars, bar);

    return bar;
}


/* Create and initialise a new bar segment */

// Rules for creation:-
// . Description is optional (NULL or 0).
// . Text size defaults to 10 and text colour defaults to BLACK if a description is present.

int bar_segment_create(BarChart *bc, Bar *bar, char *desc, const GdkRGBA *colour, 
		       const GdkRGBA *txt_colour, int txt_sz, double val)
{
    BarSegment *seg;

    if (val == 0)
    	return FALSE;

    if (colour == NULL)
    	return FALSE;

    seg = (BarSegment *) malloc(sizeof(BarSegment));
    memset(seg, 0, sizeof(BarSegment));

    seg->desc = new_chart_text(desc, txt_colour, txt_sz);
    seg->perc_txt = percent_ctext(bc->lbl_opt, "(n%)", txt_colour, txt_sz, seg->desc);

    if (val < bar->min_val || bar->min_val == 0)
    	bar->min_val = val;

    if (val > bar->max_val || bar->max_val == 0)
    	bar->max_val = val;

    if (val < bc->chart_min_val || bc->chart_min_val == 0)
    	bc->chart_min_val = val;

    if (val > bc->chart_max_val || bc->chart_max_val == 0)
    	bc->chart_max_val = val;

    bar->abs_val += fabs(val);
    seg->segment_value = val;
    seg->colour = colour;
    bar->bar_segments = g_list_append (bar->bar_segments, seg);

    return TRUE;
}


/* Free all bar chart resources */

void free_bar_chart(BarChart *bc)
{
    if (bc->title != NULL)
    	free_chart_text(bc->title);

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
    
    if (bar_seg->desc != NULL)
    	free_chart_text(bar_seg->desc);

    if (bar_seg->perc_txt != NULL)
    	free_chart_text(bar_seg->perc_txt);

    free(bar_seg);

    return;
}


/* Write a bar chart title if present (convenience if somewhat dubious function) */

int bar_chart_title(cairo_t *cr, BarChart *bc, GtkAllocation *allocation, GtkAlign h_align, GtkAlign v_align)
{
    int r;

    /* Generic title function */
    r = chart_title(cr, bc->title, allocation, h_align, v_align);

    return r;
}


/* Draw a bar chart */

int draw_bar_chart(cairo_t *cr, BarChart *bc, GtkAllocation *allocation)
{
    int i, n, x, y, bar_width;
    double xc, yc;
    double x1, y1, x2, y2;
    GList *l;
    Bar *bar;
    const int max_bar_width = 25;
    const int buf1 = 15;

    /* Draw axes if required */
    if ((bc->x_axis != NULL || bc->y_axis != NULL) && (bc->x_axis->x1 == -1))
	axes_auto_fit(cr, bc->x_axis, bc->y_axis, allocation);

    /* Set the bar width allowing for a small buffer between bars and maximum width */
    n = g_list_length (bc->bars);
    bar_width = allocation->width / n;
printf("%s draw bc 1 alloc x %d y %d w %d h %d\n", debug_hdr, allocation->x, allocation->x,
							      allocation->width, allocation->height); fflush(stdout);
printf("%s draw bc 2 bar width %d\n", debug_hdr, bar_width);fflush(stdout);

    if (bar_width < 1)
	return FALSE;
    else if (bar_width > max_bar_width)
    	bar_width = max_bar_width;
    else
    	bar_width -= 1;

    /* Initial position, starting at the x-axis centre */
    xc = (double) (allocation->x + ((allocation->width / 2) - ((bar_width * n) / 2)));
    yc = allocation->height - buf1;

    /* Loop thru the bars */
    i = 0;

    for(l = bc->bars; l != NULL; l = l->next)
    {
    	bar = (Bar *) l->data;
printf("%s draw bc 3 xc %0.4f\n", debug_hdr, xc);fflush(stdout);
	xc = xc + bar_width * i;
	cairo_move_to (cr, xc, yc);
printf("%s draw bc 3a xc %0.4f\n", debug_hdr, xc);fflush(stdout);
printf("%s draw bc 4 max val %0.4f min val %0.4f abs %0.4f\n", debug_hdr, bar->max_val, 
							       bar->min_val, bar->abs_val);fflush(stdout);
    	draw_bar(cr, bc, bar, bar_width, (allocation->height - (buf1 * 3)), xc, yc);
    	i++;
    }

    return TRUE;
}


/* Draw a bar of a chart */

void draw_bar(cairo_t *cr, BarChart *bc, Bar *bar, int bar_w, int bar_h, double xc, double yc)
{
    int pc;
    double seg_h;
    CText *txt[2];
    const GdkRGBA *rgba;
    GList *l;
    BarSegment *bar_seg;

    /* Loop thru the bar segments */
    for(l = bar->bar_segments; l != NULL; l = l->next)
    {
	/* Determine dimensions and draw segment */
    	bar_seg = (BarSegment *) l->data;
printf("\n%s draw bar 1  seg val %0.4f\n", debug_hdr, bar_seg->segment_value);fflush(stdout);
    	seg_h = (bar_seg->segment_value / bar->abs_val) * (double) bar_h;
    	yc -= seg_h;
    	rgba = bar_seg->colour;
    	cairo_set_source_rgba (cr, rgba->red, rgba->green, rgba->blue, rgba->alpha);
printf("%s draw bar 2 xc %0.4f yc %0.4f bar_w %d seg_h %0.4f\n", debug_hdr, xc, yc, bar_w, seg_h);fflush(stdout);
    	cairo_rectangle (cr, xc, yc, (double) bar_w, seg_h);
	cairo_fill (cr);

	/* Add the description (could be null) */
	txt[0] = label_text(bc->lbl_opt, bar_seg->desc);

	/* Pass percentage (could be null) */
	txt[1] = percent_text(cr, bar_seg->perc_txt, bar_seg->segment_value, bar->abs_val, bar_seg->desc);

	/* Draw the text line(s) if any */
	draw_text_lines(cr, txt, 2, bar_w, xc, yc + (seg_h/2));
    }

    return;
}


/* Create and initialise a new line graph */

LineGraph * line_graph_create(char *title, const GdkRGBA *txt_colour, int txt_sz, 
			      char *x_unit, double x_step, double x_prec,
			      const GdkRGBA *x_txt_colour, int x_txt_sz,
			      const GdkRGBA *x_step_colour, int x_step_txt_sz,
			      char *y_unit, double y_step, double y_prec,
			      const GdkRGBA *y_txt_colour, int y_txt_sz,
			      const GdkRGBA *y_step_colour, int y_step_txt_sz,
			      const GdkRGBA *line_colour)
{
    LineGraph *lg;

    /* Overall chart */
    lg = (LineGraph *) malloc(sizeof(LineGraph));
    memset(lg, 0, sizeof(LineGraph));

    /* Text */
    lg->title = new_chart_text(title, txt_colour, txt_sz);

    /* Axes */
    if ((lg->x_axis = create_axis(x_unit, x_step, x_prec, 
    				  x_txt_colour, x_txt_sz, x_step_colour, x_step_txt_sz)) == NULL)
    {
    	free_line_graph(lg);
    	return NULL;
    }

    if ((lg->y_axis = create_axis(y_unit, y_step, y_prec,
    				  y_txt_colour, y_txt_sz, y_step_colour, y_step_txt_sz)) == NULL)
    {
    	free_line_graph(lg);
    	return NULL;
    }

    lg->points = NULL;

    /* General */
    lg->line_colour = line_colour;

    return lg;
}


/* Free all line graph resources */

void free_line_graph(LineGraph *lg)
{
    if (lg->title != NULL)
    	free_chart_text(lg->title);

    if (lg->x_axis != NULL)
    	free_axis(lg->x_axis);

    if (lg->y_axis != NULL)
    	free_axis(lg->y_axis);

    if (lg->points != NULL)
    	g_list_free_full(lg->points, (GDestroyNotify) free_points);

    free(lg);

    return;
}


/* Free a line graph point */

void free_points(gpointer data)
{  
    Point *p;

    p = (Point *) data;
    free(p);

    return;
}


/* Add a line graph point */

void line_graph_add_point(LineGraph *lg, double x, double y)
{  
    Point *p;

    p = (Point *) malloc(sizeof(Point));
    p->x_val = x;
    p->y_val = y;

    lg->points = g_list_append(lg->points, p);

    return;
}


/* Set the axes high and low points */

void set_line_graph_bounds(LineGraph *lg)
{  
    double min_x, max_x, min_y, max_y;
    GList *l;
    Point *p;

    /* Data high and low values */
    l = lg->points;
    p = (Point *) l->data;

    min_x = max_x = p->x_val;
    min_y = max_y = p->y_val;
    l = l->next;

    while(l != NULL)
    {
	p = (Point *) l->data;

	if (p->x_val < min_x)
	    min_x = p->x_val;
	else if (p->x_val > max_x)
	    max_x = p->x_val;
    	

	if (p->y_val < min_y)
	    min_y = p->y_val;
	else if (p->y_val > max_y)
	    max_y = p->y_val;

	l = l->next;
    }

    lg->x_axis->low_val = min_x;
    lg->x_axis->high_val = max_x;
    lg->y_axis->low_val = min_y;
    lg->y_axis->high_val = max_y;

    /* Round out the axes high and low step bounds */
    axis_step_bounds(lg->x_axis);
    axis_step_bounds(lg->y_axis);

    return;
}


/* Determine the high and low steps for an axis */

void axis_step_bounds(Axis *axis)
{
    double rem;

    rem = fmod(axis->high_val, axis->step);

    if (rem == 0)
    	axis->high_step = axis->high_val;
    else
    	axis->high_step = (axis->high_val - rem) + axis->step;

    rem = fmod(axis->low_val, axis->step);

    if (rem == 0)
    	axis->low_step = axis->low_val;
    else
    	axis->low_step = (axis->low_val - rem) - axis->step;

    return;
}


/* Draw a line graph */

void draw_line_graph(cairo_t *cr, LineGraph *lg, GtkAllocation *allocation)
{  
    int init;
    double x, y, prev_x, prev_y, x_factor, y_factor;
    GList *l;
    Point *pt;
    const GdkRGBA *rgba;

    /* Draw the axes first */
    axes_auto_fit(cr, lg->x_axis, lg->y_axis, allocation);
    draw_axis(cr, lg->x_axis, FALSE, allocation);
    draw_axis(cr, lg->y_axis, FALSE, allocation);

    /* Plot each point */
    x_factor = (lg->x_axis->x2 - lg->x_axis->x1) / (lg->x_axis->high_step - lg->x_axis->low_step);
    y_factor = (lg->y_axis->y2 - lg->y_axis->y1) / (lg->y_axis->high_step - lg->y_axis->low_step);
    init = TRUE;
    cairo_set_line_width (cr, 1.0); 
    rgba = lg->line_colour;
    cairo_set_source_rgba (cr, rgba->red, rgba->green, rgba->blue, rgba->alpha);

    for(l = lg->points; l != NULL; l = l->next)
    {
	/* Tranform each x,y point value into corresponding graph x,y values */
    	pt = (Point *) l->data;
    	x = lg->x_axis->x1 + (pt->x_val * x_factor);
    	y = lg->y_axis->y2 - (pt->y_val * y_factor);

	/* Draw a line to each point, except the first */
	//draw_point(cr, x, y);

	if (init == FALSE)
	{
	    draw_point_line(cr, x, y, prev_x, prev_y);
	}
	else
	{
	    init = FALSE;
	    draw_point_line(cr, x, y, x, y);
	}

	prev_x = x;
	prev_y = y;
    }

    return;
}


/* Draw a line graph point */

void draw_point(cairo_t *cr, double x, double y)
{  

    return;
}


/* Draw a line graph line connecting two points */

void draw_point_line(cairo_t *cr, double x, double y, double prev_x, double prev_y)
{  
    cairo_move_to (cr, x, y);
    cairo_line_to (cr, prev_x, prev_y);
    cairo_stroke (cr);

    return;
}


/* Create an Axis */

// Rules for creation:-
// . Default colour and text size will be not supplied (NULL or 0).
// . Other values are mandatory.
// . If step precision is required (not a whole number), enter the number of decimal places. 

Axis * create_axis(char *unit, double step, int prec, 
		   const GdkRGBA *txt_colour, int txt_sz,
		   const GdkRGBA *step_colour, int step_txt_sz)
{
    int sz;
    char *s;
    Axis *axis;

    /* Initial */
    if (step == 0)
    	return NULL;

    axis = (Axis *) malloc(sizeof(Axis));
    memset(axis, 0, sizeof(Axis));

    /* Force auto fit as the default */
    axis->x1 = -1;
    axis->y1 = -1;
    axis->x2 = -1;
    axis->y2 = -1;

    axis->unit = new_chart_text(unit, txt_colour, txt_sz);

    /* Set up */
    axis->step = step;
    axis->prec = prec;

    /* Text details - use the step and precision for the step text */
    axis->unit = new_chart_text(unit, txt_colour, txt_sz);
    s = dtos(axis->step, axis->prec);
    axis->step_mk = new_chart_text(s, step_colour, step_txt_sz);
    free(s);

    return axis;
}


/* Free all axis resources */

void free_axis(Axis *axis)
{
    if (axis->unit != NULL)
    	free_chart_text(axis->unit);

    if (axis->step_mk != NULL)
    	free_chart_text(axis->step_mk);

    free(axis);

    return;
}


/* Draw an axis */

int draw_axis(cairo_t *cr, Axis *axis, int check_space, GtkAllocation *allocation)
{
    int i, n_steps;
    double step_dist, tmpx, tmpy, rem, tmp;
    double x_offset, y_offset, x_mk_offset, y_mk_offset;
    char *s;
    CText *unit, *step_mk;
    cairo_text_extents_t ext;
    const GdkRGBA *rgba;

    /* Step and axis determination (rounding if required) */
    n_steps = (int) ((axis->high_step - axis->low_step) / axis->step);
    rem = fmod((axis->high_step - axis->low_step), axis->step);

    if (rem != 0)
    	n_steps++;

    /* Space analysis */
    /*
    if (check_space == TRUE)
    	if (axis_space_analysis(cr, axis, x1, y1, x2, y2, allocation) == FALSE)
	    return FALSE;
    */

printf("***draw_axis 0 x1: %0.2f x2 %0.2f y1: %0.2f y2 %0.2f \n", axis->x1, axis->x2, axis->y1, axis->y2); fflush(stdout);
    /* Set drawing offsets */
    if (axis->x1 == axis->x2)				 // Y axis
    {
	step_dist = (axis->y2 - axis->y1) / n_steps;	
printf("***draw_axis 1 step_dist: %0.2f steps %d\n", step_dist, n_steps); fflush(stdout);
	x_offset = 0;
	y_offset = step_dist;
	x_mk_offset = mk_length * -1.0;
	y_mk_offset = 0;
    }
    else if (axis->y1 == axis->y2)			 // X axis
    {
	step_dist = (axis->x2 - axis->x1) / n_steps;	
printf("***draw_axis 2 step_dist: %0.2f steps %d\n", step_dist, n_steps); fflush(stdout);
	x_offset = step_dist;
	y_offset = 0;
	x_mk_offset = 0;
	y_mk_offset = mk_length;
    }

    /* Draw axis line */
    cairo_set_line_width (cr, 1.0); 
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
    cairo_move_to (cr, axis->x1, axis->y1);
    cairo_line_to (cr, axis->x2, axis->y2);
    cairo_stroke (cr);

    /* Zero data check */
    if ((axis->high_step - axis->low_step) == 0)
    	return TRUE;

    /* Temporarily reverse y axis y coordinates for ease of drawing step marks */
    if (axis->x1 == axis->x2)
    {
	tmp = axis->y1;
	axis->y1 = axis->y2;
	axis->y2 = tmp;
    }

    /* Save current point (axis end point) */
    tmpx = axis->x2;
    tmpy = axis->y2; 
    cairo_move_to (cr, axis->x2, axis->y2);

    /* Step mark settings */
    cairo_set_font_size (cr, axis->step_mk->sz);
    rgba = axis->step_mk->colour;

    /* Draw step marks and value text from the axis end backwards */
    for(i = 0; i <= n_steps; i++)
    {
	/* Draw step mark line */
printf("***draw_axis 3 tmpx: %0.2f  x_mk_offset: %0.2f tmpy: %0.2f y_mk_offset: %0.2f\n",
tmpx, x_mk_offset, tmpy, y_mk_offset); fflush(stdout);
	cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
	cairo_line_to (cr, tmpx - x_mk_offset, tmpy - y_mk_offset);
	cairo_stroke_preserve (cr);

	/* Draw the step text */
	tmp = axis->high_step - (axis->step * (double) i);
	s = dtos(tmp, axis->prec);
	cairo_set_source_rgba (cr, rgba->red, rgba->green, rgba->blue, rgba->alpha);
	cairo_text_extents (cr, s, &ext);

	if (axis->y1 == axis->y2)	// X axis
	    cairo_move_to (cr, tmpx - (ext.width/2), tmpy + axis_buf + ext.height);
	else				// Y axis
	    cairo_move_to (cr, tmpx - axis_buf - ext.width, tmpy + (ext.height/2));

    	cairo_show_text (cr, s);
	cairo_fill (cr);
	free(s);

	/* Move to next step mark */
printf("***draw_axis 4 move to: %0.2f , %0.2f\n", tmpx - x_offset, tmpy + y_offset); fflush(stdout);
	tmpx -= x_offset;
	tmpy += y_offset;
	cairo_move_to (cr, tmpx, tmpy);
    }

    /* Reset reversed y axis y coordinates */
    if (axis->x1 == axis->x2)
    {
	tmp = axis->y1;
	axis->y1 = axis->y2;
	axis->y2 = tmp;
    }

    /* Draw axis unit label text */
    if (axis->unit != NULL)
    {
	unit = axis->unit;
	step_mk = axis->step_mk;
	rgba = unit->colour;
	cairo_set_source_rgba (cr, rgba->red, rgba->green, rgba->blue, rgba->alpha);
	cairo_set_font_size (cr, unit->sz);

	if (axis->x1 == axis->x2)		// Y axis
	    cairo_move_to (cr, axis->x1 - (unit->ext.width / 2), axis->y1 - axis_buf);
	else					// X axis
	    cairo_move_to (cr, axis->x1 + ((axis->x2 -axis->x1) / 2) - (unit->ext.width / 2), 
			       axis->y1 + unit->ext.height + step_mk->ext.height + axis_buf);

	cairo_show_text (cr, unit->txt);
	cairo_fill (cr);
    }

/* Debug axis
*/
if (axis->unit != NULL)
printf("\n%s Unit: %s\n", debug_hdr, axis->unit->txt); fflush(stdout);
if (axis->step_mk != NULL)
printf("%s Step Mk: %s\n", debug_hdr, axis->step_mk->txt); fflush(stdout);
printf("%s draw_axis  low val: %0.3f high val: %0.3f step: %0.3f high_step: %0.3f low_step: %0.3f prec %d\n"
	"\tx1: %0.3f y1: %0.3f x2: %0.3f y2: %0.3f\n\n",
	debug_hdr, axis->low_val, axis->high_val, axis->step, axis->high_step, axis->low_step, axis->prec,
	axis->x1, axis->y1, axis->x2, axis->y2); fflush(stdout);

    return TRUE;
}


/* Analyse the axis space requirements */

int axis_space_analysis(cairo_t *cr, Axis *axis, 
		        double x1, double y1, double x2, double y2,
		        GtkAllocation *allocation)
{
    int i, sz, n_steps;
    CText *unit, *step_mk;
    char *s;

    /* Size of axis unit label */
    unit = axis->unit;
    cairo_set_font_size (cr, unit->sz);
    cairo_text_extents (cr, unit->txt, &(unit->ext));

    /* Size of step mark line and value */
    /* Use last step value (plus a little) for width or height (rather crude, but...) */
    sz = long_chars((long) axis->step);

    if (axis->prec > 0)
    	sz = sz + axis->prec + 1;

    s = (char *) malloc(sz + 2);

    step_mk = axis->step_mk;
    sprintf(s, "%*.*f", sz, axis->prec, (double) n_steps * axis->step);
    cairo_set_font_size (cr, step_mk->sz);
    cairo_text_extents (cr, s, &(step_mk->ext));
    free(s);

    /* Height and Width analysis - have to assume both axes' text is the same */
    if (x1 == x2)					// Y axis
    {
	sz = (y2 - y1) +				// Proposed axis line height
	     (unit->ext.height * 2.0) + mk_length + 	// X and Y unit text height
	     mk_length + 				// X axis step mark line height
	     step_mk->ext.height + axis_buf; 		// X axis Step mark text height plus a buffer

    	/* Need to adjust if insufficient */
    	if (allocation->height < sz)
    	{
	    axis->y1 = y1 + unit->ext.height + mk_length;
	    axis->y2 = y2 - (unit->ext.height + mk_length) - step_mk->ext.height + axis_buf;
    	}

    	if ((allocation->x - x1) < (mk_length + step_mk->ext.width) + axis_buf)
    	{
	    axis->x1 = x1 + mk_length + step_mk->ext.width + axis_buf;
	    axis->x2 = axis->x1;
    	}
    }
    else if (y1 == y2)				 	// X axis
    {
	sz = (x2 - x1) +				// Proposed axis line width
	     step_mk->ext.width + mk_length + 	// Y step mark text and line width
	     axis_buf; 					// X axis buffer

    	/* Need to adjust if insufficient */
    	if (allocation->width < sz)
    	{
	    axis->x1 = x1 + unit->ext.height + mk_length;
	    axis->x2 = allocation->width - mk_length - step_mk->ext.width - axis_buf;
    	}

    	if ((allocation->height - y1) < (mk_length + step_mk->ext.height + axis_buf))
    	{
	    axis->y1 = y1 - mk_length - step_mk->ext.height - axis_buf;
	    axis->y2 = axis->y1;
    	}
    }
    else
    {
    	return FALSE;
    }

    return TRUE;
}


/* Determine the best coordinates for X and Y axes as far as possible */

void axes_auto_fit(cairo_t *cr, Axis *x_axis, Axis *y_axis, GtkAllocation *allocation)
{
    double axis_len, zr, xyz, bzlen, pad, xpad;
    cairo_text_extents_t *ext;
    CText *txt;

    /* If not already set get the space used by axis titles, step mark values and step marks */
    get_ctext_ext(cr, x_axis->unit);
    get_ctext_ext(cr, x_axis->step_mk);
    get_ctext_ext(cr, y_axis->unit);
    get_ctext_ext(cr, y_axis->step_mk);

    /* X Axis */
    /* Initial X axis length,srv_usg->hist_days this is the current allocation point to the width less a buffer */
    axis_len = allocation->width - allocation->x - (axis_buf * 2);
printf("***AUTO 1 axis_len: %0.2f alloc width %d x %d axis_buf %0.2f\n", axis_len, allocation->width, allocation->x,axis_buf); fflush(stdout);

    /* Determine proportion of axis below zero */
    zr = (x_axis->low_step / (x_axis->high_step - x_axis->low_step));
    bzlen = zr * axis_len;
printf("***AUTO 2 zr: %0.2f low step %0.2f high step %0.2f \n", zr, x_axis->low_step, x_axis->high_step); fflush(stdout);

    /* Check for enough space for Y axis step marks and step values */
    ext = &(y_axis->step_mk->ext);
    pad = 0;

printf("***AUTO 3 bzlen: %0.2f ext->width %0.2f mk_length %ld axis_buf %0.2f \n", bzlen, ext->width, mk_length, axis_buf); fflush(stdout);
    if (bzlen < (ext->width + mk_length + axis_buf))
    {
	/* Adjust the initial length and recalulate the proportion below zero */
	pad = (ext->width + mk_length + axis_buf) - bzlen;
    	axis_len -= pad;
	bzlen = zr * axis_len;
    }
printf("***AUTO 4 bzlen: %0.2f axis_len: %0.2f mk txt %s\n", bzlen, axis_len, y_axis->step_mk->txt); fflush(stdout);

    /* Set the x1, zero and x2 points on the axis */
    x_axis->x1 = allocation->x + pad; 
    x_axis->x2 = x_axis->x1 + axis_len; 
    xyz = allocation->x + pad + bzlen;
printf("***AUTO 5 x_axis->x1: %0.2f x_axis->x2 %0.2f xyz %0.2f\n", x_axis->x1, x_axis->x2, xyz); fflush(stdout);

    /* Since X and Y axes always intersect at 0,0 the zero point forms the y axis x1 and x2 points */
    y_axis->x1 = y_axis->x2 = xyz;
printf("***AUTO 6 y_axis->x1: %0.2f y_axis->x2 %0.2f\n", y_axis->x1, y_axis->x2); fflush(stdout);

    /* Y Axis */
    /* Initial Y axis length, this is the current allocation point to the height less a buffer and title */
    ext = &(y_axis->unit->ext);
    axis_len = allocation->height - allocation->y - ext->height - (axis_buf * 2);
printf("***AUTO 7 axis_len: %0.2f alloc height %d x %d axis_buf %0.2f\n", axis_len, allocation->height, allocation->y,axis_buf); fflush(stdout);
printf("***AUTO 7a unit: %s unit height %0.2f\n", y_axis->unit->txt, ext->height); fflush(stdout);

    /* Determine proportion of axis below zero */
    if ((y_axis->high_step - y_axis->low_step) == 0)
    {
    	bzlen = 0;
    }
    else
    {
	zr = (y_axis->low_step / (y_axis->high_step - y_axis->low_step));
	bzlen = zr * axis_len;
    }
printf("***AUTO 8 zr: %0.2f low step %0.2f high step %0.2f \n", zr, y_axis->low_step, y_axis->high_step); fflush(stdout);

    /* Check for enough space for X axis title, step marks and step values */
    ext = &(x_axis->unit->ext);
    xpad = ext->height + axis_buf;
printf("***AUTO 9 xpad: %0.2f ext->height %0.2f txt %s axis_buf %0.2f \n", xpad, ext->height, x_axis->unit->txt, axis_buf); fflush(stdout);
    ext = &(x_axis->step_mk->ext);
    xpad = xpad + ext->height + axis_buf;
//xpad = xpad + ext->height + mk_length + axis_buf;
    pad = 0;
printf("***AUTO 9a xpad: %0.2f ext->height %0.2f txt %s mk_length %ld \n", xpad, ext->height, x_axis->unit->txt, mk_length); fflush(stdout);

printf("***AUTO 9b bzlen: %0.2f xpad %0.2f \n", bzlen, xpad); fflush(stdout);
    if (bzlen < xpad)
    {
	/* Adjust the initial length and recalulate the proportion below zero */
	pad = xpad - bzlen;
    	axis_len -= pad;
	bzlen = zr * axis_len;
    }
printf("***AUTO 10 bzlen: %0.2f axis_len: %0.2f pad %0.2f\n", bzlen, axis_len, pad); fflush(stdout);

    /* Set the y1, zero and y2 points on the axis */
    ext = &(y_axis->unit->ext);
    y_axis->y1 = allocation->y + ext->height + (axis_buf * 2.0); 
//y_axis->y1 = allocation->y + ext->height + axis_buf; 
    y_axis->y2 = y_axis->y1 + axis_len; 
    xyz = y_axis->y2 - bzlen;
printf("***AUTO 11 y_axis->y1: %0.2f y_axis->y2 %0.2f xyz %0.2f\n", y_axis->y1, y_axis->y2, xyz); fflush(stdout);
printf("***AUTO 11a ext->height: %0.2f txt %s\n", ext->height, y_axis->unit->txt); fflush(stdout);

    /* Since X and Y axes always intersect at 0,0 the zero point forms the x axis y1 and y2 points */
    x_axis->y1 = x_axis->y2 = xyz;

    return;
}


/* Write a chart title */

// Some notes on titles:
// The Title functions are designed to be (hopefully at least) quite flexible. 
// The 'chart_title' function may be called directly to create an overall title on
// the drawing area. Also each chart has a title function (that calls this).
// Equally, however, an individual chart title may be used as an overall
// title if desired. It all depends on what the passed allocation contains.

int chart_title(cairo_t *cr, CText *title, GtkAllocation *allocation, GtkAlign h_align, GtkAlign v_align)
{
    double xc, yc, fsz;
    const GdkRGBA *rgba;
    cairo_text_extents_t *ext;
    const double ltr_buf = 2.0;

    /* Ignore if no title */
    if (title == NULL)
    	return FALSE;

    /* Appearance */
    rgba = title->colour;
    cairo_set_source_rgba (cr, rgba->red, rgba->green, rgba->blue, rgba->alpha);

    if ((fsz = confirm_font_size(cr, title->txt, allocation->width, title->sz)) == FALSE)
    	return FALSE;

    cairo_set_font_size (cr, fsz);

    /* Determine space to be consumed by text */
    ext = &(title->ext);
    cairo_text_extents (cr, title->txt, ext);

    /* Set alignment */
    switch (h_align)
    {
    	case GTK_ALIGN_START:
	    xc = allocation->x;
	    break;

    	case GTK_ALIGN_CENTER:
	    xc = (((double) allocation->width / 2.0) - (ext->width / 2.0)) + allocation->x;
	    break;

    	case GTK_ALIGN_END:
	    xc = ((double) allocation->width - ext->width) + allocation->x;
	    break;

	default:
	    xc = allocation->x;
    }

    switch (v_align)
    {
    	case GTK_ALIGN_START:
	    yc = ext->height + allocation->y;
	    break;

    	case GTK_ALIGN_CENTER:
	    yc = (((double) allocation->height / 2.0) - (ext->height / 2.0)) + allocation->y;
	    break;

    	case GTK_ALIGN_END:
	    yc = (double) (allocation->height + allocation->y) - ltr_buf;
	    //yc = (double) (allocation->height + allocation->y) - (ext->height / 2);
	    break;

	default:
	    yc = ext->height + allocation->y;
    }

printf("%s chart title xc %0.4f yc %0.4f font sz %0.2f txt %s ext h %0.4f\n", 
  debug_hdr, xc, yc, fsz, title->txt, ext->height);fflush(stdout);
    /* Set Title */
    cairo_move_to (cr, xc, yc);
    cairo_show_text (cr, title->txt);
    cairo_fill (cr);

    return TRUE;
}


/* Draw lines of text */

void draw_text_lines(cairo_t *cr, CText *txt[], int max, int w, double xc, double yc)
{
    int i;
    double tx, ty;
    double fsz;
    CText *ctxt;
    const GdkRGBA *rgba;

printf("\n%s draw_text_lines 0 xc %0.4f yc %0.4f w %d\n", debug_hdr, xc, yc, w);fflush(stdout);
    /* May need to override the requested font size */
    for(i = 0; i < max; i++)
    {
    	ctxt = txt[i];

    	if (ctxt == NULL)
	    continue;

	if ((fsz = confirm_font_size(cr, ctxt->txt, w, ctxt->sz)) == FALSE)
	    return;

	if (fsz != ctxt->sz)
	{
	    ctxt->sz = fsz;
	    cairo_set_font_size (cr, ctxt->sz);
	    cairo_text_extents (cr, ctxt->txt, &(ctxt->ext));
	}
    }
    
printf("%s draw_text_lines 2  fsz %0.2f\n", debug_hdr, fsz);fflush(stdout);
    /* Loop thru text lines */
    ty = yc;

    for(i = 0; i < max; i++)
    {
    	ctxt = txt[i];

    	if (ctxt == NULL)
	    continue;

	cairo_set_font_size (cr, ctxt->sz);
	rgba = ctxt->colour;
	cairo_set_source_rgba (cr, rgba->red, rgba->green, rgba->blue, rgba->alpha);

	tx = xc + ((w - ctxt->ext.width) / 2);
	ty = ty + (ctxt->ext.height / 2);
printf("%s draw_text_lines 3  tx %0.4f ty %0.4f extw %0.4f exth %0.4f\n", 
			debug_hdr, tx, ty, ctxt->ext.width, ctxt->ext.height);fflush(stdout);
	cairo_move_to (cr, tx, ty);
	cairo_show_text (cr, ctxt->txt);
	cairo_fill (cr);
	ty = ty + ctxt->ext.height + 2;
    }

    return;
}


/* Set up label text */

CText * label_text(int lbl_opt, CText *desc)
{
    if (lbl_opt == PC)
    	return NULL;

    return desc;
}


/* New chart text class */

CText * percent_ctext(int lbl_opt, char *txt, const GdkRGBA *colour, int sz, CText *base_ctext)
{
    int fsz;
    CText *ctext;

    if (lbl_opt == LBL)
    	return NULL;

    fsz = sz;

    if (base_ctext != NULL)
	if (base_ctext->sz > 8)
	    fsz = (double) base_ctext->sz * 0.8;

    ctext = new_chart_text(txt, colour, fsz);

    return ctext;
}


/* Set up percentage text */

CText * percent_text(cairo_t *cr, CText *perc_txt, double item_val, double total_val, CText *base_ctext)
{
    double pc;

    if (perc_txt == NULL)
    	return NULL;

    pc = ((item_val / total_val) * 100.00);

    if (base_ctext == NULL)
    {
	sprintf(perc_txt->txt, "%0.1f%%", pc);
    }
    else
    {
	sprintf(perc_txt->txt, "(%0.1f%%)", pc);
	cairo_set_font_size (cr, (double) base_ctext->sz);
	cairo_text_extents (cr, base_ctext->txt, &(base_ctext->ext));
    }

    cairo_set_font_size (cr, (double) perc_txt->sz);
    cairo_text_extents (cr, perc_txt->txt, &(perc_txt->ext));

    return perc_txt;
}


/* Confirm and override the font size if necessary */

double confirm_font_size(cairo_t *cr, char *txt, int w, double sz)
{
    double sz_ok, fsz;
    cairo_text_extents_t ext;

    sz_ok = (double) FALSE;
    fsz = sz;

    while (sz_ok == (double) FALSE)
    {
	cairo_set_font_size (cr, fsz);
	cairo_text_extents (cr, txt, &ext);

	if (ext.width > w)
	{
	    fsz--;

	    if (fsz < 5)
	    	break;
	}
	else
	{
	    sz_ok = fsz;
	}
    }

    return sz_ok;
}


/* New chart text class */

CText * new_chart_text(char *txt, const GdkRGBA *colour, int sz)
{
    CText *ctext;

    if (txt == NULL)
    	return NULL;

    ctext = (CText *) malloc(sizeof(CText));
    memset(ctext, 0, sizeof(CText));

    ctext->txt = malloc(strlen(txt) + 1);
    strcpy(ctext->txt, txt);
    
    if (colour != NULL)
	ctext->colour = colour;
    else
	ctext->colour = &BLACK;

    if (sz > 0)
	ctext->sz = sz;
    else
	ctext->sz = 12;

    return ctext;
}


/* Free chart text resources */

void free_chart_text(CText *ctext)
{
    if (ctext->txt)
    	free(ctext->txt);

    free(ctext);

    return;
}


/* Determine the extents for a CText */

void get_ctext_ext(cairo_t *cr, CText *ctext)
{
    cairo_set_font_size (cr, ctext->sz);
    cairo_text_extents (cr, ctext->txt, &(ctext->ext));

    return;
}


/* Cairo surface & allocation info */

void show_surface_info(cairo_t *cr, GtkAllocation *allocation)
{
    int w, h;
    double x, y;
    char typ[20];
    cairo_surface_t *surface;
    cairo_surface_type_t st;
    cairo_rectangle_t rect;

    if (allocation != NULL)
    {
	printf("%s Allocation  x %d y %d w %d h %d\n", debug_hdr,
						       allocation->x, allocation->y, 
						       allocation->width, allocation->height); fflush(stdout);
    }

    surface = cairo_get_target (cr);
    st = cairo_surface_get_type (surface);

    switch (st)
    {
    	case CAIRO_SURFACE_TYPE_IMAGE:
	    strcpy(typ, "IMAGE");
	    w = cairo_image_surface_get_width (surface);
	    h = cairo_image_surface_get_height (surface);
	    cairo_get_current_point (cr, &x, &y);
	    break;

    	case CAIRO_SURFACE_TYPE_XLIB:
	    strcpy(typ, "XLIB");
	    w = cairo_xlib_surface_get_width (surface);
	    h = cairo_xlib_surface_get_height (surface);
	    cairo_get_current_point (cr, &x, &y);
	    break;

    	case CAIRO_SURFACE_TYPE_RECORDING:
	    strcpy(typ, "RECORDING");

	    if (cairo_recording_surface_get_extents (surface, &rect) == TRUE)
	    {
		x = rect.x;
		y = rect.y;
		w = rect.width;
		h = rect.height;
	    }

	    cairo_get_current_point (cr, &x, &y);
	    break;

	default:
	    sprintf(typ, "Other (%d)", st);
	    w = 0;
	    h = 0;
	    cairo_get_current_point (cr, &x, &y);
	    break;
    }

    printf("%s Surface: %s  x %0.4f y %0.4f w %d h %d\n", debug_hdr, typ, x, y, w, h); fflush(stdout);

    return;
}
