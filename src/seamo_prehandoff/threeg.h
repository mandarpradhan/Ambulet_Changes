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
#include <syslog.h>
#include <dbus/dbus.h>
#include "read_param.h"
	
#define DEBUG
#define DEBUG_L1
#define DEBUG_L2
#define MAX_DEVICE 3
#define airtel	0
#define Idea	1
#define TATA 	2

struct Gdata {
     char apn[16];
     int bandwidth;
     float level;
     int cost;
     int up;
};

/* Structure to store the wifi parameters read from the configuration file */
struct threeg_thread_data {
     char apn[16];
     int cost;
     int up;
     int polling_interval;
     int next_time;
};

void *threeg(void *);
void scan_3G(DBusConnection *conn);
void get_bandwidth_3G(struct threeg_thread_data conf_data[]);
int get_rssi(DBusConnection *conn, float level[]);
float get_rtt(char* filename);
float calculate_bandwidth(float rtt);
float read_bandwidth(char* filename);
