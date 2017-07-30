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
#include <cairo/cairo.h>
#include <cairo_chart.h>


/* Defines */


/* Types */


/* Prototypes */

PieChart * pie_chart_init(char *, double, int);
int pie_slice_create(char *, double, GdkRGBA *);
void free_pie_chart(PieChart *);


/* Globals */

static const char *debug_hdr = "DEBUG-cairo_chart.c ";



/* Create and initialise a new pie chart */

// Some rules for creation:-
// . A title is optional (may be NULL). If used, keep as short as possible.
// . Total value is optional (zero). Code will work it out anyway, but might be a useful error check.
// . Legend should be TRUE or FALSE.

PieChart * pie_chart_init(char *title, double total_val, int legend);
{
    PieChart *pc;

    if (legend < 0) || (legend > 1)
    	return NULL;

    pc = (PieChart *) malloc(sizeof(PieChart));
    memset(pc, 0, sizeof(PieChart));

    if (title != NULL)
    {
    	pc->chart_title = malloc(strlen(title) + 1);
    	strcpy(pc->chart_title, title);
    }

    //pc->cr = cr;
    pc->total_val = total_val;
    pc->legend = legend;

    return pc;
}


/* Create and initialise a new pie slice */

int pie_slice_create(PieChart *pc, char *desc, double val, GdkRGBA *colour)
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

    g_list_free(pc->pie_slices);
    free(pc);

    return;
}
