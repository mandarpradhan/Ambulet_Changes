/*
 *  wifi.c
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
 *  Authors: Seema K   <seema at eis.iisc.ernet.in>
 *           Anand SVR <anand at eis.iisc.ernet.in>
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
 *  See the file "license.terms" for information on usage and redistribution
 *  of this file.
 */


/***************************** INCLUDES *****************************/
#include <pthread.h>
#include "wifi.h"
#include "wireless.h"
#include "iwlib-private.h"
struct wireless_stats wireless_stats;
struct data data;
FILE *fp;

/***************************** VARIABLES *****************************/
typedef struct iwscan_state {
	/* State */
	int ap_num;		/* Access Point number 1->N */
	int val_index;		/* Value in table 0->(N-1) */
} iwscan_state;

int wifi_bw_cnt=0;

/***************************** SUBROUTINES *****************************/

/* Used to read the scanned results. Taken from wireless-tools */

static inline void print_scanning_token(struct stream_descr *stream,	/* Stream of events */
					struct iw_event *event,	/* Extracted token */
					struct iwscan_state *state, struct iw_range *iw_range,	/* Range info */
					int has_range)
{
	char buffer[128];	/* Temporary buffer */
        stream = NULL;

	/* Now, let's decode the event.
	 * We decode only the events required for the algorithm. We
	 * skip the rest of them */

	switch (event->cmd) {
	case SIOCGIWAP:
		memcpy((char *)&wireless_stats.scan_info[state->ap_num].ap_addr,
		       (char *)&event->u.ap_addr, sizeof(struct sockaddr));
		state->ap_num++;
		break;
	case SIOCGIWNWID:
		break;
	case SIOCGIWFREQ:
		{
			double freq;	/* Frequency/channel */
			int channel = -1;	/* Converted to channel */
			freq = iw_freq2float(&(event->u.freq));

                        if(freq < KILO)
                           channel = (int)freq;
                        else{
			  if (has_range)
				channel = iw_freq_to_channel(freq, iw_range);
                        }

			wireless_stats.scan_info[state->ap_num - 1].channel =
			    channel;
			wireless_stats.scan_info[state->ap_num - 1].freq = freq;
		}
		break;
	case SIOCGIWMODE:
		break;
	case SIOCGIWNAME:
		break;
	case SIOCGIWESSID:
		{
			char essid[4 * IW_ESSID_MAX_SIZE + 1];
			memset(essid, '\0', sizeof(essid));
			if ((event->u.essid.pointer) && (event->u.essid.length))
				iw_essid_escape(essid,
						event->u.essid.pointer,
						event->u.essid.length);
			strcpy(wireless_stats.
			       scan_info[state->ap_num - 1].essid, essid);
		}

		break;
	case SIOCGIWENCODE:
		break;
	case SIOCGIWRATE:
		break;
	case SIOCGIWMODUL:
		break;
	case IWEVQUAL:
		{
			int level = -111, quality = -111;
			iw_print_stats(buffer, sizeof(buffer), &event->u.qual,
				       iw_range, has_range, &level);
			wireless_stats.scan_info[state->ap_num - 1].level =
			    level;
			quality =
			    ((float)event->u.qual.qual /
			     (float)iw_range->max_qual.qual) * 100;
			wireless_stats.scan_info[state->ap_num - 1].quality =
			    quality;
			syslog(LOG_INFO, " QUALITY NOT CHANGING IN SWITCH CASE = %d", quality);
		}
		break;
#ifndef WE_ESSENTIAL
	case IWEVGENIE:
		break;
#endif /* WE_ESSENTIAL */
	case IWEVCUSTOM:
		break;
	default:
                break;
	}			/* switch(event->cmd) */
}

/* Performs scanning. Taken from wireless-tools. */
int scan()
{

	struct iwreq wrq;
	struct iw_scan_req scanopt;
	int scanflags = 0;	/* Flags for scan */
	unsigned char *buffer = NULL;	/* Results */
	int buflen = IW_SCAN_MAX_DATA;	/* Min for compat WE<17 */
	struct iw_range range;
	int has_range;
	struct timeval tv;	/* Select timeout */
	int timeout = 15000000;	/* 15s */

	has_range = (iw_get_range_info(wireless_stats.sockfd,
				       wireless_stats.ifname, &range) >= 0);

	/* Check if the interface could support scanning. */
	if ((!has_range) || (range.we_version_compiled < 14)) {
		syslog(LOG_WARNING,
			"%s Interface doesn't support scanning.\n\n",
			wireless_stats.ifname);
                wireless_stats.scan_entries = 0;
		printf("entries set to zero ******\n");
		return (-1);
	}

	/* Init timeout value -> 250ms between set and first get */
	tv.tv_sec = 0;
	tv.tv_usec = 200000;

	/* Clean up set args */
	memset(&scanopt, 0, sizeof(scanopt));

	/* Store the ESSID in the scan options */
	scanopt.essid_len = strlen(wireless_stats.essid);
	memcpy(scanopt.essid, wireless_stats.essid, scanopt.essid_len);

	scanflags = 0;
	/* scanflags |= IW_SCAN_THIS_ESSID; */

	wrq.u.data.pointer = &scanopt;
	wrq.u.data.flags = scanflags;
	wrq.u.data.length = 0;

#ifdef DEBUG_L1
	syslog(LOG_DEBUG, "wifi.c: Scan on interface %s initiated \n",
	       wireless_stats.ifname);
#endif

	/* Initiate Scanning */
	if (iw_set_ext
	    (wireless_stats.sockfd, wireless_stats.ifname, SIOCSIWSCAN,
	     &wrq) < 0) {
		if ((errno != EPERM) || (scanflags != 0)) {
			syslog(LOG_ERR,
				"%s Interface doesn't support scanning : %s\n\n",
				wireless_stats.ifname, strerror(errno));
                         
			return (-1);
		}
		/* If we don't have the permission to initiate the scan, we may
		 * still have permission to read left-over results.
		 * But, don't wait !!! */
		tv.tv_usec = 0;
	}

	timeout -= tv.tv_usec;

	/* Forever */
	while (1) {
		fd_set rfds;	/* File descriptors for select */
		int last_fd;	/* Last fd */
		int ret;

		/* Guess what ? We must re-generate rfds each time */
		FD_ZERO(&rfds);
		last_fd = -1;

		/* In here, add the rtnetlink fd in the list */

		/* Wait until something happens */
		ret = select(last_fd + 1, &rfds, NULL, NULL, &tv);

		/* Check if there was an error */
		if (ret < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;
			syslog(LOG_ERR, "Unhandled signal - exiting...\n");
			return (-1);
		}

		/* Check if there was a timeout */
		if (ret == 0) {
			unsigned char *newbuf;

realloc:
			/* (Re)allocate the buffer - realloc(NULL, len) == malloc(len) */
			newbuf = realloc(buffer, buflen);
			if (newbuf == NULL) {
				if (buffer)
					free(buffer);
				syslog(LOG_ERR, "%s: Allocation failed\n",
					__FUNCTION__);
				return (-1);
			}
			buffer = newbuf;

			/* Try to read the results */
			wrq.u.data.pointer = buffer;
			wrq.u.data.flags = 0;
			wrq.u.data.length = buflen;
			if (iw_get_ext
			    (wireless_stats.sockfd, wireless_stats.ifname,
			     SIOCGIWSCAN, &wrq) < 0) {
				/* Check if buffer was too small (WE-17 only) */
				if ((errno == E2BIG)
				    && (range.we_version_compiled > 16)
				    && (buflen < 0xFFFF)) {
					/* Some driver may return very large scan results, either
					 * because there are many cells, or because they have many

					 * large elements in cells (like IWEVCUSTOM). Most will
					 * only need the regular sized buffer. We now use a dynamic
					 * allocation of the buffer to satisfy everybody. Of course,
					 * as we don't know in advance the size of the array, we try
					 * various increasing sizes. Jean II */

					/* Check if the driver gave us any hints. */
					if (wrq.u.data.length > buflen)
						buflen = wrq.u.data.length;
					else
						buflen *= 2;

					/* wrq.u.data.length is 16 bits so max size is 65535 */
					if (buflen > 0xFFFF)
						buflen = 0xFFFF;

					/* Try again */
					goto realloc;
				}

				/* Check if results not available yet */
				if (errno == EAGAIN) {
					/* Restart timer for only 100ms */
					tv.tv_sec = 0;
					tv.tv_usec = 100000;
					timeout -= tv.tv_usec;
					if (timeout > 0)
						continue;	/* Try again later */
				}

				/* Bad error */
				free(buffer);
				syslog(LOG_ERR,
					"%-8.16s  Failed to read scan data : %s\n\n",
					wireless_stats.ifname, strerror(errno));
				wireless_stats.scan_entries = 0;
                		printf("entries set to zero ******\n");
				return (-2);
			} else
				/* We have the results, go to process them */
				break;

		}

		/* In here, check if event and event type
		 * if scan event, read results. All errors bad & no reset timeout */
	}

	if (wrq.u.data.length) {
		struct iw_event iwe;
		struct stream_descr stream;
		struct iwscan_state state = {.ap_num = 1,.val_index = 0 };
		int ret;

#ifdef DEBUG_L1
		syslog(LOG_DEBUG, "wifi.c:Scan completed %s\n",
		       wireless_stats.ifname);
#endif

		iw_init_event_stream(&stream, (char *)buffer,
				     wrq.u.data.length);
		do {
			/* Extract an event and print it */
			ret = iw_extract_event_stream(&stream, &iwe,
						      range.we_version_compiled);
			if (ret > 0)
				print_scanning_token(&stream, &iwe, &state,
						     &range, has_range);
		}
		while (ret > 0);
		wireless_stats.scan_entries = state.ap_num;
	} else
		syslog(LOG_WARNING, "%s No scan results\n\n", wireless_stats.ifname);

	free(buffer);

	return 0;

}

/* Initiate a scan on the interface ifname */

void prescan(int sockfd, char *ifname)
{

	wireless_stats.sockfd = sockfd;
	strcpy(wireless_stats.ifname, ifname);
	strcpy(wireless_stats.essid, "");

	scan();
}

/*************************** WIFI THREAD **************************/

/* This thread waits for a "hello" message from the algorithm. 
 * The scan results of various access points will be stored by the
 * main thread in wireless_stats structure. This thread will access
 * the structure to obtain RSSI, link quality for the networks
 * specified in configuration file. Along with these information
 * it will also obtain the cost and user preference. The thread
 * will pass all of these information together to the algorithm
 * continuosly after receiving HELLO message 
 */

void *wifi(void *thread_arg)
{
	int sockfd;
	struct sockaddr_in server, client;
	socklen_t addr_len;
	char msg[20];
	int n;
	struct wifi_thread_data *conf_file_info;

	/* The information read from the configuration file */

	conf_file_info = (struct wifi_thread_data *)thread_arg;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(55555);

	bind(sockfd, (struct sockaddr *)&server, sizeof server);
	addr_len = sizeof(client);

	/* Wait till you receive a message from the algorithm */

	n = recvfrom(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&client,
		     &addr_len);
	msg[n] = '\0';

#ifdef DEBUG_L1
	syslog(LOG_INFO, "wifi.c:Received message from client : %s\n", msg);
#endif

	for (;;) {
		int i, j;

		/* Each network specified in the configuration file   
		 * is compared with the extensive set of wi-fi networks
		 * obtained from the scan results
		 */

		for (j = 0; j < conf_file_info->count_aps; j++) {
                        printf ("****Scan Entries %d\n", wireless_stats.scan_entries);
			for (i = 0; i < wireless_stats.scan_entries; i++) {
				if (!strcmp(wireless_stats.scan_info[i].essid,
					    conf_file_info->param[j].essid)) {

					/* Data from the scanned results */

					strcpy(data.essid,
					       wireless_stats.scan_info[i].
					       essid);
					data.channel =
					    wireless_stats.scan_info[i].channel;
					data.level =
					    wireless_stats.scan_info[i].level;
					data.quality =
					    wireless_stats.scan_info[i].quality;
		syslog(LOG_INFO, " LINK-QUALITY-NOT CHANGING:  %d", wireless_stats.scan_info[i].quality);

					/* Data from the configuration file */

					data.cost =
					    conf_file_info->param[j].cost;
					data.up = conf_file_info->param[j].up;

					/* Data from NRIS */

					data.bandwidth = get_bandwidth();

#ifdef DEBUG
					printf
					    ("wifi.c: %s, %d, %f, %f %d\n",
					     data.essid, data.channel,
					     data.level, data.quality,
					     data.bandwidth);
#endif

					n = sendto(sockfd, &data, sizeof(data),
						   0,
						   (struct sockaddr *)&client,
						   addr_len);
				}
			}

                       /* No entries! */

                       if (i == 0) {
				bzero(&data,sizeof(data));
				strcpy (data.essid, "WIFI_DOWN");
				n = sendto(sockfd, &data, sizeof(data), 0,
					   (struct sockaddr *)&client, addr_len);
#ifdef DEBUG
					printf
					    ("wifi.c: %s, %d, %f, %f %d\n",
					     data.essid, data.channel,
					     data.level, data.quality,
					     data.bandwidth);
#endif
                       }
		}
		sleep(3);	/* Scanning is performed once in 3 seconds */
	}
	close(sockfd);
}

/* This function will query the Network Resource Information Service (NRIS)
 * to get the currently available bandwidth in the Wi-Fi network
 */

int get_bandwidth()
{

#if 0
	int sock, bytes_recieved;
	int recv_data;
	struct sockaddr_in server_addr;
	fd_set rfds;
	struct timeval tv;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}

	/* We do not want the socket to be blocked if
	 * no data is available from the NRIS */
	fcntl(sock, F_SETFL, O_NONBLOCK);

	FD_ZERO(&rfds);
	FD_SET(sock, &rfds);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5000);
  
        /* Specify the IP address of NRIS if implemented.
         * Else the bandwidth parameter is considered as 0.
         * Read the paper specified in README file to know more
         * about NRIS (Refered to as ALUM in paper).
         */

	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");	//NRIS IP address
	bzero(&(server_addr.sin_zero), 8);

	tv.tv_sec = 0;
	tv.tv_usec = 1000000;

	connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

	if (select(sock + 1, &rfds, NULL, NULL, &tv) == -1) {
		syslog(LOG_ERR, "wifi.c:ERROR get_bandwidth(): %s\n",
		       strerror(errno));
	}

	/* If there is data to read on the socket then execute recv call 
	 * else return 0 */

	if (FD_ISSET(sock, &rfds)) {
		bytes_recieved = recv(sock, &recv_data, sizeof(recv_data), 0);
	        close(sock);
		return recv_data;
	} else{
	        close(sock);
		return 0;
        }
#endif
int bandwidth;
char sys_call [128];

	 wifi_bw_cnt ++;
	 if(wifi_bw_cnt > 10000){
		wifi_bw_cnt = 0;
	 }

	 bandwidth = read_bandwidth_wifi("wlan0");

	 if((wifi_bw_cnt % 4) == 0){
         sprintf(sys_call,"sh wifi_bw/wget_bw.sh %s %s %s&>/dev/null &", "wlan0", "wlan0", "wtemp");
         system(sys_call);
	 }

         if(bandwidth < 75){
         	printf("AIRTEL BANDWIDTH < 75 !!\n");
	 }

return bandwidth;

}


int read_bandwidth_wifi(char* filename){

	FILE *fp;
	char file_path[128], *line = malloc(100), command[256];
	int bandwidth;
	int max_len=10;

 	sprintf(file_path,"wifi_bw/%s",filename);
	sprintf(command, "%s %s 1 %s","tail","-n",file_path);
	fp = popen(command, "r");
//	fp = popen("tail -n 1 docomo", "r");
	 if (fp == NULL) {
	     printf("Failed to run command\n" );
   	  exit -1;
 	}

	fgets(line, sizeof(line), fp);
//	printf("%s\n", line);
 	bandwidth = atoi(line);
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
			return 0;
		}
//  		printf("%s\n", line);
   		bandwidth = atoi(line);
		printf("bandwidth is %f\n",bandwidth);
 	}

       pclose(fp);
	
return(bandwidth);
}


