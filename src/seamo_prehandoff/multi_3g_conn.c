#include "paths.h"
#include "threeg.h"
#include "multi_connect.h"

struct connection_params connection_3g[MAX_DEVICES];

void init_3g_connections()
{
	int i;
	for (i=0; i < MAX_DEVICES;i++) {
 		connection_3g[i].connection_path = (char *)malloc(128);
 		connection_3g[i].device_path = (char *)malloc(128);
 		connection_3g[i].modem_path = (char *)malloc(128);
		bzero (connection_3g[i].spn, sizeof(connection_3g[i].spn));
	}
}

int get_num_of_modems(DBusConnection * conn)
{
char *dev_list[MAX_DEVICES];
int modem_count, type, num_of_dev, i;

num_of_dev = get_device_list(conn, dev_list);

for (i = 0, modem_count = 0; i < num_of_dev; i++) {

                get_device_properties(conn, dev_list[i], &type);

                if (type == 8) {
                modem_count++;
                }
  }

return modem_count;

}

void connect_all_modems(DBusConnection * conn){
	int num_of_modems= 0,i;
        char *dev_list[MAX_DEVICES];
        int num_of_dev;

        num_of_dev = get_device_list(conn, dev_list);
        num_of_modems = fill_connection_details(conn, num_of_dev, dev_list);
 

	for(i=0;i<num_of_modems;i++){
		if(!is_connected_3g(conn, connection_3g[i].spn )){
		   multi_connect(conn,i);
		   update_modem_paths(conn,connection_3g[i].spn);
		}
	}
}

void update_modem_paths (DBusConnection * conn, char *apn){
	int i=0;
        char *dev_list[MAX_DEVICES];
        int num_of_dev;
	
	
	for(i=0;i<get_num_of_modems(conn);i++){
		if (!strncmp(connection_3g[i].spn, apn, 4)){
			return ;
		}
	}

        num_of_dev = get_device_list(conn, dev_list);
        fill_connection_details(conn, num_of_dev, dev_list);

}

int fill_connection_details(DBusConnection * conn, int num_of_dev, char **dev_list)
{

	int i, j, type, num_of_con, apn_comp, num_of_modem = 0,conn_idx = 0;
	char result[128], conn_apn[30];
	char *spn = malloc(100);
	char *connection_paths[30];

	for (i = 0; i < num_of_dev; i++) {

		get_device_properties(conn, dev_list[i], &type);

		if (type == 8) {
			num_of_modem = num_of_modem + 1;
			printf("modem found\n");

			/*fill the device path */
			strcpy(connection_3g[conn_idx].device_path, dev_list[i]);
			printf("device path filled %s\n",
			       connection_3g[conn_idx].device_path);

			/*fill the modem path */
			get_device_properties_string(conn, dev_list[i], "Udi",
						     result);
			strcpy(connection_3g[conn_idx].modem_path, result);
			printf("modem path filled %s\n",
			       connection_3g[conn_idx].modem_path);

			/*fill the device interface */
			get_device_properties_string(conn, dev_list[i],
						     "Interface", result);
			strcpy(connection_3g[conn_idx].dev_interface, result);
			printf("device interface filled %s\n",
			       connection_3g[conn_idx].dev_interface);

			/*fill the spn */
			enable_modem(conn, connection_3g[conn_idx].modem_path);
			spn =
			    get_service_provider_name(conn,
						      connection_3g[conn_idx].modem_path);
			strcpy(connection_3g[conn_idx].spn, spn);
			printf("spn filled : %s \n", connection_3g[conn_idx].spn);

			/*fill the associated connection path */
			num_of_con =
			    get_device_properties_array(conn, dev_list[i],
							"AvailableConnections",
							connection_paths);
//                      printf("num of con : %d\n",num_of_con);
			for (j = 0; j < num_of_con; j++) {
//                              printf("connection paths : %s\n",connection_paths[j]);
				get_connection_apn(conn, connection_paths[j],
						   conn_apn);
//                              printf("connection apn : %s\n",conn_apn);
				apn_comp =
				    strncasecmp(conn_apn, &connection_3g[conn_idx].spn,
						4);
				if (apn_comp == 0) {
					strcpy(connection_3g[conn_idx].connection_path, connection_paths[j]);
					printf("connection path filled %s\n",
					       connection_3g[conn_idx].connection_path);
				}
			}
			/*fill the ipinterface */
			get_device_properties_string(conn, dev_list[i],
						     "IpInterface", result);
			strcpy(connection_3g[conn_idx].ip_interface, result);
			printf("device interface filled %s\n",
			       connection_3g[i].ip_interface);
			conn_idx++;
		}
         
	} 
return(num_of_modem);
}

void associate_spn_ipif(DBusConnection * conn, char *spn_if[]){
        int i=0, type;
        char *dev_list[MAX_DEVICES], result[128], *spn;
        int num_of_dev;

	num_of_dev = get_device_list(conn, dev_list);

        for (i = 0; i < num_of_dev; i++) {

                get_device_properties(conn, dev_list[i], &type);

                if (type == 8) {

                        get_device_properties_string(conn, dev_list[i], "Udi",
                                                     result);

			spn = get_service_provider_name(conn,result);

                        get_device_properties_string(conn, dev_list[i],
                                                     "IpInterface", result);
			if (!strncasecmp(spn,"airtel",4)){
				strcpy(spn_if[0],result);
			}
			if (!strncasecmp(spn,"tata docomo",4)){
				strcpy(spn_if[1],result);
			}
			if (!strncasecmp(spn,"bsnl",4)){
				strcpy(spn_if[2],result);
			}
		}
	}
}

char* fetch_interface(DBusConnection * conn, char* provider_apn){

 int num_of_modems = 0,i;
 int apn_comp = 0;
 
    num_of_modems = get_num_of_modems(conn);

    printf ("fetch_interface : %d\n", num_of_modems);

    for(i=0;i < num_of_modems;i++){
 
      apn_comp =  strncasecmp(provider_apn, connection_3g[i].spn,4);
 
           if (apn_comp == 0){ 
                 printf ("APN Found! %s interface : %d\n", provider_apn, 
                              connection_3g[i].ip_interface);
                 return connection_3g[i].ip_interface; 
                 }
         }
return NULL;
}


int multi_connect(DBusConnection * conn, int modem_num)
{
        DBusMessage *msg;
        DBusMessageIter args;
        DBusPendingCall *pending;
        char *list[30];         
        const char *empty = "/";
        int conn_state;
        char active_conn [64];
        int try_count = 0, found_active_conn_path;

        printf ("multi_connect called with modem_num %d\n", modem_num);

        msg = dbus_message_new_method_call("org.freedesktop.NetworkManager",
                                           "/org/freedesktop/NetworkManager",
                                           "org.freedesktop.NetworkManager",
                                           "ActivateConnection");
        if (NULL == msg) {
                syslog(LOG_ERR, "Message Null\n");
                exit(1);
        }

        printf ("multi_connect 1\n");

        if (!dbus_message_append_args(msg,
                                      DBUS_TYPE_OBJECT_PATH, &connection_3g[modem_num].connection_path,
                                      DBUS_TYPE_OBJECT_PATH, &connection_3g[modem_num].device_path,
                                      DBUS_TYPE_OBJECT_PATH, &empty,
                                      DBUS_TYPE_INVALID)) {
                syslog(LOG_ERR, "Out Of Memory!\n");
                exit(1);
        }
        printf ("multi_connect 2\n");
        if (!dbus_connection_send_with_reply(conn, msg, &pending, -1)) {
                syslog(LOG_ERR, "Out Of Memory!\n");
                exit(1);
        }
        dbus_connection_flush(conn);
        dbus_message_unref(msg);
        dbus_pending_call_block(pending);
        printf ("multi_connect 3\n");
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

                found_active_conn_path = 0;
                if (arg_type == DBUS_TYPE_OBJECT_PATH) {

                        read_obj(&args, list);

#define DEBUG_L1
#ifdef DEBUG_L1
                        syslog(LOG_INFO, "Active connection path is - %s\n",
                               list[0]);
                        printf("Active connection path is - %s\n", list[0]);
#endif

		        strcpy(active_conn, list[0]);
                        found_active_conn_path = 1;
                } else if (arg_type == DBUS_TYPE_STRING) {
                        char str_ptr[100];
                        read_string(&args, str_ptr);
                }
        }
        dbus_message_unref(msg);
      
        /* Wait till conn_state is CONNECTED. Also return if unknown state */

        if (!found_active_conn_path) return -1;
        try_count = 0;

        while (1) {

                if (try_count++ >= 4) {
                  printf ("multi_connect Timed out!\n");
                  return -1;
                }
                conn_state = get_active_conn_state(conn, active_conn);
                sleep(2);
                printf ("Getting connected... %s state : %d\n", active_conn, conn_state);

                if (conn_state == 0 || conn_state == -1) {      //Unknown state
                        return -1;
                }
                else if (conn_state == CONNECTED) {
                        printf ("multi_connect state CONNECTED!\n");
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

        return -1;
}

void multi_disconnect(DBusConnection * conn, char * active_connection)
{
        DBusMessage *msg;
        DBusPendingCall *pending;
        const char *empty = "/";

        msg = dbus_message_new_method_call("org.freedesktop.NetworkManager",
                                           "/org/freedesktop/NetworkManager",
                                           "org.freedesktop.NetworkManager",
                                           "DeactivateConnection");
        if (NULL == msg) {
                syslog(LOG_ERR, "Message Null\n");
                exit(1);
        }

        if (!dbus_message_append_args(msg,
                                      DBUS_TYPE_OBJECT_PATH, &active_connection,
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

/* This connection tells us wether we are connected to a particular 3G Network,
 *  *  * it returns a 1 if we are connected and a 0 if we are not connected.
 *  *  */

int is_connected_3g( DBusConnection * conn, char* spn ){
	int num_of_conn,i,apn_comp;
	char *active_connections[MAX_DEVICES];	
	char *active_conn_path;
	char conn_apn[64];

	num_of_conn = get_active_conn(conn,active_connections);

	printf("num of active conn = %d\n",num_of_conn);
	for(i=0;i<num_of_conn;i++){
		active_conn_path = get_active_conn_path(conn,active_connections[i]);
		get_connection_apn(conn, active_conn_path, conn_apn);
		apn_comp = strncasecmp(conn_apn, spn, 4);
		if(apn_comp == 0){
			printf("%s Connected\n",spn);
			return(1);
			}
		}
	printf("%s Not Connected\n",spn);
	return(0);
}

char* get_active_path_3g( DBusConnection * conn, char* spn ){
        int num_of_conn,i,apn_comp;
        char *active_connections[MAX_DEVICES];
        char *active_conn_path;
        char conn_apn[64];

        num_of_conn = get_active_conn(conn,active_connections);

        printf("num of active conn = %d\n",num_of_conn);
        for(i=0;i<num_of_conn;i++){
                active_conn_path = get_active_conn_path(conn,active_connections[i]);
                get_connection_apn(conn, active_conn_path, conn_apn);
                apn_comp = strncasecmp(conn_apn, spn, 4);
                if(apn_comp == 0){
                        printf("active path for %s found\n",spn);
                        return(active_connections[i]);
                }
        }
}

/* Returns the connection path of a active connection. We use this  to check if the
 *  * network which we are trying to connect is already connected.
 *   * The possible states are CONNECTED, ACTIVATING, DISCONNECTED and UNKNOWN
 *    * @param1: D-Bus connection handler
 *     * @param2: Object path of the connection
 *      */

char* get_active_conn_path (DBusConnection * conn, char *active_conn)
{
        DBusMessage *msg;
        DBusMessageIter args;
        DBusPendingCall *pending;
        char *connection_path;
        const char *param1 = "org.freedesktop.NetworkManager.Connection.Active";
        const char *param2 = "Connection";

        msg = dbus_message_new_method_call("org.freedesktop.NetworkManager",    /* Destination */
                                           active_conn, /* Object path */
                                           "org.freedesktop.DBus.Properties",   /* Interface */
                                           "Get");      /* Method */
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
                        read_variant_objpath(&args, &connection_path);
                        dbus_message_unref(msg);
                        return connection_path;
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

void connect_3g_modem(DBusConnection * conn, char *spn){
	int i,spn_comp;
	
	for(i=0;i<MAX_DEVICES;i++){
		spn_comp = strncasecmp(spn,connection_3g[i].spn,4);
		if (spn_comp == 0){
			multi_connect(conn,i);
                        break;
		}
	}

}

void disconnect_3g_modem(DBusConnection * conn, char *spn){
	int num_of_conn,i,apn_comp;
        char *active_connections[MAX_DEVICES];
        char *active_conn_path;
        char conn_apn[64];

        num_of_conn = get_active_conn(conn,active_connections);

        printf("num of active conn = %d\n",num_of_conn);
        for(i=0;i<num_of_conn;i++){
                active_conn_path = get_active_conn_path(conn,active_connections[i]);
                get_connection_apn(conn, active_conn_path, conn_apn);
                apn_comp = strncasecmp(conn_apn, spn, 4);
                if(apn_comp == 0){
			multi_disconnect(conn,active_connections[i]);
                        printf("%s Disconnected\n",spn);
                }else{
                        printf("%s Not Connected\n",spn);
                }

        }

}

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


