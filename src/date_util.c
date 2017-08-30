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
**  Date and time utility functions
**
** Author:	Anthony Buckley
**
** History
**	04-May-2017	Initial code
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
#include <time.h>


/* Prototypes */

int64_t msec_time();
void cur_date_str(char *, int, char *);
time_t date_tm_add(struct tm *, char *, int);
time_t strdt2tmt(char *, char *, char *, char *, char *, char *);
double difftime_days(time_t, time_t);


/* Globals */

static const char *debug_hdr = "DEBUG-date_util.c ";




/* Return the current time in milliseconds */

int64_t msec_time()
{
    int64_t msecs;
    struct timespec t;

    clock_gettime(CLOCK_REALTIME_COARSE, &t);
    msecs = t.tv_sec * INT64_C(1000) + t.tv_nsec / 1000000;

    return msecs;
}


/* Get a string for the current time */

void cur_date_str(char *date_str, int s_sz, char *fmt)
{
    struct tm *tm;
    time_t current_time;
    size_t sz;

    *date_str = '\0';
    current_time = time(NULL);
    tm = localtime(&current_time);
    sz = strftime(date_str, s_sz, fmt, tm);

    return;
}


/* Pathetic attempt at date part (eg. Month) addition (use negative amount for subtraction) */

time_t date_tm_add(struct tm *p_tm, char *dt_part, int amt)
{
    int yr, i, j, len;
    const char *dt_parts[] = {"day", "month", "year", "hour", "minute", "second", "week"};
    int max_type = 7;
    time_t tm_t; 

    /* Determine the date part */
    for(i = 0; i < max_type; i++)
    {
	len = strlen(dt_parts[i]);

	for(j = 0; j < len; j++)
	{
	    if (tolower(*(dt_part + j)) != dt_parts[i][j])
		break;
	}

	if (j >= len)
	    break;
    }

    if (i >= max_type)
    	i = -1;

    /* Add (or Subtract) the unit amount for the date part */
    switch(i)
    {
    	case 0:				// Day
	    p_tm->tm_mday += amt; break;

    	case 1:				// Month
	    p_tm->tm_mon += amt; break;

    	case 2:				// Year
	    p_tm->tm_year += amt; break;

    	case 3:				// Hour
	    p_tm->tm_hour += amt; break;

    	case 4:				// Minute
	    p_tm->tm_min += amt; break;

    	case 5:				// Second
	    p_tm->tm_sec += amt; break;

    	case 6:				// Week
	    p_tm->tm_mday += (amt * 7); break;

    	default:
	    break;
    }

    /* Normalize new time */
    tm_t = mktime(p_tm);      
    //printf("%s date_tm_add: %s\n", debug_hdr, asctime(p_tm)); fflush(stdout);

    return tm_t;
}


/* Convert a date string to a time_t */

time_t strdt2tmt(char *yyyy, char *mm, char *dd, char *hh, char *min, char *sec)
{
    time_t tm_t;
    struct tm dtm;
    char *end;

    dtm.tm_year = ((int) strtol(yyyy, &end, 10)) - 1900;
    dtm.tm_mon = ((int) strtol(mm, &end, 10)) - 1;
    dtm.tm_mday = ((int) strtol(dd, &end, 10));
    dtm.tm_hour = ((int) strtol(hh, &end, 10));
    dtm.tm_min = ((int) strtol(min, &end, 10));
    dtm.tm_sec = ((int) strtol(sec, &end, 10));

    tm_t = mktime(&dtm);

    return tm_t;
}


/* Return the number of days difference between 2 dates */

double difftime_days(time_t tm_t1, time_t tm_t0)
{
    double ndays;

    ndays = difftime(tm_t1, tm_t0);
    ndays /= (24 * 60 * 60);

    return ndays;
}
