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
#include <main.h>
#include <isp.h>
#include <defs.h>
#include <version.h>



/* Prototypes */

int service_details(IspData *, MainUi *);
int isp_ip(IspData *, MainUi *);
int create_socket(IspData *, MainUi *);
int send_request(char *, IspData *, MainUi *);

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

    /* 1. Service Listing */
    sprintf(url, "%s%s/api/%s/", API_PROTO, HOST, API_VER);
    
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
    get_qry = setup_get(HOST, url);

    /* Send query */

    /* Receive data */

    return TRUE;
}  


/* Create a socket */

char * setup_get(IspData *isp_data, MainUi *m_ui)
{  
    char *query;
    char *getpage = page;
    char *tpl = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
    if(getpage[0] == '/'){
    getpage = getpage + 1;
    fprintf(stderr,"Removing leading \"/\", converting %s to %s\n", page, getpage);
    }
    // -5 is to consider the %s %s %s in tpl and the ending \0
    query = (char *)malloc(strlen(host)+strlen(getpage)+strlen(USERAGENT)+strlen(tpl)-5);
    sprintf(query, tpl, getpage, host, USERAGENT);
    return query;

    return TRUE;
}  





    int create_tcp_socket();
    char *get_ip(char *host);
    char *build_get_query(char *host, char *page);
    void usage();
     
    #define HOST "coding.debuntu.org"
    #define PAGE "/"
    #define PORT 80
    #define USERAGENT "HTMLGET 1.0"
     
    int main(int argc, char **argv)
    {
      struct sockaddr_in *remote;
      int sock;
      int tmpres;
      char *ip;
      char *get;
      char buf[BUFSIZ+1];
      char *host;
      char *page;
     
      if(argc == 1){
        usage();
        exit(2);
      }  
      host = argv[1];
      if(argc > 2){
        page = argv[2];
      }else{
        page = PAGE;
      }
      sock = create_tcp_socket();
      ip = get_ip(host);
      fprintf(stderr, "IP is %s\n", ip);
      remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
      remote->sin_family = AF_INET;
      tmpres = inet_pton(AF_INET, ip, (void *)(&(remote->sin_addr.s_addr)));
      if( tmpres < 0)  
      {
        perror("Can't set remote->sin_addr.s_addr");
        exit(1);
      }else if(tmpres == 0)
      {
        fprintf(stderr, "%s is not a valid IP address\n", ip);
        exit(1);
      }
      remote->sin_port = htons(PORT);
     
      if(connect(sock, (struct sockaddr *)remote, sizeof(struct sockaddr)) < 0){
        perror("Could not connect");
        exit(1);
      }
      get = build_get_query(host, page);
      fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", get);
     
      //Send the query to the server
      int sent = 0;
      while(sent < strlen(get))
      {
        tmpres = send(sock, get+sent, strlen(get)-sent, 0);
        if(tmpres == -1){
          perror("Can't send query");
          exit(1);
        }
        sent += tmpres;
      }
      //now it is time to receive the page
      memset(buf, 0, sizeof(buf));
      int htmlstart = 0;
      char * htmlcontent;
      while((tmpres = recv(sock, buf, BUFSIZ, 0)) > 0){
        if(htmlstart == 0)
        {
          /* Under certain conditions this will not work.
          * If the \r\n\r\n part is splitted into two messages
          * it will fail to detect the beginning of HTML content
          */
          htmlcontent = strstr(buf, "\r\n\r\n");
          if(htmlcontent != NULL){
            htmlstart = 1;
            htmlcontent += 4;
          }
        }else{
          htmlcontent = buf;
        }
        if(htmlstart){
          fprintf(stdout, htmlcontent);
        }
     
        memset(buf, 0, tmpres);
      }
      if(tmpres < 0)
      {
        perror("Error receiving data");
      }
      free(get);
      free(remote);
      free(ip);
      close(sock);
      return 0;
    }
     
    void usage()
    {
      fprintf(stderr, "USAGE: htmlget host [page]\n\
    \thost: the website hostname. ex: coding.debuntu.org\n\
    \tpage: the page to retrieve. ex: index.html, default: /\n");
    }
     
     
    int create_tcp_socket()
    {
      int sock;
      if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        perror("Can't create TCP socket");
        exit(1);
      }
      return sock;
    }
     
     
    char *get_ip(char *host)
    {
      struct hostent *hent;
      int iplen = 15; //XXX.XXX.XXX.XXX
      char *ip = (char *)malloc(iplen+1);
      memset(ip, 0, iplen+1);
      if((hent = gethostbyname(host)) == NULL)
      {
        herror("Can't get IP");
        exit(1);
      }
      if(inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, iplen) == NULL)
      {
        perror("Can't resolve host");
        exit(1);
      }
      return ip;
    }
     
    char *build_get_query(char *host, char *page)
    {
      char *query;
      char *getpage = page;
      char *tpl = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
      if(getpage[0] == '/'){
        getpage = getpage + 1;
        fprintf(stderr,"Removing leading \"/\", converting %s to %s\n", page, getpage);
      }
      // -5 is to consider the %s %s %s in tpl and the ending \0
      query = (char *)malloc(strlen(host)+strlen(getpage)+strlen(USERAGENT)+strlen(tpl)-5);
      sprintf(query, tpl, getpage, host, USERAGENT);
      return query;
    }

    return TRUE;
}  
