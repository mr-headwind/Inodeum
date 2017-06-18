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
    GtkWidget *file_menu, *service_menu, *help_menu;
    GtkWidget *file_hdr, *service_hdr, *help_hdr;
    GtkWidget *file_exit;
    GtkWidget *user_login;
    GtkWidget *reset_pw;
    GtkWidget *help_about;

    /* Main Frame widgets */
    GtkWidget *ctrl_box;
    GtkWidget *btn_panel;
    GtkWidget *scrollwin;

    /* Button panel widgets */
    GtkWidget *overview_btn;
    GtkWidget *service_btn;
    GtkWidget *monitor_btn;
    GtkWidget *history_btn;
    GtkWidget *log_btn;
    GtkWidget *about_btn;

    /* Data panels */
    GtkWidget *oview_cntr;
    GtkWidget *srv_cntr;
    GtkWidget *mon_cntr;
    GtkWidget *hist_cntr;
    GtkWidget *log_cntr;
    GtkWidget *about_cntr;

    /* Data widgets */
    GtkWidget *quota_lbl, *next_dt_lbl, *rem_days_lbl, *usage_lbl;
    GtkWidget *quota, *rollover_dt, *rem_days, *usage;

    /* Other */
    GtkWidget *curr_cntr;

    /* XML processing (for debug) */
    GtkWidget *txt_view;

    /* Callback Handlers */
    int close_hndlr_id;

    int duration;
} MainUi;
