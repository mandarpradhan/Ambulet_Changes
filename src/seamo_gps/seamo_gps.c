#include "seamo_gps.h"

double glob_longitude, glob_latitude;
char kml_name[128];

void kml_init (char *name)
{

  FILE *fp;

  fp = fopen (name, "w+");

  fprintf (fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf (fp, "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
  fprintf (fp, "  <Document>\n");
  fprintf (fp, "    <Style id=\"bluePlacemark\">\n");
  fprintf (fp, "      <IconStyle>\n");
  fprintf (fp, "        <Icon>\n");
  fprintf (fp,
	   "          <href>http://maps.google.com/mapfiles/kml/pushpin/blue-pushpin.png</href>\n");
  fprintf (fp, "        </Icon>\n");
  fprintf (fp, "      </IconStyle>\n");
  fprintf (fp, "    </Style>\n");
  fprintf (fp, "    <Style id=\"redPlacemark\">\n");
  fprintf (fp, "      <IconStyle>\n");
  fprintf (fp, "        <Icon>\n");
  fprintf (fp,
	   "          <href>http://maps.google.com/mapfiles/kml/pushpin/red-pushpin.png</href>\n");
  fprintf (fp, "        </Icon>\n");
  fprintf (fp, "      </IconStyle>\n");
  fprintf (fp, "    </Style>\n");
  fprintf (fp, "    <Style id=\"yellowPlacemark\">\n");
  fprintf (fp, "      <IconStyle>\n");
  fprintf (fp, "        <Icon>\n");
  fprintf (fp,
	   "          <href>http://maps.google.com/mapfiles/kml/pushpin/ylw-pushpin.png</href>\n");
  fprintf (fp, "        </Icon>\n");
  fprintf (fp, "      </IconStyle>\n");
  fprintf (fp, "    </Style>\n");
  fprintf (fp, "    <Style id=\"greenPlacemark\">\n");
  fprintf (fp, "      <IconStyle>\n");
  fprintf (fp, "        <Icon>\n");
  fprintf (fp,
	   "          <href>http://maps.google.com/mapfiles/kml/pushpin/grn-pushpin.png</href>\n");
  fprintf (fp, "        </Icon>\n");
  fprintf (fp, "      </IconStyle>\n");
  fprintf (fp, "    </Style>\n");

  fclose (fp);

}

void
get_time_date (char *day_time)
{

  time_t rawtime;
  struct tm *info;

  time (&rawtime);

  info = localtime (&rawtime);

  strftime (day_time, 32, "%d-%m-%y-%H-%M-%S", info);

}


void insert_placemarker(char *spn,char *file_name)
{
  FILE *my_fp;
  char style_url[128];

  my_fp = fopen(file_name, "a+");


  if (!strncasecmp (spn, "airtel", 4)) {
      sprintf (style_url, "%s%s%s", "<styleUrl>", "#redPlacemark",
               "</styleUrl>");
    }
  if (!strncasecmp (spn, "tata docomo", 4)) {
      sprintf (style_url, "%s%s%s", "<styleUrl>", "#bluePlacemark",
               "</styleUrl>");
    }
  if (!strncasecmp (spn, "bsnl", 4)) {
      sprintf (style_url, "%s%s%s", "<styleUrl>", "#greenPlacemark",
               "</styleUrl>");
    }
  if (!strncasecmp (spn, "wlan0", 4)) {
      sprintf (style_url, "%s%s%s", "<styleUrl>", "#yellowPlacemark",
               "</styleUrl>");
    }


  fprintf (my_fp, "    <Placemark>\n");
  fprintf (my_fp, "      %s\n", style_url);
  fprintf (my_fp, "        <Point>\n");
  fprintf (my_fp, "          <coordinates>%lf,%lf,0</coordinates>\n",
           glob_longitude, glob_latitude);
  fprintf (my_fp, "        </Point>\n");
  fprintf (my_fp, "    </Placemark>\n");

  fclose(my_fp);

}

void *get_lat_lon(){
  
  struct gps_data_t *my_gps_data;
  int gps_conn;

  my_gps_data = calloc(1,sizeof(struct gps_data_t));	

  /*Connect to GPSD else Quit*/
  gps_conn = gps_open("localhost", "2947", my_gps_data);
  if (gps_conn != 0){
	printf("couldnt connect to gps\n");	
	exit(0);
  }

  (void) gps_stream(my_gps_data, WATCH_ENABLE | WATCH_JSON, NULL);

  while(1){
    if (gps_waiting (my_gps_data, 500)) {
        errno = 0;
        if (gps_read (my_gps_data) == -1) {
            printf("cant read from gps\n");
            exit(0);
        } else {
            if(!isnan(my_gps_data->fix.latitude)){
	       glob_latitude = my_gps_data->fix.latitude;
	    }
	    if(!isnan(my_gps_data->fix.longitude)){
	       glob_longitude = my_gps_data->fix.longitude;	
	    }
        }
    }

    usleep(10000);
  }

   /* When you are done... */
   (void) gps_stream(my_gps_data, WATCH_DISABLE, NULL);
   (void) gps_close (my_gps_data);
}

void termination_handler () {

  FILE *fp_end;
  
  fp_end = fopen( kml_name, "a+");
  fprintf (fp_end, "  </Document>\n");
  fprintf (fp_end, "</kml>\n");
  fclose(fp_end);
  printf ("kml file ended.\n");
  exit (0);
  }

void *recv_and_place (){

  int sock, bytes_read;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_len;
  char day_time[32], gps_log_name[128], recv_data[1024];
  FILE *fp;


  get_time_date (day_time);

  sprintf (kml_name, "kml_files/%s.kml", day_time);
  sprintf(gps_log_name,"logs/%s",day_time);

  /*Initialise the kml and log files*/
  kml_init(kml_name);

  fp = fopen(gps_log_name,"w+");
  fprintf(fp,"latitude,longitude,current_network\n");
  fclose(fp);

  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
      perror("Socket");
      exit(1);
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(8989);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  bzero(&(server_addr.sin_zero),8);

  if (bind(sock,(struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
      perror("Bind");
      exit(1);
  }

  addr_len = sizeof(struct sockaddr);

  while(1){
       printf("waiting to recv\n");
       bytes_read = recvfrom(sock,recv_data,1024,0,
                            (struct sockaddr *)&client_addr, &addr_len);

       if (bytes_read <= 0) {
       perror("recvfrom error");
       continue;
       }

       recv_data[bytes_read] = '\0';
       printf("(%s , %d) said : %s \n",inet_ntoa(client_addr.sin_addr),
                                       ntohs(client_addr.sin_port),recv_data);
       /*Check if latitude and longitude are non zero values only then inset placemarker*/
       if (glob_latitude > 0 && glob_longitude > 0){
	   printf("inserting placemarker at %lf-lat and %lf-lon\n",glob_latitude,glob_longitude);
           /*Insert Placemarker and fill the log files*/
           insert_placemarker(recv_data,kml_name);

           fp = fopen(gps_log_name,"a+");
           fprintf(fp,"%f,%f,%s \n ",glob_latitude,glob_longitude,recv_data);
           fclose(fp);
//       sleep(1);
	}
  }

}

int main(){
  pthread_t read_gps, place_data;
  struct sigaction new_action, old_action;

  /* Set up the structure to specify the new action. */
  new_action.sa_handler = termination_handler;
  sigemptyset (&new_action.sa_mask);
  new_action.sa_flags = 0;

  sigaction (SIGINT, NULL, &old_action);
  if (old_action.sa_handler != SIG_IGN)
  sigaction (SIGINT, &new_action, NULL);

  if (pthread_create(&read_gps, NULL, get_lat_lon, NULL)) {
      printf(" Error while creating realtime video thread:- \n");
      abort();
     }
  if (pthread_create(&place_data, NULL, recv_and_place, NULL)) {
      printf(" Error while  creating the vnc start thread:- \n");
      abort();
  }
  
  pthread_join(read_gps, 0);

  return (0);

}
