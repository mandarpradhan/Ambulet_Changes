/*
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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

struct Ginfo {
     char apn[16];
     int bandwidth;
     float level;
     int cost;
     int up;
};

#define MAX_QUEUE 10
struct Ginfo queue[MAX_QUEUE];
int received_3g;
int front, q3count;


void add_to_3gqueue(struct Ginfo data);
void read_3g_queue();
void collect3gdata(struct Ginfo new_data);
void add_3g_network(struct Ginfo newdata);
void observe_3g();
void qev_level_3g(float qev[]);	
void qev_bw_3g(float qev_bw[]);
char* get_3g_qdv(float *qdv_3g, char *apn);
void qdv_3g_networks(float qev[], float qev_bandwidth[], float qev_cost[], float qdv[]);
void qev_cost_3g(float qev_cost[]);
int threeg_param();
