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
** Description:	ISP details include file
**
** Author:	Anthony Buckley
**
** History
**	10-Jan-2017	Initial
**
*/




/* Defines */

#ifndef ISP_HDR
#define ISP_HDR
#endif


/* Includes */

#include <openssl/bio.h>
#include <openssl/ssl.h>


/* Fixed Info */

//#define API_PROTO "https://"
#define API_VER "1.5"
#define PORT 80							// 80 = http
#define SSL_PORT "443"						// 443 = https
#define SSL_CERT_PATH "/etc/ssl/certs"				
#define HOST "customer-webtools-api.internode.on.net"
#define REALM "internode-api"
// GET message template is as follows:
//		"GET "\						Method
//		"%s "\						URI requested
//		"HTTP/1.0\r\n"\					HTTP Version (1.1 ?)
//		"Host: %s\r\n"\					Host
//		"User-Agent: %s\r\n"\				User Agent
//		"Authorization: BASIC %s\r\n"\			username:password in base 64
//		"WWW-Authenticate: BASIC realm=\"%s\"\r\n"\	Realm
//		"Accept-Language: en\r\n"\			Language
//		"\r\n"						End of query
#define GET_TPL "GET "\
	        "%s "\
	        "HTTP/1.0\r\n"\
	        "Host: %s\r\n"\
	        "User-Agent: %s\r\n"\
	        "Authorization: BASIC %s\r\n"\
	        "WWW-Authenticate: BASIC realm=\"%s\"\r\n"\
	        "Accept-Language: en\r\n"\
	        "\r\n"


/* Structure to contain isp and related details */

typedef struct _isp_data
{
    char username[100];
    char password[100];
    gchar *enc64;
    char user_agent[50];
    char url[500];
    char *ip;
    int tcp_sock;
    struct sockaddr_in *isp_addr;
    char *xml_recv;
    SSL_CTX* ctx;
    BIO *web;
    SSL *ssl;
} IspData;
