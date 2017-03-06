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
** Description: Preferences user interface and management.
**
** Author:	Anthony Buckley
**
** History
**	8-Aug-2014	Initial code
**
*/


/* Includes */

#include <gtk/gtk.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


/* Defines */


/* Types */


/* Prototypes */

int get_user_pref(char *, char **);


/* Globals */

static const char *debug_hdr = "DEBUG-prefs.c ";



/* Return a pointer to a user preference value for a key or NULL */

int get_user_pref(char *key, char **val)
{
    int i;

printf("%s USER PREFERENCES Not Implemented yet\n", debug_hdr);
    *val = NULL;
    i = 0;

    return i;
}
