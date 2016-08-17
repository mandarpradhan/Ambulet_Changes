/*
 *  main.c
 *  Initiates scanning and creates threads for passing the data to 
 *  the algorithm phase
 *
 *  (c) SeaMo, version 0.1, 2011, ECE Department, IISc, Bangalore &
 *  Department of IT, MCIT, Government of India
 *
 *  Copyright (c) 2009 - 2011
 *  MIP Project group, ECE Department, Indian
 *  Institute of Science, Bangalore and Department of IT, Ministry of
 *  Communications and IT, Government of India. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  Authors: Seema K   <seema at eis.iisc.ernet.in>
 *           Anand SVR <anand at eis.iisc.ernet.in>
 *	     Chandana S Deshpande <deshpandesanvi@gmail.com>
 
 *  See the file "license.terms" for information on usage and redistribution
 *  of this file.
 */

/***************************** INCLUDES *****************************/
#include <stdio.h>
#include <libgen.h>
#include "wifi.h"
#include "threeg.h"
#include "iwlib-private.h"
#include "iwlib.h"
#include "read_param.h"
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
/**************************** GLOBAL DECLARATIONS **********************/ 
#define POLLING_INTERVAL 3
int auto_switch;              

/***************************** SUBROUTINES *****************************/
int main(int argc, char **argv)
{
	int sockfd, no_of_wifi = 0, i;
	pthread_t wifi_thread, threeg_thread;
	FILE *fp;
	struct wifi_thread_data thread_data;
	struct threeg_thread_data threeg_data[MAX_DEVICE];
	char conf[10][30], *line = malloc(100), *ptr;
	DBusConnection *conn;
        int c;
     
       while (1)
         {
           static struct option long_options[] =
             {
               {"autoswitch",     no_argument,       0, 'a'},
               {0, 0, 0, 0}
             };
           /* getopt_long stores the option index here. */
           int option_index = 0;
     
           c = getopt_long (argc, argv, "a",
                            long_options, &option_index);
     
           /* Detect the end of the options. */
           if (c == -1)
             break;
     
           switch (c)
             {
     
               case 'a':
	       auto_switch = 1;
               break;
	    
	     }
	}
	if(auto_switch)
		printf("autoswitch set \n");

	openlog("SeaMo:PRE-HO", LOG_PID, LOG_DAEMON);

	/* Obtain a D-Bus connection */

	conn = get_connection();

	if ((sockfd = iw_sockets_open()) < 0)
		syslog(LOG_ERR, "sock error\n");

	/* Reading the configuration File */

	fp = fopen(CONF_DIR, "r");
	while (!feof(fp)) {
		line = fgets(line, 100, fp);

		if (line != NULL && line[0] == '#')
			continue;

		char *token = strtok(line, " ");
		i = 0;
		while (token != NULL) {
			strcpy(conf[i], token);
			token = strtok(NULL, " ");
			i++;
		}
		
		/* If essid is specified then it is Wi-Fi network. If
		 * NAME is given then its 3G network. Based on essid or apn
		 * entered in configuration file we store data in the 3G
		 * Wi-Fi structures */

		if (!strcmp(conf[0], "ESSID")) {
			strcpy(thread_data.param[no_of_wifi].essid,
			       conf[ESSID]);
			thread_data.param[no_of_wifi].cost = atoi(conf[COST]);
			thread_data.param[no_of_wifi].up = atoi(conf[UP]);
			strcpy(thread_data.param[no_of_wifi].ifname,
			       conf[IFNAME]);

			/* Handle the newline character */

			if ((ptr = strchr(conf[IFNAME], '\n')) != NULL)
				*ptr = '\0';
			no_of_wifi++;
		} else if (!strcmp(conf[0], "CONNECTION_NAME")) {
			if(!strcmp(conf[APN], "TATA")){
				strcpy(threeg_data[TATA].apn, conf[APN]);
                       		threeg_data[TATA].cost = atoi(conf[COST]);
				threeg_data[TATA].up = atoi(conf[UP]);
                        	threeg_data[TATA].polling_interval = POLLING_INTERVAL;
                        	threeg_data[TATA].next_time = 0;
				}
			if(!strcmp(conf[APN], "airtel")){
				strcpy(threeg_data[airtel].apn, conf[APN]);
        	                threeg_data[airtel].cost = atoi(conf[COST]);
	                        threeg_data[airtel].up = atoi(conf[UP]);
                        	threeg_data[airtel].polling_interval = POLLING_INTERVAL;
                        	threeg_data[AIRTEL].next_time = 0;
				}
			if(!strcmp(conf[APN], "BSNL")){
				strcpy(threeg_data[BSNL].apn, conf[APN]);
                                threeg_data[BSNL].cost = atoi(conf[COST]);
                                threeg_data[BSNL].up = atoi(conf[UP]);
                        	threeg_data[BSNL].polling_interval = POLLING_INTERVAL;
                        	threeg_data[BSNL].next_time = 0;
				}
                    }
	}
	thread_data.count_aps = no_of_wifi;
	fclose(fp);

	/* Creating a thread to pass 3G and wifi parameters to the algorithm */

	if (pthread_create(&threeg_thread, NULL, threeg, (void *)&threeg_data)){
		syslog(LOG_ERR, "error in server threads\n");
        }

	if (pthread_create(&wifi_thread, NULL, wifi, (void *)&thread_data)){
		syslog(LOG_ERR, "error in server threads\n");
         }

        /* We fork a new process to start the algorithm process */
/*
	pid_t vho_pid = fork();
	if (vho_pid == 0) {
		execv(SBIN_DIR, NULL);
		syslog(LOG_ERR, "Could Not start VHO\n");
		exit(1);
	} else {
*/
        /* We fork a new process to start the vrms process */
/*
	pid_t vrms_pid = fork();
	if (vrms_pid == 0) {
		execv("/home/kill/SeaMo/src/seamo_vrms/seamo_vrms", NULL);
		syslog(LOG_ERR, "Could Not start VHO\n");
		exit(1);
	}else{ 
*/
		/* Invoking 3G and Wifi scanning */

		while (1) {

			prescan(sockfd, conf[IFNAME]);

                        /* If 3G device is available, initiate scan */

			if (ismodem(conn)) {
				scan_3G(conn);
			}
			sleep(POLLING_INTERVAL);
		}
}
