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

#define DEFAULT_SRV_TYPE "Personal_ADSL"
#define USAGE "usage"
#define SERVICE "service"
#define HISTORY "history"

//#define API_PROTO "https://"
#define API_VER "v1.5"
#define PORT 80						// 80 = http
#define SSL_PORT "443"					// 443 = https
#define SSL_CERT_PATH "/etc/ssl/certs"				
#define HOST "customer-webtools-api.internode.on.net"
#define VER_HOST "github.com"
#define GIT_OWNER "mr-headwind"
#define REALM "internode-api"
// GET message template is as follows:
//	"GET "\						Method (POST is more secure - below)
//	"%s "\						URI requested
//	"HTTP/1.0\r\n"\					HTTP Version (1.1 - gives error)
//	"Host: %s\r\n"\					Host
//	"User-Agent: %s\r\n"\				User Agent
//	"Authorization: BASIC %s\r\n"\			username:password in base 64
//	"WWW-Authenticate: BASIC realm=\"%s\"\r\n"\	Realm
//	"Accept-Language: en\r\n"\			Language
//	"\r\n"						End of query
#define GET_TPL "POST "\
	        "%s "\
	        "HTTP/1.0\r\n"\
	        "Host: %s\r\n"\
	        "User-Agent: %s\r\n"\
	        "Authorization: BASIC %s\r\n"\
	        "WWW-Authenticate: BASIC realm=\"%s\"\r\n"\
	        "Accept-Language: en\r\n"\
	        "\r\n"

#define PARAM_GET_TPL "POST "\
		      "%s "\
		      "HTTP/1.0\r\n"\
		      "Host: %s\r\n"\
		      "User-Agent: %s\r\n"\
		      "Content-Type: text/plain\r\n"\
		      "Content-Length: %d\r\n"\
		      "Authorization: BASIC %s\r\n"\
		      "WWW-Authenticate: BASIC realm=\"%s\"\r\n"\
		      "Accept-Language: en\r\n"\
		      "\r\n"

		      //"Content-Type: application/x-www-form-urlencoded\r\n"\

// Structure to contain the Service and Service Resources listings.
// Each Service Type will have a number Resources available.
// A typical Service would be 'Personal_ADSL' and the Resources available
// are History, Usage & Service.
// The overall list (and head) for all service types is in _isp_data (below)
// and the resources list will a child list for each service type found.
// At the bottom of the tree (only 2 levels but this format allows flexibility)
// the lists will be NULL and the count zero.

typedef struct _xmllistobj
{
    char *type;
    char *href;
    char *val;

    /* Child list related */
    int cnt;
    GList *sub_list;
    GList *sub_list_head;
} IspListObj;


/* Structure to contain Service Usage Data */

typedef struct _srvusage
{
    char *rollover_dt;				// Next rollover date
    char *plan_interval;			// Period quota valid for
    char *quota;				// Plan quota
    char *metered_bytes;			// Total metered (up/down) - optional
    char *unmetered_bytes;			// Total unmetered (up/down) - optional
    char *total_bytes;				// Total used so far in period
    char *unit;					// Unit measure (bytes)
    char hist_from_dt[11]; 			// History start date (yyyy-mm-dd)
    char hist_to_dt[11];			// History end date (yyyy-mm-dd)
    int last_cat_idx;				// Most recent usage category
    int hist_days;				// Rows in history data array
    long **hist_usg_arr;			// Data (array) for the history chart
    long long hist_tot_arr[5];			// History totals array
} ServUsage;


/* Structure to contain Service Data */

typedef struct _service_plan
{
    char *srv_plan_item[13];
    char quota_units[10];			// Quota units (eg. bytes)
    char plan_cost_units[10];			// Plan Cost units (eg. aud)
    char excess_cost_units[10];			// Excess Cost units (eg. aud)

    /* Plan tags array is equivalent to:
    char *username;				// Username
    char *quota;				// Plan quota
    char *plan;					// Plan name
    char *carrier;				// Carrier (Internode)
    char *speed;				// Speed (eg. 24 Mbits/sec)
    char *usage_rating;				// Rating (eg. down)
    char *rollover;				// Rollover date (yyyy-dd-mm)
    char *excess_cost;				// If applicable
    char *excess_charged;			// Excess charging? (eg. no)
    char *excess_shaped;			// Excess shaping? (eg. yes)
    char *excess_restrict;			// Excess restrict? (eg. no)
    char *plan_interval;			// Interval (eg. Monthly)
    char *plan_cost;				// Cost (eg. 59.99)
    */
} SrvPlan;


/* Structure to contain isp related details, connection fields & results */

typedef struct _isp_data
{
    /* Base detail */
    char *uname;
    char *pw;
    gchar *enc64;
    char user_agent[50];
    char url[500];

    /* Standard and SSL connection */
    char *ip;
    int tcp_sock;
    struct sockaddr_in *isp_addr;
    char *xml_recv;
    SSL_CTX* ctx;
    BIO *web;
    SSL *ssl;

    /* Service Type related */
    char *curr_srv_id;
    int srv_cnt;
    GList *srv_list;
    GList *srv_list_head;
} IspData;
