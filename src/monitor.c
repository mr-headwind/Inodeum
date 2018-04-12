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
#include <main.h>
#include <defs.h>
#include <version.h>


/* Types */

struct net_device {
    char *name;
    char *ip;
    char *mac;
};


/* Prototypes */

void monitor_panel(MainUi *m_ui);
GtkWidget * monitor_log(MainUi *m_ui);
GtkWidget * monitor_net(MainUi *m_ui);

extern char * log_name();
extern int ip_address(char *, char *);
extern void OnViewLog(GtkWidget*, gpointer);
extern void OnSetNetDev(GtkWidget*, gpointer);


/* Globals */

static const char *debug_hdr = "DEBUG-monitor.c ";



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
    GtkWidget *frame;
    GtkWidget *nbox;
    GList *ndevs

    /* Containers */
    frame = gtk_frame_new("Network");

    /* Box for interface details and network monitoring */
    nbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);

    /* Build a list and ComboBox for each interface found */
    m_ui->ndevs_cbox = gtk_combo_box_text_new();
    ndevs = get_netdevices();

    /* Pack */
    //gtk_container_add(GTK_CONTAINER (frame), log_grid);

    /* Callback */
    g_signal_connect(m_ui->ndevs_cbox, "changed", G_CALLBACK(OnSetNetDev), m_ui);

    return frame;
}


/* Find network devices */

GList * get_netdevices(MainUi *m_ui)
{  
    int r;
    char *ip;
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t *alldevs, *dev;
    struct net_device *ndev;

    /* Ask pcap to find all devices for use */
    r = pcap_findalldevs(&alldevs, errbuf);

    /* Error checking */
    if (r == -1)
    {
	memcpy(app_msg_extra, '\0', sizeof(app_msg_extra));
	strncpy(app_msg_extra, errbuf, sizeof(app_msg_extra));
	log_msg("ERR0047", NULL, "ERR0047", m_ui->window);
	return FALSE;
    }

    if (alldevs == NULL)
    {
	sprintf(app_msg_extra, "No active, working devices found.");
	log_msg("ERR0047", NULL, "ERR0047", m_ui->window);
	return FALSE;
    }

    /* Save devices */
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
	
	/* Device name (eg. eth0) */
	ndev->name = (char *) malloc(strlen(dev->name) + 1);
	strcpy(ndev->name, dev->name);

	/* IP Address */
	if (ip_address(dev->name, ip) < 0)
	{
	    sprintf(app_msg_extra, "%s\n", strerror(errno));
	    log_msg("ERR0047", NULL, "ERR0047", m_ui->window);
	    return FALSE;
	}

	ndev->ip = (char *) malloc(strlen(ip) + 1);
	strcpy(ndev->ip, ip);

	/* Mac Address */



	printf("DEV: %s", dev->name);

	if (dev->description != NULL)
	    printf("  %s", dev->description);

	printf("\n");
	net_address(dev->name);
	ip_address(dev->name);
	printf("\n");
    }

    pcap_freealldevs(alldevs);

    exit(0);
}

    return l;
}




/* ldev_all.c
   To compile:
   >cc -o ldev_all ldev_all.c -lpcap

   Looks for all interfaces, and lists the network ip
   and mask associated with that interface.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pcap.h>  /* GIMME a libpcap plz! */
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>


int net_address(char *);
int ip_address(char *);


int main(int argc, char **argv)
{
    int r;   /* return code */
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t *alldevs, *dev;

    /* ask pcap to find a valid device for use to sniff on */
    r = pcap_findalldevs(&alldevs, errbuf);

    /* error checking */
    if (r == -1)
    {
	printf("Error: %s\n",errbuf);
	exit(1);
    }

    if (alldevs == NULL)
    {
	printf("Error: No devices found\n");
	exit(1);
    }

    /* print out devices */
    for(dev = alldevs; dev != NULL; dev = dev->next)
    {
	if (dev->flags & PCAP_IF_LOOPBACK)
	    continue;

	if (!(dev->flags & PCAP_IF_RUNNING))
	    continue;

	if (!(dev->flags & PCAP_IF_UP))
	    continue;

	printf("DEV: %s", dev->name);

	if (dev->description != NULL)
	    printf("  %s", dev->description);

	printf("\n");
	net_address(dev->name);
	ip_address(dev->name);
	printf("\n");
    }

    pcap_freealldevs(alldevs);

    exit(0);
}


int net_address(char *dev)
{
    char *net; /* dot notation of the network address */
    char *mask;/* dot notation of the network mask    */
    char errbuf[PCAP_ERRBUF_SIZE];
    int r;   /* return code */
    bpf_u_int32 netp; /* ip          */
    bpf_u_int32 maskp;/* subnet mask */
    struct in_addr addr;

    /* ask pcap for the network address and mask of the device */
    r = pcap_lookupnet(dev, &netp, &maskp,errbuf);

    if (r == -1)
    {
	printf("Error: %s\n",errbuf);
	return -1;
    }

    /* get the network address in a human readable form */
    addr.s_addr = netp;
    net = inet_ntoa(addr);

    if (net == NULL)/* thanks Scott :-P */
    {
	perror("inet_ntoa");
	return -1;
    }

    printf("NET: %s\n",net);

    /* do the same as above for the device's mask */
    addr.s_addr = maskp;
    mask = inet_ntoa(addr);

    if(mask == NULL)
    {
	perror("inet_ntoa");
	return -1;
    }

    printf("MASK: %s\n",mask);

    return 0;
}


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
