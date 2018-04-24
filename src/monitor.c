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
** Description:	Monitor current items, eg. log file.
**
** Author:	Anthony Buckley
**
** History
**	10-Jul-2017	Initial code
**
*/



/* Defines */


/* Includes */

#include <stdlib.h>  
#include <string.h>  
#include <libgen.h>  
#include <stdio.h>
#include <gtk/gtk.h>  
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <pcap.h> 
#include <errno.h>
#include <main.h>
#include <defs.h>
#include <version.h>


/* Types */

typedef struct _net_device {
    char *name;
    char ip[16];
    unsigned char mac[18];
} NetDevice;


/* Prototypes */

void monitor_panel(MainUi *m_ui);
GtkWidget * monitor_log(MainUi *m_ui);
GtkWidget * monitor_net(MainUi *m_ui);
GList * get_netdevices(MainUi *);
void get_net_details(MainUi *);
NetDevice * new_dev();
void free_dev(void *);
int monitor_device(MainUi *);
void * net_speed(void *);
void reset_fn(MainUi *);
int network_totals(char *, char *, char *, char *, const int);
void display_speed(GtkWidget *, double, double, double, double *);
int session_speed(MainUi *);
void bps_abbrev(double, double *, char *);

extern char * log_name();
extern void log_msg(char*, char*, char*, GtkWidget*);
extern int ip_address(char *, char [16], unsigned char [13]);
extern void create_label(GtkWidget **, char *, char *, GtkWidget *, int, int, int, int);
extern void create_entry(GtkWidget **, char *, GtkWidget *, int, int);
extern void set_sz_abbrev(char *);
extern FILE * open_file(char *, char *);
extern int read_file(FILE *, char *, int);
extern int check_errno();
extern void OnViewLog(GtkWidget*, gpointer);
extern void OnSetNetDev(GtkWidget*, gpointer);


/* Globals */

static const char *debug_hdr = "DEBUG-monitor.c ";
static const char *rtx_bytes_pfx = "/sys/class/net/";
static const char *rx_bytes_sfx = "/statistics/rx_bytes";
static const char *tx_bytes_sfx = "/statistics/tx_bytes";
static const int tx_rx_len = 35;
static const int fsz = 30;		// Max size allowed
const double kbps_dv = 128.0;		// 1024.0/8.0
static int net_mon;



/* Application 'Monitor' current items display panel */

void monitor_panel(MainUi *m_ui)
{  
    /* Log file */
    m_ui->log_cntr = monitor_log(m_ui);

    /* Network information */
    m_ui->net_cntr = monitor_net(m_ui);
    
    /* Combine everything for display */
    m_ui->mon_cntr = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_name(m_ui->mon_cntr, "monitor_panel");
    gtk_widget_set_margin_top (m_ui->mon_cntr, 10);
    gtk_widget_set_margin_left (m_ui->mon_cntr, 5);

    gtk_box_pack_start (GTK_BOX (m_ui->mon_cntr), m_ui->log_cntr, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (m_ui->mon_cntr), m_ui->net_cntr, FALSE, FALSE, 0);

    /* Add to the panel stack */
    gtk_stack_add_named (GTK_STACK (m_ui->panel_stk), m_ui->mon_cntr, "monitor_panel");

    return;
}


/* Log file details and a viewing option */

GtkWidget * monitor_log(MainUi *m_ui)
{  
    char *fn;
    GtkWidget *frame;
    GtkWidget *log_grid;
    GtkWidget *label_fn;
    GtkWidget *view_btn;

    /* Containers */
    frame = gtk_frame_new("Log File");
    log_grid = gtk_grid_new();

    /* Name and location */
    fn = log_name();
    label_fn = gtk_label_new(fn);
    gtk_widget_set_name(label_fn, "data_1");
    gtk_widget_set_margin_top(label_fn, 5);
    gtk_widget_set_margin_start(label_fn, 10);
    gtk_widget_set_halign (label_fn, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID (log_grid), label_fn, 0, 0, 1, 1);

    /* View button */
    view_btn = gtk_button_new_with_label("View");
    gtk_widget_set_margin_top(view_btn, 5);
    gtk_widget_set_margin_start(view_btn, 10);
    gtk_widget_set_halign (label_fn, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID (log_grid), view_btn, 1, 1, 1, 1);

    /* Pack */
    gtk_container_add(GTK_CONTAINER (frame), log_grid);

    /* Callback */
    g_signal_connect(view_btn, "clicked", G_CALLBACK(OnViewLog), m_ui);

    return frame;
}


/* Network traffic details and monitoring */

GtkWidget * monitor_net(MainUi *m_ui)
{  
    GtkWidget *frame, *frame2;
    GtkWidget *vbox, *bar_grid, *dev_grid, *stat_grid;
    GtkWidget *lbl;

    /* Containers */
    frame = gtk_frame_new("Network");
    gtk_widget_set_margin_top (frame, 10);

    /* Box for interface details and network monitoring */
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);

    /* ComboBox for interfaces, but only populate each time the panel is activated */
    m_ui->ndevs_cbox = gtk_combo_box_text_new();
    gtk_widget_set_margin_start (m_ui->ndevs_cbox, 20);
    gtk_widget_set_margin_end (m_ui->ndevs_cbox, 30);

    /* Grid for device details */
    dev_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID (dev_grid), 2);
    gtk_grid_set_column_spacing(GTK_GRID (dev_grid), 2);
    gtk_widget_set_margin_start (dev_grid, 20);

    create_label(&lbl, "iplbl", "IP Address: ", dev_grid, 0, 0, 1, 1);
    create_label(&(m_ui->ip_addr), "data_1", "", dev_grid, 1, 0, 1, 1);
    create_label(&lbl, "maclbl", "MAC Address: ", dev_grid, 0, 1, 1, 1);
    create_label(&(m_ui->mac_addr), "data_1", "", dev_grid, 1, 1, 1, 1);

    /* Grid for stats */
    stat_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID (stat_grid), 2);
    gtk_grid_set_column_spacing(GTK_GRID (stat_grid), 2);
    gtk_widget_set_margin_start (stat_grid, 20);

    create_label(&lbl, "rxlbl", "RX bytes this session: ", stat_grid, 0, 0, 1, 1);
    create_label(&(m_ui->rx_bytes), "data_1", "", stat_grid, 1, 0, 1, 1);
    create_label(&lbl, "txlbl", "TX bytes this session: ", stat_grid, 0, 1, 1, 1);
    create_label(&(m_ui->tx_bytes), "data_1", "", stat_grid, 1, 1, 1, 1);

    /* Network speed frame */
    frame2 = gtk_frame_new("Network speed (approx.)");
    gtk_widget_set_margin_top (frame2, 10);
    gtk_widget_set_margin_bottom (frame2, 10);
    gtk_widget_set_margin_start (frame2, 10);
    gtk_widget_set_margin_end (frame2, 10);

    /* Grid for device details */
    bar_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID (bar_grid), 2);
    gtk_grid_set_column_spacing(GTK_GRID (bar_grid), 2);
    gtk_widget_set_margin_start (bar_grid, 20);

    /* Network speed progress bars */
    create_label(&lbl, "barrxbl", "RX:", bar_grid, 0, 0, 1, 1);
    create_label(&lbl, "bartxbl", "TX:", bar_grid, 0, 1, 1, 1);

    m_ui->rx_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text (GTK_PROGRESS_BAR (m_ui->rx_bar), TRUE);
    gtk_widget_set_name (m_ui->rx_bar, "pbar_1");
    gtk_widget_set_margin_bottom (m_ui->rx_bar, 5);
    gtk_widget_set_margin_start (m_ui->rx_bar, 10);
    gtk_widget_set_margin_end (m_ui->rx_bar, 10);

    m_ui->tx_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text (GTK_PROGRESS_BAR (m_ui->tx_bar), TRUE);
    gtk_widget_set_name (m_ui->tx_bar, "pbar_1");
    gtk_widget_set_margin_bottom (m_ui->tx_bar, 5);
    gtk_widget_set_margin_start (m_ui->tx_bar, 10);
    gtk_widget_set_margin_end (m_ui->tx_bar, 10);

    gtk_grid_attach(GTK_GRID (bar_grid), m_ui->rx_bar, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID (bar_grid), m_ui->tx_bar, 1, 1, 1, 1);
    gtk_container_add(GTK_CONTAINER (frame2), bar_grid);

    /* Pack */
    gtk_box_pack_start (GTK_BOX (vbox), m_ui->ndevs_cbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), dev_grid, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), stat_grid, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), frame2, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER (frame), vbox);

    /* Callback */
    m_ui->dvcbx_hndlr_id = g_signal_connect(m_ui->ndevs_cbox, "changed", G_CALLBACK(OnSetNetDev), m_ui);

    return frame;
}


/* Set all the monitoring details */

void get_net_details(MainUi *m_ui)
{  
    GList *l;
    NetDevice *dev;
    
    /* Device list can be dynamic so reset every time */
    g_signal_handler_block (m_ui->ndevs_cbox, m_ui->dvcbx_hndlr_id);
    gtk_combo_box_text_remove_all (GTK_COMBO_BOX_TEXT (m_ui->ndevs_cbox));
    g_signal_handler_unblock (m_ui->ndevs_cbox, m_ui->dvcbx_hndlr_id);

    g_list_free_full (m_ui->ndevs, (GDestroyNotify) free_dev);

    /* New list */
    m_ui->ndevs = get_netdevices(m_ui);

    /* Set combo box */
    for(l = m_ui->ndevs; l != NULL; l = l->next)
    {
    	dev = (NetDevice *) l->data;
    	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (m_ui->ndevs_cbox), dev->name, dev->name);
    }

    gtk_combo_box_set_active (GTK_COMBO_BOX (m_ui->ndevs_cbox), 0);

    return;
}


/* Find network devices */

GList * get_netdevices(MainUi *m_ui)
{  
    int r;
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t *alldevs, *dev;
    NetDevice *ndev;
    GList *l;

    /* Ask pcap to find all devices for use */
    r = pcap_findalldevs(&alldevs, errbuf);

    /* Error checking */
    if (r == -1)
    {
	memset(app_msg_extra, '\0', sizeof(app_msg_extra));
	strncpy(app_msg_extra, errbuf, sizeof(app_msg_extra));
	log_msg("ERR0047", NULL, "ERR0047", m_ui->window);
	return FALSE;
    }

    if (alldevs == NULL)
    {
	sprintf(app_msg_extra, "No active, working dMAC_strevices found.");
	log_msg("ERR0047", NULL, "ERR0047", m_ui->window);
	return FALSE;
    }

    /* Save devices */
    l = NULL;

    for(dev = alldevs; dev != NULL; dev = dev->next)
    {
	/* Select valid devices */
	if (dev->flags & PCAP_IF_LOOPBACK)
	    continue;

	if (!(dev->flags & PCAP_IF_RUNNING))
	    continue;

	if (!(dev->flags & PCAP_IF_UP))
	    continue;

	if (strcmp(dev->name, "any") == 0)
	    continue;
	
	ndev = new_dev();

	/* Device name (eg. eth0) */
	ndev->name = (char *) malloc(strlen(dev->name) + 1);
	strcpy(ndev->name, dev->name);

	/* IP & MAC Address */
	if (ip_address(dev->name, ndev->ip, ndev->mac) < 0)
	{
	    sprintf(app_msg_extra, "%s\n", strerror(errno));
	    log_msg("ERR0047", NULL, "ERR0047", m_ui->window);
	    return FALSE;
	}

	/* Add to list */
	l = g_list_prepend(l, ndev);
    }

    l = g_list_reverse(l);
    pcap_freealldevs(alldevs);

    return l;
}


/* Create new network device */

NetDevice * new_dev()
{  
    NetDevice *dev = (NetDevice *) malloc(sizeof(NetDevice));

    return dev;
}


/* Free network device */

void free_dev(void *l)
{  
    NetDevice *dev;

    dev = (NetDevice *) l;
    free(dev->name);
    free(dev);

    return;
}


/* Display details for a selected device and initiate a thread to monitor performance */

int monitor_device(MainUi *m_ui)
{
    int p_err;
    char rx_s[31], tx_s[31];	// Possibly unsafe, but it would be a huge number
    gchar *txt;
    GList *l;
    NetDevice *dev;

    /* Selected device */
    txt = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (m_ui->ndevs_cbox));

    /* Reset filenames */
    reset_fn(m_ui);

    /* Match device details and display */
    for(l = m_ui->ndevs; l != NULL; l = l->next)
    {
    	dev = (NetDevice *) l->data;

	if (strcmp(dev->name, txt) == 0)
	{
	    /* Hardware */
	    gtk_label_set_text(GTK_LABEL (m_ui->ip_addr), dev->ip);
	    gtk_label_set_text(GTK_LABEL (m_ui->mac_addr), dev->mac);

	    /* Get new network totals */
	    m_ui->rxfn = (char *) malloc(tx_rx_len + strlen(dev->name) + 1);
	    sprintf(m_ui->rxfn, "%s%s%s", rtx_bytes_pfx, dev->name, rx_bytes_sfx);
	    m_ui->txfn = (char *) malloc(tx_rx_len + strlen(dev->name) + 1);
	    sprintf(m_ui->txfn, "%s%s%s", rtx_bytes_pfx, dev->name, tx_bytes_sfx);

	    if (network_totals(m_ui->rxfn, rx_s, m_ui->txfn, tx_s, fsz) == FALSE)
	    	return FALSE;

	    /* Statistics */
	    m_ui->rx1 = (double) atol(rx_s);
	    m_ui->tx1 = (double) atol(tx_s);
printf("%s monitor_device 1 rx_s: %s rx1 %0.2f   tx_s %s tx1 %0.2f\n", 
debug_hdr, rx_s, m_ui->rx1, tx_s, m_ui->tx1); fflush(stdout);
	    set_sz_abbrev(rx_s);
	    set_sz_abbrev(tx_s);
	    gtk_label_set_text(GTK_LABEL (m_ui->rx_bytes), rx_s);
	    gtk_label_set_text(GTK_LABEL (m_ui->tx_bytes), tx_s);

	    if (session_speed(m_ui) == FALSE)
	    	break;

	    /* Start performance monitor thread */
	    if ((p_err = pthread_create(&(m_ui->net_speed_tid), NULL, &net_speed, (void *) m_ui)) != 0)
	    {
		sprintf(app_msg_extra, "Error: %s", strerror(p_err));
		log_msg("ERR0048", NULL, "ERR0048", m_ui->window);
		return FALSE;
	    }

	    break;
	}
    }

    g_free(txt);

    return TRUE;
}


/* Network speed performance thread */

void * net_speed(void *arg)
{  
    double rx2, tx2;
    char rx_s[31], tx_s[31];		// Possibly unsafe, but it would be a huge number
    const gchar *nm;
    MainUi *m_ui;
    const int interval = 1000000;	// microseconds (1.0 seconds)
    //const int interval = 500000;	// microseconds (0.5 seconds)

    /* Initial */
    m_ui = (MainUi *) arg;
    nm = gtk_widget_get_name (m_ui->curr_panel);

    /* Refresh and display current net speed */
    while(1)
    {
	/* Wait interval */
	usleep(interval);

	/* Exit if 'monitor' is not the current panel */
	if (strcmp(nm, "monitor_panel") != 0)
	{
	    reset_fn(m_ui);
	    break;
	}

	/* If file name(s) is null, there may be new selection so just loop */
	if (m_ui->rxfn == NULL || m_ui->txfn == NULL)
	    continue;

	/* Get new network totals */
	if (network_totals(m_ui->rxfn, rx_s, m_ui->txfn, tx_s, fsz) == FALSE)
	    return FALSE;

	/* Statistics */
	rx2 = (double) atol(rx_s);
	tx2 = (double) atol(tx_s);

printf("%s net_speed 8 rx_s: %s rx2 %0.2f   tx_s %s tx2 %0.2f\n", debug_hdr, rx_s, rx2, tx_s, tx2); fflush(stdout);
	/* Set progressbar */
	display_speed(m_ui->rx_bar, m_ui->rx1, rx2, m_ui->sn_rx_kbps, &m_ui->max_kbps);
	display_speed(m_ui->tx_bar, m_ui->tx1, tx2, m_ui->sn_tx_kbps, &m_ui->max_kbps);

	/* Ready for next loop */
	m_ui->rx1 = rx2;
	m_ui->tx1 = tx2;
    }
    
    pthread_exit(&net_mon);
}


/* Reset filenames */

void reset_fn(MainUi *m_ui)
{  
    if (m_ui->rxfn != NULL)
    {
	free(m_ui->rxfn);
	m_ui->rxfn = NULL;
    }

    if (m_ui->txfn != NULL)
    {
	free(m_ui->txfn);
	m_ui->txfn = NULL;
    }

    return;
}


/* Read the network device current totals */

int network_totals(char *rxfn, char *rx, char *txfn, char *tx, const int fsz)
{
    int r;
    char *p;
    FILE *fd;

    /* RX */
    fd = open_file(rxfn, "r");
    
    if ((r = read_file(fd, rx, (int) fsz)) != EOF)
	return FALSE;

    if ((p = strchr(rx, '\n')) != NULL)
	*p = '\0';

    /* TX */
    fd = open_file(txfn, "r");
    
    if ((r = read_file(fd, tx, (int) fsz)) != EOF)
	return FALSE;

    if ((p = strchr(tx, '\n')) != NULL)
	*p = '\0';

    return TRUE;
}


// Set the Progress bars to indicate speeds.
// This is only an approximate speed indicator and not a performance monitor.
// It is really a fudge using ProgressBars to display a meter by continually
// changing the value.
// Doing this requires a notional 100 percent complete speed never to attained.
// The formula for this is entirely arbitrary:-
//   . Work out the average speed for the session (total bytes / uptime secs).
//   . Set this as the 75% mark, thus 1.5x is the maximum.
//   . From then on, at each iteration, if the current speed >= the maximum,
//     reset the maximum to current speed plus 25% - This will almost certainly
//     happen as the session figure will be skewed low, but it should stabalise
//   (NB 75% and 25% area set constants that may change - see code).
// All speeds are worked out based on Kb/s (kilobits/sec), but should be displayed as Kb/s, Mb/s
// and Gb/s as appropriate.

void display_speed(GtkWidget *pbar, double x1, double x2, double sn_kbps, double *max_kbps)
{
    double x_kbps, tmp_bps;
    char s[30], abbrev[5];
    const double max_kbps_init = 1.5;
    const double max_kbps_adj = 1.25;

printf("%s display_speed 1 x1:  %0.2f   x2  %0.2f\n", debug_hdr, x1, x2); fflush(stdout);
    /* Calculate current speed */
    x_kbps = (x2 - x1) / kbps_dv;
printf("%s display_speed 1a x_kbps:  %0.2f   max_kbps  %0.2f\n", debug_hdr, x_kbps, *max_kbps); fflush(stdout);

    /* Set maximun if required */
    if (*max_kbps == 0.0)
    {
	*max_kbps = sn_kbps * max_kbps_init;
printf("%s display_speed 3 max_kbps:  %0.2f\n", debug_hdr, *max_kbps); fflush(stdout);
    }
    else if (x_kbps >= *max_kbps)
    {
    	*max_kbps = x_kbps * max_kbps_adj;
printf("%s display_speed 4 max_kbps:  %0.2f\n", debug_hdr, *max_kbps); fflush(stdout);
    }

    bps_abbrev(x_kbps, &tmp_bps, abbrev);
    sprintf(s, "%0.2f %s", tmp_bps, abbrev);
printf("%s display_speed 1 s: %s\n", debug_hdr, s); fflush(stdout);
    gtk_progress_bar_set_text (GTK_PROGRESS_BAR (pbar), s);
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (pbar), x_kbps / *max_kbps);

    return;
}


/* Determine the session speed averages */

int session_speed(MainUi *m_ui)
{
    struct sysinfo info;

    if (sysinfo(&info) != 0)
    {
	check_errno;
	return FALSE;
    }

    m_ui->max_kbps = 0.0;
    m_ui->sn_rx_kbps = (m_ui->rx1 / (double) info.uptime) / kbps_dv;
    m_ui->sn_tx_kbps = (m_ui->tx1 / (double) info.uptime) / kbps_dv;
printf("%s session_speed 1 rx %0.2f  tx %0.2f\n", debug_hdr, m_ui->sn_rx_kbps, m_ui->sn_tx_kbps); fflush(stdout);

    return TRUE;
}


/* Set speed abbreviation */

void bps_abbrev(double xbps, double *tmp_bps, char *abbrev)
{
    if (xbps < 1000.0)
    {
    	strcpy(abbrev, "Kbps");
    	*tmp_bps = xbps;
    }
    else if (xbps < 1000000.0)
    {
    	strcpy(abbrev, "Mbps");
    	*tmp_bps = xbps / 1000.0;
    }
    else 
    {
    	strcpy(abbrev, "Gbps");
    	*tmp_bps = xbps / 1000000.0;
    }

    return;
}


/*
while trueutility.c
do
        R1=`cat /sys/class/net/$1/statistics/rx_bytes`
        T1=`cat /sys/class/net/$1/statistics/tx_bytes`
        sleep $INTERVAL
        R2=`cat /sys/class/net/$1/statistics/rx_bytes`
        T2=`cat /sys/class/net/$1/statistics/tx_bytes`
        TBPS=`expr $T2 - $T1`
        RBPS=`expr $R2 - $R1`
        TKBPS=`expr $TBPS / 1024`
        RKBPS=`expr $RBPS / 1024`
        echo "TX $1: $TKBPS kB/s RX $1: $RKBPS kB/s"
done
*/
