/*
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


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define MAX_QUEUE 10 

struct Winfo {
     char essid[16];
     int channel;
     int bandwidth;
     float level;
     float quality;
     int cost;
     int up;
};

struct Winfo wqueue[MAX_QUEUE];
int received_wifi;
int wfront, qcount;

void add_to_wqueue(struct Winfo data);
void read_queue();
void collectdata(struct Winfo new_data);
void add_network(struct Winfo newdata);
void vho_trigger();
void observe_wifi(const char* essid, int channel);
void get_wifi_qdv(float *qdv_wifi, char *essid);
void qev_level_wifi(float qev[]);
void qev_quality_wifi(float qev_quality[]);
void qev_bw_wifi(float qev_bandwidth[]);
void qdv_wifi_networks(float qev[], float qev_bandwidth[], float qev_quality[], float qev_cost[], float qdv[]);
void qev_cost_wifi(float qev_cost[]);
int wifi_param();


