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
** Description:	Module for (Main) Callback functions
**
** Author:	Anthony Buckley
**
** History
**	12-Jan-2017	Initial code
*/


/* Includes */

#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <libgen.h>  
#include <gtk/gtk.h>  
#include <glib/gbase64.h>
#include <main.h>
#include <isp.h>
#include <defs.h>
#include <version.h>



/* Prototypes */

int service_details(IspData *, MainUi *);
int isp_ip(IspData *, MainUi *);
int create_socket(IspData *, MainUi *);
int send_request(char *, IspData *, MainUi *);
char * setup_get(char *, IspData *);
void encode_un_pw(IspData *, MainUi *);
int send_query(char *, MainUi *);
int recv_data(MainUi *);

extern void log_msg(char*, char*, char*, GtkWidget*);


/* Globals */

static const char *debug_hdr = "DEBUG-service.c ";


/* API Webtools service requests */


/* Get all the Service details - each request is discrete */

int service_details(IspData *isp_data, MainUi *m_ui)
{  
    char url[500];

    /* Initial */
    if (isp_ip(isp_data, m_ui) == FALSE)
    	return FALSE;

    if (create_socket(isp_data, m_ui) == FALSE)
    	return FALSE;

    sprintf(isp_data->user_agent, "%s %s", TITLE, VERSION);

    encode_un_pw(isp_data, m_ui);

    /* 1. Service Listing */
    sprintf(url, "/api/%s/", HOST, API_VER);
    //sprintf(url, "%s%s/api/%s/", API_PROTO, HOST, API_VER);
    
    if (send_request(url, isp_data, m_ui) == FALSE)
    	return FALSE;

    /* 2. Service Type - Personal ADSL */

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

    if (inet_ntop(AF_INET, (void *) hent->h_addr_list[0], ip, len) == NULL)
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


/* Encode the username and password in base64 */

void encode_un_pw(IspData *isp_data, MainUi *m_ui)
{ 
    const gchar *uname, *pw;
    gchar *unpw_b64;
    int len;
    char *tmp;

    uname = gtk_entry_get_text (GTK_ENTRY (m_ui->uname_ent));
    pw = gtk_entry_get_text (GTK_ENTRY (m_ui->pw_ent));

    len = strlen(uname) + strlen(pw);
    tmp = (char *) malloc(len + 2);
    sprintf(tmp, "%s:%s", uname, pw);

    isp_data->enc64 = g_base64_encode ((const guchar *) tmp, len + 1);

    free(tmp);

    return;
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
    if (send_query(get_gry, m_ui) == FALSE)
    	return FALSE;

    /* Receive data */
    if (recv_data(m_ui) == FALSE)
    	return FALSE;

    /* Clean up */
    free(get_qry);

    return TRUE;
}  


/* Set up the query */

char * setup_get(char *url, IspData *isp_data)
{  
    char *query;

    query = (char *) malloc(strlen(url) +
			    strlen(HOST) +
			    strlen(isp_data->user_agent) +
			    strlen(isp_data->enc64) +
			    strlen(REALM) +
			    strlen(GET_TPL) - 7);	// Note 7 accounts for 4 x %s in template plus \0

    sprintf(query, GET_TPL, url, HOST, isp_data->user_agent, REALM);

    return query;
}  


/* Send the query to the server */

int send_query(char *get_qry, MainUi *m_ui)
{  
    int r, sent;

    sent = 0;

    while(sent < strlen(get_qry))
    {
	r = send(sock, get_qry + sent, strlen(get_qry) - sent, 0);

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


/* Receive xml from the server */

int recv_data(MainUi *m_ui)
{  
    int r;
    int xmlstart = FALSE;
    char buf[BUFSIZ + 1];
    char *xml;

    memset(buf, 0, sizeof(buf));

    while((r = recv(sock, buf, BUFSIZ, 0)) > 0)
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
	    fprintf(stdout, xml);
	}

	memset(buf, 0, r);
    }

    if(r < 0)
    {
	log_msg("ERR0011", NULL, "ERR0011", m_ui->window);
	return FALSE;
    }

    return TRUE;
}  
