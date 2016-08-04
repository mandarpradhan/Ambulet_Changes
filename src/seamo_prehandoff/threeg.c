/*
 *  threeg.c
 *  Gathers wifi parameters and passes it to the algorithm phase
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
 *
 *  See the file "license.terms" for information on usage and redistribution
 *  of this file.
 */


/***************************** INCLUDES *****************************/
#include "threeg.h"
#include "multi_connect.h"
/***************************** VARIABLES *****************************/
#define BANDWIDTH_INTERVAL 15
struct Gdata data_3g[MAX_DEVICE];
int received = 0;
int count = 0,j=0,get_bw_cnt=0;
int  provider1_switch = 1,provider2_switch = 0,provider3_switch = 0;
char *spn_if[MAX_DEVICE];
extern int auto_switch;
/***************************** SUBROUTINES *****************************/

/* This function performs scan on 3G/GSM device and
 * fills the structure data_3g, which is passed to the
 * algorithm by the thread running.
 */

void scan_3G(DBusConnection *conn){
	
        float level[MAX_DEVICE];
        int i=0;
        float normalize = 0; 
	
        bzero(level,sizeof(level));

        get_rssi(conn,level);
 	

	for(i=0;i<MAX_DEVICE;i++){

            data_3g[i].level = level[i];
		

	if (auto_switch == 1){
               if( provider1_switch  == 1 && i == 0)  {
                  data_3g[i].level = -115.0000;
                  }
 
               if( provider2_switch == 1 && i == 1) {
                  data_3g[i].level = -115.0000;
                  }
          	if ( provider3_switch == 1 && i == 2)
            	{
                  data_3g[i].level = -115;
		}

	}
          if ( data_3g[i].level == normalize)
            {
              data_3g[i].level = -115;
      printf("rssi level is normalized to -115\n");
                  }

  }
if(auto_switch == 1){
count++;
if (count == 300){
count = 0;
}

j = count%10;
printf("value of j= %d\n",j);

if (j == 9 ){

if (provider1_switch == 1){
      provider1_switch = 0;
      provider2_switch = 1;
      provider3_switch = 0;
}else if(provider2_switch == 1) {
      provider1_switch = 0;
      provider2_switch = 0;
      provider3_switch = 1;
}else if(provider3_switch == 1){
      provider1_switch = 1;
      provider2_switch = 0;
      provider3_switch = 0;
	}
}
  printf("value of count = %d\n",count);
}
}


void get_bandwidth_3G(struct threeg_thread_data conf_data[])
{
	/* With 3G connection, we can obtain maximum
	 * of 384kbps when the user is mobile. We do not
	 * have a mechanism to get the currently available
	 * bandwidth from 3G, hence we use 384kbps 
	 */
	int i = 0;
	float rtt, bandwidth;
	char sys_call[128];
	
	get_bw_cnt += conf_data[0].polling_interval;

	if(get_bw_cnt > 10000){
		get_bw_cnt = 0;
	}


	for(i=0;i<MAX_DEVICE;i++){
//		data_3g[i].bandwidth = 384;
	printf("*********!!!!!!!!get_bw_cnt = %d next_time = %d polling_int = %d\n!!!!!!! ",get_bw_cnt,conf_data[i].next_time,conf_data[i].polling_interval);
		
		if(strlen(spn_if[i])){
			if(i==0){
				bandwidth = read_bandwidth("airtel");

				if(get_bw_cnt > conf_data[i].next_time){
				sprintf(sys_call,"sh threeg_bw/wget_bw.sh %s %s %s&>/dev/null &", spn_if[i], "airtel", "atemp");
				system(sys_call);
				conf_data[i].next_time = (BANDWIDTH_INTERVAL + get_bw_cnt);
				printf("!!!!!get_bw_cnt = %d next_time = %d executed wget\n!!!!!!! ",get_bw_cnt,conf_data[i].next_time);
				}

				data_3g[i].bandwidth = bandwidth;
				if(bandwidth < 75){
					printf("AIRTEL BANDWIDTH < 75 !!\n");
				}
			}
			if(i==1){
				bandwidth = read_bandwidth("docomo");

				if(get_bw_cnt > conf_data[i].next_time){
				sprintf(sys_call,"sh threeg_bw/wget_bw.sh %s %s %s&>/dev/null &", spn_if[i], "docomo", "dtemp");
				system(sys_call);
				conf_data[i].next_time = (BANDWIDTH_INTERVAL + get_bw_cnt);
				printf("!!!!!get_bw_cnt = %d next_time = %d executed wget\n!!!!!!! ",get_bw_cnt,conf_data[i].next_time);
				}

				data_3g[i].bandwidth = bandwidth;
				if(bandwidth < 75){
					printf("DOCOMO BANDWIDTH < 75 !!\n");
				}
			}
			if(i==2){
				bandwidth = read_bandwidth("bsnl");

				if(get_bw_cnt > conf_data[i].next_time){
				sprintf(sys_call,"sh threeg_bw/wget_bw.sh %s %s %s&>/dev/null &", spn_if[i], "bsnl", "btemp");
				system(sys_call);
				conf_data[i].next_time = (BANDWIDTH_INTERVAL + get_bw_cnt);
				printf("!!!!!get_bw_cnt = %d next_time = %d executed wget\n!!!!!!! ",get_bw_cnt,conf_data[i].next_time);
				}

				data_3g[i].bandwidth = bandwidth;
				if(bandwidth < 75){
					printf("BSNL BANDWIDTH < 75 !!\n");
				}
			}
		}else{
			data_3g[i].bandwidth = 0;
		}
		
	}

}

float read_bandwidth(char* filename){

	FILE *fp;
	char file_path[128], *line = malloc(100), command[256];
	float bandwidth;
	int max_len=10;

 	sprintf(file_path,"threeg_bw/%s",filename);
	sprintf(command, "%s %s 1 %s","tail","-n",file_path);

	fp = popen(command, "r");
//	fp = popen("tail -n 1 docomo", "r");
	 if (fp == NULL) {
	     printf("Failed to run command\n" );
   	  exit -1;
 	}

	fgets(line, sizeof(line), fp);
//	printf("%s\n", line);
 	bandwidth = atof(line);
	printf("bandwidth is %f \n",bandwidth);

 	if(bandwidth == 0){
  		printf("empty line detected\n");
		sprintf(command, "%s %s 2 %s","tail","-n",file_path);
		fp = popen(command, "r");
		 
//   		fp = popen("tail -n 2 docomo", "r");
   		if (fp == NULL) {
     		printf("Failed to run command\n" );
     		exit -1;
   		}

		fgets(line, sizeof(line), fp);
		if(line == NULL){
			return (0);
		}
//  		printf("%s\n", line);
   		bandwidth = atof(line);
		printf("bandwidth is %f\n",bandwidth);
 	}

       pclose(fp);
	
return(bandwidth);
}

float get_rtt(char* filename){

	FILE *fp;
	char file_path[128], *line = malloc(100), *token;
	float rtt;
 	sprintf(file_path,"threeg_bw/%s",filename);

	fp = fopen(file_path,"r+");
        line = fgets(line, 100, fp);	
	fclose(fp);

	printf("line got = %s\n",line);
	token = strtok(line, "\n");
	
	rtt = atof(token);
return(rtt);
}

float calculate_bandwidth(float rtt){

	float bandwidth;
	
	bandwidth = (2048 * 8 * 1000)/rtt; 
	
return bandwidth;
}


void init_threeg_data()
{

float level[MAX_DEVICE];
int i=0;
float normalize = 0;

        bzero(level,sizeof(level));

        for(i=0;i<MAX_DEVICE;i++){

           data_3g[i].level = level[i]; 
           if ( data_3g[i].level == normalize) {
              data_3g[i].level = -115;
                  }
	   /*Allocate memory and bzero the struct to hold ip_interface associated with the service providers*/
	   spn_if[i] = calloc(10,sizeof(char));	
        }
}


/**************************** 3G THREAD *********************/

/* This thread waits for a "hello" message from the algorithm.
 * The thread will pass all of the information together to the 
 * algorithm continuosly after receiving HELLO message
 */

void *threeg(void *thread_conf_data)
{
	int sockfd, len_recv = 0, iResult = 0,i;
	struct sockaddr_in server, client;
	socklen_t addr_len;
	char msg[20];
	struct threeg_thread_data *conf_data = (struct threeg_thread_data *)thread_conf_data;

	/* Obtain a D-Bus connection handler for all D-Bus activities */
	DBusConnection *conn = (DBusConnection *) get_connection();

	/* The information read from the configuration file */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(55556);

	iResult = bind(sockfd, (struct sockaddr *)&server, sizeof server);
	if(iResult < 0){
		perror("BIND 55555");
	}
	addr_len = sizeof(client);

        init_threeg_data();

	/* Wait for the HELLO message from the algorithm */

        printf ("Waiting for HELLO from VHO\n");

	iResult = recvfrom(sockfd, msg, len_recv, 0, (struct sockaddr *)&client,
		     &addr_len);
	msg[iResult] = '\0';
        received = 1;
        printf ("Received HELLO from VHO\n");
	
 	associate_spn_ipif(conn,spn_if);	
	printf("spn_if[0]= %s \t spn_if[1]= %s \t spn_if[2]= %s\n",spn_if[0],spn_if[1],spn_if[2]);
	
	for(i=0;i<MAX_DEVICE;i++){
	strcpy(data_3g[i].apn, conf_data[i].apn);
	data_3g[i].cost = conf_data[i].cost;
	data_3g[i].up = conf_data[i].up;
	}
	

	/* Send 3G data (RSS, bandwidth, cost and user preference) 
	 * to the algorithm once in 't' seconds 
	 */

	for (;;) {
		get_bandwidth_3G(conf_data);

                if( received ){
		for(i=0;i<MAX_DEVICE;i++){
		iResult = sendto(sockfd, &data_3g[i], sizeof(data_3g[i]), 0,
			   (struct sockaddr *)&client, addr_len);
		printf("sent data of %s \n",data_3g[i].apn);
//		usleep(10);
//		printf("sent data of %s \n",data_3g[i].apn);
		}
#ifdef DEBUG
		for(i=0;i<MAX_DEVICE;i++){
		printf("threeg.c: %s, %f, %d\n", data_3g[i].apn,
		       data_3g[i].level, data_3g[i].bandwidth);
		}
#endif
                 }
		sleep(conf_data[0].polling_interval);	// Scanning is performed once in 3 seconds
	}
	close(sockfd);
}
