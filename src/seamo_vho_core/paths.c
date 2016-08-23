/*
 *  paths.c
 e  Gets the D-Bus object paths for devices, connections etc
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
/**************************** VARIABLES ******************************/
char *dev_list[30];

/*************************** SUBROUTINES ******************************/

/* This function gets the available interfaces on a device
 * which has been stored 
 */

void get_stored_device_list(char *list[30])
{
	int i = 0;
	for (i = 0; i < 3; i++) {
		*list = malloc(100);
		strcpy(*list, dev_list[i]);
		list++;
	}
}

/* This function sets the preferred mode of cellular operation as 3G.
 * @param: D-Bus connection handler
 */

void prefer_3g(DBusConnection * conn, char* modem_path)
{
	DBusMessage *msg;
	DBusPendingCall *pending;
	int prefer = PREFER_3G;
        printf("the modem path is %s\n",modem_path);
	/* Send a D-Bus command to set preferred mode */

	msg = dbus_message_new_method_call("org.freedesktop.ModemManager1",	/* Destination */
					    modem_path,	/*"/org/freedesktop/ModemManager1/Modems/0",	 Object Path */
					   "org.freedesktop.ModemManager1.Modem",	/*This Interface is not available for latest version of ModemManager */
					   "SetCurrentModes");	/* Method */
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

/* This function initiates a connection to the new specified target network 
 * @param1: D-Bus connection handler
 * @param2: Network name (ESSID or APN) to connect
 * @param3: type of the interface to connect on (3G or Wi-Fi) 
 */

int handoff(DBusConnection * conn, const char *essid, int dev_type)
{
	char *dev_path, *con_path, *active_conn = malloc(100);
	int ret, conn_state;

//	printf("\nbefore calling dev_type=%d, essid =%s  \n", dev_type, essid);

	if(is_connected_3g(conn, essid )){
//		printf("%s already connected",essid);
		return -1;
        }


	dev_path = get_device_path(conn, dev_type);	/* Get object path for a particular device */
	con_path = get_conn_path(conn, essid);	/* Get object path for the particular connection */

	if (!con_path) {
		printf("Connection path NOT FOUND\n");
		return -1;
	}
#ifdef DEBUG_L1
	syslog(LOG_INFO, "Connection path is:%s\n", con_path);
//            printf("\n\n(inside_handoff)Connection path is:%s\n\n", con_path);
	syslog(LOG - INGO, "Device path is:%s\n", dev_path);
#endif

	/* Connect to new network */
        else{
		ret = my_connect(conn, con_path, dev_path, active_conn);
		printf("active conn in handoff %s \n",active_conn);
	}
	if (ret == -1) {
		syslog(LOG_ERR, "SORRY COULD NOT CONNECT TO THE NEW NETWORK\n");
		printf
		    ("(paths.c)my_connect:-SORRY COULD NOT CONNECT TO THE NEW NETWORK\n");
		return -1;
	}

	/* Wait till the connection is successful or unsuccesful
	 * and return the status of active new connection 
	 */

	while (1) {
		conn_state = get_active_conn_state(conn, active_conn);

		if (conn_state == 0 || conn_state == -1) {	//Unknown state
			return -1;
		}
		else if (conn_state == CONNECTED) {
			return 1;
		}
		else if (conn_state == ACTIVATING) {
			continue;
		}
		else if (conn_state == DEACTIVATING) {
			continue;
		}

		else if (conn_state == DEACTIVATED) {
			continue;
		}

		return -1;
	}
}



int get_modem_path(DBusConnection * conn, char *modem_list[5])
{
        DBusMessage *msg;
        DBusPendingCall *pending;
        DBusMessageIter args;
        int num_of_dev = 0;

        msg = dbus_message_new_method_call("org.freedesktop.ModemManager1",
                                           "/org/freedesktop/ModemManager1",
                                           "org.freedesktop.ModemManager1",
                                           "EnumerateDevices");

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
                num_of_dev = read_array(&args, modem_list);
        }
return (num_of_dev);
}






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

/*

connect_all(DBusConnection * conn, type)
{
  int i = 0, type, num_of_dev = 0;


        dev_path = get_device_path(conn, dev_type);     
        con_path = get_conn_path(conn, essid);


}
*/

/* This function returns the object path for the interface
 * requested (3G/Wi-Fi). 
 * @param1: D-Bus connection handler
 * @param2: device type (3G/Wi-Fi)
 */

char *get_device_path(DBusConnection * conn, int dev_type)
{
	int i = 0,j=0, type;
         char *connection_list[30];

	/* get the list of available interfaces on a particular system */

	get_device_list(conn, dev_list);

	if (dev_list[i] == NULL)
		return NULL;
	while (dev_list != NULL) {

		/* get the property of the device to get the  
		 * object path of the required interface */

		if (dev_list[i] == NULL) {
			return NULL;
		}

		get_device_properties(conn, dev_list[i], &type);
                      
		printf("type=%d \n",type);
                if (type == 2){

                connection_list[j++] = dev_list[i];
             /* 
                printf("(inside get_device_path)CONNECTION_LIST = %s, Type = %d\n",
                             connection_list[i], type);
	*/	}

		if (type == dev_type) {
		printf("(inside get_device_path)dev_list = %s, dev_type= %d, Type = %d\n",
			     dev_list[i], dev_type, type);
              
                      connection_list[j++] = dev_list[i];
			return dev_list[i];
		}
		i++;
	}
}

/* This function returns the object path for the connection
 * requested 
 * @param1: D-Bus connection handler
 * @param2: name of the network (ESSID/APN) 
 */

char *get_conn_path(DBusConnection * conn, const char *essid)
{
	char *con_list[30], auto_essid[50];
	char con_name[100];
	int i = 0;

	/* Initialise con_list with NULLs */

          for (i=0;i < 30;i++)
               con_list[i] = NULL;

	/* get the list of available connections */

	get_conn_list(conn, con_list);
	//syslog(LOG_INFO, "GET_CONN_LIST = %s \n", con_list);
	printf("get_conn_path conlist\n ");

	strcpy(auto_essid, "Auto ");
	strcat(auto_essid, essid);

	/* NetworkManager daemon will add "Auto" for the essid of a 
	 * network. So get the object path for the network matching
	 * @param2 or @param2 with prefix "Auto" */
//edited by sir
	for (i = 0; con_list[i]; i++) {

		get_conn_settings(conn, con_list[i], con_name);

                // printf("con_name : %s, essid : %s\n", con_name, essid);

		if (!strcasecmp(essid, con_name)) {
			return con_list[i];
		}

		else if (!strcasecmp(auto_essid, con_name)) {
			return con_list[i];
		}

	}

	return NULL;

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

/* Gets the current state of an interface in the system. This helps
 * us to find current active network.
 * The possible states are CONNECTED, ACTIVATING, DISCONNECTED and UNKNOWN (ERROR)
 * @param1: D-Bus connection handler
 * @param2: object path of an device in the system whose state needs to known
 */

int get_device_state(DBusConnection * conn, char dev[])
{
	DBusMessage *msg;
	DBusMessageIter args;
	DBusPendingCall *pending;
	const char *param1 = "org.freedesktop.NetworkManager.Device";
	const char *param2 = "State";
//	uint32_t state = -1;
	int state;
	msg = dbus_message_new_method_call("org.freedesktop.NetworkManager",	/* Destination */
					   dev,	/* Object Path */
					   "org.freedesktop.DBus.Properties",	/* Interface */
					   "Get");	/* Method */
	if (NULL == msg) {
		syslog(LOG_ERR, "Message Null\n");
		syslog(LOG_ERR, "Message Null get_device_state\n");
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
			read_variant(&args, &state);
	}
	dbus_message_unref(msg);
	return state;
}

/* For the object of a connection specified finds the connection name (ESSID/APN)
 * @param1: D-Bus connection handler
 * @param2: Object path of a connection whose identity needs to be known
 * @param3: Buffer to store the connection name
 */

void get_conn_settings(DBusConnection * conn, char con[], char *con_name)
{
	DBusMessage *msg;
	DBusMessageIter args;
	DBusPendingCall *pending;
	char name[50];

	msg = dbus_message_new_method_call("org.freedesktop.NetworkManager",	/* Destination */
					   con,	/* Object path */
					   "org.freedesktop.NetworkManager.Settings.Connection",	/* Interface */
					   "GetSettings");	/* Method */

	if (NULL == msg) {
		syslog(LOG_ERR, "Message Null\n");
		syslog(LOG_ERR, "Message Null get_conn_settings\n");
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
		read_essid(&args, name);
		strcpy(con_name, name);
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

/* This function gives the list connections stored in the NetworkManager's list
 * @param1: D-Bus connection handler
 * @param2: Buffer to store all the available connections
 */

void get_conn_list(DBusConnection * conn, char *dev_list[30])
{
	DBusMessage *msg;
	DBusMessageIter args;
	DBusPendingCall *pending;
	msg = dbus_message_new_method_call("org.freedesktop.NetworkManager",	/* Destination */
					   "/org/freedesktop/NetworkManager/Settings",	/* Object Path */
					   "org.freedesktop.NetworkManager.Settings",	/* Interface */
					   "ListConnections");	/* Method */

	if (NULL == msg) {
		syslog(LOG_ERR, "Message Null\n");
		syslog(LOG_ERR, "Message Null get_conn_list\n");
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
		read_array(&args, dev_list);
	dbus_message_unref(msg);

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

/* This function attempts to connect to a new network, if
 * successfull will disconnect from the old network 
 * @param1: D-Bus connection handler
 * @param2: network name to initiate connection
 * @param3: device type of current network (3G or Wi-Fi)
 * @param4: device type of network to handoff to (3G or Wi-Fi) 
 */

void post_handoff(DBusConnection * conn, const char *essid, int old_dev_type,
		  int new_dev_type)
{
	char *old_dev_path;
	int status;
	static int sfd;
	static socklen_t len;
	static struct sockaddr_in saddr;

#ifdef DEBUG
	syslog(LOG_INFO, "old_Dev = %d, new_dev = %d\n", old_dev_type,
	       new_dev_type);
#endif
	syslog(LOG_INFO, "HANDING OFF TO %s\n", essid);
//	printf("\n(inside post_handoff)old_Dev = %d, new_dev = %d\n",
//	       old_dev_type, new_dev_type);

//	printf("\n(inside post_handoff) HANDING OFF TO= %s\n", essid);

	/* Initiate connection to new network */

	status = handoff(conn, essid, new_dev_type);

	/* send network change information to VRMS */

	/* Calculate to time taken to connect to a new network */
/*su
	if (tv_first.tv_usec > tv_second.tv_usec) {
		tv_second.tv_usec += 1000000;
		tv_second.tv_sec--;
	}

	delay_micro = tv_second.tv_usec - tv_first.tv_usec;
	delay = tv_second.tv_sec - tv_first.tv_sec;

	syslog(LOG_INFO, "\nHANDOFF SUCCESSFUL...\n");
	syslog(LOG_INFO, "HANDOFF DELAY = %d sec:%d microsec\n\n", delay, delay_micro);
*/
	/* Get the status of the connection of the new network.
	 * If could not connect to the new network try again
	 * and then we give up and remain connected to the old
	 * network as long as possible */

	if (status == -1) {
//		syslog(LOG_WARNING, "COULD NOT CONNECT. Retrying....\n");
//		printf("COULD NOT CONNECT. Retrying....\n");
		status = handoff(conn, essid, new_dev_type);
	}

	/* If connection to the new network was successful
	 * then disconnect from the old interface. If old device
	 * is same as new device (i.e., horizontal handoff) then
	 * do not perform disconnection */

	if (status == 1 && old_dev_type != new_dev_type) {
		old_dev_path = get_device_path(conn, old_dev_type);	/* Get device path of old interface */

/*		if (old_dev_path)*/

//		disconnect_old_network(conn, old_dev_path);	/* Invoke disconnection on the old_interface */

		/*      else
		   printf("disconnect_old_network Failed!!! %d\n",
		   old_dev_type);
		 */
		int len_send = sizeof(new_dev_type);

		sendto(sfd, &new_dev_type, len_send, 0,
		       (struct sockaddr *)&saddr, len);

//		printf
//		    (" N/W change detection = %d new_dev_type old n/w is disconnected \n",
//		     new_dev_type);
//		system("route -n");
	}
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

char* get_service_provider_name(DBusConnection * conn, char *modem_path)
{
        DBusMessage *msg;
        DBusPendingCall *pending;
        char* spn = malloc(100);
        DBusMessageIter args;

#if 0
        msg = dbus_message_new_method_call("org.freedesktop.ModemManager1",      /* Destination */
                                           modem_path,  /* Object Path */
                                           "org.freedesktop.ModemManager1.Modem.Gsm.Card",       /* Interface */
                                           "GetSpn");   /* Method */
        if (NULL == msg) {
                syslog(LOG_ERR, "Message Null\n");
                exit(1);
        }

#endif

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
			printf("\nspn is:%s\n", spn);
        }
        dbus_message_unref(msg);
        return spn;
}

