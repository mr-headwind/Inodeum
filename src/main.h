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
**	09-Jan-2017	Initial
**
*/




/* Defines */

#ifndef MAIN_HDR
#define MAIN_HDR
#endif

/* Structure to contain main interface items for easy access */

typedef struct _main_ui
{
    /* Main view widgets */
    GtkWidget *window;
    GtkWidget *status_info;  
    GtkWidget *mbox;  

    /* Menu items */
    GtkWidget *menu_bar; 
    GtkWidget *file_menu, *help_menu;
    GtkWidget *file_hdr, *help_hdr;
    GtkWidget *file_exit;
    GtkWidget *help_about;

    /* Main Frame widgets */
    GtkWidget *ctrl_box;
    GtkWidget *cntl_grid;
    GtkWidget *main_frame_grid;  
    GtkWidget *frame;

    /* Data widgets */
    GtkWidget *uname_ent;
    GtkWidget *pw_ent;
    GtkWidget *txt_view;

    /* Other */
    GtkWidget *ok_btn;
    GtkWidget *scrollwin;

    /* Callback Handlers */
    int close_hndlr_id;

    int duration;
} MainUi;
