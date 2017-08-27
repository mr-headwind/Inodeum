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
** Description:	Main program include file
**
** Author:	Anthony Buckley
**
** History
**	28-Jul-2017	Initial
**
*/


/* Includes */

#include <glib.h>
#include <cairo/cairo.h>


/* Defines */

#ifndef CAIRO_CHART_HDR
#define CAIRO_CHART_HDR
#endif


/* Pie Chart control details */

typedef struct _pie_chart
{
    char *chart_title;
    const GdkRGBA *txt_colour;
    int txt_sz;
    double total_value;
    int legend;
    GList *pie_slices;
} PieChart;


/* Chart slice details */

typedef struct _pie_slice
{
    char *desc;
    double slice_value;
    const GdkRGBA *colour;
    const GdkRGBA *txt_colour;
    int txt_sz;
} PieSlice;


/* Axis */

typedef struct _axis
{
    char *unit;
    const GdkRGBA *txt_colour;
    int txt_sz;
    double start_val;
    double end_val;
    double step;
    double x1, y1, x2, y2;
} Axis;


/* Bar Chart control details */

typedef struct _bar_chart
{
    char *chart_title;
    const GdkRGBA *txt_colour;
    int txt_sz;
    int show_perc;
    double chart_min_val;
    double chart_max_val;
    Axis *x_axis;
    Axis *y_axis;
    GList *bars;
} BarChart;


/* Individual Bar details */

typedef struct _bar
{
    const GdkRGBA *txt_colour;
    int txt_sz;
    double abs_val;
    double min_val;
    double max_val;
    GList *bar_segments;
} Bar;


/* Bar Chart segment details */

typedef struct _bar_segment
{
    char *desc;
    const GdkRGBA *colour;
    double segment_value;
} BarSegment;
