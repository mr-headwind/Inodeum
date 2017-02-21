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
#include <libgen.h>  
#include <gtk/gtk.h>  
#include <glib/gbase64.h>
#include <main.h>
#include <isp.h>
#include <defs.h>
#include <version.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>



/* Prototypes */

int ssl_service_details(IspData *, MainUi *);
int get_url(char *, IspData *, MainUi *);
int ssl_service_init(IspData *, MainUi *);
int ssl_isp_connect(IspData *, MainUi *);
int isp_ip(IspData *, MainUi *);
int create_socket(IspData *, MainUi *);
int send_request(char *, IspData *, MainUi *);
char * setup_get(char *, IspData *);
void encode_un_pw(IspData *, MainUi *);
int send_query(char *, IspData *, MainUi *);
int recv_data(IspData *, MainUi *);
int service_list(IspData *, MainUi *);
int bio_send_query(BIO *, char *, MainUi *);
int get_serv_list(BIO *, IspData *, MainUi *);
char * bio_read_xml(BIO *, MainUi *);
char * get_tag(char *, char *, MainUi *);
char * get_tag_attr(char *, char *, char *, MainUi *);
int get_tag_val(char *, char **, MainUi *);
int check_srv(IspServ **);

extern void log_msg(char*, char*, char*, GtkWidget*);


/* Globals */

static const char *debug_hdr = "DEBUG-service.c ";


/* API Webtools service requests */


/* Get all the Service details using a secure connection - each request is discrete */

int ssl_service_details(IspData *isp_data, MainUi *m_ui)
{  
    /* Initial */
    if (ssl_service_init(isp_data, m_ui) == FALSE)
    	return FALSE;

    /* Connection */
    if (ssl_isp_connect(isp_data, m_ui) == FALSE)
    	return FALSE;

    /* User Agent and encoded username/password */
    sprintf(isp_data->user_agent, "%s %s", TITLE, VERSION);
    encode_un_pw(isp_data, m_ui);

    /* 1. Service Listing */
    if (service_list(isp_data, m_ui) == FALSE)
    	return FALSE;

    BIO_reset(isp_data->web);

    /* 2. Service Type - Personal ADSL */

    return TRUE;
}  


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


/* Initialise the ssl and bio setup */

int ssl_service_init(IspData *isp_data, MainUi *m_ui)
{  
    isp_data->ctx = NULL;
    isp_data->web = NULL;
    isp_data->ssl = NULL;

    /* Initialise the ssl and crypto libraries and load required algorithms */
    SSL_library_init();
    SSL_load_error_strings();
    ERR_load_BIO_strings();

    /* Set SSLv2 client hello, also announce SSLv3 and TLSv1 */
    const SSL_METHOD* method = SSLv23_method();		// SSLv23_client_method ?

    if (!(NULL != method))
    {
	log_msg("ERR0012", NULL, "ERR0012", m_ui->window);
    	return FALSE;
    }

    /* Create a new SSL context */
    isp_data->ctx = SSL_CTX_new(method);

    if (!(isp_data->ctx != NULL))
    {
	log_msg("ERR0013", NULL, "ERR0013", m_ui->window);
    	return FALSE;
    }

    /* Options for negotiation */
    const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
    SSL_CTX_set_options(isp_data->ctx, flags);

    /* Certificate chain */
    if (! SSL_CTX_load_verify_locations(isp_data->ctx, NULL, SSL_CERT_PATH))
    {
	log_msg("ERR0014", SSL_CERT_PATH, "ERR0014", m_ui->window);
    	return FALSE;
    }

    return TRUE;
}  


/* Setup the BIO connection and verify */

int ssl_isp_connect(IspData *isp_data, MainUi *m_ui)
{  
    /* New connection */
    if ((isp_data->web = BIO_new_ssl_connect(isp_data->ctx)) == NULL)
    {
	log_msg("ERR0015", NULL, "ERR0015", m_ui->window);
    	return FALSE;
    }

    /* Host and port */
    if (! BIO_set_conn_hostname(isp_data->web, HOST ":" SSL_PORT))
    {
	log_msg("ERR0016", NULL, "ERR0016", m_ui->window);
    	return FALSE;
    }

    /* Connection object */
    BIO_get_ssl(isp_data->web, &(isp_data->ssl));

    if (isp_data->ssl == NULL)
    {
	log_msg("ERR0017", NULL, "ERR0017", m_ui->window);
    	return FALSE;
    }

    /* Remove unwanted ciphers */
    /* PRETTY SURE RC4-MD5 IS OK, but connection still fails if the list below is set 
    const char* const PREFERRED_CIPHERS = "HIGH:RC4:MD5:!aNULL:!kRSA:!PSK:!SRP";

    if (! SSL_set_cipher_list(isp_data->ssl, PREFERRED_CIPHERS))
    {
	log_msg("ERR0018", NULL, "ERR0018", m_ui->window);
    	return FALSE;
    }
    */

    /* Fine tune host if possible */
    if (! SSL_set_tlsext_host_name(isp_data->ssl, HOST))
    {
	log_msg("ERR0019", NULL, "ERR0019", m_ui->window);
    	return FALSE;
    }

    /* Connection and handshake */
    if (BIO_do_connect(isp_data->web) <= 0)
    {
	log_msg("ERR0020", NULL, "ERR0020", m_ui->window);
    	return FALSE;
    }

    /* Verify a server certificate was presented during the negotiation */
    X509* cert = SSL_get_peer_certificate(isp_data->ssl);

    if (cert) 
    {
    	X509_free(cert); 			// Free immediately
    }
    else if (NULL == cert)
    {
	log_msg("ERR0021", NULL, "ERR0021", m_ui->window);
    	return FALSE;
    }

    /* Verify the certificate */
    if (SSL_get_verify_result(isp_data->ssl) != X509_V_OK)
    {
	log_msg("ERR0022", NULL, "ERR0021", m_ui->window);
    	return FALSE;
    }

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


/* ISP service listing */

int service_list(IspData *isp_data, MainUi *m_ui)
{  
    int r;
    char *get_qry;

    r = TRUE;
    sprintf(isp_data->url, "/api/%s/", API_VER);
    
    /* Construct GET */
    get_qry = setup_get(isp_data->url, isp_data);

    /* Send the query */
    bio_send_query(isp_data->web, get_qry, m_ui);
    r = get_serv_list(isp_data->web, isp_data, m_ui);

    /* Clean up */
    free(get_qry);

    return r;
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

    sprintf(query, GET_TPL, url, HOST, isp_data->user_agent, isp_data->enc64, REALM);

    return query;
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


/* Send the query to the server - encrypted */

int bio_send_query(BIO *web, char *get_qry, MainUi *m_ui)
{  
    int r, sent, qlen;
    char s[20];

    sent = 0;
    qlen = strlen(get_qry);

    while(sent < qlen)
    {
	r = BIO_write(web, get_qry + sent, qlen - sent);

	if (r <= 0)
	{
	    log_msg("ERR0010", NULL, "ERR0010", m_ui->window);
	    return FALSE;
	}
	else
	{
	    sent += r;
	}
    }

    //r = BIO_puts(web, get_qry);		// Try this for starters - perhaps BIO_write may better

    /*
    if (r >= 0 && r < strlen(get_qry))
    {
    	sprintf(s, "%d", r);
	log_msg("ERR0023", s, "ERR0023", m_ui->window);
	return FALSE;
    }

    if (r < 0)
    {
	log_msg("ERR0024", NULL, "ERR0024", m_ui->window);
	return FALSE;
    }
    */
    
    return TRUE;
}  


/* Read and Parse xml and set up a list of services */

int get_serv_list(BIO *web, IspData *isp_data, MainUi *m_ui)
{  
    char *xml = NULL;
    char *p;
    char s_val[200];
    int i, srv_cnt, r;
    IspServ *isp_srv;

    /* Read xml */
    xml = bio_read_xml(web, m_ui);

    if (xml == NULL)
    	return FALSE;

    /* Services count */
    if ((p = get_tag(xml, "<services ", m_ui)) == NULL)
    {
    	free(xml);
    	return FALSE;
    }

    if ((p = get_tag_attr(p + 10, "count=\"", s_val, m_ui)) == NULL)
    {
    	free(xml);
    	return FALSE;
    }

    isp_data->srv_cnt = atoi(s_val);
printf("%s srv cnt %d\n", debug_hdr, isp_data->srv_cnt); fflush(stdout);

    if (isp_data->srv_cnt == 0)
    {
	log_msg("ERR0033", NULL, "ERR0033", m_ui->window);
    	return FALSE;
    }

    r = TRUE;

    /* Create a service list */
    for(i = 0; i < isp_data->srv_cnt; i++)
    {
	if ((p = get_tag(p, "<service ", m_ui)) != NULL)
	{
	    isp_srv = (IspServ *) malloc(sizeof(IspServ));
	    memset(isp_srv, 0, sizeof(IspServ));

	    /* Service Type */
	    if ((p = get_tag_attr(p, "type=\"", s_val, m_ui)) != NULL)
	    {
		isp_srv->type = (char *) malloc(strlen(s_val) + 1);
		strcpy(isp_srv->type, s_val);
printf("%s srv type %s\n", debug_hdr, isp_srv->type); fflush(stdout);
	    }

	    /* Service URL */
	    if ((p = get_tag_attr(p, "href=\"", s_val, m_ui)) != NULL)
	    {
		isp_srv->href = (char *) malloc(strlen(s_val) + 1);
		strcpy(isp_srv->href, s_val);
printf("%s srv href %s\n", debug_hdr, isp_srv->href); fflush(stdout);
	    }

	    /* Service Id */
	    get_tag_val(p, &(isp_srv->id), m_ui);
printf("%s srv id %s\n", debug_hdr, isp_srv->id); fflush(stdout);

	    /* List */
	    if (check_srv(&isp_srv) == FALSE)
	    {
		r = FALSE;
		break;
	    }

	    isp_data->srv_list = g_list_append (isp_data->srv_list_head, isp_srv);

	    if (isp_data->srv_list_head == NULL)
		isp_data->srv_list_head = isp_data->srv_list;
	}
	else
	{
	    log_msg("ERR0030", "<service ", "ERR0030", m_ui->window);
	    r = FALSE;
	    break;
	}
    }

    free(xml);
    
    return r;
}  


/* Check the service structure is valid */

int check_srv(IspServ **isp_srv)
{  
    if ((*isp_srv)->type && (*isp_srv)->href && (*isp_srv)->id)
	return TRUE;

    if (&(*isp_srv)->type)
    	free((*isp_srv)->type);
    
    if (&(*isp_srv)->href)
    	free((*isp_srv)->href);
    
    if (&(*isp_srv)->id)
    	free((*isp_srv)->id);

    free(isp_srv);
    
    return FALSE;
}  


/* Read the encrypted output from the server */

char * bio_read_xml(BIO *web, MainUi *m_ui)
{  
    int len = 0, txt_sz;
    char buf[BUFSIZ + 1];
    char *txt, *p;
    GtkTextBuffer *txt_buffer;  		// tmp
    GtkTextIter iter;				// tmp

    /* Initial */
    txt_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (m_ui->txt_view));	// tmp
    txt = NULL;
    p = NULL;
    txt_sz = 0;

    /* Get the return text */
    do
    {
	memset(buf, 0, sizeof(buf));
	len = BIO_read(web, buf, sizeof(buf));
            
	if (len > 0)
	{
	    txt = (char *) realloc(txt, txt_sz + len + 1);
	    p = txt + txt_sz;
	    txt_sz += len;
	    *(txt + txt_sz) = '\0';
	    memcpy((char *) p, (char *) buf, len);
	    	
	    gtk_text_buffer_get_end_iter (txt_buffer, &iter);			// tmp
	    gtk_text_buffer_insert (txt_buffer, &iter, buf, -1);		// tmp
	    gtk_text_iter_forward_to_end (&iter);				// tmp
	}
    } while (len > 0 || BIO_should_retry(web));
    
    return txt;
}  


/* Return a pointer to a tag */

char * get_tag(char *xml, char *tag, MainUi *m_ui)
{  
    char *p;

    if ((p = strstr(xml, tag)) == NULL)
	log_msg("ERR0030", tag, "ERR0030", m_ui->window);

    return p;
}  


/* Return a position pointer and an attibute value */

char * get_tag_attr(char *xml, char *attr, char *s_val, MainUi *m_ui)
{  
    char *p;

    s_val == NULL;

    if ((p = strstr(xml, attr)) != NULL)
    {
    	p += strlen(attr);

    	while(*p != '\"')
	    *s_val++ = *p++;

	*s_val = '\0';
    }
    else
    {
	log_msg("ERR0031", attr, "ERR0031", m_ui->window);
    }

    return p;
}  


/* Determine a tag value */

int get_tag_val(char *xml, char **s, MainUi *m_ui)
{  
    char *p, *p2;

    *s == NULL;

    if ((p = strchr(xml,'>')) == NULL)
    {
	log_msg("ERR0032", NULL, "ERR0032", m_ui->window);
    	return FALSE;
    }

    if ((p2 = strchr(p,'<')) == NULL)
    {
	log_msg("ERR0032", NULL, "ERR0032", m_ui->window);
    	return FALSE;
    }

printf("%s val 1  sz %ld\n", debug_hdr, p2 - p); fflush(stdout);
    *s = (char *) malloc(p2 - p);
printf("%s val 2\n", debug_hdr); fflush(stdout);
    //(*s)[p2 - p] = '\0';
    memset(*s, 0, p2 - p);
printf("%s val 3\n", debug_hdr); fflush(stdout);
    memcpy(&(*s), p + 1, p2 - p - 1);
printf("%s val 4 id %s\n", debug_hdr, *s); fflush(stdout);

    return TRUE;
}  
