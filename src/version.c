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

int version_req_chk(IspData *, MainUi *);
int setup_version_check(IspData *, MainUi *);
int version_check_init(VersionData *, MainUi *);
int ssl_version_connect(VersionData *, MainUi *);
int get_release_file(VersionData *, IspData *, MainUi *);
int get_version(BIO *, VersionData *, MainUi *);
char * setup_ver_get(char *, VersionData *, IspData *);

extern int bio_send_query(BIO *, char *, MainUi *);
extern char * bio_read_xml(BIO *, MainUi *);
extern void log_status_msg(char *, char *, char *, char *, GtkWidget *);
extern int check_http_status(char *, int *, MainUi *);


/* Globals */

static const char *debug_hdr = "DEBUG-version.c ";



/* Check if already checked for a new version */

int version_req_chk(IspData *isp_data, MainUi *m_ui)
{
    if (m_ui->ver_chk_flg == FALSE)
    	setup_version_check(isp_data, m_ui);

    return m_ui->ver_chk_flg;
}  


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
    if ((r = get_release_file(&ver, isp_data, m_ui)) != TRUE)
    	return r;

    BIO_free_all(ver.web);
    SSL_CTX_free(ver.ctx);
    m_ui->ver_chk_flg = TRUE;

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

int get_release_file(VersionData *ver, IspData *isp_data, MainUi *m_ui)
{  
    int r;
    char *get_qry;

    r = TRUE;
    sprintf(ver->url, "/%s/%s/tree/master/DIST_PACKAGES/latest_version", GIT_OWNER, TITLE);
    log_status_msg("INF0005", NULL, "INF0005", NULL, m_ui->status_info);
    
    /* Construct GET */
    get_qry = setup_ver_get(ver->url, ver, isp_data);

    /* Send the query */
    bio_send_query(ver->web, get_qry, m_ui);
    r = get_version(ver->web, ver, m_ui);

    /* Clean up */
    free(get_qry);

    return r;
}  


/* Set up the query */

char * setup_ver_get(char *url, VersionData *ver, IspData *isp_data)
{  
    char *query;

    query = (char *) malloc(strlen(url) +
			    strlen(VER_HOST) +
			    strlen(isp_data->user_agent) +
			    strlen(GET_TPL) - 6);	// Note 6 accounts for 3 x %s in template plus \0

    sprintf(query, GET_VER_TPL, url, VER_HOST, isp_data->user_agent);

    return query;
}


/* Read and Parse xml and find current version */

int get_version(BIO *web, VersionData *ver, MainUi *m_ui)
{  
    char *xml = NULL;
    int r, html_code;

    /* Read xml */
    xml = bio_read_xml(web, m_ui);
printf("%s get_version:xml\n%s\n", debug_hdr, xml); fflush(stdout);

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
    //r = parse_serv_list(xml, isp_data, m_ui);
    free(xml);

    return r;
}
