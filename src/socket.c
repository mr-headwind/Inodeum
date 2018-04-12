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
** Description:	Module for standard socket functions
**
** Author:	Anthony Buckley
**
** History
**	12-Jan-2017	Initial code
*/


/* Defines */



/* Includes */

#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>  
#include <libgen.h>  
#include <gtk/gtk.h>  
#include <main.h>
#include <isp.h>
#include <defs.h>
#include <version.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>



/* Prototypes */

int get_url(char *, IspData *, MainUi *);
int isp_ip(IspData *, MainUi *);
int create_socket(IspData *, MainUi *);
int send_request(char *, IspData *, MainUi *);
int send_query(char *, IspData *, MainUi *);
int recv_data(IspData *, MainUi *);
int ip_address(char *, char *);

extern void log_msg(char*, char*, char*, GtkWidget*);
extern char * setup_get(char *, IspData *);


/* Globals */

static const char *debug_hdr = "DEBUG-socket.c ";





/* Get a requested url */

int get_url(char *url, IspData *isp_data, MainUi *m_ui)
{  
    /* Initial */
    if (isp_ip(isp_data, m_ui) == FALSE)
    	return FALSE;

    if (create_socket(isp_data, m_ui) == FALSE)
    	return FALSE;

    /* Send url get request */
    if (send_request(url, isp_data, m_ui) == FALSE)
    	return FALSE;

    return TRUE;
}  


/* Get ip details - lookup the host and convert the address to text */

int isp_ip(IspData *isp_data, MainUi *m_ui)
{  
    struct hostent *hent;
    int len = 15; 				// ip format is xxx.xxx.xxx.xxx

    isp_data->ip = (char *) malloc(len + 1);
    memset(&(isp_data->ip), 0, len + 1);

    if ((hent = gethostbyname(HOST)) == NULL)
    {
	log_msg("ERR0004", HOST, "ERR0004", m_ui->window);
	return FALSE;
    }

    if (inet_ntop(AF_INET, (void *) hent->h_addr_list[0], isp_data->ip, len) == NULL)
    {
	log_msg("ERR0005", HOST, "ERR0005", m_ui->window);
	return FALSE;
    }

    return TRUE;
}  


/* Create a socket */

int create_socket(IspData *isp_data, MainUi *m_ui)
{  
    int r;

    /* New socket */
    if ((isp_data->tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
	log_msg("ERR0006", NULL, "ERR0006", m_ui->window);
	return FALSE;
    }

    isp_data->isp_addr = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in *));
    isp_data->isp_addr->sin_family = AF_INET;

    /* Host byte order to network byte order */
    isp_data->isp_addr->sin_port = htons(PORT);;

    /* Convert address from text to binary */
    r = inet_pton(AF_INET, isp_data->ip, (void *) (&(isp_data->isp_addr->sin_addr.s_addr)));

    if (r < 0)  
    {
	log_msg("ERR0007", "AF_INET", "ERR0007", m_ui->window);
	return FALSE;

    }
    else if (r == 0)
    {
	log_msg("ERR0008", isp_data->ip, "ERR0008", m_ui->window);
	return FALSE;
    }

    return TRUE;
}  


/* Construct and send a GET request */

int send_request(char *url, IspData *isp_data, MainUi *m_ui)
{  
    char *get_qry;

    /* Connect */
    if (connect(isp_data->tcp_sock, (struct sockaddr *) isp_data->isp_addr, sizeof(struct sockaddr)) < 0)
    {
	log_msg("ERR0009", NULL, "ERR0009", m_ui->window);
	return FALSE;
    }

    /* Construct GET */
    get_qry = setup_get(url, isp_data);

    /* Send query */
    if (send_query(get_qry, isp_data, m_ui) == FALSE)
    	return FALSE;

    /* Receive data */
    if (recv_data(isp_data, m_ui) == FALSE)
    	return FALSE;

    /* Clean up */
    free(get_qry);

    return TRUE;
}  


/* Send the query to the server */

int send_query(char *get_qry, IspData *isp_data, MainUi *m_ui)
{  
    int r, sent;
    sent = 0;

    while(sent < strlen(get_qry))
    {
	r = send(isp_data->tcp_sock, get_qry + sent, strlen(get_qry) - sent, 0);

	if (r == -1)
	{
	    log_msg("ERR0010", NULL, "ERR0010", m_ui->window);
	    return FALSE;
	}
	else
	{
	    sent += r;
	}

    return TRUE;
    }  
}  


/* Receive xml from the server and add to the text view */

int recv_data(IspData *isp_data, MainUi *m_ui)
{  
    int r;
    int xmlstart = FALSE;
    char buf[BUFSIZ + 1];
    char *xml;
    GtkTextBuffer *txt_buffer;  
    GtkTextIter iter;

    memset(buf, 0, sizeof(buf));
    txt_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (m_ui->txt_view));

    while((r = recv(isp_data->tcp_sock, buf, BUFSIZ, 0)) > 0)
    {
	if (xmlstart == 0)
	{
	    /* Under certain conditions this will not work.
	     * If the \r\n\r\n part is splitted into two messages
	     * it will fail to detect the beginning of HTML content */

	    xml = strstr(buf, "\r\n\r\n");

	    if (xml != NULL)
	    {
		xmlstart = TRUE;
		xml += 4;
	    }
	}
	else
	{
	    xml = buf;
	}

	if (xmlstart)
	{
	    gtk_text_buffer_get_end_iter (txt_buffer, &iter);
	    gtk_text_buffer_insert (txt_buffer, &iter, buf, -1);
	    gtk_text_iter_forward_to_end (&iter);
	}

	memset(buf, 0, r);
    }

    if (r < 0)
    {
	log_msg("ERR0011", NULL, "ERR0011", m_ui->window);
	return FALSE;
    }

    return TRUE;
}  


int ip_address(char *dev, char *ip)
{
    int fd;
    struct ifreq ifr;
    int r; 
    char *s;
    const int min_ip_len = 16;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;		 /* IPv4 IP address */
    strncpy(ifr.ifr_name, dev, IFNAMSIZ-1);

    r = ioctl(fd, SIOCGIFADDR, &ifr);

    if (r < 0)
    {
	printf("Error: (%d) %s\n",  errno, strerror(errno));
	close(fd);
	return -1;
    }

    close(fd);

    if (strlen(ip) < min_ip_len)
    	return -2;

    ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);

    return 0;
}
