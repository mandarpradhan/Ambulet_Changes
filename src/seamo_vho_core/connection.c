/*
 *  connection.c
 *  Performs association and disassociation from various networks
 * 
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

/***************************** INCLUDES *****************************/

#include "paths.h"
#include "multi_connect.h"

/**************************** SUBROUTINES ******************************/

bool isconnected(DBusConnection * conn)
{
	char *active_conn[30];
	int connected = 0;

	memset(active_conn, 0, sizeof(active_conn));
	get_active_conn(conn, active_conn);

	if (*active_conn != '\0')
		connected = 1;

	return connected;
}

/* This function will check if the system is currently
 * connected to any network 
 * Returns 1 if connected to any network else returns 0
 */
int get_active_conn(DBusConnection * conn, char *active_conn[30])
{
	int num_of_conn;
	DBusMessage *msg;
	DBusMessageIter args;
	DBusPendingCall *pending;
	const char *service = "org.freedesktop.NetworkManager";
	const char *param = "ActiveConnections";
	msg = dbus_message_new_method_call("org.freedesktop.NetworkManager",
					   "/org/freedesktop/NetworkManager",
					   "org.freedesktop.DBus.Properties",
					   "Get");

	if (NULL == msg) {
		syslog(LOG_ERR, "Message Null\n");
		exit(1);
	}
	if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &service,
				      DBUS_TYPE_STRING, &param,
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
	if (!dbus_message_iter_init(msg, &args)) {
		syslog(LOG_ERR, "Message has no arguments!\n");
		memset(active_conn, 0, sizeof(active_conn));
	} else
		num_of_conn = read_variant_array(&args, active_conn);

	dbus_message_unref(msg);
return(num_of_conn);
}

/* This function is used to associate to particular network 
 * @param1: D-Bus connection handler
 * @param2: Object path of the connection name
 * @param3: Object path of the device to connect
 * @param4: Buffer to store the object path of the active connection
 */
int my_connect(DBusConnection * conn, const char *con_path,
	       const char *dev_path, char *active_conn)
{
	DBusMessage *msg;
	DBusMessageIter args;
	DBusPendingCall *pending;
	char *list[30];		//su
	const char *service = "org.freedesktop.NetworkManager";
	const char *empty = "/";
	msg = dbus_message_new_method_call("org.freedesktop.NetworkManager",
					   "/org/freedesktop/NetworkManager",
					   "org.freedesktop.NetworkManager",
					   "ActivateConnection");
	if (NULL == msg) {
		syslog(LOG_ERR, "Message Null\n");
		exit(1);
	}
	if (!dbus_message_append_args(msg,
				      DBUS_TYPE_OBJECT_PATH, &con_path,
				      DBUS_TYPE_OBJECT_PATH, &dev_path,
				      DBUS_TYPE_OBJECT_PATH, &empty,
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
		printf("\n");
	else {
		int arg_type = dbus_message_iter_get_arg_type(&args);
		if (arg_type == DBUS_TYPE_OBJECT_PATH) {
			read_obj(&args, list);

#ifdef DEBUG_L1
			syslog(LOG_INFO, "Active connection path is - %s\n",
			       list[0]);
#endif

			strcpy(active_conn, list[0]);
			return 1;
		} else if (arg_type == DBUS_TYPE_STRING) {
			char str_ptr[100];
			read_string(&args, str_ptr);
		}
	}
	dbus_message_unref(msg);
	return -1;
}

/* This function is used to disconnect from a particular interface
 * @param1: D-Bus connection handler
 * @param2: Object path the device on which disconnection has to be performed
 */
void disconnect_old_network(DBusConnection * conn, char *old_dev_path)
{
	DBusMessage *msg;
	DBusPendingCall *pending;
	msg = dbus_message_new_method_call("org.freedesktop.NetworkManager",
					   old_dev_path,
					   "org.freedesktop.NetworkManager.Device",
					   "Disconnect");

	printf("\n\ndisconnecting the old network\n\n");

	if (NULL == msg) {
		syslog(LOG_ERR, "Message Null\n");
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
	dbus_message_unref(msg);
}
