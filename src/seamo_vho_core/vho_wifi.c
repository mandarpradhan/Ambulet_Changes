/*
 *  vho_wifi.c
 *  Receives Wi-Fi data from pre-handoff, calculates QEV for parameters
 *  and gives the QDV of a network
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
#include "vho_wifi.h"
#include "vho.h"

/***************************** VARIABLES *****************************/
struct Winfo data_wifi;
struct node *networks;		/* Linked list to hold the networks */


void averagedata(struct node *);
void observe_network(struct node *);

int wifi_valid(struct Winfo *wifi_info)
{

//printf ("wifi_valid : %s\n", wifi_info->essid);

if (!strcmp (wifi_info->essid, "WIFI_DOWN")) {
printf ("WIFI_DOWN!\n");
  return -1;
}
else 
  return 0;
}

/************************** WIFI THREAD ****************************/

/* Thread function which receives the data from the pre-handoff phase
 * and stores in the queue. */

void *wifi_data()
{
	int sockfd, n, len_send, addr_len, ret;
	char msg[20] = "Hello";
	struct sockaddr_in server_addr, client_addr;
	fd_set rfds;
	struct timeval tv;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}

        client_addr.sin_family = AF_INET;
        client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");;
        client_addr.sin_port = htons(55554);

        ret = bind(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr));
        if(ret < 0) {
        perror ("ThreeG bind failed\n");
        }


	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(55555);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	bzero(&(server_addr.sin_zero), 8);

	len_send = strlen(msg);
	addr_len = sizeof(server_addr);

	/* Send a message to unblock the sender (pre-handoff), so that the sender 
	 * can send the data continuously to the VHO phase */

	sendto(sockfd, msg, len_send, 0, (struct sockaddr *)&server_addr,
	       addr_len);

	/* Receive the data continuously */

	while (1) {

		FD_ZERO(&rfds);
		FD_SET(sockfd, &rfds);

		tv.tv_sec = 5;	/* Time out after 30 secs if we do not receive data from pre-handoff */
		tv.tv_usec = 0;

		select(sockfd + 1, &rfds, NULL, NULL, &tv);

		/* We check if there is data on the socket to read */

                syslog(LOG_INFO, "Waiting for data\n");
		if (FD_ISSET(sockfd, &rfds)) {
			n = recvfrom(sockfd, &data_wifi, sizeof(data_wifi), 0,
				     NULL, NULL);
                        syslog(LOG_INFO, "Data received\n");
			if (n < 0) {
				syslog(LOG_ERR,
				       "vho_wifi: ERROR in receving data\n");
				return 0;
			}
                      
                        if (wifi_valid(&data_wifi) < 0) {
			   received_wifi = 0;
                           continue;
                           }
                        else { 
			   received_wifi = 1;
			  /* Add the received data to the queue */
			   add_to_wqueue(data_wifi);
                         }
		} else {

			/* If we do not receive any data for more than 30 seconds,
			 * then we can imply that there are no wifi networks available.
			 * Hence change the flag value to indicate that there are no wifi
			 * networks */

			syslog(LOG_INFO, "No Wi-Fi networks\n");
			received_wifi = 0;
		}
	}
	return 0;

}

/* Add the data obtained from the pre-handoff to a circular queue.
 * @param1: Winfo structure data obtained from pre-handoff
 */

void add_to_wqueue(struct Winfo data)
{
	static int wrear = -1;

	if (qcount == MAX_QUEUE) {
		return;
	} else {
		wrear = (wrear + 1) % MAX_QUEUE;
		strcpy(wqueue[wrear].essid, data.essid);
		wqueue[wrear].channel = data.channel;
		wqueue[wrear].bandwidth = data.bandwidth;
		wqueue[wrear].level = data.level;
		wqueue[wrear].quality = data.quality;
		wqueue[wrear].cost = data.cost;
		wqueue[wrear].up = data.up;
		qcount = qcount + 1;
	}
}

/****************************** END OF THREAD *********************************/

/**************************** SUBROUTINES ******************************/

int wifi_param()
{

	/* If the thread has not received any data from the
	 * wifi network return -1 */

	if (!received_wifi){
                syslog(LOG_INFO, "Returning\n");
		return -1;
        }

	/* Read a data set from the queue */

	read_queue();

#ifdef DEBUG_L2
	struct node *temp;
	int i;
	temp = networks;
	while (temp != NULL) {
		syslog(LOG_INFO, "Network\nessid = %s %d\n", temp->essid,
		       temp->channel);
		for (i = 0; i < 4; i++)
			syslog(LOG_INFO,
			       " level = %f, quality = %f, bandwidth = %d\n",
			       temp->wdata[i].level, temp->wdata[i].quality,
			       temp->wdata[i].bandwidth);
		temp = temp->link;
	}
	temp = networks;
	while (temp != NULL) {
		syslog(LOG_INFO, "AVG Network\nessid = %s %d\n", temp->essid,
		       temp->channel);
		syslog(LOG_INFO, " level = %f, quality = %f, bandwidth = %d\n",
		       temp->avg_wdata[2].level, temp->avg_wdata[2].quality,
		       temp->avg_wdata[2].bandwidth);
		temp = temp->link;
	}
#endif

	return 1;
}

/* If we are connected to wifi network, we wait till the network/link
 * conditions are degrading and then invoke the algorithm. Else we
 * observe the current wifi network */

void observe_wifi(const char *essid, int channel)
{
	struct node *temp;
	temp = networks;

	/* Traverse the linked list to get the index of current network
	 * and observe the current network */

	while (temp != NULL) {
		if ((!strcmp(temp->essid, essid)) && (temp->channel == channel))
			observe_network(temp);
		temp = temp->link;
	}
}

void read_queue()
{
	struct Winfo new_data;

	while (qcount == 0 && received_wifi) {
		sleep(1);
	}
	
        if (wqueue[wfront].level == 0) {
             printf ("%s Ignored, wifi not available ?\n", wqueue[wfront].essid);
             reset_wifi_data(wqueue[wfront].essid);
        }

	strcpy(new_data.essid, wqueue[wfront].essid);
	new_data.channel = wqueue[wfront].channel;
	new_data.level = wqueue[wfront].level;
	new_data.quality = wqueue[wfront].quality;
	new_data.bandwidth = wqueue[wfront].bandwidth;
	new_data.cost = wqueue[wfront].cost;
	new_data.up = wqueue[wfront].up;
	wfront = (wfront + 1) % MAX_QUEUE;
	qcount = qcount - 1;

	/* If the linked list is empty, then add the new network into the list */

	if (networks == NULL)
		add_network(new_data);
	else {
		/* If there is already networks stored in the list, collect the data
		 * and store in the list */

		collectdata(new_data);
	}
}

reset_wifi_data(char *essid)
{
        struct node *temp;
        int i;

        temp = networks;

        while (temp != NULL) {
                if (!strcmp(temp->essid, essid)) {

                for (i = 0; i < 4; i++) {
                               temp->wdata[i].level = -100;
                               temp->wdata[i].bandwidth = 150;
                               temp->wdata[i].quality = 30;

                   }
                   break;
                 }
                temp = temp->link;
        }
}

/* Stores the data to the linked list in the appropriate index */

void collectdata(struct Winfo new_data)
{
	int found = 0, i;
	struct node *temp;
	temp = networks;

	while (temp != NULL) {

		/* Traverse through the list to find if the network for the 
		 * obtained data is already added.*/

		if (!strcmp(temp->essid, new_data.essid)
		    && (temp->channel == new_data.channel)) {
			found = 1;

			/* We are storing 4 sets of values inorder to average them.
			 * The Obtained latest value is stored at the end of the array 
			 * and the oldest value is ignored */

			for (i = 0; i < 3; i++) {
				temp->wdata[i].level = temp->wdata[i + 1].level;
				temp->wdata[i].quality =
				    temp->wdata[i + 1].quality;
				temp->wdata[i].bandwidth =
				    temp->wdata[i + 1].bandwidth;
			}
			temp->wdata[i].level = new_data.level;
			temp->wdata[i].quality = new_data.quality;
			temp->wdata[i].bandwidth = new_data.bandwidth;
			averagedata(temp);
		}
		temp = temp->link;
	}
	if (found == 0)
		add_network(new_data);
}

void averagedata(struct node *net)
{
	int i, j, count = 0;

	/* We maintain 3 successive sets of averaged data in order to trigger the 
	 * algorithm. The oldest averaged value is ignored. */

	for (i = 0; i < 2; i++) {
		net->avg_wdata[i].level = net->avg_wdata[i + 1].level;
		net->avg_wdata[i].bandwidth = net->avg_wdata[i + 1].bandwidth;
		net->avg_wdata[i].quality = net->avg_wdata[i + 1].quality;
	}
	net->avg_wdata[2].level = net->avg_wdata[2].quality =
	    net->avg_wdata[2].bandwidth = 0;

	/* Calculate the no of raw data stored to average accordingly */

	for (i = 0; i < 4; i++)
		if (net->wdata[i].level != 0.0)
			count++;

	/* Averaging the 4 sets of data stored */

	for (j = 0; j < 4; j++) {
		net->avg_wdata[2].level += net->wdata[j].level;
		net->avg_wdata[2].quality += net->wdata[j].quality;
		net->avg_wdata[2].bandwidth += net->wdata[j].bandwidth;
	}
	net->avg_wdata[2].level = net->avg_wdata[2].level / count;
	net->avg_wdata[2].quality = net->avg_wdata[2].quality / count;
	net->avg_wdata[2].bandwidth = net->avg_wdata[2].bandwidth / count;
}

/* Add a new network into the linked list.*/

void add_network(struct Winfo newdata)
{
	int i;
	struct node *temp;

	temp = (struct node *)malloc(sizeof(struct node));

	strcpy(temp->essid, newdata.essid);
	temp->channel = newdata.channel;
	temp->cost = newdata.cost;
	temp->up = newdata.up;

	/* Initialize the data structure */

	temp->wdata[0].level = temp->wdata[1].level = temp->wdata[2].level = 0;
	temp->wdata[0].quality = temp->wdata[1].quality =
	    temp->wdata[2].quality = 0;
	temp->wdata[0].bandwidth = temp->wdata[1].bandwidth =
	    temp->wdata[2].bandwidth = 0;

	temp->wdata[3].level = newdata.level;
	temp->wdata[3].quality = newdata.quality;
	temp->wdata[3].bandwidth = newdata.bandwidth;

	for (i = 0; i < 3; i++)
		temp->avg_wdata[i].level = temp->avg_wdata[i].bandwidth =
		    temp->avg_wdata[i].quality = 0;

	temp->link = NULL;

	/* Add a node to the end of the linked list */

	if (networks == NULL)
		networks = temp;
	else {
		struct node *cur;
		cur = networks;
		while (cur->link != NULL) {
			cur = cur->link;
		}
		cur->link = temp;
	}
}

/* Observe the current network. Trigger the VHO algorithm
 * if the current network's condition degrades */

void observe_network(struct node *current_net)
{
	struct node *temp;
	int i, count = 0;
	temp = current_net;

	for (i = 0; i < 3; i++) {
		if (temp->avg_wdata[i].level > temp->avg_wdata[i + 1].level)
			count++;
		if (temp->avg_wdata[i].quality > temp->avg_wdata[i + 1].quality)
			count++;
		if (temp->avg_wdata[i].bandwidth >
		    temp->avg_wdata[i + 1].bandwidth)
			count++;
	}

	if (count >= 1)
		vho_trigger();
}

/* Get the qdv value for the wifi network. In case of multiple
 * wifi networks, we return the network with highest QDV.
 * @param1: Buffer to store the qdv
 * @param2: buffer to store the essid of the highest QDV
 */

void get_wifi_qdv(float *qdv_wifi, char *essid)
{
	struct node *temp;
	temp = networks;
	float qev[10], qev_quality[10], qev_bandwidth[10], qev_cost[10],
	    qdv[10];
	int i = 0, max = 0;

	qev_level_wifi(qev);	/* QEV of RSSI */
	qev_bw_wifi(qev_bandwidth);	/* QEV of available bandwidth */
	qev_quality_wifi(qev_quality);	/* QEV of link quality */
	qev_cost_wifi(qev_cost);	/* QEV of the cost */
	qdv_wifi_networks(qev, qev_bandwidth, qev_quality, qev_cost, qdv);	/* Get the QDV of all the networks */

	/* In case of multiple networks, return the network
	 * with highest QDV */

	while (temp != NULL) {
		if (qdv[i] >= qdv[max]) {
			max = i;
			strcpy(essid, temp->essid);
		}
		i++;
		temp = temp->link;
	}
	*qdv_wifi = qdv[max];
}

/************************************************** QEV Calculations ******************************************/

void qev_level_wifi(float qev[])
{
	int i = 0;
	struct node *temp;
	temp = networks;
	while (temp != NULL) {
		int k;
		float a[1][5] = { {0, 0, 0, 0, 0} };
		float NUErssi[5][1] = { {0}, {0.25}, {0.5}, {0.75}, {1} };

		qev[i] = 0;

		if (temp->avg_wdata[2].level == 0)
			temp->avg_wdata[2].level = -110;

		if (temp->avg_wdata[2].level < RSSI_WIFI_TH) {
			a[0][0] = 1;
		} else if (RSSI_WIFI_TH <= temp->avg_wdata[2].level
			   && temp->avg_wdata[2].level < RSSI_WIFI_LOW) {
			a[0][0] =
			    -(temp->avg_wdata[2].level -
			      RSSI_WIFI_LOW) / (float)(RSSI_WIFI_LOW -
						       RSSI_WIFI_TH);
			a[0][1] =
			    (temp->avg_wdata[2].level -
			     RSSI_WIFI_TH) / (float)(RSSI_WIFI_LOW -
						     RSSI_WIFI_TH);
		} else if (RSSI_WIFI_LOW <= temp->avg_wdata[2].level
			   && temp->avg_wdata[2].level < RSSI_WIFI_MED) {
			a[0][1] =
			    -(temp->avg_wdata[2].level -
			      RSSI_WIFI_MED) / (float)(RSSI_WIFI_MED -
						       RSSI_WIFI_LOW);
			a[0][2] =
			    (temp->avg_wdata[2].level -
			     RSSI_WIFI_LOW) / (float)(RSSI_WIFI_MED -
						      RSSI_WIFI_LOW);
		} else if (RSSI_WIFI_MED <= temp->avg_wdata[2].level
			   && temp->avg_wdata[2].level < RSSI_WIFI_HIGH) {
			a[0][2] =
			    -(temp->avg_wdata[2].level -
			      RSSI_WIFI_HIGH) / (float)(RSSI_WIFI_HIGH -
							RSSI_WIFI_MED);
			a[0][3] =
			    (temp->avg_wdata[2].level -
			     RSSI_WIFI_MED) / (float)(RSSI_WIFI_HIGH -
						      RSSI_WIFI_MED);
		} else if (RSSI_WIFI_HIGH <= temp->avg_wdata[2].level
			   && temp->avg_wdata[2].level < RSSI_WIFI_VHIGH) {
			a[0][3] =
			    -(temp->avg_wdata[2].level -
			      RSSI_WIFI_VHIGH) / (float)(RSSI_WIFI_VHIGH -
							 RSSI_WIFI_HIGH);
			a[0][4] =
			    (temp->avg_wdata[2].level -
			     RSSI_WIFI_HIGH) / (float)(RSSI_WIFI_VHIGH -
						       RSSI_WIFI_HIGH);
		} else if (temp->avg_wdata[2].level >= RSSI_WIFI_VHIGH) {
			a[0][4] = 1;
		}
		for (k = 0; k < 5; k++) {
			qev[i] = qev[i] + a[0][k] * NUErssi[k][0];
		}
		temp = temp->link;
		i++;
	}
}

void qev_quality_wifi(float qev_quality[])
{
	int i = 0;
	struct node *temp;
	temp = networks;
	while (temp != NULL) {
		int k;
		float a[1][5] = { {0, 0, 0, 0, 0} };
		float NUEquality[5][1] = { {0}, {0.25}, {0.5}, {0.75}, {1} };

		qev_quality[i] = 0;
		if (temp->avg_wdata[2].quality <= QUALITY_WIFI_TH) {
			a[0][0] = 1;
		} else if (QUALITY_WIFI_TH <= temp->avg_wdata[2].quality
			   && temp->avg_wdata[2].quality < QUALITY_WIFI_LOW) {
			a[0][0] =
			    -(temp->avg_wdata[2].quality -
			      QUALITY_WIFI_LOW) / (float)(QUALITY_WIFI_LOW -
							  QUALITY_WIFI_TH);
			a[0][1] =
			    (temp->avg_wdata[2].quality -
			     QUALITY_WIFI_TH) / (float)(QUALITY_WIFI_LOW -
							QUALITY_WIFI_TH);;
		} else if (QUALITY_WIFI_LOW <= temp->avg_wdata[2].quality
			   && temp->avg_wdata[2].quality < QUALITY_WIFI_MED) {
			a[0][1] =
			    -(temp->avg_wdata[2].quality -
			      QUALITY_WIFI_MED) / (float)(QUALITY_WIFI_MED -
							  QUALITY_WIFI_LOW);
			a[0][2] =
			    (temp->avg_wdata[2].quality -
			     QUALITY_WIFI_LOW) / (float)(QUALITY_WIFI_MED -
							 QUALITY_WIFI_LOW);
		} else if (QUALITY_WIFI_MED <= temp->avg_wdata[2].quality
			   && temp->avg_wdata[2].quality < QUALITY_WIFI_HIGH) {
			a[0][2] =
			    -(temp->avg_wdata[2].quality -
			      QUALITY_WIFI_HIGH) / (float)(QUALITY_WIFI_HIGH -
							   QUALITY_WIFI_MED);
			a[0][3] =
			    (temp->avg_wdata[2].quality -
			     QUALITY_WIFI_MED) / (float)(QUALITY_WIFI_HIGH -
							 QUALITY_WIFI_MED);
		} else if (QUALITY_WIFI_HIGH <= temp->avg_wdata[2].quality
			   && temp->avg_wdata[2].quality < QUALITY_WIFI_VHIGH) {
			a[0][3] =
			    -(temp->avg_wdata[2].quality -
			      QUALITY_WIFI_VHIGH) / (float)(QUALITY_WIFI_VHIGH -
							    QUALITY_WIFI_HIGH);
			a[0][4] =
			    (temp->avg_wdata[2].quality -
			     QUALITY_WIFI_HIGH) / (float)(QUALITY_WIFI_VHIGH -
							  QUALITY_WIFI_HIGH);
		} else if (temp->avg_wdata[2].quality >= QUALITY_WIFI_VHIGH) {
			a[0][4] = 1;
		}
		for (k = 0; k < 5; k++) {
			qev_quality[i] =
			    qev_quality[i] + a[0][k] * NUEquality[k][0];
		}
		i++;
		temp = temp->link;
	}
}

void qev_bw_wifi(float qev_bandwidth[])
{
	int i = 0;
	struct node *temp;
	temp = networks;
	while (temp != NULL) {
		int k;
		float a[1][5] = { {0, 0, 0, 0, 0} };
		float NUEbandwidth[5][1] = { {0}, {0.25}, {0.5}, {0.75}, {1} };

		qev_bandwidth[i] = 0;
		if (temp->avg_wdata[2].bandwidth <= BW_WIFI_TH) {
			a[0][0] = 1;
		} else if (BW_WIFI_TH <= temp->avg_wdata[2].bandwidth
			   && temp->avg_wdata[2].bandwidth < BW_WIFI_LOW) {
			a[0][0] =
			    -(temp->avg_wdata[2].bandwidth -
			      BW_WIFI_LOW) / (float)(BW_WIFI_LOW - BW_WIFI_TH);
			a[0][1] =
			    (temp->avg_wdata[2].bandwidth -
			     BW_WIFI_TH) / (float)(BW_WIFI_LOW - BW_WIFI_TH);;
		} else if (BW_WIFI_LOW <= temp->avg_wdata[2].bandwidth
			   && temp->avg_wdata[2].bandwidth < BW_WIFI_MED) {
			a[0][1] =
			    (BW_WIFI_MED -
			     temp->avg_wdata[2].bandwidth) /
			    (float)(BW_WIFI_MED - BW_WIFI_LOW);
			a[0][2] =
			    (temp->avg_wdata[2].bandwidth -
			     BW_WIFI_LOW) / (float)(BW_WIFI_MED - BW_WIFI_LOW);
		} else if (BW_WIFI_MED <= temp->avg_wdata[2].bandwidth
			   && temp->avg_wdata[2].bandwidth < BW_WIFI_HIGH) {
			a[0][2] =
			    -(temp->avg_wdata[2].bandwidth -
			      BW_WIFI_HIGH) / (float)(BW_WIFI_HIGH -
						      BW_WIFI_MED);
			a[0][3] =
			    (temp->avg_wdata[2].bandwidth -
			     BW_WIFI_MED) / (float)(BW_WIFI_HIGH - BW_WIFI_MED);
		} else if (BW_WIFI_HIGH <= temp->avg_wdata[2].bandwidth
			   && temp->avg_wdata[2].bandwidth < BW_WIFI_VHIGH) {
			a[0][3] =
			    -(temp->avg_wdata[2].bandwidth -
			      BW_WIFI_VHIGH) / (float)(BW_WIFI_VHIGH -
						       BW_WIFI_HIGH);
			a[0][4] =
			    (temp->avg_wdata[2].bandwidth -
			     BW_WIFI_HIGH) / (float)(BW_WIFI_VHIGH -
						     BW_WIFI_HIGH);
		} else if (temp->avg_wdata[2].bandwidth >= BW_WIFI_VHIGH) {
			a[0][4] = 1;
		}
		for (k = 0; k < 5; k++) {
			qev_bandwidth[i] =
			    qev_bandwidth[i] + a[0][k] * NUEbandwidth[k][0];
		}
		i++;
		temp = temp->link;
	}
}

void qev_cost_wifi(float qev_cost[])
{
	int i = 0;
	struct node *temp;
	temp = networks;
	while (temp != NULL) {
		int k;
		float a[1][5] = { {0, 0, 0, 0, 0} };
		float NUEcost[5][1] = { {1}, {0.75}, {0.5}, {0.25}, {0} };

		qev_cost[i] = 0;
		if (temp->cost <= COST_TH) {
			a[0][0] = 1;
		} else if (COST_TH <= temp->cost && temp->cost < COST_LOW) {
			a[0][0] =
			    -(temp->cost - COST_LOW) / (float)(COST_LOW -
							       COST_TH);
			a[0][1] =
			    (temp->cost - COST_TH) / (float)(COST_LOW -
							     COST_TH);
		} else if (COST_LOW <= temp->cost && temp->cost < COST_MED) {
			a[0][1] =
			    -(temp->cost - COST_MED) / (float)(COST_MED -
							       COST_LOW);
			a[0][2] =
			    (temp->cost - COST_LOW) / (float)(COST_MED -
							      COST_LOW);
		} else if (COST_MED <= temp->cost && temp->cost < COST_HIGH) {
			a[0][2] =
			    -(temp->cost - COST_HIGH) / (float)(COST_HIGH -
								COST_MED);
			a[0][3] =
			    (temp->cost - COST_MED) / (float)(COST_HIGH -
							      COST_MED);
		} else if (COST_HIGH <= temp->cost && temp->cost < COST_VHIGH) {
			a[0][3] =
			    -(temp->cost - COST_VHIGH) / (float)(COST_VHIGH -
								 COST_HIGH);
			a[0][4] =
			    (temp->cost - COST_HIGH) / (float)(COST_VHIGH -
							       COST_HIGH);
		} else if (temp->cost >= COST_VHIGH) {
			a[0][4] = 1;
		}
		for (k = 0; k < 5; k++) {
			qev_cost[i] = qev_cost[i] + a[0][k] * NUEcost[k][0];
		}
		i++;
		temp = temp->link;
	}
}

/********************************************** END OF QEV Calculations ****************************************/

void qdv_wifi_networks(float qev[], float qev_bandwidth[], float qev_quality[],
		       float qev_cost[], float qdv[])
{
	struct node *temp;
	float uweight;
	temp = networks;
	int i = 0;

	/* Add extra weights for the networks based on the user preference */
/*	switch (temp->up) {
	case 1:
		uweight = 0.2;
		break;
	case 2:
		uweight = 0.1;
		break;
	default:
		uweight = 0.0;
	}
*/

/*	while (temp != NULL) {
		qdv[i] =
		    RSSI_WT_WIFI * qev[i] + QUALITY_WT_WIFI * qev_quality[i] +
		    BW_WT_WIFI * qev_bandwidth[i] + COST_WT * qev_cost[i] +
		    uweight;
*/

while (temp != NULL) {
                qdv[i] =
                    RSSI_WT_WIFI * qev[i] + QUALITY_WT_WIFI * qev_quality[i] +
                    BW_WT_WIFI * qev_bandwidth[i] + COST_WT * qev_cost[i];

#if 0
printf("----------------------------------vho_wifi_BEGIN-------------------------\n");
    printf( "QEVs: rssi %f\t, quality %f\t, bandwidth %f\t, cost %f\n", qev[i], qev_quality[i], qev_bandwidth[i], qev_cost[i]);
               printf( "QDV= %f ESSID:%s\n", qdv[i],temp->essid);
printf("----------------------------------vho_wifi_END-------------------------\n");
#endif
		i++;
		temp = temp->link;
	}

//#ifdef DEBUG_L2
/*	i = 0;
	temp = networks;

	while (temp != NULL) {
		syslog(LOG_INFO, "%s %d:", temp->essid, temp->channel);
		syslog(LOG_INFO, "QEVs: rssi %f, quality %f, bandwidth %f, cost %f\n", qev[i], qev_quality[i], qev_bandwidth[i], qev_cost[i]);
		syslog(LOG_INFO, "QDV %f\n", qdv[i]);
		i++;
		temp = temp->link;
	}
//#endif
*/
}
