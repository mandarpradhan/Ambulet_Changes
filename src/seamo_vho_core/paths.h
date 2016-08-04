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


#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/time.h>
#include "read_param.h"
#include <sys/types.h>
#include<sys/socket.h>
#include <sys/select.h>
#include<netinet/in.h>
#include<netinet/tcp.h>
#define WLAN 2 
#define GSM 8 
#define  PREFER_3G 22
#define CONNECTED 2
#define ACTIVATING 1

#define DEACTIVATING 4
#define DEACTIVATED 4

int get_device_list(DBusConnection* conn, char *dev_list[30]);
int handoff(DBusConnection* conn, const char *essid, int dev_type);
DBusConnection* get_connection();
char * get_device_path(DBusConnection* conn, int dev_type);
void get_device_properties(DBusConnection* conn, char dev[], int *);
void get_device_properties_string(DBusConnection * conn, char *dev_path,char *prop, char *modem_path);
int get_device_properties_array(DBusConnection * conn, char *dev_path,char *prop, char *connection_paths[]);
int get_device_state(DBusConnection* conn, char dev[]);
char* get_conn_path(DBusConnection* conn, const char *essid);
void get_conn_list(DBusConnection* conn, char *dev_list[30]);
void get_conn_settings(DBusConnection* conn, char con[], char *con_name);
int my_connect(DBusConnection* conn, const char* dev_path, const char* con_path, char *active_conn);
void disconnect_old_network(DBusConnection* conn, char* old_dev_path);
int get_active_conn_state(DBusConnection* conn, const char* active_conn);
void post_handoff(DBusConnection * conn, const char *essid, int old_dev_type,
		  int new_dev_type);
bool isconnected(DBusConnection* conn);
void get_connection_apn(DBusConnection * conn, char *conn_path, char *conn_settings);
void prefer_3g(DBusConnection * conn, char* modem_path);

//battery.c
int get_remaining_battery(DBusConnection* conn);
void get_battery_path(DBusConnection * conn, char *battery_path);
bool ischarging(DBusConnection * conn, const char *battery_path);
int get_battery_left_time(DBusConnection * conn, const char *battery_path);

//for spn

char* get_service_provider_name(DBusConnection * conn, char *modem_path);
int get_modem_path(DBusConnection * conn, char *modem_list[5]);
bool ismodem(DBusConnection *conn);
