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
** Description:	Module ISP service and data usage functions
**
** Author:	Anthony Buckley
**
** History
**	12-Jan-2017	Initial code
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
#include <main.h>
#include <isp.h>
#include <defs.h>
#include <version.h>



/* Prototypes */

void serv_plan_panel(MainUi *);
void serv_plan_details(int, MainUi *);
IspListObj * default_srv_type(IspData *, MainUi *);
int parse_serv_list(char *, IspData *, MainUi *);
int parse_resource_list(char *, IspListObj *, IspData *, MainUi *);
int load_usage(char *, IspData *, MainUi *);
int total_usage(char *, ServUsage *, MainUi *);
int load_service(char *, IspData *, MainUi *);
int load_usage_hist(char *, IspData *, MainUi *);
char * get_list_count(char *, char *, int *, MainUi *);
int process_list_item(char *, IspListObj **, MainUi *);
int check_listobj(IspListObj **);
IspListObj * search_list(char *, GList *);
char * get_tag(char *, char *, int, MainUi *);
char * get_next_tag(char *, char **, MainUi *);
char * get_named_tag_attr(char *, char *, char **, MainUi *);
char * get_next_tag_attr(char *, char **, char **, MainUi *);
char * get_tag_attr(char *, char **, char **, MainUi *);
int get_tag_val(char *, char **, MainUi *);
char * next_rollover_dt();
void clean_up(IspData *);
void free_srv_list(gpointer);
int check_http_status(char *, int *, MainUi *);
char * resp_status_desc(char *, MainUi *);
ServUsage * get_service_usage();
void set_service_retry_txt(MainUi *, char *);

extern void log_msg(char*, char*, char*, GtkWidget*);
extern void log_status_msg(char *, char *, char *, char *, GtkWidget *);
extern void app_msg(char*, char*, GtkWidget*);
extern int get_user_pref(char *, char **);
extern time_t string2tm(char *, struct tm *);
extern double difftime_days(time_t, time_t);
extern void create_label(GtkWidget **, char *, char *, GtkWidget *, int, int, int, int);
extern void set_sz_abbrev(char *);
extern GtkWidget * find_widget_by_data(GtkWidget *, char *, const gchar *, char *);


/* Globals */

static const char *debug_hdr = "DEBUG-service.c ";
static char retry_txt[RETRY_SZ];
static ServUsage srv_usage;
static SrvPlan srv_plan;



/* Servcie Plan details display panel */

void serv_plan_panel(MainUi *m_ui)
{  
    /* Main panel container */
    m_ui->srv_cntr = gtk_frame_new("Service Plan");
    gtk_widget_set_name(m_ui->srv_cntr, "service_panel");
    gtk_widget_set_margin_top (m_ui->srv_cntr, 15);
    gtk_widget_set_margin_bottom (m_ui->srv_cntr, 15);
    gtk_widget_set_margin_left (m_ui->srv_cntr, 10);
    gtk_widget_set_margin_right (m_ui->srv_cntr, 10);

    /* Add the Plan Items grid, but populate it when the plan details have been requested */
    m_ui->plan_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER (m_ui->srv_cntr), m_ui->plan_grid);

    /* Add to the panel stack */
    gtk_stack_add_named (GTK_STACK (m_ui->panel_stk), m_ui->srv_cntr, "service_panel");

    return;
}


/* Servcie Plan information */

void serv_plan_details(int init, MainUi *m_ui)
{  
    int i, unit_free;
    char *s;
    GtkWidget *item_lbl, *item_val;
    const char *item_names[] = { "Username:", "Plan Quota:", "Plan:", "Carrier:", "Speed:", "Usage Rating:", 
    				 "Rollover:", "Excess Cost:", "Excess Charging:", "Excess Shaping:", 
    				 "Excess Restrict:", "Plan Interval:", "Cost:" }; 
    const int item_cnt = 13;

    /* Display each service plan item found */
    for(i = 0; i < item_cnt; i++)
    {
	if (srv_plan.srv_plan_item[i] == NULL)
	    continue;

	/* Some items have 'units' descriptor */
	switch(i)
	{
	    case 1:		// Quota
		if (strcmp(srv_plan.quota_units, "bytes") == 0)
		{
		    set_sz_abbrev(srv_plan.srv_plan_item[i]);
		    s = srv_plan.srv_plan_item[i];
		    unit_free = FALSE;
		}
		else
		{
		    s = (char *) malloc(strlen(srv_plan.srv_plan_item[i]) + strlen(srv_plan.quota_units) + 4);
		    sprintf(s, "%s (%s)", srv_plan.srv_plan_item[i], srv_plan.quota_units);
		    unit_free = TRUE;
		}

		break;

	    case 7: 		// Excess Costs
		s = (char *) malloc(strlen(srv_plan.srv_plan_item[i]) + strlen(srv_plan.excess_cost_units) + 4);
		sprintf(s, "%s (%s)", srv_plan.srv_plan_item[i], srv_plan.excess_cost_units);
		unit_free = TRUE;
		break;

	    case 12:		// Plan Cost
		s = (char *) malloc(strlen(srv_plan.srv_plan_item[i]) + strlen(srv_plan.plan_cost_units) + 4);
		sprintf(s, "%s (%s)", srv_plan.srv_plan_item[i], srv_plan.plan_cost_units);
		unit_free = TRUE;
		break;

	    default:
		s = srv_plan.srv_plan_item[i];
		unit_free = FALSE;
		break;
	}
	
	/* First time requires widgets to be created and populated */
	if (init == TRUE)
	{
	    /* Create an entry for each service plan item found */
	    create_label(&(item_lbl), "lbl", (char *) item_names[i], m_ui->plan_grid, 0, i, 1, 1);
	    create_label(&(item_val), "data_1", s, m_ui->plan_grid, 1, i, 1, 1);
	    g_object_set_data_full (G_OBJECT (item_val), 
				    "lbl_name", 
				    g_strdup (item_names[i]), 
				    (GDestroyNotify) g_free);
	    gtk_widget_set_margin_left (item_lbl, 5);
	    gtk_widget_set_margin_left (item_val, 5);
	}
	else
	{
	    // Re-populate plan details - potentially a problem here should the plan change with
	    // new or fewer items (rare) and messy to make perfect. A restart is probably the easiest
	    // and simplest fix.
	    item_val = find_widget_by_data(m_ui->plan_grid, "data_1", "lbl_name", (char *) item_names[i]);

	    if (item_val != NULL)
	    	gtk_label_set_text (GTK_LABEL (item_val), s);
	    else
	    	app_msg("MSG0005", NULL, m_ui->window);
	}

	if (unit_free == TRUE)
	    free(s);
    }

    return;
}


// Set default order - User sets a default
//		     - User has only a single service type
//		     - If 'Personal_ADSL' is present, use it
//		     - Pick the first in the list

IspListObj * default_srv_type(IspData *isp_data, MainUi *m_ui)
{  
    char *p;
    IspListObj *srv_type;
    GList *l;

    get_user_pref(DEFAULT_SRV_TYPE, &p);

    if (p != NULL)
    {
    	if ((srv_type = search_list(p, isp_data->srv_list_head)) == NULL)
    	{
	    log_status_msg("ERR0034", p, "INF0006", retry_txt, m_ui->status_info);
	    return NULL;
	}
    }
    else if ((srv_type = search_list(DEFAULT_SRV_TYPE, isp_data->srv_list_head)) == NULL)
    {
    	l = isp_data->srv_list_head;
    	srv_type = (IspListObj *) l->data;
    }

    return srv_type;
}  


/* Read and Parse xml and set up a list of services */

int parse_serv_list(char *xml, IspData *isp_data, MainUi *m_ui)
{  
    char *p;
    int i, r;
    IspListObj *isp_srv;

    /* Clean up any previous list */
    if (isp_data->srv_list_head != NULL)
    {
	g_list_free_full(isp_data->srv_list_head, (GDestroyNotify) free_srv_list);
	isp_data->srv_list_head = NULL;
	isp_data->srv_list = NULL;
    }

    /* Services count */
    if ((p = get_list_count(xml, "services", &(isp_data)->srv_cnt, m_ui)) == NULL)
    	return FALSE;

    r = TRUE;

    /* Create a service list */
    for(i = 0; i < isp_data->srv_cnt; i++)
    {
	if ((p = get_tag(p, "service", TRUE, m_ui)) != NULL)
	{
	    isp_srv = (IspListObj *) malloc(sizeof(IspListObj));
	    memset(isp_srv, 0, sizeof(IspListObj));

	    if ((r = process_list_item(p, &isp_srv, m_ui)) == FALSE)
	    	break;

	    isp_data->srv_list = g_list_append (isp_data->srv_list_head, isp_srv);

	    if (isp_data->srv_list_head == NULL)
		isp_data->srv_list_head = isp_data->srv_list;
	}
	else
	{
	    log_status_msg("ERR0030", "<service ", "INF0007", retry_txt, m_ui->status_info);
	    r = FALSE;
	    break;
	}
    }

    return r;
}  


/* Read and Parse xml and set up a list of resources for a service type */

int parse_resource_list(char *xml, IspListObj *isp_srv, IspData *isp_data, MainUi *m_ui)
{  
    char *p;
    int i, r;
    IspListObj *rsrc;

    /* Resources count */
    if ((p = get_list_count(xml, "resources", &(isp_srv)->cnt, m_ui)) == NULL)
    	return FALSE;

    r = TRUE;

    /* Create a resource list */
    for(i = 0; i < isp_srv->cnt; i++)
    {
	if ((p = get_tag(p, "resource", TRUE, m_ui)) != NULL)
	{
	    rsrc = (IspListObj *) malloc(sizeof(IspListObj));
	    memset(rsrc, 0, sizeof(IspListObj));
	    p += 9;

	    if ((r = process_list_item(p, &rsrc, m_ui)) == FALSE)
	    	break;

	    isp_srv->sub_list = g_list_append (isp_srv->sub_list_head, rsrc);

	    if (isp_srv->sub_list_head == NULL)
		isp_srv->sub_list_head = isp_srv->sub_list;
	}
	else
	{
	    log_status_msg("ERR0030", "resource ", "INF0007", retry_txt, m_ui->status_info);
	    r = FALSE;
	    break;
	}
    }

    return r;
}  


/* Save the current usage data */

int load_usage(char *xml, IspData *isp_data, MainUi *m_ui)
{  
    int r, err;
    char *p, *val;

    err = TRUE;
    r = TRUE;
    p = xml;
    memset(&srv_usage, 0, sizeof(ServUsage));

    while((p = get_tag(p, "traffic", err, m_ui)) != NULL)
    {
	p += 8;
	err = FALSE;

	if ((p = get_named_tag_attr(p, "name", &val, m_ui)) != NULL)
	{
	    if (strcmp(val, "metered") == 0)
	    {
		get_tag_val(p, &(srv_usage.metered_bytes), m_ui);
		free(val);
		continue;
	    }
	    else if (strcmp(val, "unmetered") == 0)
	    {
		get_tag_val(p, &(srv_usage.unmetered_bytes), m_ui);
		free(val);
		continue;
	    }
	    else if (strcmp(val, "total") == 0)
	    {
		r = total_usage(p, &srv_usage, m_ui);
		free(val);

		if (r == TRUE)
		    continue;
		else
		    break;
	    }
	}
    }

    if (p == NULL && err == TRUE)		// No data
    	r = FALSE;

    return r;
}  


/* Save the total usage details */

int total_usage(char *xml, ServUsage *usg, MainUi *m_ui)
{  
    int i, r, len;
    char *p, *val;
    const char *tag_arr[] = {"rollover", "plan-interval", "quota", "unit"};
    const int tag_cnt = 4;

    /* Setup */
    r = TRUE;

    /* Get all the tag attributes */
    for(i = 0, p = xml; i < tag_cnt; i++)
    {
	if ((p = get_named_tag_attr(p, (char *) tag_arr[i], &val, m_ui)) == NULL)
	{
	    r = FALSE;
	    break;
	}

	len = strlen(val) + 1;

	switch(i)
	{
	    case 0:
		usg->rollover_dt = (char *) malloc(len);
	    	strcpy(usg->rollover_dt, val);
	    	free(val);
	    	break;
	    case 1:
		usg->plan_interval = (char *) malloc(len);
	    	strcpy(usg->plan_interval, val);
	    	free(val);
	    	break;
	    case 2:
		usg->quota = (char *) malloc(len);
	    	strcpy(usg->quota, val);
	    	free(val);
	    	break;
	    case 3:
		usg->unit = (char *) malloc(len);
	    	strcpy(usg->unit, val);
	    	free(val);
	    	break;
	    default:
		log_status_msg("ERR0035", val, "INF0007", retry_txt, m_ui->status_info);
		r = FALSE;
	    	free(val);
	    	break;
	}
    }

    /* Flag a warning if not all are found */
    if (i != tag_cnt)
    {
	log_status_msg("ERR0036", NULL, "INF0007", retry_txt, m_ui->status_info);
	r = FALSE;
    }

    /* Get the actual value */
    get_tag_val(p, &(usg->total_bytes), m_ui);

/* Test debug
printf("%s\nTotal Usage: %s %s %s %s %s %s %s\n\n", debug_hdr, usg->rollover_dt, usg->plan_interval,
						    usg->quota, usg->unit, usg->metered_bytes,
						    usg->unmetered_bytes, usg->total_bytes); fflush(stdout);
*/

    return r;
}  


/* Save the current usage data */

int load_service(char *xml, IspData *isp_data, MainUi *m_ui)
{  
    int i, r;
    char *p, *tag, *val, *units;
    char msg[20];
    const char *tag_arr[] = {"username", "quota", "plan", "carrier", "speed", "usage-rating",
    			     "rollover", "excess-cost", "excess-charged", "excess-shaped", 
    			     "excess-restrict-access", "plan-interval", "plan-cost"};
    const int tag_cnt = 13;

    r = TRUE;
    p = xml;
    memset(&srv_plan, 0, sizeof(SrvPlan));

    // It appears that some tags may not be present depending on the plan
    // so just search thru the xml and get whatever is present
    while(p != NULL)
    {
    	/* Find any tag */
    	if ((p = get_next_tag(p, &tag, m_ui)) == NULL)
	    continue;

	p += strlen(tag) + 1;

	/* Try to match with one we want */
	for(i = 0; i < tag_cnt; i++)
	{
	    if (strcmp(tag, tag_arr[i]) == 0)	    
	    	break;
	}

	/* No match */
	if (i >= tag_cnt)
	{
	    free(tag);
	    continue;
	}

	/* Some tags have 'units' attribute */
	switch(i)
	{
	    case 1:		// Quota
		units = srv_plan.quota_units;
		strcpy(msg, "Quota units");
		break;

	    case 7:		// Excess Cost
		units = srv_plan.excess_cost_units;
		strcpy(msg, "Excess Cost units");
		break;

	    case 12:		// Plan Cost
		units = srv_plan.plan_cost_units;
		strcpy(msg, "Plan Cost units");
		break;

	    default:
		units = NULL;
		break;
	}

	if (units != NULL)
	{
	    if ((p = get_named_tag_attr(p, "units", &val, m_ui)) == NULL)
	    {
		log_status_msg("ERR0031", msg, "INF0007", retry_txt, m_ui->status_info);
		r = FALSE;
	    }
	    else
	    {
		strncpy(units, val, UNIT_MAX);
		free(val);
	    }
	}

	/* Get the tag value */
	get_tag_val(p, &(srv_plan.srv_plan_item[i]), m_ui);
	free(tag);
    }

/* Test debug
printf("%s\nService Plan \n", debug_hdr); fflush(stdout);
for(i = 0; i < tag_cnt; i++)
{
printf("%s: %s\n", tag_arr[i], srv_plan.srv_plan_item[i]); fflush(stdout);
}
printf("Quota units: %s Plan Cost units: %s Excess Cost units: %s\n\n", 
		srv_plan.quota_units, srv_plan.plan_cost_units, srv_plan.excess_cost_units); 
fflush(stdout);
*/

    return r;
}  


/* Keep a list of the history usage days
**
** Build a dynamic array for the history period thus:
** 
**           total  metered up  metered down  un-metered up  un-metered down  
**  Day 0
**  Day 1
**  Day 2
**  ...
*/

int load_usage_hist(char *xml, IspData *isp_data, MainUi *m_ui)
{  
    int i, j, hday, dir, cat, idx, r;
    long days, total;
    char *p, *attr, *tag, *val;
    struct tm tm_fr, tm_to, tm_tmp;
    time_t tmt_fr, tmt_to, tmt_tmp;
    const int max_traffic_attr = 3;		// direction, name & unit

    const int traffic[3][3] = { {0, 0, 0},		// total met'd unmet'd
    				{0, 1, 3},		// up
    				{0, 2, 4} };		// down
    
    /* Clear history if necessary */
    for(i = 0; i < 5; i++)
    	srv_usage.hist_tot_arr[i] = 0;

    for(i = 0; i < srv_usage.hist_days; i++)
    	free(srv_usage.hist_usg_arr[i]);

    if (srv_usage.hist_days > 0)
	free(srv_usage.hist_usg_arr);

    srv_usage.hist_usg_arr = NULL;
    srv_usage.last_cat_idx = 0;
    hday = 0;

    /* Determine the size of the array, round days up and include day 0 (+2) */
    tmt_fr = string2tm(srv_usage.hist_from_dt, &tm_fr);
    tmt_to = string2tm(srv_usage.hist_to_dt, &tm_to);

    days = (long) difftime_days(tmt_to, tmt_fr);
    days += 2;		
    srv_usage.hist_days = days;

    //long (*arr)[days] = malloc(sizeof(long[days][5]));	// Elegant but problematic to point to
    //memset(arr, 0, sizeof(long[days][5]));

    srv_usage.hist_usg_arr = malloc(days * sizeof(long));

    for(i = 0; i < days; i++)
    {
    	srv_usage.hist_usg_arr[i] = malloc(5 * sizeof(long));
	memset(srv_usage.hist_usg_arr[i], 0, 5 * sizeof(long));
    }

    /* Process all the '<usage tags' */
    r = TRUE;
    p = xml;

    while(p != NULL)
    {
	/* New usage day */
	if ((p = get_tag(p, "usage", FALSE, m_ui)) == NULL)
	    break;

	p += 6;

	/* Date */
	if ((p = get_named_tag_attr(p, "day", &val, m_ui)) == NULL)
	{
	    r = FALSE;
	    log_status_msg("ERR0031", "day", "INF0007", retry_txt, m_ui->status_info);
	    break;
	}

	/* Dates with no usage are not returned, so array index must be determined */
	tmt_tmp = string2tm(val, &tm_tmp);
	idx = (int) difftime_days(tmt_tmp, tmt_fr);
	hday = idx + 1;
	free(val);

    	/* Process the traffic tags (metered, unmetered, up, down) */
    	while((p = get_next_tag(p, &tag, m_ui)) != NULL)
	{
	    if (strcmp(tag, "traffic") != 0)
	    {
	    	free(tag);
	    	break;
	    }

	    dir = 0;

	    for(i = 0; i < max_traffic_attr; i++)
	    {
		if ((p = get_next_tag_attr(p, &attr, &val, m_ui)) == NULL)
		    break;

		if (strcmp(attr, "direction") == 0)
		{
		    /* Direction is 'up' or 'down' */
		    if (strcmp(val, "up") == 0)
			dir = 1;
		    else if (strcmp(val, "down") == 0)
		    	dir = 2;
		}
		else if (strcmp(attr, "name") == 0)
		{
		    /* Traffic name is 'metered' or 'unmetered' or 'total' */
		    if (strcmp(val, "metered") == 0)
		    {
			cat = 1;
		    }
		    else if (strcmp(val, "total") == 0)
		    {
			cat = 0;
			i++;
		    }
		    else 
		    {
			cat = 2;
		    }
		}
		else if (strcmp(attr, "unit") == 0)
		{
		    /* Unit of measurement */
		    if (strcmp(val, srv_usage.unit) != 0)
			log_msg("MSG0004", val, NULL, NULL);
		}

		free(attr);
		free(val);
	    }

	    /* Amount of data */
	    get_tag_val(p, &val, m_ui);
	    idx = traffic[dir][cat];
	    srv_usage.hist_usg_arr[hday][idx] = atol(val);
	    free(val);

	    /* Add to column total */
	    srv_usage.hist_tot_arr[idx] += srv_usage.hist_usg_arr[hday][idx];
	}
    }


/* Test debug
for(i = 0; i < days; i++)
{
    for(j = 0; j < 5; j++)
    {
    printf(" arr[%d][%d] =%ld  ", i, j, srv_usage.hist_usg_arr[i][j]); fflush(stdout);
    }
    printf("\n"); fflush(stdout);
}
for(i = 0; i < 5; i++)
{
    printf(" tot_arr[%d] =%lld  ", i, srv_usage.hist_tot_arr[i]); fflush(stdout);
}
printf("\n\n");
*/

    return r;
}  


/* Read and Parse xml and determine the list count */

char * get_list_count(char *xml, char *tag, int *cnt, MainUi *m_ui)
{  
    char *p, *val;

    if ((p = get_tag(xml, tag, TRUE, m_ui)) == NULL)
    	return NULL;
    
    if ((p = get_named_tag_attr(p + strlen(tag) + 1, "count", &val, m_ui)) == NULL)
    	return NULL;

    *cnt = atoi(val);
    free(val);

    if (*cnt == 0)
    {
	log_status_msg("ERR0033", tag, "INF0007", retry_txt, m_ui->status_info);
    	return NULL;
    }

    return p;
}  


/* Extract the Type, URL and Value from an xml object */

int process_list_item(char *p, IspListObj **listobj, MainUi *m_ui)
{  
    char *val;
    int r;

    r = TRUE;

    /* Type */
    if ((p = get_named_tag_attr(p, "type", &val, m_ui)) != NULL)
    {
	(*listobj)->type = (char *) malloc(strlen(val) + 1);
	strcpy((*listobj)->type, val);
	free(val);
    }

    /* URL */
    if ((p = get_named_tag_attr(p, "href", &val, m_ui)) != NULL)
    {
	(*listobj)->href = (char *) malloc(strlen(val) + 1);
	strcpy((*listobj)->href, val);
	free(val);
    }

    /* Value */
    get_tag_val(p, (&(*listobj)->val), m_ui);

    /* Validate */
    r = check_listobj(&(*listobj));

    return r;
}  


/* Check the list object structure is valid */

int check_listobj(IspListObj **listobj)
{  
    if ((*listobj)->type && (*listobj)->href && (*listobj)->val)
	return TRUE;

    if ((*listobj)->type)
    	free((*listobj)->type);
    
    if ((*listobj)->href)
    	free((*listobj)->href);
    
    if ((*listobj)->val)
    	free((*listobj)->val);

    free(*listobj);
    
    return FALSE;
}


/* Search the service list for a given type */

IspListObj * search_list(char *type, GList *srv_list)
{  
    GList *l;
    IspListObj *srv;

    for(l = srv_list; l != NULL; l = l->next)
    {
    	srv = (IspListObj *) l->data;

    	if (strcmp(type, srv->type) == 0)
	    return srv;
    }

    return NULL;
}  


/* Return a pointer to a tag */

char * get_tag(char *xml, char *tag, int err, MainUi *m_ui)
{  
    int len;
    char *p;
    int fnd;

    fnd = FALSE;
    p = xml;
    len = strlen(tag);

    while(fnd == FALSE)
    {
	if ((p = strstr(p, tag)) == NULL)
	{
	    if (err == TRUE)
		log_status_msg("ERR0030", tag, "INF0007", retry_txt, m_ui->status_info);

	    break;
	}

	if ((*(p + len) == ' ' || *(p + len) == '>') && (*(p - 1) == '<'))
	{
	    fnd = TRUE;
	    p--;
	}
	else
	{
	    p++;
	}
    }

//printf("%s get_tag tag %s fnd %d p\n%s\n", debug_hdr, tag, fnd, p); fflush(stdout);
    return p;
}  


/* Find and return the next tag */

char * get_next_tag(char *xml, char **tag, MainUi *m_ui)
{  
    int i;
    char *p, *p2;

    p = xml;
    *tag = NULL;

    while(p != NULL)
    {
    	if ((p = strchr(p, '<')) == NULL)
	    break;

	if (*(p + 1) == '/')
	{
	    p++;
	    continue;
	}

	for(p2 = p + 1, i = 0; *p2 != ' ' && *p2 != '>'; p2++)
	    i++;

	if (i > 0)
	{
	    *tag = (char *) malloc(i + 1);
	    memcpy(*tag, p + 1, i);
	    *(*tag + i) = '\0';
	}

	break;
    }

//printf("%s get_next_tag tag %s p\n%s\n", debug_hdr, *tag, p); fflush(stdout);
    return p;
}  


/* Return a position pointer and an attibute value of a named attribute */

char * get_named_tag_attr(char *xml, char *attr, char **val, MainUi *m_ui)
{  
    char *p;

    p = get_tag_attr(xml, &attr, &(*val), m_ui);

    if (*val == NULL)
	log_status_msg("ERR0031", attr, "INF0007", retry_txt, m_ui->status_info);

    return p;
}  


/* Return a position pointer and an attibute value of the next attribute */

char * get_next_tag_attr(char *xml, char **attr, char **val, MainUi *m_ui)
{  
    char *p;

    *attr = NULL;
    p = get_tag_attr(xml, &(*attr), &(*val), m_ui);

    return p;
}  


/* Return a position pointer and the value and (optionally) the name of an attribute */

char * get_tag_attr(char *xml, char **attr, char **val, MainUi *m_ui)
{  
    int fnd, i;
    char *p, *p2;

    /* Initial, current pointer must be either '<' (tag start) or space (attribute start) */ 
    *val = NULL;
    p = xml;
    fnd = FALSE;

    if (*p == '<')
    	p = strchr(p, ' ');

    if (*p != ' ')
    	return NULL;

    /* Should now point at attribute */
    p++;

    /* Examine each for a match or if the search attribute is NULL, return the next one */
    while(! fnd)
    {
	/* Search for start of attribute value or end tag (or NULL) */
	for(p2 = p; p2 != NULL; p2++)
	{
	    if (*p2 == '=' && *(p2 + 1) == '\"')
	    	break;

	    if ((*p2 == '>' && *(p2 + 1) == '<') || (*p2 == '<' && *(p2 + 1) == '/'))
	    	break;
	}

	if (p2 == NULL)
	{
	    log_status_msg("ERR0031", "Tag Attribute", "INF0007", retry_txt, m_ui->status_info);
	    break;
	}
	    
	/* If the search attribute is null, set it to the one found */ 
	if (*attr == NULL)
	{
	    *attr = (char *) malloc(p2 - p + 1);
	    memcpy(*attr, p, p2 - p);
	    *(*attr + (p2 - p)) = '\0';
	    fnd = TRUE;
	}
	else
	{
	    /* Named attribute */
	    if (strlen(*attr) == (p2 - p))
	    	if (strncmp(*attr, p, p2 - p) == 0)
		    fnd = TRUE;
	}

	/* Get the attibute value */
	if (fnd)
	{
	    for(p2 += 2, i = 0; *p2 != '\"'; p2++)
	    	i++;

	    if (i > 0)
	    {
	    	*val = (char *) malloc(i + 1);
	    	memcpy(*val, p2 - i, i);
	    	*(*val + i) = '\0';
	    }
	}

	p = ++p2;
    }

//printf("%s get_tag_attr attr %s val %s p\n%s\n", debug_hdr, *attr, *val, p); fflush(stdout);
    return p;
}  


/* Determine a tag value */

int get_tag_val(char *xml, char **s, MainUi *m_ui)
{  
    char *p, *p2;

    *s == NULL;

    if ((p = strchr(xml,'>')) == NULL)
    {
	log_status_msg("ERR0032", NULL, "INF0007", retry_txt, m_ui->status_info);
    	return FALSE;
    }

    if ((p2 = strchr(p,'<')) == NULL)
    {
	log_status_msg("ERR0032", NULL, "INF0007", retry_txt, m_ui->status_info);
    	return FALSE;
    }

    *s = (char *) malloc(p2 - p);
    memset(*s, 0, p2 - p);
    memcpy(*s, p + 1, p2 - p - 1);

    return TRUE;
}  


/* Return the next rollover date */

char * next_rollover_dt()
{  
    const int dt_item = 6;
    char *p;

    p = srv_plan.srv_plan_item[dt_item];

    return p;
}  


/* Clear any service lists, memory, etc. */

void clean_up(IspData *isp_data)
{  
    int i;
    const int item_cnt = 13;

    g_list_free_full(isp_data->srv_list_head, (GDestroyNotify) free_srv_list);

    if (srv_usage.rollover_dt)
    	free(srv_usage.rollover_dt);

    if (srv_usage.plan_interval)
    	free(srv_usage.plan_interval);

    if (srv_usage.quota)
    	free(srv_usage.quota);

    if (srv_usage.unit)
    	free(srv_usage.unit);

    if (srv_usage.total_bytes)
    	free(srv_usage.total_bytes);

    if (srv_usage.metered_bytes)
    	free(srv_usage.metered_bytes);

    if (srv_usage.unmetered_bytes)
    	free(srv_usage.unmetered_bytes);

    for(i = 0; i < item_cnt; i++)
    {
    	if (srv_plan.srv_plan_item[i])
	    free(srv_plan.srv_plan_item[i]);
    }

    return;
}  


/* Free a service list item */

void free_srv_list(gpointer data)
{  
    IspListObj *isp_srv;

    isp_srv = (IspListObj *) data;
    free(isp_srv->val);
    free(isp_srv->href);
    free(isp_srv->type);

    if (isp_srv->sub_list_head != NULL)
    {
	g_list_free_full(isp_srv->sub_list_head, (GDestroyNotify) free_srv_list);
	isp_srv->sub_list_head = NULL;
	isp_srv->sub_list = NULL;
    }

    free(isp_srv);

    return;
}  


// Check http status 
// 'Response' Status is always 1st line (formatted as such: version code reason CRLF)
// html document has full description (if any)

int check_http_status(char *xml, int *html_code, MainUi *m_ui)
{
    int n_code, r;
    char s_code[5];
    char *p, *p2, *txt, *err_txt;

    /* Get the 3 digit code to see if there was a problem and what, if any, action is required */
    if ((p = strchr(xml, ' ')) == NULL)
    	return FALSE;

    strncpy(s_code, p + 1, 3);
    s_code[3] = '\0';
    n_code = atoi(s_code);
    *html_code = n_code;

    /* Get the reason text and code */
    if ((p2 = strstr(p + 1, "\r\n")) == NULL)
    	return FALSE;

    txt = (char *) malloc(p2 - p + 1);
    memset(txt, 0, p2 - p + 1);
    memcpy(txt, p, p2 - p);

    /* Determine status type */
    r = TRUE;

    switch(s_code[0])
    {
	case '1':		// Informational
	    sprintf(app_msg_extra, "The 'Information' code received was unexpected.\n"
	    			   "Note only. Continue anyway.");
	    log_msg("ERR0025", txt, NULL, NULL);
	    break;

	case '2':		// Success
	    if (n_code == 200)
	    	break;

	    sprintf(app_msg_extra, "While 'Success' code %d was received, it is not the "
				   "code expected.\n"
	    			   "However, it is probably not a concern. Continue anyway.", n_code);
	    log_msg("ERR0025", txt, NULL, NULL);
	    break;

	case '3':		// Redirection
	    sprintf(app_msg_extra, "The 'Redirection' code received was unexpected.\n"
	    			   "This indicates that the Client needs to take additional action.\n"
	    			   "Continue anyway, but there may be problems requiring investigation.");
	    log_msg("ERR0025", txt, NULL, NULL);
	    break;

	case '4':		// Client error
	    r = FALSE;
	    err_txt = resp_status_desc(xml, m_ui);

	    if (n_code == 401)
	    {
		sprintf(app_msg_extra, "%s", err_txt);
		log_status_msg("ERR0025", txt, "INF0008", retry_txt, m_ui->status_info);
		sprintf(app_msg_extra, "If your password has been changed you may\n"
				       "need to log in and store securely again.\n"
				       "Check the log file for more details.");
		log_status_msg("ERR0026", NULL, "INF0009", retry_txt, m_ui->status_info);
	    }
	    else
	    {
		snprintf(app_msg_extra, sizeof(app_msg_extra),
			    "Client errors indicate a problem with the program "
			    "or the setup of the request.\n"
			    "The program cannot continue and investigation is required.\n"
			    "Further details follow:\n %s", err_txt);
		log_status_msg("ERR0025", txt, "INF0010", retry_txt, m_ui->status_info);
	    }
	    	
	    free(err_txt);
	    break;

	case '5':		// Server error
	    if (n_code == 500)
	    {
		sprintf(app_msg_extra, "This is an error of unknown origin at the Server (ISP).\n"
				       "It is 'Note only' and should not effect results.\n"
				       "Continue anyway.");
	    }
	    else
	    {
		err_txt = resp_status_desc(xml, m_ui);
		snprintf(app_msg_extra, sizeof(app_msg_extra),
			    "A Server error was received that may or may not effect the "
			    "program. It indicates some problem at the Server (ISP) "
			    "preventing the request being fulfilled. Continue anyway.\n"
			    "Further details (if any) follow:\n %s", err_txt);
		free(err_txt);
	    }

	    log_msg("ERR0025", txt, NULL, NULL);
	    break;

	default:
	    break;
    }

    free(txt);

    return r;
}  


/* Return any error or message text associated with a html response status */

char * resp_status_desc(char *xml, MainUi *m_ui)
{  
    char *p, *txt;
    const char *htmldoc = "<!DOCTYPE HTML PUBLIC";
    const char *no_msg = "No further description provided.";

    /* Check for html document and subsequent paragraph */
    if ((p = strstr(xml, htmldoc)) == NULL)
    {
    	txt = (char *) malloc(strlen(no_msg) + 1);
    	strcpy(txt, no_msg);
    }
    else
    {
	if ((p = get_tag(p, "p", FALSE, m_ui)) == NULL)
	{
	    txt = (char *) malloc(strlen(no_msg) + 1);
	    strcpy(txt, no_msg);
	}
	else
	{
	    get_tag_val(p, &txt, m_ui);
	}
    }

    return txt;
}  


/* Return a pointer to the service usage details */

ServUsage * get_service_usage()
{
    ServUsage *su;

    su = &srv_usage;

    return su;
}  


/* Return a pointer to the service usage details */

void set_service_retry_txt(MainUi *_m_ui, char *buf)
{
    strcpy(retry_txt, buf);

    return;
}
