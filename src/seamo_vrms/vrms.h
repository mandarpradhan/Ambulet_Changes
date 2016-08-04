#include <stdio.h>
#include <errno.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <syslog.h>
#include <sys/time.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netdb.h>

#define PKT_BUFSIZ	1500
#define BUFFERSIZE	1500
#define WIFI		2    /* network type values giving by SeaMo may vary from differnet OS */
#define GSM	    	3
#define MSG_NOBLOCK 	0x01
#define PKT_BUFSIZ  	1500
#define INVALID_SOCK 	0
#define INVALID_SELECT 	0
#define INVALID_LEN  	0
#define LISTEN_QUEUE	5
#define SBIN_DIR	"/usr/local/seamo/sbin/"
#define TEARDOWN        1
#define ACTIVE          2
#define BLOCK		3
#define MAX_SIZE 	1016
#define MAX_PPP		3

  char *replace(const char *s, const char *old, const char *new);
  void removeSubstr(const char *src, const char *substr, char *target);
  void *network_change_detection();
  void *network_monitor_task();
  void  ConnectToVideoServer();
  void *client_pkts_discard();
  void get_current_ipaddr(char *ip_interface);
  void command_seq_change();
  void networkchangedip();
  int  fire_voip_server();
  int  vncServer_conn();
  void *seamless_video();
  void seamless_conn();
  void *fire_voip_rs();
  void vnc_start();
  void *vncserver();
  void *localvnc();
  void *voip_sip();
  void check_add_ip_rule(char *ip_interface);

  /*Functions for RTSP-UDP*/
  void * rtsp_vlc();
  int vlc_server_conn();
  void *vlcserver();
  void *localvlc();
  void *rtcp_ltor_conn();
  void *rtcp_rtol_conn();
  void *rtp_rtol_conn ();
  void *rtp_ltor_conn ();
  void *rtsp_ltor();
  void *rtsp_rtol();
  void stop_streaming();
  void store_handshake(char packet[], int len);
  void reconnect();
  void construct_teardown( char session_id[]);
  void save_first_session_id(char reply[]);
  void save_new_session_id(char reply[]);
  void replace_first_session_id(char change_pkt[]);
  void replace_new_session_id(char change_pkt[]);
  void save_replace_local_ports (char buf[]);
  void save_replace_remote_ports (char buf[]);
  void udp_sockets_init();
  void construct_teardown(char session_id[]);
  void send_teardown();

  /*Functions for RTSP-TCP*/

  void construct_teardown( char session_id[]);
  void seamless_switch();
  void save_rtp_header(unsigned char rtp_packet[], int pkt_len);
  void capture_ssrc(unsigned char recv_rpt[]);
  void save_old_session_id(char reply[]);
  void save_new_session_id(char reply[]);
  void replace_old_session_id(char change_pkt[]);
  void replace_new_session_id(char change_pkt[]);

  /*Functions for ambulet_demo.c*/

  void* ambulet_demo_thread();
  int get_available_ifs(char *ip_addrs[]);

  /* Structures */
  typedef struct ntwSock {
  int sfd;
  int flag;
  }nts;

  typedef struct iface_ip{
  char ppp0[128];
  char ppp1[128];
  char ppp2[128];
  char wlan0[128];
  } iface_ip;

  typedef struct rtp_header{
  unsigned char rtp_ver;
  unsigned char payload_type;
  unsigned short seq_no;
  int time_stamp;
  int ssrc;
  }__attribute_packed;

  struct packet{
  int interface;
  int packet_number;
  char data [MAX_SIZE];
  };
