/*
 *  paths.c
 *  Gets the D-Bus object paths for devices, connections etc
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

/**************************** VARIABLES ******************************/
char *dev_list[30];

/*************************** SUBROUTINES ******************************/


/* This function sets the preferred mode of cellular operation as 3G.
 * @param: D-Bus connection handler
 */

void prefer_3g(DBusConnection * conn)
{
	DBusMessage *msg;
	DBusPendingCall *pending;
	int prefer = PREFER_3G;

	/* Send a D-Bus command to set preferred mode */

	msg = dbus_message_new_method_call("org.freedesktop.ModemManager",	/* Destination */
					   "/org/freedesktop/ModemManager/Modems/0",	/* Object Path */
					   "org.freedesktop.ModemManager.Modem.Gsm.Network",	/* Interface */
					   "SetAllowedMode");	/* Method */
	if (NULL == msg) {
		syslog(LOG_ERR, "Message Null\n");
		syslog(LOG_ERR, "Message Null prefer_3g\n");
		exit(1);
	}

	/* Append the arguments required by the D-Bus send command */

	if (!dbus_message_append_args(msg, DBUS_TYPE_UINT32, &prefer,
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
}

/* Gets the interface type (GSM/Wi-Fi/Ethernet etc) of the requested 
 * object path.
 * @param1: D-Bus handler
 * @param2: Object path an interface which is obtained by listing the
            available devices on a system
 * @param3: buffer to store the device type
 */

void get_device_properties(DBusConnection * conn, char dev[], int *type)
{
	DBusMessage *msg;
	DBusMessageIter args;
	DBusPendingCall *pending;
	const char *param1 = "org.freedesktop.NetworkManager.Device";
	const char *param2 = "DeviceType";

	msg = dbus_message_new_method_call("org.freedesktop.NetworkManager",	/* Destination */
					   dev,	/* Object Path */
					   "org.freedesktop.DBus.Properties",	/* Interface */
					   "Get");	/* Method name */
	if (NULL == msg) {
		syslog(LOG_ERR, "Message Null\n");
		syslog(LOG_ERR, "Message Null get_device_properties\n");
		exit(1);
	}

	if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &param1,
				      DBUS_TYPE_STRING, &param2,
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
		int arg_type = dbus_message_iter_get_arg_type(&args);
		if (arg_type != DBUS_TYPE_INVALID)
			read_variant(&args, type);
	}
	dbus_message_unref(msg);

}

/* This function gives the object path of extensive list of devices/interfaces
 * available on a given system.
 * @param1: D-Bus connection handler
 * @param2: Buffer to store all the available devices
 */

int get_device_list(DBusConnection * conn, char *dev_list[30])
{
	DBusMessage *msg;
	DBusMessageIter args;
	DBusPendingCall *pending;
	int num_of_dev = 0;

	msg = dbus_message_new_method_call("org.freedesktop.NetworkManager",	/* Destination */
					   "/org/freedesktop/NetworkManager",	/* Object Path */
					   "org.freedesktop.NetworkManager",	/* Interface */
					   "GetDevices");	/* Method */
	if (NULL == msg) {
		syslog(LOG_ERR, "Message Null in \n");
		syslog(LOG_ERR, "Message Null in get_device_list \n");
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
	else
		num_of_dev = read_array(&args, dev_list);
	dbus_message_unref(msg);
	return (num_of_dev);
}


/* Returns the state of a connection. We use this state to check if our
 * initiated connection was successfull or not.
 * The possible states are CONNECTED, ACTIVATING, DISCONNECTED and UNKNOWN 
 * @param1: D-Bus connection handler
 * @param2: Object path of the connection
 */

int get_active_conn_state(DBusConnection * conn, const char *active_conn)
{
	DBusMessage *msg;
	DBusMessageIter args;
	DBusPendingCall *pending;
	int state;
	const char *param1 = "org.freedesktop.NetworkManager.Connection.Active";
	const char *param2 = "State";

	msg = dbus_message_new_method_call("org.freedesktop.NetworkManager",	/* Destination */
					   active_conn,	/* Object path */
					   "org.freedesktop.DBus.Properties",	/* Interface */
					   "Get");	/* Method */
	if (NULL == msg) {
		syslog(LOG_ERR, "Message Null\n");
		syslog(LOG_ERR, "Message Null get_active_conn_state\n");
		exit(1);
	}
	if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &param1,
				      DBUS_TYPE_STRING, &param2,
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
		int arg_type = dbus_message_iter_get_arg_type(&args);
		if (arg_type == DBUS_TYPE_VARIANT) {
			read_variant(&args, &state);
			dbus_message_unref(msg);
			return state;
		}
		if (arg_type == DBUS_TYPE_STRING) {
			char str_ptr[100];
			read_string(&args, str_ptr);
			syslog(LOG_ERR, "paths.c:%s\n", str_ptr);
			return -1;
		}

	}
	dbus_message_unref(msg);
	return -1;
}

void get_device_properties_string(DBusConnection * conn, char *dev_path,
                                  char *prop, char *modem_path)
{
        DBusMessage *msg;
        DBusMessageIter args;
        DBusPendingCall *pending;
        const char *service = "org.freedesktop.NetworkManager.Device";

        msg = dbus_message_new_method_call("org.freedesktop.NetworkManager",
                                           dev_path,
                                           "org.freedesktop.DBus.Properties",
                                           "Get");

        if (NULL == msg) {
                syslog(LOG_ERR, "Message Null\n");
                exit(1);
        }
        if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &service,
                                      DBUS_TYPE_STRING, &prop,
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
                memset(modem_path, 0, sizeof(modem_path));
        } else
                read_variant_string(&args, modem_path);

        dbus_message_unref(msg);
}

int get_device_properties_array(DBusConnection * conn, char *dev_path,
                                char *prop, char *connection_paths[])
{
        DBusMessage *msg;
        DBusMessageIter args;
        DBusPendingCall *pending;
        const char *service = "org.freedesktop.NetworkManager.Device";
        int num_of_con;
        msg = dbus_message_new_method_call("org.freedesktop.NetworkManager",
                                           dev_path,
                                           "org.freedesktop.DBus.Properties",
                                           "Get");

        if (NULL == msg) {
                syslog(LOG_ERR, "Message Null\n");
                exit(1);
        }
        if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &service,
                                      DBUS_TYPE_STRING, &prop,
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
                memset(connection_paths, 0, sizeof(connection_paths));
        }
        if (!dbus_message_iter_init(msg, &args))
                syslog(LOG_ERR, "Message has no arguments!\n");
        else {
                num_of_con = read_variant_array(&args, connection_paths);
        }
        dbus_message_unref(msg);
        return (num_of_con);
}

void get_connection_apn(DBusConnection * conn, char *conn_path,
                        char *conn_settings)
{
        DBusMessage *msg;
        DBusMessageIter args;
        DBusPendingCall *pending;
        msg = dbus_message_new_method_call("org.freedesktop.NetworkManager",    /* Destination */
                                           conn_path,   /* Object Path */
                                           "org.freedesktop.NetworkManager.Settings.Connection",        /* Interface */
                                           "GetSettings");      /* Method name */
        if (NULL == msg) {
                syslog(LOG_ERR, "Message Null\n");
                syslog(LOG_ERR, "Message Null get_device_properties\n");
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
                int arg_type = dbus_message_iter_get_arg_type(&args);
                if (arg_type != DBUS_TYPE_INVALID)
                        read_gsm(&args, conn_settings);
        }
        dbus_message_unref(msg);
}

