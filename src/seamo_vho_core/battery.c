/*
 *  battery.c
 *  Gets the remaining battery value of the host
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
 *
 */     
    
/***************************** INCLUDES *****************************/

#include "paths.h"


/**************************** SUBROUTINES ******************************/

/* This function gets the object path of the battery 
 * @param1: D-Bus connection handler
 * @param2: buffer to store the battery's object path
 */

void get_battery_path(DBusConnection * conn, char *battery_path)
{
	DBusMessage *msg;
	const char *param = "battery";
	char str_ptr[100];
	DBusPendingCall *pending;
	DBusMessageIter args, subiter;

	msg = dbus_message_new_method_call("org.freedesktop.Hal",
					   "/org/freedesktop/Hal/Manager",
					   "org.freedesktop.Hal.Manager",
					   "FindDeviceByCapability");

	if (NULL == msg) {
		syslog(LOG_ERR, "Message Null\n");
		exit(1);
	}

	if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &param,
				      DBUS_TYPE_INVALID)) {
		syslog(LOG_ERR, "Out Of Memory!\n");
		exit(1);
	}

	if (!dbus_connection_send_with_reply(conn, msg, &pending, -1)) {
		syslog(LOG_ERR, "Out Of Memory!\n");
		exit(1);
	}

	dbus_connection_flush(conn);
	dbus_message_unref(msg);
	dbus_pending_call_block(pending);
	msg = dbus_pending_call_steal_reply(pending);
	if (NULL == msg) {
		syslog(LOG_ERR, "Reply Null\n");
		exit(1);
	}

	dbus_pending_call_unref(pending);

	if (!dbus_message_iter_init(msg, &args))
		syslog(LOG_ERR, "Message has no arguments!\n");
	else {
		int type = dbus_message_iter_get_arg_type(&args);
		if (type == DBUS_TYPE_ARRAY) {
			dbus_message_iter_recurse(&args, &subiter);
			int sub_type = dbus_message_iter_get_arg_type(&subiter);
			if (sub_type == DBUS_TYPE_STRING)
				read_string(&subiter, str_ptr);
		}
	}
	strcpy(battery_path, str_ptr);
}

/* This function checks if the battery specified by
 * the object path is charging or not 
 * @param1: D-Bus connection handler
 * @param2: Object path of the battery
 */

bool ischarging(DBusConnection * conn, const char *battery_path)
{
	DBusMessage *msg;
	const char *param = "battery.rechargeable.is_charging";
	char str_ptr[100];
	bool charging;
	DBusPendingCall *pending;
	DBusMessageIter args;

	msg = dbus_message_new_method_call("org.freedesktop.Hal",
					   battery_path,
					   "org.freedesktop.Hal.Device",
					   "GetProperty");

	if (NULL == msg) {
		syslog(LOG_ERR, "Message Null\n");
		exit(1);
	}

	if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &param,
				      DBUS_TYPE_INVALID)) {
		syslog(LOG_ERR, "Out Of Memory!\n");
		exit(1);
	}

	if (!dbus_connection_send_with_reply(conn, msg, &pending, -1)) {
		syslog(LOG_ERR, "Out Of Memory!\n");
		exit(1);
	}

	dbus_connection_flush(conn);
	dbus_message_unref(msg);
	dbus_pending_call_block(pending);
	msg = dbus_pending_call_steal_reply(pending);
	if (NULL == msg) {
		syslog(LOG_ERR, "Reply Null\n");
		exit(1);
	}

	dbus_pending_call_unref(pending);

	if (!dbus_message_iter_init(msg, &args))
		syslog(LOG_ERR, "Message has no arguments!\n");
	else {
		int type = dbus_message_iter_get_arg_type(&args);
		if (type == DBUS_TYPE_BOOLEAN)
			charging = read_bool(&args);
		else if (type == DBUS_TYPE_STRING)
			read_string(&args, str_ptr);
	}
	return charging;
}

/* Returns the remaining battery time left in seconds 
 * @param1: D-Bus connection handler
 * @param2: Object path of the battery
 */

int get_battery_left_time(DBusConnection * conn, const char *battery_path)
{
	DBusMessage *msg;
	const char *param = "battery.remaining_time";
	char str_ptr[100];
	int rate;
	DBusPendingCall *pending;
	DBusMessageIter args;

	msg = dbus_message_new_method_call("org.freedesktop.Hal",
					   battery_path,
					   "org.freedesktop.Hal.Device",
					   "GetProperty");

	if (NULL == msg) {
		syslog(LOG_ERR, "Message Null\n");
		exit(1);
	}

	if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &param,
				      DBUS_TYPE_INVALID)) {
		syslog(LOG_ERR, "Out Of Memory!\n");
		exit(1);
	}

	if (!dbus_connection_send_with_reply(conn, msg, &pending, -1)) {
		syslog(LOG_ERR, "Out Of Memory!\n");
		exit(1);
	}

	dbus_connection_flush(conn);
	dbus_message_unref(msg);
	dbus_pending_call_block(pending);
	msg = dbus_pending_call_steal_reply(pending);
	if (NULL == msg) {
		syslog(LOG_ERR, "Reply Null\n");
		exit(1);
	}

	dbus_pending_call_unref(pending);

	if (!dbus_message_iter_init(msg, &args))
		syslog(LOG_ERR, "Message has no arguments!\n");
	else {
		int type = dbus_message_iter_get_arg_type(&args);
		if (type == DBUS_TYPE_STRING)
			read_string(&args, str_ptr);
		else
			read_int_byte(&args, &rate);
	}
	return rate;
}

int get_remaining_battery(DBusConnection * conn)
{
	char battery_path[100];
	int battery_time;

	get_battery_path(conn, battery_path);

	if (!ischarging(conn, battery_path)) {
		battery_time = get_battery_left_time(conn, battery_path);
	} else
		battery_time = 0;

	return (battery_time / 60);
}
