#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <curses.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#ifndef S_SPLINT_S
#include <unistd.h>
#endif /* S_SPLINT_S */
#include <ctype.h>

#include "gpsd_config.h"
#include "gps.h"
#include "gpsdclient.h"
#include "revision.h"


int gps_init(struct gps_data_t *gpsdata);
void get_time_date(char*day_time);
void kml_init(char *kml_name);
void insert_placemarker(char *spn, char *kml_name);
//void get_lat_lon(struct gps_data_t *gpsdata);
void *get_lat_lon();
void *recv_and_place();
