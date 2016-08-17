/*
 *  rssi.c
 *  Gets the RSSI of 3G  
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
#include "read_param.h"
#include "threeg.h"

/***************************** SUBROUTINES *****************************/

/* This function checks if there are any 3G/GSM devices
 * present.
 * @param1: DBus connection
 */

void enable_modem(DBusConnection *, char *);
int get_modem_path(DBusConnection * , char **);

int get_signal_strength(DBusConnection *, char *);
bool ismodem(DBusConnection * conn)
{
	char *modem_list[5];

	memset(modem_list, 0, sizeof(modem_list));
	get_modem_path(conn, modem_list);

	if (modem_list[0] == NULL)
		return false;
	else
		return true;
}

/* This function returns the RSSI of the 3G network */

int get_rssi(DBusConnection * conn, float level[])
{
	char *modem_list[5];
	char *service_provider = malloc(100);
	int rssi = 0,num_of_dev,i;
	float rssi_dbm;
        float normalize = 0.0000 ;         

	/* Obtain the object path of the modem */
	num_of_dev = get_modem_path(conn, modem_list);
	printf("number of 3G dev = %d \n",num_of_dev);
	for(i=0;i<num_of_dev;i++){
		
	/* Enable the 3G device */
	enable_modem(conn, modem_list[i]);

	/* Get the Service Provider Name for indexing*/
	service_provider = get_service_provider_name(conn, modem_list[i]);
	/* Get the rssi status of the modem */
	rssi = get_signal_strength(conn, modem_list[i]);
	
	/* Convert percentage into RSSI */
        rssi_dbm = (rssi * 31) / 100;

        /* Convert from RSSI into dBm */
        rssi_dbm = (rssi_dbm * 2) - 113;

        printf("rssi=%f\t spn= %s\n",rssi_dbm,service_provider);

            if (rssi_dbm == normalize){
              rssi_dbm = -115;
                    }
	if(strncmp(service_provider , "TATA",4) == 0){
		level[TATA] = rssi_dbm;
		}
	if(strncmp(service_provider , "airtel",6) == 0){
                level[AIRTEL] = rssi_dbm;
		}
	if(strncmp(service_provider , "Idea",11) == 0){
                level[BSNL] = rssi_dbm;
		}


	}
	return rssi;
}

/* Get the system bus connection. Returns a D-Bus handler */

DBusConnection *get_connection()
{
	DBusConnection *conn;
	DBusError err;

	dbus_error_init(&err);

	/* Obtain a system bus */

	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if (dbus_error_is_set(&err)) {
		syslog(LOG_ERR, "Connection Error (%s)\n", err.message);
		dbus_error_free(&err);
	}
	if (NULL == conn) {
		exit(1);
	}
	return conn;
}

/* This function returns the object path of the available 3G/GSM devices 
 * @param1: D-Bus connection handler
 * @param2: Buffer where 3G/GSM modem paths will be stored
 */

int get_modem_path(DBusConnection * conn, char *modem_list[5])
{
	DBusMessage *msg;
	DBusPendingCall *pending;
	DBusMessageIter args;
	int num_of_dev = 0;

	msg = dbus_message_new_method_call("org.freedesktop.ModemManager1",
					   "/org/freedesktop/ModemManager1",
					   "org.freedesktop.DBus.ObjectManager",
					   "GetManagedObjects");

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

	if (!dbus_message_iter_init(msg, &args))
		syslog(LOG_ERR, "Message has no arguments!\n");
	else {
		num_of_dev = read_array_dict(&args, modem_list);

	}
return (num_of_dev);
}

/* This function will enable the 3G/GSM device specified by the
 * object path in @param2.
 * @param1: D-Bus connection handler
 * @param2: contains the object path which should be enabled
 */

void enable_modem(DBusConnection * conn, char *modem_path)
{
	DBusMessage *msg;
	DBusPendingCall *pending;
	int status = 1;

	msg = dbus_message_new_method_call("org.freedesktop.ModemManager1",       /* Destination */
					   modem_path,	                         /* Object Path */
					   "org.freedesktop.ModemManager1.Modem", /* Interface */
					   "Enable");	                         /* Method */
	if (NULL == msg) {
		syslog(LOG_ERR, "Message Null\n");
		exit(1);
	}

	/* Append the arguments required by the D-Bus send command */
	if (!dbus_message_append_args(msg, DBUS_TYPE_BOOLEAN, &status,
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


/* This function will return the signal strength of the 3G/GSM network
 * as seen by the device specified in the object path in @param2.
 * @param1: D-Bus connection handler
 * @param2: contains the object path of the device
 */

int get_signal_strength(DBusConnection * conn, char *modem_path)
{
	DBusMessage *msg;
	DBusPendingCall *pending;
	int rssi = 0;
	DBusMessageIter args;

	msg = dbus_message_new_method_call("org.freedesktop.ModemManager1",	/* Destination */
					   modem_path,	/* Object Path */
					   "org.freedesktop.ModemManager1.Modem.Simple",	/* Interface */
					   "GetStatus");	/* Method */
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
	if (!dbus_message_iter_init(msg, &args))
		syslog(LOG_ERR, "Message has no arguments!\n");
	else {
		int arg_type = dbus_message_iter_get_arg_type(&args);
		if (arg_type != DBUS_TYPE_INVALID)
			read_dict_variant(&args, &rssi);
	}
	dbus_message_unref(msg);
	return rssi;
}


char* get_service_provider_name(DBusConnection * conn, char *modem_path)
{
	DBusMessage *msg;
	DBusPendingCall *pending;
	char* spn = malloc(100);
	DBusMessageIter args;


	msg = dbus_message_new_method_call("org.freedesktop.ModemManager1",     /* Destination */
                                           modem_path,  /* Object Path */
                                           "org.freedesktop.ModemManager1.Modem.Simple",        /* Interface */
                                           "GetStatus");        /* Method */

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
	if (!dbus_message_iter_init(msg, &args))
		syslog(LOG_ERR, "Message has no arguments!\n");
	else {
		int arg_type = dbus_message_iter_get_arg_type(&args);
		if (arg_type != DBUS_TYPE_INVALID)
			read_spn(&args, spn);
	}
	dbus_message_unref(msg);
	//strcpy(spn, "airtel");
	//strcpy(spn, "Idea");
	return spn;
}
