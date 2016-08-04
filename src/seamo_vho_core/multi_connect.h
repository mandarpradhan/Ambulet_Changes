#define MAX_DEVICES 5

struct connection_params{
        char spn[64];
        char *modem_path;
        char *device_path;
        char dev_interface[10];
        char ip_interface[10];
        char *connection_path;
};

extern struct connection_params connection_3g[MAX_DEVICES];
int get_num_of_modems(DBusConnection * conn);
void enable_modem(DBusConnection * conn, char *modem_path);
char* get_active_conn_path (DBusConnection * conn, char *active_conn);
char* fetch_interface(DBusConnection * conn,char* provider_apn);
