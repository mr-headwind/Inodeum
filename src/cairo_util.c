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
**  Error and Message Reference functions
**  Logging functions
**  Session management
**  Window management
**  General usage functions
**
** Author:	Anthony Buckley
**
** History
**	26-Nov-2017	Initial code
**
*/


/* Defines */



/* Includes */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <math.h>


/* Prototypes */

int long_chars(long);
int llong_chars(long long);
int double_chars(double);
char * dtos(double, int);


/* Globals */

static const char *debug_hdr = "DEBUG-cairo_util.c ";


/* Return the length of a long int */

int long_chars(long numb)
{
    int i;
    long l;

    l = labs(numb);
    i = 1;

    while(l > 9)
    {
    	i++;
    	l /= 10;
    }

    return i;
}


/* Return the length of a long long int */

int llong_chars(long long numb)
{
    int i;
    long long ll;

    ll = llabs(numb);
    i = 1;

    while(ll > 9)
    {
    	i++;
    	ll /= 10;
    }

    return i;
}


/* Return the length of a double. Not possible really, due to precision. This discards the mantissa. */

int double_chars(double numb)
{
    long long ll;

    ll = (long long) numb;

    return llong_chars(ll);
}


/* Convert a double to a string (kind of) */

char * dtos(double d, int precision)
{
    int sz;
    char *s;

    sz = double_chars(d);
    s = (char *) malloc(sz + precision + 2);
    sprintf(s, "%*.*f", sz, precision, d);

    return s;
}
