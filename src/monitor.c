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
#include <pcap.h> 
#include <errno.h>
#include <main.h>
#include <defs.h>
#include <version.h>


/* Types */

typedef struct _net_device {
    char *name;
    char *ip;
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

extern char * log_name();
extern void log_msg(char*, char*, char*, GtkWidget*);
extern int ip_address(char *, char *, unsigned char [13]);
extern void create_label(GtkWidget **, char *, char *, GtkWidget *, int, int, int, int);
extern void create_entry(GtkWidget **, char *, GtkWidget *, int, int);
extern void set_sz_abbrev(char *, int);
extern char * read_file(char *);
extern void OnViewLog(GtkWidget*, gpointer);
extern void OnSetNetDev(GtkWidget*, gpointer);


/* Globals */

static const char *debug_hdr = "DEBUG-monitor.c ";
static const char *rx_bytes = "/sys/class/net/$1/statistics/rx_bytes";
static const char *tx_bytes = "/sys/class/net/$1/statistics/tx_bytes";



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
    GtkWidget *hbox, *dev_grid, *stat_grid;
    GtkWidget *lbl;

    /* Containers */
    frame = gtk_frame_new("Network");

    /* Box for interface details and network monitoring */
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);

    /* ComboBox for interfaces, but only populate each time the panel is activated */
    m_ui->ndevs_cbox = gtk_combo_box_text_new();

    /* Grid for device details */
    dev_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID (dev_grid), 2);
    gtk_grid_set_column_spacing(GTK_GRID (dev_grid), 2);

    create_label(&lbl, "iplbl", "IP Address", dev_grid, 0, 0, 1, 1);
    create_entry(&(m_ui->ip_addr), "ip", dev_grid, 1, 0);
    create_label(&lbl, "maclbl", "MAC Address", dev_grid, 0, 1, 1, 1);
    create_entry(&(m_ui->mac_addr), "mac", dev_grid, 1, 1);

    /* Grid for stats */
    stat_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID (stat_grid), 2);
    gtk_grid_set_column_spacing(GTK_GRID (stat_grid), 2);

    create_label(&lbl, "rxlbl", "RX bytes this session", stat_grid, 0, 1, 1, 1);
    create_entry(&(m_ui->rx_bytes), "rx", stat_grid, 1, 1);
    create_label(&lbl, "txlbl", "TX bytes this session", stat_grid, 0, 0, 1, 1);
    create_entry(&(m_ui->tx_bytes), "tx", stat_grid, 1, 0);

    /* Network speed progress bar */
    frame2 = gtk_frame_new("Network speed");
    m_ui->tx_bar = gtk_progress_bar_new();
    gtk_container_add(GTK_CONTAINER (frame2), m_ui->tx_bar);

    /* Pack */
    gtk_box_pack_start (GTK_BOX (hbox), m_ui->ndevs_cbox, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), dev_grid, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), stat_grid, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), frame2, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER (frame), hbox);

    /* Callback */
    g_signal_connect(m_ui->ndevs_cbox, "changed", G_CALLBACK(OnSetNetDev), m_ui);

    return frame;
}


/* Set all the monitoring details */

void get_net_details(MainUi *m_ui)
{  
    GList *l;
    NetDevice *dev;
    
    /* Device list can be dynamic so reset every time */
    gtk_combo_box_text_remove_all (GTK_COMBO_BOX_TEXT (m_ui->ndevs_cbox));
    g_list_free_full (m_ui->ndevs, (GDestroyNotify) free_dev);

    /* New list */
    m_ui->ndevs = get_netdevices(m_ui);

    /* Set combo box */
    for(l = m_ui->ndevs; l != NULL; l = l->next)
    {
    	dev = (NetDevice *) l->data;
    	gtk_combo_box_text_append (GTK_COMBO_BOX_TEXT (m_ui->ndevs_cbox), dev->name, dev->name);
    }

    gtk_combo_box_set_active (GTK_COMBO_BOX (m_ui->ndevs_cbox), 1);

    return;
}


/* Find network devices */

GList * get_netdevices(MainUi *m_ui)
{  
    int r;
    char *ip;
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
	if (ip_address(dev->name, ip, ndev->mac) < 0)
	{
	    sprintf(app_msg_extra, "%s\n", strerror(errno));
	    log_msg("ERR0047", NULL, "ERR0047", m_ui->window);
	    return FALSE;
	}

	ndev->ip = (char *) malloc(strlen(ip) + 1);
	strcpy(ndev->ip, ip);

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
    free(dev->ip);
    free(dev);

    return;
}


/* Display details for a selected device and initiate a thread to monitor performance */

int monitor_device(MainUi *m_ui)
{
    char *rx, *tx;
    gchar *txt;
    GList *l;
    NetDevice *dev;

    /* Selected device */
    txt = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (m_ui->ndevs_cbox));

    /* Match device details and display */
    for(l = m_ui->ndevs; l != NULL; l = l->next)
    {
    	dev = (NetDevice *) l->data;

	if (strcmp(dev->name, txt) == 0)
	{
	    /* Hardware */
	    gtk_entry_set_text(GTK_ENTRY (m_ui->ip_addr), dev->ip);
	    gtk_entry_set_text(GTK_ENTRY (m_ui->mac_addr), dev->mac);

	    if ((rx = read_file((char *) rx_bytes)) == NULL)
	    	return FALSE;

	    if ((tx = read_file((char *) tx_bytes)) == NULL)
	    	return FALSE;

	    /* Statistics */
	    set_sz_abbrev(rx, 1);
	    set_sz_abbrev(tx, 1);
	    gtk_entry_set_text(GTK_ENTRY (m_ui->rx_bytes), rx);
	    gtk_entry_set_text(GTK_ENTRY (m_ui->tx_bytes), tx);
	    free(rx);
	    free(tx);

	    /* Start performance monitor thread */

	    break;
	}
    }

    g_free(txt);

    return TRUE;
}


/*
while true
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
