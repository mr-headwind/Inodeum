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


/* Includes */

#include <cairo_chart.h>


/* Structure for main loop and data refresh timer */

typedef struct _refresh_tmr
{
    pthread_t refresh_tid;
    guint tmr_id;
    int refresh_req;
    time_t start_t;
    time_t curr_t;
    long ref_interval;
    char info_txt[50];
} RefreshTmr;


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
    GtkWidget *help_about;

    /* Main Frame widgets */
    GtkWidget *ctrl_box;
    GtkWidget *btn_panel;
    GtkWidget *panel_stk;

    /* Button panel widgets */
    GtkWidget *overview_btn;
    GtkWidget *service_btn;
    GtkWidget *monitor_btn;
    GtkWidget *history_btn;
    GtkWidget *pref_btn;
    GtkWidget *about_btn;

    /* Data panels */
    GtkWidget *oview_cntr;
    GtkWidget *srv_cntr;
    GtkWidget *mon_cntr;
    GtkWidget *hist_cntr;
    GtkWidget *pref_cntr;
    GtkWidget *about_cntr;

    /* Widgets - overview */
    GtkWidget *quota_lbl, *next_dt_lbl, *rem_days_lbl, *usage_lbl;
    GtkWidget *quota, *rollover_dt, *rem_days, *usage;
    GtkWidget *sum_cntr, *graph_area;
    PieChart *pie_chart;
    BarChart *bar_chart;

    /* Widgets - history */
    GtkWidget *from_dt_lbl, *to_dt_lbl, *cat_lbl, *hist_total;
    GtkWidget *hist_from_dt, *hist_to_dt, *fr_btn, *to_btn;
    GtkWidget *usgcat_cbox, *hist_search_btn;
    GtkWidget *hist_search_cntr, *hist_graph_area;
    LineGraph *hist_usg_graph;

    /* Widgets - about */
    GtkWidget *hdr_box, *misc_box, *tab_nb;
    GtkWidget *app_icon, *home_page;

    /* Widgets - monitor */
    GtkWidget *log_cntr, *net_cntr;
    GtkWidget *ip_addr, *mac_addr, *tx_bytes, *rx_bytes, *ndevs_cbox;
    GtkWidget *rx_bar, *tx_bar;
    GList *ndevs;

    /* Widgets - preferences */
    GtkWidget *reset_pw_btn;
    GtkWidget *refresh_tm;

    /* Other */
    GtkWidget *curr_panel;
    GtkWidget *cal_dt_select;

    /* XML processing (for debug) */
    GtkWidget *txt_view;

    /* Callback Handlers */
    int close_hndlr_id;
    int dvcbx_hndlr_id;

    /* Misc */
    int duration, user_cd;
    long rx1, tx1;
    double sn_rx_kbps, sn_tx_kbps, rx_max_kbps, tx_max_kbps;
    char *rxfn, *txfn;
    double days_rem, days_quota;
    RefreshTmr RefTmr;
    pthread_t net_speed_tid;
} MainUi;
