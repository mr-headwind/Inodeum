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


/* Chart control details */

typedef struct _pie_chart
{
    //cairo_t *cr;
    char *chart_title;
    double total_value;
    int legend;
    int num_slices;
    GList *pie_slices;
} PieChart;


/* Chart slice details */

typedef struct _pie_slice
{
    char *desc;
    double slice_value;
    const GdkRGBA *colour;
} PieSlice;
