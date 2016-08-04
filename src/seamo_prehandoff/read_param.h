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

void read_int_byte(DBusMessageIter * args, int *value);
void read_string(DBusMessageIter * args, char *str_value);
int read_array(DBusMessageIter * args, char *list[30]);
int read_array_dict(DBusMessageIter * args, char *list[30]);
void read_essid(DBusMessageIter * args, char *name);
void read_dict_variant(DBusMessageIter * args, int *name);
void read_connection(DBusMessageIter * iter, char *name);
int read_obj(DBusMessageIter * subiter, char *list[30]);
void read_variant(DBusMessageIter * args, int *value);
bool read_bool(DBusMessageIter * args);
int read_variant_array(DBusMessageIter * args, char *connections[30]);
void read_variant_string(DBusMessageIter * args, char *connections);
int get_active_conn(DBusConnection* conn, char *active_conn[30]);
bool isconnected(DBusConnection* conn);
void read_dict(DBusMessageIter *subiter, char *list[30]);
void read_connection(DBusMessageIter * iter, char *name);
void read_gsm(DBusMessageIter * iter, char *name);
void read_apn(DBusMessageIter * iter, char *name);
char* get_service_provider_name(DBusConnection * conn, char *modem_path);
