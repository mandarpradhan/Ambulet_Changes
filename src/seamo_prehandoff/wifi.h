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


#include "iwlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <asm/ioctls.h>
#include <errno.h>
#include <pthread.h>
#include <syslog.h>

#define CONF_DIR "/usr/local/seamo/conf/seamo.conf"
#define SBIN_DIR "/usr/local/seamo/sbin/seamo_vho"

#define DEBUG
#define DEBUG_L1
#define DEBUG_L2

#define ESSID    1
#define APN      1
#define COST     3
#define UP       5
#define IFNAME   7

typedef struct scan_info {
      struct sockaddr ap_addr;
      int channel;
      double freq;
      char essid[16];
      int quality;
      int level;
}scan_info;

struct wireless_stats {
     int sockfd;
     char ifname[16];
     char essid [16];
     int bandwidth;
     int cost;
     int up;
     struct wireless_info info;
     struct scan_info scan_info [24];
     int scan_entries;
};

/* Structure to store all the data to be sent to the algorithm */
struct data {
     char essid[16];
     int channel;
     int bandwidth;
     float level;
     float quality;
     int cost;
     int up;
};

struct conf_param
{
     char essid[16];
     char ifname[16];
     int cost;
     int up;
};

/* Structure to store the wifi parameters read from the configuration file */
struct wifi_thread_data {
     int count_aps;
     struct conf_param param[5];
     int polling_interval;
};


int dev_num, busy;
int scan();
void prescan(int sockfd, char *ifname);
void *wifi(void *thread_arg);
void print_essid();
int read_from_3g(int *);
int get_bandwidth();
