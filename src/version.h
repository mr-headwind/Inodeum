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
** Description:	Keep latest Appication version number
**
** Author:	Anthony Buckley
**
** History
**	10-Jan-2017	Initial
**	15-Jun-2020	Need to set daylight saving flag in function strdt2tmt (date_util.c)
**	05-Jul-2020	Update code to use openssl 1.1 (ssl_socket.c, version.c)
**	01-Nov-2020	Changes for Gnome update (libgnome) keyring to (lib) secret
**
*/


//#define VERSION "0.53"    ** debug only
#define VERSION "0.64"
#define VERSION_DATE "01-Nov-2020"
#define LATEST_VERSION "Inodeum version: "


/* Includes */

#include <openssl/bio.h>
#include <openssl/ssl.h>


/* Fixed info */

#define GET_VER_TPL "GET "\
		    "%s "\
		    "HTTP/1.0\r\n"\
		    "Host: %s\r\n"\
		    "User-Agent: %s\r\n"\
		    "Accept-Language: en\r\n"\
		    "\r\n"


/* Structure to contain version related details, connection fields & results */

typedef struct _ver_data
{
    /* Base detail */
    char url[500];

    /* Standard and SSL connection */
    char *ip;
    int tcp_sock;
    struct sockaddr_in *isp_addr;
    char *xml_recv;
    SSL_CTX *ctx;
    BIO *web;
    SSL *ssl;
} VersionData;
