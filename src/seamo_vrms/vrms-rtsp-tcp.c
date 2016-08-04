#include "vrms.h"

int vlc_connsock, vlc_serversock, vlc_sockfd, rtp_remote, rtp_local,
    rtcp_remote, rtcp_local, vlc_serversock_new;
pthread_t local_vlc, vlc_server, rtp_thread, rtcp_ltor_thread, rtcp_rtol_thread,
    rtsp_reconnect;
static unsigned short rtcp_client_port, rtcp_server_port;
extern int real_network_view, seamless_busy;
char new_IPaddr[128];
char rtsp_options[512], rtsp_describe[512], rtsp_setup[512], rtsp_play[512],
    rtsp_new_id[9], rtsp_old_id[9], rtsp_teardown[512];
char rtp_report[512];
unsigned char rtsp_ssrc[4];
int state;
struct rtp_header right_rtp_header, last_rtp_header, left_rtp_header;
unsigned short rtp_seq_offset = 0;
int rtp_time_offset = 0, rtp_ssrc;
int seamless_switch_flag = 0;
int rtp_diff = 0;
int rtp_started = 0;
int time_diff, last_time;
struct timeval wall_clock_time;
double recv_rtp_wall_clock;

enum state {
SOCKET_ACTIVE = 200,
NEW_SOCKET_READY,
RTSP_CONNECT_IN_PROGRESS
};

void update_rtp_started(char pkt[])
{
	unsigned char *tmp;

	tmp = (unsigned char*) pkt;

	printf("%x %x %x %x %x %x %x %x %x %x %x %x\n", tmp[0], tmp[1], tmp[2],
	       tmp[3], tmp[4], tmp[5], tmp[6], tmp[7], tmp[8], tmp[9], tmp[10],
	       tmp[11]);
	if (tmp[4] == 0x80 && ((tmp[5] == 0xe0) || (tmp[5] == 0x60))) {
		printf("RTSP session established....Streaming data on RTP \n");
		rtp_started = 1;
	}

}

void *rtsp_vlc()
{

	struct sockaddr_in servaddr;
	int optval = 1, ret, vlc_listen;

	while (1) {

		if (!real_network_view) {
			sleep(1);
			continue;
		}
		break;
	}

	vlc_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (vlc_sockfd <= INVALID_SOCK) {
		perror("sock error: open in vlc \n");
	}

	setsockopt(vlc_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval,
		   sizeof(optval));
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(554);

	ret = bind(vlc_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if (ret < 0) {
		perror(" Bind failed in rtsp_vlc start \n");
	}
	vlc_listen = listen(vlc_sockfd, LISTEN_QUEUE);
	if (vlc_listen < 0) {
		printf(" rtsp_vlc: listen failed errno = %d ", errno);
	}

	while (1) {
		printf
		    (" rtsp_vlc started and waiting for conn acceptence :- \n");
		vlc_connsock =
		    accept(vlc_sockfd, (struct sockaddr *)NULL, (socklen_t *)NULL);
		if (vlc_connsock < 0) {
			perror("rtsp_vlc accept error:...\n");
		}

		printf(" rtsp_vlc- Client Connection is received ...\n");
		vlc_serversock = vlc_server_conn();
		if(vlc_serversock < 0){
			printf("could not connect to remote server \n");
			return 0;
		}
		state = SOCKET_ACTIVE;
		if (pthread_create(&local_vlc, NULL, localvlc, &vlc_connsock)) {
			printf(" error creating localvlc thread \n");
			abort();
		}

		if (pthread_create
		    (&vlc_server, NULL, vlcserver, &vlc_serversock)) {
			printf(" Error creating vlc_server thread \n");
			abort();
		}
#if 0
		while(1){
		sleep(30);
		printf("%s\n",rtsp_teardown);
		reconnect();
		}
		if (pthread_create
		    (&rtcp_ltor_thread, NULL, rtcp_ltor_conn, NULL)) {
			printf(" error creating rtcp_ltor_thread thread \n");
			abort();
		}
#endif
		pthread_join(local_vlc, 0);

	}
}

int vlc_server_conn()
{
	int server_cfd, optval = 1;
	struct sockaddr_in rem_addr, ip_addr;
	int select_err;
	struct timeval tv;
	fd_set rfds;

	bzero(&rem_addr, sizeof(rem_addr));
	rem_addr.sin_family = AF_INET;
	rem_addr.sin_addr.s_addr = inet_addr("202.141.1.20");
	rem_addr.sin_port = htons(554);

	bzero(&ip_addr, sizeof(ip_addr));
	ip_addr.sin_family = AF_INET;
	ip_addr.sin_addr.s_addr = inet_addr(new_IPaddr);
	printf("the new socket is binding to %s\n", new_IPaddr);

	server_cfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_cfd <= INVALID_SOCK) {
		printf("video server : sock errno = %d \n", errno);
	}

	setsockopt(server_cfd, SOL_SOCKET, SO_REUSEADDR, &optval,
		   sizeof(optval));

	if (bind(server_cfd, (struct sockaddr *)&ip_addr, sizeof(ip_addr)) < 0) {
		perror("bind to current ipinterface failed! \n");
	}
	printf("Connecting to remote video server...\n");
#if 0
	fcntl (server_cfd, F_SETFL, O_NONBLOCK);
        FD_ZERO (&rfds);
	FD_CLR(server_cfd, &rfds);
	FD_SET(server_cfd, &rfds);
	tv.tv_sec = 3;
	tv.tv_usec = 0;

	if (connect(server_cfd, (struct sockaddr *)&rem_addr, sizeof(rem_addr))
	    < 0) {
		perror("remote vlc server connect error.. \n");
		vlc_server_conn();
		return -1;
	}

	select_err = select(server_cfd + 1, &rfds, NULL, NULL, &tv);

	if (select_err < INVALID_SELECT) {
		perror("select error in vlc_server_conn:\n");
		
	}

       if (FD_ISSET(vlc_serversock, &rfds)) {
	printf(" remote vlc server connection done = %d sfd \n", server_cfd);
	return server_cfd;
       }

	perror("Could not connect to remote Camera");
        return -1;
#endif
	if (connect(server_cfd, (struct sockaddr *)&rem_addr, sizeof(rem_addr))
	    < 0) {
		perror("remote vlc server connect error.. \n");
		vlc_server_conn();
		return -1;
	}
	return server_cfd;

}

/* @param thread to receive data from local vsa
 * sending to remote server 
 *
 *
 */
void *localvlc()
{
	int right_send_len = 0;
	int local_recv_len = 0;
	char buf[PKT_BUFSIZ];
	fd_set wfds;

	while (1) {
//      printf ("In localvlc waiting to receive\n");
		local_recv_len = recv(vlc_connsock, buf, PKT_BUFSIZ, 0);
		store_handshake(buf, local_recv_len);

		if (!strncasecmp(buf, "get_parameter", 13)) {
			if (strlen(rtsp_new_id) != 0) {
				replace_new_session_id(buf);
			}
		}
		//printf ("Received %d bytes\n", local_recv_len);
//      printf ("local ::%s\n", buf);

		right_send_len = send(vlc_serversock, buf, local_recv_len, 0);

		if (!strncasecmp(buf, "TEARDOWN", 8)) {
			stop_streaming(buf);
		}

		if (right_send_len <= INVALID_LEN) {
			close(vlc_serversock);
			FD_CLR(vlc_serversock, &wfds);
			perror(" send failed in localvlc:\n");
			printf
			    (" failed in localvlc & vlc_serverSock = %d\n errno = %d",
			     vlc_serversock, errno);
			continue;
		}
		//  }
	}

}

void rtp_packet_handle (char buf[], int len)
{
if (!rtp_started) {
          update_rtp_started(buf);
         }
if (rtp_started == 1) {
         save_rtp_header((unsigned char* )buf, len);
        }
}

void send_play_cmd(int vlc_serversock_new)
{
int right_send_len, right_recv_len;
char buf[PKT_BUFSIZ];

printf("play packet being sent : %s\n", rtsp_play);

        right_send_len =
            send(vlc_serversock_new, rtsp_play, strlen(rtsp_play), 0);
        if (right_send_len < 0) {
                perror("send play failed");
        }
        printf("play packet send\n");

        right_recv_len = recv(vlc_serversock_new, buf, PKT_BUFSIZ, 0);
        if (right_recv_len < 0) {
                perror("recv play ack failed");
        }
}
/*
 *
 *
 *
 */
void *vlcserver()
{
	char buf[PKT_BUFSIZ];
	int server_send_len = 0, server_receive_len = 0;
	int my_vlc_network_view = 0;
	int select_err;
	struct timeval tv;
	fd_set rfds;
#if 0
	/* Waiting for n/w info from SeaMo */
	while (1) {

		if (!real_network_view) {
			sleep(1);
			continue;
		}
		break;
	}

#endif
	my_vlc_network_view = real_network_view;
	while (1) {
		/* If vnc n/w view is not equal real n/w view then enable busy flag 
		 * and establish a new connection 
		 */
		if (my_vlc_network_view != real_network_view) {
			seamless_busy = 1;
			//vlc_serversock = vlc_server_conn ();
			// seamless_conn();
			if (state == SOCKET_ACTIVE) {
                           state = RTSP_CONNECT_IN_PROGRESS;
			if (pthread_create
			    (&rtsp_reconnect, NULL,(void *) reconnect, NULL)) {
				printf
				    (" Error while creating realtime video thread:- \n");
				// abort();
			     
			}
			my_vlc_network_view = real_network_view;
                       }

		}
                

                fcntl (vlc_serversock, F_SETFL, O_NONBLOCK);
                FD_ZERO (&rfds);
		FD_CLR(vlc_serversock, &rfds);
		FD_SET(vlc_serversock, &rfds);
		tv.tv_sec = 0;
		tv.tv_usec = 500000;

		select_err = select(vlc_serversock + 1, &rfds, NULL, NULL, &tv);

		if (select_err < INVALID_SELECT) {
			perror("select error in vlc:\n");
		}

		if (FD_ISSET(vlc_serversock, &rfds)) {
			server_receive_len =
			    recv(vlc_serversock, buf, PKT_BUFSIZ, 0);
			if (server_receive_len <= 0) {
				perror("recv error in vlcserver..\n");
				continue;
			}

			if (strstr(buf, "Session") != NULL) {
				if (strlen(rtsp_old_id) != 0) {
					replace_old_session_id(buf);
				}
			}

                        rtp_packet_handle(buf, server_receive_len);

			server_send_len =
			    send(vlc_connsock, buf, server_receive_len, 0);
			if (server_send_len < 0) {
				perror("send error in vlcserver..\n");
			}
		} else {

             //   printf ("vlc SERVER SOCKET TIMEOUT !\n");
                if (state == NEW_SOCKET_READY) {
                 printf ("****************NEW_SOCKET_READY old %d new %d\n", vlc_serversock, vlc_serversock_new);
                 // send_play_cmd (vlc_serversock_new);
                 close (vlc_serversock);
                 vlc_serversock = vlc_serversock_new;
	         seamless_switch_flag = 1;
                 state = SOCKET_ACTIVE;
                }

		}
	}
}

void get_rtsp_port_no(char req[])
{

	char *portnums;
	unsigned short p1, p2;

	portnums = strstr(req, "Transport");
	if (portnums == NULL) {
		return;
	}

	printf("detected setup packet\n");

	printf("^^^^^^^^^^^^^^^^^^%s\n", portnums);
	portnums = strstr(req, "client_port=");
	sscanf(portnums, "client_port=%hu-%hu", &p1, &p2);
	rtcp_client_port = p2;
	printf("CLIENT PORT = %d\n", rtcp_client_port);

	portnums = strstr(req, "server_port=");
	sscanf(portnums, "server_port=%hu-%hu", &p1, &p2);
	printf("p1=%d p2=%d\n", p1, p2);
	rtcp_server_port = p2;
	printf("SERVER PORT = %hu\n", rtcp_server_port);

	if (pthread_create(&rtcp_ltor_thread, NULL, rtcp_ltor_conn, NULL)) {
		printf(" error creating rtcp_ltor_thread thread \n");
		abort();
	}

}

void stop_streaming()
{
	printf("Killing all the threads\n");
	rtp_seq_offset = 0;
	bzero(rtsp_new_id, sizeof(rtsp_new_id));
	bzero(rtsp_old_id, sizeof(rtsp_new_id));
	close(vlc_serversock);
	close(rtp_remote);
	close(rtp_local);
	close(rtcp_local);
	close(rtcp_remote);
	pthread_cancel(local_vlc);
	pthread_cancel(vlc_server);
	// pthread_cancel (rtcp_ltor_thread);

}

void *rtcp_ltor_conn()
{

	int iresult, optval = 1;
	struct sockaddr_in remote_addr, local_addr, src_addr, right_addr;
	unsigned char recvd_pkt[PKT_BUFSIZ];
	socklen_t src_addr_len = sizeof(struct sockaddr_in);

	//rtcp_send_port = rtcp_recv_port + 1;

	rtcp_remote = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (rtcp_remote == -1) {
		printf("rtcp_remote failed: %s", strerror(errno));
	}

	rtcp_local = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (rtcp_local == -1) {
		printf("rtcp_local failed: %s", strerror(errno));
	}
//      setsockopt(rtcp_local, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	setsockopt(rtcp_remote, SOL_SOCKET,
		   SO_REUSEADDR, &optval, sizeof(optval));

	bzero(&local_addr, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = INADDR_ANY;	//inet_addr("127.0.0.1");
	local_addr.sin_port = htons(rtcp_server_port);	//htons(*port);

	bzero(&remote_addr, sizeof(remote_addr));
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = inet_addr("202.141.1.20");
	remote_addr.sin_port = htons(rtcp_server_port);	//htons(*myport);

	bzero(&right_addr, sizeof(right_addr));
	right_addr.sin_family = AF_INET;
	right_addr.sin_addr.s_addr = INADDR_ANY;	//inet_addr("202.141.1.20");
	right_addr.sin_port = htons(rtcp_client_port);	//htons(*myport);

	printf("*************RTCP local binding to %d\n", rtcp_server_port);
	if (bind
	    (rtcp_local, (struct sockaddr *)&local_addr,
	     sizeof(struct sockaddr)) < 0) {
		perror("RTCP Left Bind");
		exit(1);
	}
	while (1) {
		iresult =
		    recvfrom(rtcp_local, recvd_pkt,
			     PKT_BUFSIZ, 0,
			     (struct sockaddr *)&src_addr, &src_addr_len);
		if (iresult == -1) {
			//printf("RCTP recvfrom failed: %s\n", strerror(errno));
			perror("RTCP recvfrom");
		}
		printf("Received RTCP packet");

		if (recvd_pkt[1] == 201) {
			printf("Receiver Report Caputed!\n");
			capture_ssrc(recvd_pkt);
		}
		iresult =
		    sendto(rtcp_remote, recvd_pkt,
			   iresult, 0, (struct sockaddr *)
			   &remote_addr, sizeof(struct sockaddr_in));
		if (iresult <= 0) {
			//printf("RCTP sendto failed: %s\n", strerror(errno));
			perror("RTCP sendto");
		}
		printf("Sent RTCP packet\n");
	}
}

void store_handshake(char packet[], int len)
{

	if (!strncasecmp(packet, "options", 7)) {
		memset(rtsp_options, 0, sizeof(rtsp_options));
		memcpy(rtsp_options, packet, len);
		printf("options packet saved\n");
		return;
	}
	if (!strncasecmp(packet, "describe", 8)) {
		memset(rtsp_describe, 0, sizeof(rtsp_describe));
		memcpy(rtsp_describe, packet, len);
		printf("describe packet saved\n");
		return;
	}
	if (!strncasecmp(packet, "setup", 5)) {
		memset(rtsp_setup, 0, sizeof(rtsp_setup));
		memcpy(rtsp_setup, packet, len);
		printf("setup packet saved\n");
		return;
	}
	if (!strncasecmp(packet, "play", 4)) {
		memset(rtsp_play, 0, sizeof(rtsp_play));
		memcpy(rtsp_play, packet, len);
		if (strlen(rtsp_old_id) == 0) {
			printf
			    ("!!!!!!!!!!!!!!! Saved the first Session ID....\n");
			save_old_session_id(rtsp_play);
		}
		printf("play packet saved\n");
		return;
	}
}

void reconnect()
{

	char buf[PKT_BUFSIZ];
	int iresult, right_send_len, right_recv_len;

	/*set rtp started =0 */
	//rtp_started = 0;
	//

	vlc_serversock_new = vlc_server_conn();

        if (vlc_serversock_new < 0) {
           state = SOCKET_ACTIVE;
           return;
        }

	printf
	    ("second connection to video server established!!! %d-newsock",
	     vlc_serversock_new);

	right_send_len =
	    send(vlc_serversock_new, rtsp_options, strlen(rtsp_options), 0);
	if (right_send_len < 0) {
		perror("send options failed");
	}
	printf("options packet send\n");

	right_recv_len = recv(vlc_serversock_new, buf, PKT_BUFSIZ, 0);
	if (right_recv_len < 0) {
		perror("recv options ack failed");
	}

	right_send_len =
	    send(vlc_serversock_new, rtsp_describe, strlen(rtsp_describe), 0);
	if (right_send_len < 0) {
		perror("send describe failed");
	}
	printf("describe packet send\n");

	right_recv_len = recv(vlc_serversock_new, buf, PKT_BUFSIZ, 0);
	if (right_recv_len < 0) {
		perror("recv describe ack failed");
	}

	// send(vlc_connsock, buf, right_recv_len, 0);

	right_send_len =
	    send(vlc_serversock_new, rtsp_setup, strlen(rtsp_setup), 0);
	if (right_send_len < 0) {
		perror("send setup failed");
	}
	printf("setup packet send\n");

	right_recv_len = recv(vlc_serversock_new, buf, PKT_BUFSIZ, 0);
	if (right_recv_len < 0) {
		perror("recv setup ack failed");
	}

	     iresult =  send(vlc_serversock, rtsp_teardown, strlen(rtsp_teardown), 0);
	         if (iresult <= 0) {
		//printf("RCTP sendto failed: %s\n", strerror(errno));
		        perror("Teardown sendto");
	        }

	save_new_session_id(buf);
	replace_new_session_id(rtsp_play);

	printf("play packet being sent : %s\n", rtsp_play);
        send_play_cmd (vlc_serversock_new);

	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!Teardown sent expecting the streaming to stop\n");
        state = NEW_SOCKET_READY;
	printf("state is NEW_SOCKET_READY\n");
}

void save_old_session_id(char reply[])
{

	char *tmp;

	tmp = strstr(reply, "Session");
	if(tmp){
		tmp = tmp + 9;

		memcpy(rtsp_old_id, tmp, 8);
		printf("!!!!!!!! First Session ID =%s\n", rtsp_old_id);
	
		construct_teardown(rtsp_old_id);
	}
}

void save_new_session_id(char reply[])
{

	char *tmp = NULL;

	tmp = strstr(reply, "Session");
	if (tmp) {

//      offset = rtsp_play - tmp - 9;
		tmp = tmp + 9;

		memcpy(rtsp_new_id, tmp, 8);
		printf("@@@@@@@@@@@@ new session id =%s\n", rtsp_new_id);

		construct_teardown(rtsp_new_id);
	}

}

void replace_old_session_id(char change_pkt[])
{

	char *tmp;

	tmp = strstr(change_pkt, "Session");

	tmp = tmp + 9;

	memcpy(tmp, rtsp_old_id, 8);

}

void replace_new_session_id(char change_pkt[])
{

	char *tmp;

	tmp = strstr(change_pkt, "Session");

	tmp = tmp + 9;

	memcpy(tmp, rtsp_new_id, 8);

}

void capture_ssrc(unsigned char recv_rpt[])
{

	unsigned char *tmp = (unsigned char*)recv_rpt;

	tmp = tmp + 4;

	memcpy(rtsp_ssrc, tmp, 4);

	printf("session_id =  %02X%02X%02X%02X\n ",
	       rtsp_ssrc[0], rtsp_ssrc[1], rtsp_ssrc[2], rtsp_ssrc[3]);
}

#if 0
void save_header(char rtp_packet[])
{

	char *tmp;

	tmp = rtp_packet;

	tmp = tmp + 4;

	if (tmp == 0x80) {
		printf("rtp packet detected \n");
		tmp = (struct rtp_header *)tmp;

		memcpy(&right_header, tmp, sizeof(struct rtp_header));
		printf("header saved");
	}
}
#endif
void save_rtp_header(unsigned char rtp_packet[], int pkt_len)
{

	unsigned char *tmp;
	unsigned short right_seq_no;
	int right_timestamp, i = 0;


        for (i=0; i < pkt_len - 12; i++) {
              if ((rtp_packet[i] == 0x24) && (rtp_packet[i + 1] == 0x00)) {
                tmp = &rtp_packet[i];
  /*              printf("%x %x %x %x %x %x %x %x %x %x %x %x\n", tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5],tmp[6],tmp[7],tmp[8],tmp[9],tmp[10], tmp[11]);
*/
		if (tmp[4] == 0x80 && ((tmp[5] == 0xe0) || (tmp[5] == 0x60))) {

			memcpy(&right_rtp_header, tmp + 4,
			       sizeof(struct rtp_header));

			if (seamless_switch_flag
			    &&
			    (abs
			     (ntohs(right_rtp_header.seq_no) -
			      last_rtp_header.seq_no) > 100)) {
				seamless_switch();
				seamless_switch_flag = 0;
			}

			right_seq_no = last_rtp_header.seq_no =
			    ntohs(right_rtp_header.seq_no);
			right_timestamp = last_rtp_header.time_stamp =
			    ntohl(right_rtp_header.time_stamp);
			last_rtp_header.ssrc = ntohl(right_rtp_header.ssrc);

/*
		printf("Incoming parameters Seq %d \t RTP TS %lu \t Wall CLK %lf\n", 
                                      right_seq_no, right_timestamp,recv_rtp_wall_clock);
*/

			right_rtp_header.seq_no = right_seq_no + rtp_seq_offset;
			left_rtp_header.seq_no = right_rtp_header.seq_no;

			right_rtp_header.time_stamp =
			    right_timestamp + rtp_time_offset;
			left_rtp_header.time_stamp =
			    right_rtp_header.time_stamp;

			time_diff = last_time - right_rtp_header.time_stamp;
			last_time = right_rtp_header.time_stamp;

			/*      if(abs(time_diff) > 3500){
			   printf("time diff = %d rtp_ts = %lu recv_wall_clock %lf\n", time_diff, recv_rtp_wall_clock);
			   }
			 */
			right_rtp_header.seq_no =
			    htons(right_rtp_header.seq_no);
			right_rtp_header.time_stamp =
			    htonl(right_rtp_header.time_stamp);

			memcpy(tmp + 6, &right_rtp_header.seq_no,
			       sizeof(unsigned short));
			memcpy(tmp + 8, &right_rtp_header.time_stamp,
			       sizeof(unsigned int));
			if (rtp_ssrc) {
				memcpy(tmp + 12, &rtp_ssrc,
				       sizeof(unsigned int));
			}
/*

                printf("Outgoing parameters %d-seq\t", ntohs (right_rtp_header.seq_no));	
                printf("TS %lu\n",ntohl(right_rtp_header.time_stamp));
*/
#if 0
			if (rtp_diff <= ntohs(pkt_size)) {
				printf
				    ("#####################The remaining packet cannot be accommodated! %d %d\n",
				     rtp_diff, ntohs(pkt_size));
				break;	/* */
			}
#endif
		} else {
//			printf("Non rtp packet handle \n");
		}
	}
      }
}

void seamless_switch()
{
	struct timeval now;
	double now_sec;

	gettimeofday(&now, NULL);
	now_sec = now.tv_sec + (now.tv_usec / 1000000.0);

	rtp_seq_offset =
	    (left_rtp_header.seq_no - ntohs(right_rtp_header.seq_no));
	rtp_time_offset = (left_rtp_header.time_stamp - ntohl(right_rtp_header.time_stamp));	// + 3000;
	if (!rtp_ssrc) {
		rtp_ssrc = last_rtp_header.ssrc;
	}
	printf
	    ("Seamless SWitch ********** Offset %d-seq %d-time, absolute_diff %lf \n",
	     rtp_seq_offset, rtp_time_offset, now_sec - recv_rtp_wall_clock);
	printf("rtp ssrc %d\n", rtp_ssrc);

}

void construct_teardown( char session_id[]){

sprintf(rtsp_teardown, "TEARDOWN rtsp://127.0.0.1:554/mpeg4/media.amp/ RTSP/1.0\r\nCSeq: 1579\r\nUser-Agent: LibVLC/2.0.9 (LIVE555 Streaming Media v2012.10.18)\r\nSession: %s\r\n\r\n",session_id);

}
