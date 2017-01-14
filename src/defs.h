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
** Description:	Local standard constant defines
**
** Author:	Anthony Buckley
**
** History
**	09-Jan-2017	Initial
**
*/


/* Includes */


/* General */
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef AC_COLOURS
#define AC_COLOURS
#ifdef MAIN_UI
const GdkRGBA BLUE_GRAY = {.9, .9, 1.0, 1};
const GdkRGBA LIGHT_BLUE = {.5, .5, 1.0, 1.0};
const GdkRGBA MID_BLUE = {.5, .5, 1.0, 1.0};
const GdkRGBA DARK_BLUE = {0, 0, 0.5, 1.0};
const GdkRGBA RED1 = {.8, .1, .1, 1.0};
const GdkRGBA NIGHT = {0.57, 0.24, 0.24, 0.7};
#else
const GdkRGBA BLUE_GRAY;
extern const GdkRGBA LIGHT_BLUE;
extern const GdkRGBA MID_BLUE;
extern const GdkRGBA DARK_BLUE;
extern const GdkRGBA RED1;
extern const GdkRGBA NIGHT;
#endif
#endif


/* Application Name et al */
#ifndef TITLE
#define TITLE "Inodeum"
#define APP_URI "http://Inodeum.sourceforge.net"
//#define PACKAGE_DATA_DIR "/usr/share" // Not sure of PACKAGE_DATA_DIR name, also -D on compile may be better
#define PACKAGE_DATA_DIR "/home/tony/.local/share"      // Dev only
#endif


/* Window size */
#define STD_VWIDTH 640
#define STD_VHEIGHT 480


/* Interface Names */
#ifndef UI_TITLES
#define UI_TITLES
#define ABOUT_UI "About"
#endif


/* Utility globals */
#ifndef ERR_INCLUDED
#define ERR_INCLUDED
#ifdef ERR_FILE
char app_msg_extra[256];
#else
extern char app_msg_extra[256];
#endif
#endif
