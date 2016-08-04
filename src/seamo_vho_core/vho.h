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

#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <syslog.h>
#include <sys/prctl.h>
#include "iwlib.h"

#define CONF_DIR "/usr/local/seamo/conf/seamo.conf"

#define DEBUG                
#define DEBUG_L1             
#define DEBUG_L2             

#define INCLUDE_RRD          1
#define WIFI                2 
#define GSM                  8
#define ACTIVE               100
#define DEBUG_GENERATE_FILE  1
#define BATTERY_LOW          30
#define INCREASE_THRESHOLD   0.05
#define DECREASE_THRESHOLD   0.01
#define NORMAL_THRESHOLD     0.03

#define IFNAME   7
#define APN      1

struct Gdata {
     int bandwidth;
     float level;
};

struct node_3g
{
  int state;
  char apn[16];
  int cost;
  int up;
  struct Gdata data[4], avg_data[3];
  bool new;
  struct node_3g *link;
};

struct Wdata {
     int bandwidth;
     float level;
     float quality;
};
struct node
{
  int state;
  char essid[16];
  int channel;
  int cost;
  int up;
  struct Wdata wdata[4], avg_wdata[3];
  bool new;
  struct node *link;
};

int wifi_avail, threeg_avail;
//int vrms_nw = 0;
void *threeg_data();
void *wifi_data();
void get_current_network(int *device, char *name, int *channel);
int get_essid(char *name, int *channel);
int get_current_network_device();
void vho_trigger();
void catch_alarm(int sig);
void read_config();
int get_param(int sockfd, int req, const char *ifname, struct iwreq *wrq);

#define RSSI_WIFI_TH        -100
#define RSSI_WIFI_LOW	    -90
#define RSSI_WIFI_MED       -60
#define RSSI_WIFI_HIGH      -40
#define RSSI_WIFI_VHIGH     -30
#define QUALITY_WIFI_TH     30
#define QUALITY_WIFI_LOW    50
#define QUALITY_WIFI_MED    55
#define QUALITY_WIFI_HIGH   70
#define QUALITY_WIFI_VHIGH  90
#define BW_WIFI_TH          150
#define BW_WIFI_LOW         200
#define BW_WIFI_MED         250
#define BW_WIFI_HIGH        300
#define BW_WIFI_VHIGH       500
#define COST_TH        0
#define COST_LOW       10
#define COST_MED       30
#define COST_HIGH      40
#define COST_VHIGH     60
#define RSSI_3G_TH          -105
#define RSSI_3G_LOW         -95
#define RSSI_3G_MED         -80
#define RSSI_3G_HIGH        -70
#define RSSI_3G_VHIGH       -60

#define BW_3G_TH            25
#define BW_3G_LOW           50
#define BW_3G_MED           75
#define BW_3G_HIGH          100
#define BW_3G_VHIGH         125
/*
#define BW_3G_TH            150
#define BW_3G_LOW           200
#define BW_3G_MED           250
#define BW_3G_HIGH          300
#define BW_3G_VHIGH         500
*/
#define RSSI_WT_WIFI        0.4
#define BW_WT_WIFI          0.3
#define QUALITY_WT_WIFI     0.2
#define COST_WT             0.1
#define RSSI_WT_3G          0.5
#define BW_WT_3G            0.4
