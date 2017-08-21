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
** Application:	Inodeum
**
** Author:	Anthony Buckley
**
** Description:
**  	Main application control for Inodeum - a Linux Usage Monitor for Internode
**
** History
**	09-Jan-2017	Initial code
**
*/


/* Includes */

#include <stdlib.h>  
#include <string.h>  
#include <gtk/gtk.h>  
#include <gnome-keyring-memory.h>
#include <main.h>
#include <isp.h>
#include <defs.h>


/* Defines */


/* Prototypes */

void initialise(IspData *, MainUi *);
void final(IspData *, MainUi *);

extern void main_ui(IspData *, MainUi *);
extern int check_app_dir();
extern int reset_log();
extern void close_log();
extern void log_msg(char*, char*, char*, GtkWidget*);
extern void clean_up(IspData *);
extern void free_pie_chart(PieChart *);
extern void free_bar_chart(BarChart *);


/* Globals */

static const char *debug_hdr = "DEBUG-um_main.c ";


/* Main program control */

int main(int argc, char *argv[])
{  
    IspData isp_data;
    MainUi m_ui;

    /* Initial work */
    initialise(&isp_data, &m_ui);

    /* Initialise Gtk */
    gtk_init(&argc, &argv);  

    main_ui(&isp_data, &m_ui);

    gtk_main();  

    final(&isp_data, &m_ui);

    exit(0);
}  


/* Initial work */

void initialise(IspData *isp_data, MainUi *m_ui)
{
    /* Set variables */
    app_msg_extra[0] = '\0';
    memset(isp_data, 0, sizeof (IspData));
    memset(m_ui, 0, sizeof (MainUi));

    /* Set application directory */
    if (! check_app_dir())
    	exit(-1);

    /* Start session */
    if (! reset_log())
    	exit(-1);

    log_msg("MSG0001", NULL, NULL, NULL);

    return;
}


/* Final work */

void final(IspData *isp_data, MainUi *m_ui)
{
    /* Close log file */
    log_msg("MSG0002", NULL, NULL, NULL);
    close_log();

    /* Clean up */
    if (isp_data->uname != NULL)
	free(isp_data->uname);

    if (isp_data->pw != NULL)
	gnome_keyring_memory_free (isp_data->pw);
	//free(isp_data->pw);

    if (isp_data->enc64 != NULL)
	g_free(isp_data->enc64);

    if (isp_data->isp_addr != NULL)
	free(isp_data->isp_addr);

    if (isp_data->ip != NULL)
	free(isp_data->ip);

    if (isp_data->web != NULL)
	BIO_free_all(isp_data->web);

    if (isp_data->ctx != NULL)
	SSL_CTX_free(isp_data->ctx);

    /* ??? Not sure if needed
    if (isp_data->ssl != NULL)
	SSL_free(isp_data->ssl);
	*/

    if (m_ui->pie_chart != NULL)
	free_pie_chart(m_ui->pie_chart);

    if (m_ui->bar_chart != NULL)
	free_bar_chart(m_ui->bar_chart);

    clean_up(isp_data);

    return;
}
