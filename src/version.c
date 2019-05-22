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
** Description:	Check latest version 
**
** Author:	Anthony Buckley
**
** History
**	08-May-2019	Initial code
*/


/* Defines */

#define UNIT_MAX 9


/* Includes */

#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <libgen.h>  
#include <time.h>  
#include <gtk/gtk.h>  
#include <glib.h>
//#include <glib/gbase64.h>
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

int setup_version_check(IspData *, MainUi *);
int version_check_init(VersionData *, MainUi *);
int ssl_version_connect(IspData *, MainUi *);
int get_release_file(VersionData *, MainUi *);

char * setup_get(char *, IspData *);
char * setup_get_param(char *, char *, IspData *);
int bio_send_query(BIO *, char *, MainUi *);
char * bio_read_xml(BIO *, MainUi *);
void set_param(int, char *);

extern void log_status_msg(char *, char *, char *, char *, GtkWidget *);
extern int check_http_status(char *, int *, MainUi *);


/* Globals */

static const char *debug_hdr = "DEBUG-version.c ";



/* Go to the application GitHub repo url and determine the latest version */

int setup_version_check(IspData *isp_data, MainUi *m_ui)
{  
    int r;
    VersionData ver;

    /* Initial */
    if (version_check_init(&ver, m_ui) == FALSE)
	return FALSE;

    /* Connection */
    if (ssl_version_connect(&ver, m_ui) == FALSE)
	return FALSE;

    /* User Agent and encoded username/password */
    sprintf(isp_data->user_agent, "%s %s", TITLE, VERSION);

    /* Latest release file */
    if ((r = get_release_file(&ver, m_ui)) != TRUE)
    	return r;

    BIO_free_all(ver.web);
    SSL_CTX_free(ver.ctx);

    log_status_msg("INF0005", "Version check success", "INF0005", "Version check success", m_ui->status_info);
    return TRUE;
}  


/* Initialise the ssl and bio setup */

int version_check_init(VersionData *ver, MainUi *m_ui)
{  
    /* Initialise */
    ver->ctx = NULL;
    ver->web = NULL;
    ver->ssl = NULL;

    /* Initialise the ssl and crypto libraries and load required algorithms */
    SSL_library_init();
    SSL_load_error_strings();
    ERR_load_BIO_strings();

    /* Set SSLv2 client hello, also announce SSLv3 and TLSv1 */
    const SSL_METHOD* method = SSLv23_method();		// SSLv23_client_method ?

    if (!(NULL != method))
    {
	log_status_msg("ERR0012", NULL, "INF0001", "Version check", m_ui->status_info);
    	return FALSE;
    }

    /* Create a new SSL context */
    ver->ctx = SSL_CTX_new(method);

    if (!(ver->ctx != NULL))
    {
	log_status_msg("ERR0013", NULL, "INF0001", "Version check", m_ui->status_info);
    	return FALSE;
    }

    /* Options for negotiation */
    const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
    SSL_CTX_set_options(ver->ctx, flags);

    /* Certificate chain */
    if (! SSL_CTX_load_verify_locations(ver->ctx, NULL, SSL_CERT_PATH))
    {
	log_status_msg("ERR0014", SSL_CERT_PATH, "INF0001", "Version check", m_ui->status_info);
    	return FALSE;
    }

    return TRUE;
}  


/* Setup the BIO connection and verify */

int ssl_version_connect(VersionData *ver, MainUi *m_ui)
{  
    /* New connection */
    if ((ver->web = BIO_new_ssl_connect(ver->ctx)) == NULL)
    {
	log_status_msg("ERR0015", NULL, "INF0001", "Version check", m_ui->status_info);
    	return FALSE;
    }

    /* Host and port */
    if (! BIO_set_conn_hostname(ver->web, VER_HOST ":" SSL_PORT))
    {
	log_status_msg("ERR0016", NULL, "INF0001", "Version check", m_ui->status_info);
    	return FALSE;
    }

    /* Connection object */
    BIO_get_ssl(ver->web, &(ver->ssl));

    if (ver->ssl == NULL)
    {
	log_status_msg("ERR0017", NULL, "INF0001", "Version check", m_ui->status_info);
    	return FALSE;
    }

    SSL_set_mode(ver->ssl, SSL_MODE_AUTO_RETRY);

    /* Fine tune host if possible */
    if (! SSL_set_tlsext_host_name(ver->ssl, VER_HOST))
    {
	log_status_msg("ERR0019", NULL, "INF0001", "Version check", m_ui->status_info);
    	return FALSE;
    }

    /* Connection and handshake */
    log_status_msg("INF0003", NULL, "INF0003", NULL, m_ui->status_info);

    if (BIO_do_connect(ver->web) <= 0)
    {
	log_status_msg("ERR0020", NULL, "INF0001", "Version check", m_ui->status_info);
    	return FALSE;
    }

    log_status_msg("INF0004", NULL, "INF0004", NULL, m_ui->status_info);

    if (BIO_do_handshake(ver->web) <= 0)
    {
	log_status_msg("ERR0020", NULL, "INF0001", "Version check", m_ui->status_info);
    	return FALSE;
    }

    /* Verify a server certificate was presented during the negotiation */
    X509* cert = SSL_get_peer_certificate(ver->ssl);

    if (cert) 
    {
    	X509_free(cert); 			// Free immediately
    }
    else if (NULL == cert)
    {
	log_status_msg("ERR0021", NULL, "INF0001", "Version check", m_ui->status_info);
    	return FALSE;
    }

    /* Verify the certificate */
    if (SSL_get_verify_result(ver->ssl) != X509_V_OK)
    {
	log_status_msg("ERR0022", NULL, "INF0001", "Version check", m_ui->status_info);
    	return FALSE;
    }

    return TRUE;
}  


/* Read the latest version file in the repo releases folder */

int get_release_file(VersionData *ver, MainUi *m_ui)
{  
    int r;
    char *get_qry;

    r = TRUE;
    sprintf(ver->url, "/%s/%s/tree/master/DIST_PACKAGES/latest_version", GIT_OWNER, TITLE);
    log_status_msg("INF0005", NULL, "INF0005", NULL, m_ui->status_info);
    
    /* Construct GET */
    get_qry = setup_ver_get(ver->url, ver);

    /* Send the query */
    bio_send_query(ver->web, get_qry, m_ui);
    r = get_version(ver->web, ver, m_ui);

    /* Clean up */
    free(get_qry);

    return r;
}  


/* Set up the query */

char * setup_ver_get(char *url, VersionData *ver)
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


/* Read and Parse xml and set up a list of services */

int get_serv_list(BIO *web, IspData *isp_data, MainUi *m_ui)
{  
    char *xml = NULL;
    int r, html_code;

    /* Read xml */
    xml = bio_read_xml(web, m_ui);
printf("%s get_serv_list:xml\n%s\n", debug_hdr, xml); fflush(stdout);

    if (xml == NULL)
    	return FALSE;

    r = check_http_status(xml, &html_code, m_ui);

    if (r != TRUE)
    {
    	if (html_code == 401)
	    return -1;
    	else
	    return r;
    }

    /* Services list */
    r = parse_serv_list(xml, isp_data, m_ui);
    free(xml);

    return r;
}


/* Iterate each service type found and get the associated resource listing */

int srv_resource_list(IspData *isp_data, MainUi *m_ui)
{  
    int r;
    char *get_qry;
    IspListObj *isp_srv;

    GList *l;
    r = TRUE;

    for(l = isp_data->srv_list_head; l != NULL; l = l->next)
    {
    	isp_srv = (IspListObj *) l->data;
	sprintf(isp_data->url, "/api/%s/%s/", API_VER, isp_srv->val);
	
	/* Construct GET */
	get_qry = setup_get(isp_data->url, isp_data);

	/* Send the query, then clean up */
	bio_send_query(isp_data->web, get_qry, m_ui);
	free(get_qry);

	r = get_resource_list(isp_data->web, isp_srv, isp_data, m_ui);
	 
	if (r == FALSE)
	    break;
    }

    return r;
}  


/* Read and Parse xml and set up a list of resources for a service type */

int get_resource_list(BIO *web, IspListObj *isp_srv, IspData *isp_data, MainUi *m_ui)
{  
    char *xml = NULL;
    int i, r, html_code;

    /* Read xml */
    xml = bio_read_xml(web, m_ui);
printf("%s get_resource_list:xml\n%s\n", debug_hdr, xml); fflush(stdout);

    if (xml == NULL)
    	return FALSE;

    if (check_http_status(xml, &html_code, m_ui) == FALSE)
    	return FALSE;

    /* Resources list */
    r = parse_resource_list(xml, isp_srv, isp_data, m_ui);
    free(xml);
    
    return r;
}  


/* Get the current usage and details for the default service */

int get_default_service(IspData *isp_data, MainUi *m_ui)
{  
    int r;
    IspListObj *srv_type, *rsrc;
    GList *l;

    /* Determine the appropriate default */
    if ((srv_type = default_srv_type(isp_data, m_ui)) == NULL)
    	return FALSE;

    /* Get the current Usage */
    r = TRUE;
    isp_data->curr_srv_id = srv_type->val;

    for(l = g_list_last(srv_type->sub_list_head); l != NULL; l = l->prev)
    {
    	rsrc = (IspListObj *) l->data;
    	
    	if (strcmp(rsrc->type, USAGE) == 0)
    	{
	    r = get_usage(rsrc, isp_data, m_ui);
	    BIO_reset(isp_data->web);
	}
	else if (strcmp(rsrc->type, SERVICE) == 0)
    	{
	    r = get_service(rsrc, isp_data, m_ui);
	    BIO_reset(isp_data->web);
	}
	else if (strcmp(rsrc->type, HISTORY) == 0)
    	{
	    r = get_history(rsrc, 2, isp_data, m_ui);
	    BIO_reset(isp_data->web);
	}

	if (r == FALSE)
	    break;
    }

    return r;
}  


/* Get a specific resource for the default service */

IspListObj * get_resource(char *rsrc_type, IspData *isp_data, MainUi *m_ui)
{  
    IspListObj *srv_type, *rsrc;
    GList *l;

    /* Determine the appropriate default */
    if ((srv_type = default_srv_type(isp_data, m_ui)) == NULL)
    	return FALSE;

    /* Find resource */
    for(l = g_list_last(srv_type->sub_list_head); l != NULL; l = l->prev)
    {
    	rsrc = (IspListObj *) l->data;
    	
	if (strcmp(rsrc->type, rsrc_type) == 0)
	    return rsrc;
    }

    return NULL;
}  


/* Get the current period usage */

int get_usage(IspListObj *rsrc, IspData *isp_data, MainUi *m_ui)
{  
    int r, html_code;
    char *get_qry;
    char *xml = NULL;
    
    r = TRUE;

    sprintf(isp_data->url, "/api/%s/%s/%s/", API_VER, isp_data->curr_srv_id, rsrc->type);
// ******* either verbose is wrong here - does nothing as it is here - INVESTIGATE!!!
    /* Construct GET */
    //get_qry = setup_get_param(isp_data->url, "verbose=1", isp_data);
    get_qry = setup_get(isp_data->url, isp_data);
printf("%s get_usage:query\n%s\n", debug_hdr, get_qry); fflush(stdout);

    /* Send the query and read xml result */
    bio_send_query(isp_data->web, get_qry, m_ui);
    free(get_qry);

    xml = bio_read_xml(isp_data->web, m_ui);
printf("%s get_usage:xml\n%s\n", debug_hdr, xml); fflush(stdout);

    if (xml == NULL)
    	return FALSE;

    if (check_http_status(xml, &html_code, m_ui) == FALSE)
    	return FALSE;

    /* Save the current usage data */
    r = load_usage(xml, isp_data, m_ui);

    return r;
}


/* Get the service details */

int get_service(IspListObj *rsrc, IspData *isp_data, MainUi *m_ui)
{  
    int r, html_code;
    char *get_qry;
    char *xml = NULL;
    
    r = TRUE;

    sprintf(isp_data->url, "/api/%s/%s/%s/", API_VER, isp_data->curr_srv_id, rsrc->type);
	
    /* Construct GET */
    get_qry = setup_get(isp_data->url, isp_data);
printf("%s get_service:query\n%s\n", debug_hdr, get_qry); fflush(stdout);

    /* Send the query and read xml result */
    bio_send_query(isp_data->web, get_qry, m_ui);
    free(get_qry);

    xml = bio_read_xml(isp_data->web, m_ui);
printf("%s get_service:xml\n%s\n", debug_hdr, xml); fflush(stdout);

    if (check_http_status(xml, &html_code, m_ui) == FALSE)
    	return FALSE;

    if (xml == NULL)
    	return FALSE;

    /* Save the current service data */
    r = load_service(xml, isp_data, m_ui);

    return r;
}


/* Get the usage day history details as per a parameter type */

int get_history(IspListObj *rsrc, int param_type, IspData *isp_data, MainUi *m_ui)
{  
    int r, html_code;
    char s_param[60];
    char *get_qry;
    char *xml = NULL;
    
    r = TRUE;

    sprintf(isp_data->url, "/api/%s/%s/%s/", API_VER, isp_data->curr_srv_id, rsrc->type);
	
    /* Build an appropriate parameter string */
    set_param(param_type, s_param);

    /* Construct GET */
    get_qry = setup_get_param(isp_data->url, s_param, isp_data);
printf("%s get_history:query\n%s\n", debug_hdr, get_qry); fflush(stdout);

    /* Send the query and read xml result */
    bio_send_query(isp_data->web, get_qry, m_ui);
    free(get_qry);

    xml = bio_read_xml(isp_data->web, m_ui);
//printf("%s get_history:xml\n%s\n", debug_hdr, xml); fflush(stdout);

    if (xml == NULL)
    	return FALSE;

    if (check_http_status(xml, &html_code, m_ui) == FALSE)
    	return FALSE;

    /* Save a list of the usage data days */
    r = load_usage_hist(xml, isp_data, m_ui);

    return r;
}


/* Get the usage day history details for a requested date range */

int get_hist_service_usage(IspData *isp_data, MainUi *m_ui)
{  
    int r;
    IspListObj *rsrc;

    /* Set up a connection to retrieve history */
    if (ssl_service_init(isp_data, m_ui) == FALSE)
	return FALSE;

    if (ssl_isp_connect(isp_data, m_ui) == FALSE)
	return FALSE;

    /* Set up History resource and get */
    rsrc = get_resource(HISTORY, isp_data, m_ui);
    r = get_history(rsrc, 3, isp_data, m_ui);

    return r;
}


/* Encode the username and password in base64 */

void encode_un_pw(IspData *isp_data, MainUi *m_ui)
{ 
    gchar *unpw_b64;
    int len;
    char *tmp;

    len = strlen(isp_data->uname) + strlen(isp_data->pw);
    tmp = (char *) malloc(len + 2);
    sprintf(tmp, "%s:%s", isp_data->uname, isp_data->pw);

    isp_data->enc64 = g_base64_encode ((const guchar *) tmp, len + 1);

    free(tmp);

    return;
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


/* Set up the query and parameters */

char * setup_get_param(char *url, char *param, IspData *isp_data)
{  
    int param_len;
    char *query;

    param_len = strlen(param);

    query = (char *) malloc(strlen(url) +
			    strlen(HOST) +
			    strlen(isp_data->user_agent) +
			    param_len +
			    strlen(isp_data->enc64) +
			    strlen(REALM) +
			    strlen(PARAM_GET_TPL) - 8);	// Note 8 accounts for 4 x %s, %d and \0 in template

    sprintf(query, PARAM_GET_TPL, url, HOST, isp_data->user_agent, param_len, isp_data->enc64, REALM);
    strcat(query, param);

    return query;
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
	    log_status_msg("ERR0010", NULL, "INF0002", retry_txt, m_ui->status_info);
	    return FALSE;
	}
	else
	{
	    sent += r;
	}
    }

    return TRUE;
}  


/* Read the encrypted output from the server */

char * bio_read_xml(BIO *web, MainUi *m_ui)
{  
    int len = 0, txt_sz;
    char buf[BUFSIZ + 1];
    char *txt, *p;
    //GtkTextBuffer *txt_buffer;  		// Debug
    //GtkTextIter iter;				// Debug

    /* Initial */
    //txt_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (m_ui->txt_view));	// Debug
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
	    	
	    //gtk_text_buffer_get_end_iter (txt_buffer, &iter);			// Debug
	    //gtk_text_buffer_insert (txt_buffer, &iter, buf, -1);		// Debug
	    //gtk_text_iter_forward_to_end (&iter);				// Debug
	}
    } while(len > 0 || BIO_should_retry(web));
    
    return txt;
}  


/* Set up an appropriate parameter string */

void set_param(int param_type, char *s_param)
{  
    time_t current_tm;
    struct tm *tm;
    struct tm p_tm;
    size_t sz;
    char s[20], s_dt[20];
    char *dt;
    ServUsage *srv_usg;

    *s_param = '\0';
    current_tm = time(NULL);
    tm = localtime(&current_tm);
    sz = strftime(s_dt, 11, "%Y-%m-%d", tm);
    srv_usg = get_service_usage();

    switch(param_type)
    {
    	case 1:						// Total all for month to date
	    sz = strftime(s, 11, "%Y-%m-01", tm);
	    sprintf(s_param, "start=%s&stop=%s&verbose=1", s, s_dt);

	    strcpy(srv_usg->hist_from_dt, s);
	    strcpy(srv_usg->hist_to_dt, s_dt);

	    break;

    	case 2:						// Total all for period to date
	    dt = next_rollover_dt();			// Next Rollover date
	    string2tm(dt, &p_tm);

	    date_tm_add(&p_tm, "Month", -1);
	    sz = strftime(s, 11, "%Y-%m-%d", &p_tm);
	    sprintf(s_param, "start=%s&stop=%s&verbose=1", s, s_dt);

	    strcpy(srv_usg->hist_from_dt, s);
	    strcpy(srv_usg->hist_to_dt, s_dt);

	    break;

    	case 3:						// History for a specified period
	    sprintf(s_param, "start=%s&stop=%s&verbose=1", srv_usg->hist_from_dt, srv_usg->hist_to_dt);
	    break;

    	default:
	    break;
    }

    return;
}  
