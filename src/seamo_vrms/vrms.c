#include "vrms.h"
  
  int   vnc_connSock, vnc_serverSock, vnc_sockfd, one_time_flag = 0;
  int   recv_len_ack,recv_len_new = 0, content_len, string_len_umts;
  int real_network_view = 0, seamless_busy = 0;

  char  same_invite_buff[PKT_BUFSIZ], ack_resume_invite_pkt[PKT_BUFSIZ], send_buffer[1500];
  char *newstr_ack = NULL,*newstr_invite = NULL;
  int   networkchanged = 0, Cseq = 20, sip_server_sock, sip_client_sock, string_len_wifi = 13;
  char  old_IPaddr [128], new_IPaddr [128];
  int   cseq, result_ok, prev_network = 1, updated_sockfds;
  char  sack_seq[] = "CSeq: 20 INVITE";
  int curr_sockfd, new_sockfd;
	
  struct sockaddr_in saddr_voip_client, saddr_voip_server, caddr;
  char packet_net_ip_interface[IF_NAMESIZE];
  char ip_interface[IF_NAMESIZE];

  struct iface_ip iface_ip_diff;

  pthread_t network_monitor;
  pthread_t vsa3_voip, network_change;
  pthread_t local_vnc, vnc_server,vnc_startthread, network_status, realtime_video, vlc;
  pthread_t ambulet_demo;
int main()
{
	
        printf("\n\n\n SeaMo + VRMS   =    SeaMoPlus running \n\n");
	if (pthread_create(&realtime_video, NULL, seamless_video, NULL))
	   {
		 printf(" Error while creating realtime video thread:- \n");
		 abort();
           } 
        if (pthread_create(&vnc_startthread, NULL, (void*)&vnc_start, NULL)) 
	  {
                 printf(" Error while  creating the vnc start thread:- \n");
                 abort();
          } 
    	if (pthread_create(&vsa3_voip, NULL, &voip_sip, NULL)) 
	 {
      	  	 printf(" Issue while creating `vsa3-voip` thread \n");
	      	 abort();
	 }
        if (pthread_create(&network_status, NULL, network_change_detection, NULL)) 
	 {
                printf(" Error while creating n/w change detection thread:- \n");
                abort(); 
	 }
        if (pthread_create(&vlc, NULL, rtsp_vlc, NULL)){
                printf(" Error while creating rtsp_vlc thread:- \n");
                abort();
        }
        if (pthread_create(&ambulet_demo, NULL, ambulet_demo_thread, NULL)){
                printf(" Error while creating rtsp_vlc thread:- \n");
                abort();
        }

        pthread_join(network_status, 0);
	
	return (1);
 }
/*
 *
 *
 *
 *
 */

int is_network_changed()
{

if (!strcmp (packet_net_ip_interface, ip_interface))
return 0;
else{
return 1;
 }
}

void vnc_start()
{
  	struct sockaddr_in servaddr;
	int optval = 1, ret, vnc_listen;

	vnc_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      	if(vnc_sockfd <= INVALID_SOCK) 
	 {
	    perror("sock error: open in vnc \n");
	 }

  	 setsockopt(vnc_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  	 bzero(&servaddr, sizeof(servaddr));
	 servaddr.sin_family = AF_INET;
   	 servaddr.sin_addr.s_addr =  inet_addr("127.0.0.1");
   	 servaddr.sin_port = htons(5800);

    	 ret = bind(vnc_sockfd,(struct sockaddr *)&servaddr, sizeof(servaddr));
	 if(ret < 0) 
	  {
              perror(" Bind failed in vnc start \n");
          }
    	 vnc_listen = listen(vnc_sockfd, LISTEN_QUEUE); 
	 if(vnc_listen < 0) 
	  {
	      printf(" vnc: listen failed errno = %d ", errno);
	  }
 	 printf("vnc-vsa is waiting for n/w info from SeaMo:- \n"); 

	/* Waiting for n/w info from SeaMo */
         while (1) 
 	  {
           if (!real_network_view) 
	    { 
                sleep (1); 
                continue;
            }
            break;
          }

  	  printf(" VNC started and waiting for conn acceptence :- \n");
		  vnc_connSock = accept(vnc_sockfd,(struct sockaddr *)NULL, (socklen_t *) NULL);
  	  if(vnc_connSock < 0) 
	   {
	       perror("vnc accept error:...\n");
	   }

   	      printf(" VNC- Client Connection is received ...\n");
    /* VRMS-VSA(VNC) Establishing connection to remote vnc server */

   	      vnc_serverSock = vncServer_conn();

          if(pthread_create(&local_vnc, NULL, localvnc, &vnc_serverSock ))
	   {
              printf(" error creating localvnc thread \n"); abort();
           } 

             if(pthread_create(&vnc_server, NULL, vncserver, &vnc_serverSock))
	      {
                   printf(" Error creating vnc_server thread \n"); abort();
	      }

          pthread_join(local_vnc, 0);

 }
/* @param thread to receive data from local vsa
 * sending to remote server 
 *
 *
 */
void *localvnc()
{
	  int right_send_len = 0;
	  int local_recv_len = 0, select_err; 
          char buf[PKT_BUFSIZ];
          double t1, t2;
	  struct timeval time, tv;
	  fd_set wfds;

          while(1) 
           {
                local_recv_len = recv(vnc_connSock, buf, PKT_BUFSIZ, 0);
                if(seamless_busy == 1) 
		 {
                      printf(" seamless_busy flag is up in local .....wait fd : %d, len : %d\n",
                      vnc_serverSock, local_recv_len);
                      usleep(10000);
                      continue;
                 }
		if(local_recv_len == 10) 
		  {
		 /* @param  prev_network is enabled just before handoff to new n/w 
	          * Discarding if any frame buffer update request sending to remote 
		  * just before handoff bcz client is expecting frame in old n/w which 
                  * not available 
                   */
		    if(prev_network == 0) 
		      {
			 prev_network = 1;
			 printf(" FRAMEBUFF UPDATE REQ received just before handoff = %d \n", local_recv_len);
			 continue;
		      }
			gettimeofday(&time, NULL);
			t1 = time.tv_sec+(time.tv_usec/1000000.0);
			printf("  Time t1-t2 = %.6lf seconds & currnet time t1 = %.6lf", t1-t2, t1);
		        t2 = t1;
		  }
                //fcntl (vnc_serverSock, F_SETFL, O_NONBLOCK);

                FD_CLR (vnc_serverSock, &wfds);
                FD_SET (vnc_serverSock, &wfds);
                tv.tv_sec = 0;
                tv.tv_usec = 100000;
                /* If timeout is NULL (no
        	    timeout), select() can block indefinitely
		  */
                select_err = select (vnc_serverSock + 1, NULL, &wfds, NULL, &tv); 
		if(select_err < INVALID_SELECT) 
		 {
		     printf(" vnc: select errno = %d \n", errno);	
		     perror("select error in vnc:\n");
                 }
                if(FD_ISSET(vnc_serverSock, &wfds)) 
		{
		// printf(" localvnc: Sending to right side server fd : %d len : %d\n",
                          //vnc_serverSock, local_recv_len);
                    right_send_len = send(vnc_serverSock, buf, local_recv_len, 0);

		    if(right_send_len <= INVALID_LEN)
		      {
			 close(vnc_serverSock);
			 FD_CLR(vnc_serverSock, &wfds);
		         perror(" send failed in localvnc:\n");
		         printf(" failed in localvnc & vnc_serverSock = %d\n errno = %d", vnc_serverSock, errno);
			 continue;
                      }
                }
	  }         

}
/*
 *
 *
 *
 */
void *vncserver()
{
        char buf [PKT_BUFSIZ];
        int server_send_len =0, server_receive_len = 0; 
	int my_vnc_network_view = 0;
	int select_err;
	struct timeval tv;
        fd_set rfds;

	/* Waiting for n/w info from SeaMo */
         while (1) 
	 {
	   
           if (!real_network_view)
	    {
                sleep (1); 
                continue;
            }
            break;
         }
          
         my_vnc_network_view = real_network_view;

         while(1) 
	  {
	   /* If vnc n/w view is not equal real n/w view then enable bussy flag 
            * and establish a new connection 
            */
            if(my_vnc_network_view != real_network_view) 
             {
                 seamless_busy = 1;
		 vnc_serverSock = vncServer_conn();
		 if(vnc_serverSock < 0){
			printf("Could not connect to vnc server\n");
			continue;
		 }
	         seamless_conn();
                 my_vnc_network_view = real_network_view;
             }

                 fcntl (vnc_serverSock, F_SETFL, O_NONBLOCK);

                 FD_CLR (vnc_serverSock, &rfds);
                 FD_SET (vnc_serverSock, &rfds);
                 tv.tv_sec = 0;
                 tv.tv_usec = 100000;
                /* If timeout is NULL (no
        	    timeout), select() can block indefinitely
		  */
                select_err = select (vnc_serverSock + 1, &rfds, NULL, NULL, &tv);
		if(select_err < INVALID_SELECT) 
                 {
		       printf(" vnc: select errno = %d \n", errno);	
                 }
                if (FD_ISSET (vnc_serverSock, &rfds)) 
                 {

                    //printf("vncserver : server fd %d before recv\n", vnc_serverSock);
                    server_receive_len = recv(vnc_serverSock, buf, PKT_BUFSIZ, 0);
		    if(server_receive_len <= 0)
		     {
		         close(vnc_serverSock);
		         FD_CLR(vnc_serverSock, &rfds);
		         perror("recv error in vncserver..\n");
		         printf (" recv fd vncserver : vnc_server : %d\n", vnc_serverSock);
			 continue;
		      } 
	
                    //printf("vncserver : server fd %d after recv\n", vnc_serverSock);

                    server_send_len = send(vnc_connSock, buf, server_receive_len, 0);
                      if(server_send_len < 0) 
                       {
                          perror ("send error in vncserver..\n");
		          printf ("vncserver : vnc_server : %d\n", vnc_serverSock);
                       }

                  //  printf(" From Right side fd : %d sent to Left & pkt len = %d \n", vnc_serverSock, server_receive_len);

                }
       }
}
/*
 *
 *
 *
 */
int vncServer_conn()
{
      int server_cfd;
      struct sockaddr_in addr;

           server_cfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	   bzero(&addr, sizeof(addr));
           addr.sin_family = AF_INET;
           addr.sin_addr.s_addr = inet_addr("202.41.124.51");
           addr.sin_port = htons(5900);

           if(connect(server_cfd, (struct sockaddr *)&addr, sizeof(addr)) <0) 
            {
            	 perror("remote vnc server connect error.. \n");
            	 vncServer_conn();
             	 return -1;
            }
           printf(" remote vnc server connection done = %d sfd \n", server_cfd);
  
     return server_cfd;
}
/*
 *
 *
 *
 */
void seamless_conn()
{
       char buf[PKT_BUFSIZ];
       int i,recv_len, total_len = 0;
 
       char authentication[] = { 0x01 };

       char sharedskflag[] = { 0x01 };

       char clientproversion[] ={
	0x52, 0x46, 0x42, 0x20, 0x30, 0x30,
        0x33, 0x2e, 0x30, 0x30, 0x38, 0x0a };

       char pixlformat[] = {
        0x00, 0x00, 0x00, 0x00, 0x08, 0x08,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

       char encodings[] = {
        0x02, 0x46, 0x00, 0x0c, 0x00, 0x00,0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x16, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00,
        0x00, 0x0f, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00,
        0x00, 0x05, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
        0x00, 0x00, 0xff, 0xff, 0xfe, 0xc9, 0xff, 0xff,
        0xff, 0x11, 0xff, 0xff, 0xff, 0x21 };

       char framebufupdate[] = {
         0x03, 0x01, 0x00, 0x00, 0x00, 0x00,
         0x05, 0x56, 0x03, 0x00 };

        printf(" Seamless Conn pkt are sending \n");

       recv_len = recv(vnc_serverSock, buf, PKT_BUFSIZ, 0);
                    if(recv_len < 0) 
		      {
                          perror(" recv error in handshake \n");
		      }
                   printf("Recived server protocol version ..recv fd = %d recv len = %d\n", vnc_serverSock, recv_len);
                   send(vnc_serverSock, clientproversion, sizeof(clientproversion), 0);
                   printf("sent client protocol version ..\n");
        recv_len = recv(vnc_serverSock, buf, PKT_BUFSIZ, 0);
                   printf(" recevived auth..from server .. recv fd = %d , recv len = %d\n", vnc_serverSock, recv_len);
                   send(vnc_serverSock, authentication, sizeof(authentication), 0);
                   printf("sent authentication from client ...\n");
        recv_len = recv(vnc_serverSock, buf, PKT_BUFSIZ, 0);
                   printf("recived one more pkt from server .. recv fd = %d  recv len = %d \n", vnc_serverSock, recv_len);
                   send(vnc_serverSock, sharedskflag, sizeof(sharedskflag), 0);
                   printf(" sent shared desktop flag ..\n");
                	for(i = 0; i< 17 ; i++) 
                        {
	                   recv_len = recv(vnc_serverSock, buf, PKT_BUFSIZ, 0);
	                   total_len += recv_len;
        	           if(total_len >= 30)
                	   break; 
			}
                   send(vnc_serverSock, encodings, sizeof(encodings), 0);
                   printf(" ENCODING PKTS SENT \n");

                   send(vnc_serverSock, pixlformat, sizeof(pixlformat), 0);
                   printf(" PIXEL FRMT SENT \n");
                // recv_len = recv(server_fd, buf, PKT_BUFSIZ, 0);

                   seamless_busy = 0;

                   send(vnc_serverSock, framebufupdate, sizeof(framebufupdate), 0);
                   printf(" FRAMEBUFUPDATE SENT \n");

}
/*
 *
 *
 *
 */

void *seamless_video()
{

        int sockfd, sockfd1 = 0, connSock;
        int optval = 1, recv_len, ret, send_len;
        int my_network_view, select_err, web_req_len;
        char buffer[BUFFERSIZE];
        struct sockaddr_in servaddr;
        struct timeval tv;
        fd_set rfds;

        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");;
        servaddr.sin_port = htons(8888);

        ret = bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if(ret < 0) 
         {
             perror(" Bind failed in video \n"); 
         }
        listen(sockfd, LISTEN_QUEUE);
	/* Waiting for n/w info from SeaMo */
	printf("video-vsa is waiting for n/w info from SeaMo:- \n");
         while (1) 
         {
           if (!real_network_view) 
            { 
                sleep (1); 
                continue;
            }
            break;
        }
       
        printf(" Video connection waiting for conn acceptence :- \n");	
        connSock = accept(sockfd, (struct sockaddr *)NULL,(socklen_t *) NULL);
        if(connSock == -1 || connSock < 0) 
         {
              perror("accept ..\n"); 
	 }
 
        printf(" Video Connection Received :- !!!!\n");
        //sockfd1 = ConnectToVideoServer(sockfd1);
        ConnectToVideoServer();
	updated_sockfds = 0;
	sockfd1 = new_sockfd;
        printf("Remote Video connectio established & camsockfd =  %d \n", sockfd1);
        recv_len = recv(connSock, buffer, BUFFERSIZE, 0);

/********* Storing initial handshake or HTTP GET method pkt in buffer for seamless conn establishment **********/

        memset(send_buffer, sizeof(send_buffer), '\0');
        memcpy(send_buffer, buffer, recv_len);

        web_req_len = recv_len;

        if(recv_len <= 0) 
         {
            printf("received =  %d bytes from browser \n", recv_len);
         }

        send(sockfd1, send_buffer, recv_len, 0);
        my_network_view = real_network_view;

		/* IP Video camera
		 * Once the initial GET method pkt sent from browser, then onwards 
		 * the process of browser is just to receive pkt's from camera 
		 * nothing need to be send back to camera except ack pkt's 
		 */

        while (1) 
         {
//		printf(" while 1 my_network_view = %d , real_network_view = %d \n", my_network_view, real_network_view);
		
            if (my_network_view != real_network_view) 
             {

		     /* Enable only IPv6 over IPv4 using 6to4 tunnel & make sure that 6to4.sh 
			 * file is there in same dir *******/

	       	my_network_view = real_network_view;
		printf("%d my network view %d real network view\n",my_network_view,real_network_view);
		updated_sockfds = 0;
		if (pthread_create(&realtime_video, NULL,(void *) ConnectToVideoServer, NULL))
			{
                	    printf(" Error while creating realtime video thread:- \n");
              		   // abort();
          	 }
            } 
		if(updated_sockfds){
//                sockfd2 = ConnectToVideoServer(sockfd1);
	          printf("Network change detected in video thread, new conn established:- \n");
                                 // close(sockfd1);
                  printf("New Connection is Done OLD Connection gone seamlessly :):) !!!!!\n");
                  sockfd1 = new_sockfd; 
                  FD_ZERO (&rfds);
               	  send_len = send(sockfd1, send_buffer, web_req_len, 0);
		  if( send_len <= 0)
                    {
		        perror("send error \n");
                        continue;
                    }       
		  updated_sockfds = 0;
                }
                FD_CLR (sockfd1, &rfds); 
                FD_SET (sockfd1, &rfds);
                tv.tv_sec = 0;
                tv.tv_usec = 100000;

                select_err = select (sockfd1 +1, &rfds, NULL, NULL, &tv);
                if(select_err < INVALID_SELECT) 
                 {
		      printf(" video : select errno = %d \n", errno);
		      perror(" In video thread  select error:- \n");
		 }
                if (FD_ISSET (sockfd1, &rfds)) 
                 {
                
		//printf(" FD_ISSET my_network_view = %d , real_network_view = %d \n", my_network_view, real_network_view);

                   recv_len = recv(sockfd1, buffer, BUFFERSIZE, 0);
                   recv_len = send(connSock, buffer, recv_len, 0);
                }
      }
}

/************************** IPv6 version of video connection establishment function ********************/

#if 0
int ConnectToVideoServer()
{
        int  sockfd6;
        struct sockaddr_in6 server;
        socklen_t addr_len;

        server.sin6_family = AF_INET6;
        server.sin6_port = htons(80);

       /* iisc ipv6 if( !inet_pton(AF_INET6, "2001:e30:1c01:1:240:8cff:fea8:bbe", &(server.sin6_addr))) */

           if( !inet_pton(AF_INET6, "2001:e30:1c03:1:240:8cff:fea8:bbe", &(server.sin6_addr)))
             {
           printf(" Invalid IPv6 Address\n");
                 }
          addr_len = sizeof(server);

           sockfd6 = socket(AF_INET6, SOCK_STREAM, 0);
           if(connect(sockfd6, (struct sockaddr *)&server, sizeof(server)) < 0) {
                printf(" IPv6 Connect Error !!!!!!! \n");
                 //exit(0);
                 }
        fcntl (sockfd6, F_SETFL, O_NONBLOCK);
        return sockfd6;
}
#endif

/************************** IPv4 version of video connection establishment function ********************/

void ConnectToVideoServer()      
{
        int sockfd4, optval = 1;
        struct sockaddr_in server, ip_addr;
    	
	bzero(&server, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr("202.141.1.20");
        server.sin_port = htons(80);

	ip_addr.sin_family = AF_INET;
	ip_addr.sin_addr.s_addr = inet_addr(new_IPaddr);
	printf("the new socket is binding to %s\n",new_IPaddr);
        
        sockfd4 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        setsockopt(sockfd4, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if(sockfd4 <= INVALID_SOCK) 
        {
	    printf("video server : sock errno = %d \n", errno);
	}

	if(bind(sockfd4,(struct sockaddr *)&ip_addr, sizeof(ip_addr)) < 0){
	    perror("bind to current ipinterface failed! \n");
	}

        printf ("Connecting to remote video server...\n");

        if(connect(sockfd4, (struct sockaddr *)&server, sizeof(server)) < 0) 
         {
		perror("Video connect:");
                close (sockfd4);
                return;
         }


        fcntl (sockfd4, F_SETFL, O_NONBLOCK);
	printf("ConnectToVideoServer Success!! sockfd4 =%d\n",sockfd4);
        new_sockfd = sockfd4;

	/* break the old connection after making a new one*/
	if(curr_sockfd){
                printf ("Closing old socket! %d\n", curr_sockfd);
		close(curr_sockfd);
	}
	
	updated_sockfds = 1;
}

/* Get the current IP address, to replace prev IP
 * with new IP in re-INVITE SIP packet and ACK packet
 */

void get_current_ipaddr(char *ip_interface)
{
 int fd;
 static int count;
 struct ifreq ifr;
// socklen_t len;

         fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
         if(fd <= INVALID_SOCK) 
         {
            perror(" Could not open get IP sock \n"); 
	    close(fd);
         }
         ifr.ifr_addr.sa_family = AF_INET;

/*         if(real_network_view == 2)
         strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ-1);
         else if(real_network_view == 3)
         strncpy(ifr.ifr_name, "ppp0", IFNAMSIZ-1);
            else printf(" Interface doesn't support to get IP Address ..\n");
*/
         strncpy(ifr.ifr_name,ip_interface, IFNAMSIZ-1);
//         len = sizeof(ifr);
         if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
          perror("Failed");
         }
          close(fd);
         if(count == 1)
          {
             strncpy(old_IPaddr, new_IPaddr, sizeof(old_IPaddr));
             printf("%s  in strncpy Old IP\n", old_IPaddr);
          }
         if(count == 0)
           {
            	printf (" Interface Address : %s\n", inet_ntoa( ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
            	strcpy (old_IPaddr,inet_ntoa( ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
           	count = 1;
                //if (old_IPaddr)
          	//printf("%s  count = 0 Old IP\n", old_IPaddr);
           }
        if(count >= 1)
	{
            	strcpy (new_IPaddr,inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
                // if (new_IPaddr)
           	// printf("%s count = 1  New IP\n", new_IPaddr);
        }
}


void *network_change_detection()
{
       int sock, recv_len, ret;
       struct sockaddr_in nw_server, nw_client;
       socklen_t length;
       char buffer[128];

       sock = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
       if(sock <= INVALID_SOCK)
       {
	   perror("Could not open n/w change sock \n"); close(sock);
       }
       memset(&nw_server, 0 , sizeof(nw_server));
       nw_server.sin_family = AF_INET;
       nw_server.sin_addr.s_addr = htonl(INADDR_ANY);
       nw_server.sin_port = htons(5678);

       ret = bind(sock,(struct sockaddr *)&nw_server, sizeof(nw_server));
       if(ret < 0) 
       {
          perror(" Bind failed in network-change \n");
	}
	   length = sizeof(nw_client);
	        while(1) 
		{
		   bzero(buffer,sizeof(buffer));
         	   recv_len = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&nw_client, &length);
	           if(recv_len <= 0)
		    {
	 	        perror(" Error recv data from n/w thread \n");
	       	    } 
                   strcpy(packet_net_ip_interface, buffer);
	 	   printf(" NW_TYPE = %s \n ", packet_net_ip_interface);
		  if (is_network_changed()){
                  strcpy (ip_interface,packet_net_ip_interface);
	          get_current_ipaddr(packet_net_ip_interface);
	     	  real_network_view++;
	    	  check_add_ip_rule(packet_net_ip_interface);	        

		  }
         	}	
  
}

void check_add_ip_rule(char *ip_interface){

	char command[200];
/*
	sprintf(command,"%s%s %s",SBIN_DIR,"ip_rules.sh",ip_interface);
        printf("concatinated command = %s \n",command);

        system(command);
*/
	if(!strcmp(ip_interface,"ppp0")){
		if(strcmp(iface_ip_diff.ppp0,new_IPaddr)){
	           sprintf(command,"%s%s %s",SBIN_DIR,"ip_rules.sh",ip_interface);
        	   system(command);
        	   printf("ip rule for ppp0 updated!!!\n");
		   strcpy(iface_ip_diff.ppp0,new_IPaddr);
		}
	}
	if(!strcmp(ip_interface,"ppp1")){
		if(strcmp(iface_ip_diff.ppp1,new_IPaddr)){
		   sprintf(command,"%s%s %s",SBIN_DIR,"ip_rules.sh",ip_interface);
        	   system(command);
        	   printf("ip rule for ppp1 updated!!!\n");
		   strcpy(iface_ip_diff.ppp1,new_IPaddr);
		}
	}
	if(!strcmp(ip_interface,"ppp2")){
		if(strcmp(iface_ip_diff.ppp2,new_IPaddr)){
		   sprintf(command,"%s%s %s",SBIN_DIR,"ip_rules.sh",ip_interface);
        	   system(command);
        	   printf("ip rule for ppp2 updated!!!\n");
		   strcpy(iface_ip_diff.ppp2,new_IPaddr);
		}
	}
	if(!strcmp(ip_interface,"wlan0")){
		if(strcmp(iface_ip_diff.wlan0,new_IPaddr)){
	           sprintf(command,"%s%s %s",SBIN_DIR,"ip_rules.sh",ip_interface);
        	   system(command);
        	   printf("ip rule for wlan0 updated!!!\n");
		   strcpy(iface_ip_diff.wlan0,new_IPaddr);
		}
	}

} 

/*
 *
 *
 *
 *
 */

void *voip_sip()
{
   int recv_len,send_len, bind_err;
   char buf [PKT_BUFSIZ];
   int invite =0, cseq =0;
   socklen_t len1, len;
   pthread_t fire_voip;

        sip_client_sock = socket(AF_INET,SOCK_DGRAM, IPPROTO_UDP);
        if(sip_client_sock < INVALID_SOCK)
         {
              perror(" Could not open vsa-sip sock \n");
         }
        bzero(&saddr_voip_server, sizeof(saddr_voip_server));

        saddr_voip_server.sin_family = AF_INET;
        saddr_voip_server.sin_addr.s_addr = inet_addr("127.0.0.1");
        saddr_voip_server.sin_port = htons(8887);

        bind_err = bind(sip_client_sock, (struct sockaddr *)&saddr_voip_server, sizeof(saddr_voip_server));
        if (bind_err < 0)
           perror("sfd_voip bind failed\n");
        printf("voip-vsa waiting for n/w info from SeaMo:- \n");
	/* Waiting for n/w info from SeaMo */
        while (1) 
 	 {
           if (!real_network_view) 
	    { 
                usleep (500000); 
                continue;
            }
            break;
          }

        sip_server_sock = fire_voip_server();

                if(pthread_create(&fire_voip, NULL, fire_voip_rs, NULL ))
                  {
                        printf("problem while creating the `fire voip` thread....\n");
                        abort();
                  }
                if(pthread_create(&network_monitor, NULL, network_monitor_task, NULL ))
                 {
                       printf("problem while creating the `fire voip` thread....\n");
                       abort();
                 }
		 len1 = sizeof(saddr_voip_client);
                 len  = sizeof(caddr);

               for(;;)
                {
                   recv_len = recvfrom(sip_client_sock, buf, PKT_BUFSIZ, 0, (struct sockaddr *)&caddr, &len);
                   if(recv_len < 0)
                     {
                           perror(" voip recv len error \n");
                     }

             if(cseq == 0)
              {
                if(strstr(buf, "CSeq: 20 ACK"))
                 {
                    memcpy(ack_resume_invite_pkt, buf, recv_len);
                    printf(" INITAIL ACK PKT  \n %s \n", ack_resume_invite_pkt);
                    cseq = 1;
                 }
              }

            if(invite == 0)
             {
               if(strstr(buf, "INVITE"))
                {
                        memcpy (same_invite_buff, buf, recv_len);
                        recv_len_new = recv_len;

                        printf(" recv_len = %d \t and strlen(buf) = %d \n", recv_len, (int) strlen(buf));
                        printf(" INITIAL INVITE PKT = \n  %s", same_invite_buff);
                        invite=1;
                }
             }
                        send_len = sendto(sip_server_sock, buf, recv_len, 0, (struct sockaddr *)&saddr_voip_client, len1);
                        if(send_len <= INVALID_LEN)
                         {
                             perror ("cfd_voip send");
                             switch (errno)
                              {

                                case ENETUNREACH:
                                           printf(" sendto: sip_server_sock %d errno \n", errno);
                                           printf(" no network coverage storing data in file \n");
                                           break;
                                case ENETDOWN:
                                           printf(" sendto: sip_server_sock %d errno \n", errno);
                                           printf(" network down storing data in file \n");
                                           break;
                                default: printf(" Error not matched: sendto sip_server_sock..\n");
                             }
                          }
             
		 }

}

/* vrms :- read from real-server and send to local client */

void *fire_voip_rs()
{
  int send_len, recv_len, only_once = 0;
  char buf[PKT_BUFSIZ];
  struct timeval tv;
  fd_set rfds;
  socklen_t len, len1;
  len = sizeof(saddr_voip_client);

       for(;;)
         {
                FD_ZERO (&rfds);
                FD_SET (sip_server_sock, &rfds);
                tv.tv_sec = 0;
                tv.tv_usec = 10000;
                select (sip_server_sock + 1, &rfds, NULL, NULL, &tv);

                if(FD_ISSET (sip_server_sock, &rfds))
                  {

                     recv_len = recvfrom(sip_server_sock, buf, PKT_BUFSIZ, MSG_NOBLOCK, NULL, NULL);
                     if( recv_len <= 0)
                      {
                         perror(" recv_len less than zero \n");
                         printf(" recv len is MSG_NOBLOCK = %d errno = %d \n", recv_len, errno);
                      }
                    if(strstr(buf, "200 OK"))
                    {
                         printf(" 200 OK PKT RECEIVED %s \n", buf);
                         printf(" SACK_SEQ is %s \n", sack_seq);
		    if(only_once > 1){
                    if(strstr(buf, sack_seq) != NULL)
                      {
                         recv_len_ack = strlen(newstr_ack);
                         send_len = sendto(sip_server_sock, newstr_ack, recv_len_ack,
                                                0, (struct sockaddr *)&saddr_voip_client, len);
                         if(send_len <= INVALID_LEN)
                            printf("Error in sendto = %d \n", errno);

                         printf(" Sent an ACK for re-INVITE  %s \t and strlen send len = %d \n", newstr_ack, send_len);
                      }
		    }
		   only_once = 1;
                    }
                          len1 = sizeof(caddr);
                          send_len = sendto(sip_client_sock, buf, recv_len, 0 , (struct sockaddr*)&caddr, len1);
                 }
            }

}
/* get the client socket discriptor */
int fire_voip_server()
{
    int sfd_voip;

         sfd_voip = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
         if(sfd_voip < INVALID_SOCK)
         perror("Couldn't open sip dest sock \n");

         bzero(&saddr_voip_client, sizeof(saddr_voip_client));
         saddr_voip_client.sin_addr.s_addr = inet_addr("202.41.124.55");
         saddr_voip_client.sin_port = htons(9876);

  return sfd_voip;
}

void *network_monitor_task()
 {
   return 0; 
   int sip_network_view = 0, i, j,result_len, re_invite_sendlen = 0;
   char *ptrdest_len;
   char content_len_add[5];
   socklen_t len1;

        len1 = sizeof(saddr_voip_client);
        sip_network_view = real_network_view;

        while(1)
         {
           if(sip_network_view != real_network_view)
            {
                    printf("Network change trigger \n");
                                                                    close(sip_server_sock);
                    sip_server_sock = fire_voip_server();
                    fcntl (sip_server_sock, F_SETFL, O_NONBLOCK);
                    get_current_ipaddr(ip_interface);
                    command_seq_change();
           if(real_network_view == 3)
            {
                    string_len_umts = string_len_umts * 2;
                    printf("string_len umts *2 = %d \n", string_len_umts);
                    content_len = 381 + string_len_umts;
                    printf("content_len umts + 381 = %d \n", content_len);
            }
             else if(real_network_view == 2)
              {
                    string_len_wifi = string_len_wifi * 2;
                    printf("string_len wifi *2 = %d \n", string_len_wifi);
                    content_len = 381 + string_len_wifi;
                    printf("content_len wifi + 381 = %d \n", content_len);
              }
                else printf(" Content-Length error \n");


                    snprintf(content_len_add, 5, "%d", content_len);
                    printf("content_len_add = %s \n", content_len_add);
                    ptrdest_len = strstr(same_invite_buff, "Content-Length:");
                    result_len = (int)(ptrdest_len - same_invite_buff + 1);
                    printf(" result_len = %d \n",  result_len);
                    for(i = result_len + 17, j =0; i <= result_len + 19; i++)
                     {
                         same_invite_buff[i] = content_len_add[j]; j++;
                         printf(" content_len_add in invite pkt = %c \n", same_invite_buff[i]);
                     }
                    newstr_invite = replace(same_invite_buff, old_IPaddr, new_IPaddr);
                    newstr_ack = replace(ack_resume_invite_pkt, old_IPaddr, new_IPaddr);
                    recv_len_new = strlen(newstr_invite);
                    re_invite_sendlen = sendto(sip_server_sock, newstr_invite, recv_len_new, 0, (struct sockaddr *)&saddr_voip_client, len1);
                    printf(" Sent a re-INVITE pkt %s \t and send len = %d \n", newstr_invite, re_invite_sendlen);
                    sip_network_view = real_network_view;
           }
         usleep(20000);
       }

 }


/* Generic function for replacing substring with new string */

char *replace(const char *s, const char *old, const char *new)
{
 char *ret, *sr;
 size_t i, count = 0;
 size_t newlen = strlen(new);
 size_t oldlen = strlen(old);

 if (newlen != oldlen) {
    for (i = 0; s[i] != '\0'; ) {
    if (memcmp(&s[i], old, oldlen) == 0)
            count++, i += oldlen;
      else
            i++;
             }
  } else
      i = strlen(s);

       ret = malloc(i + 1 + count * (newlen - oldlen));
       if (ret == NULL)
           return NULL;

         sr = ret;
      while (*s) {
 if (memcmp(s, old, oldlen) == 0) {
               memcpy(sr, new, newlen);
               sr += newlen;
               s += oldlen;
                   } else
                *sr++ = *s++;
                }
   *sr = '\0';

    return ret;
}
/* Generic function for removing substring with new string */

void removeSubstr(const char *src, const char *substr, char *target)
{
  /**
 *     * Use the strstr() library function to find the beginning of
 *           the substring in src; if the substring is not present, 
 *                 strstr returns NULL.
 *                             */
  char *start = strstr(src, substr);
  if (start)
  {
    /**
 *        * Copy characters from src up to the location of the substring
 *               */
    while (src != start) *target++ = *src++;
    /**
 *       * Skip over the substring
 *             */
    src += strlen(substr);
  //  printf(" %d substring length \n ", strlen(substr));
     }
       /**
           * Copy the remaining characters to the target, including 0 terminator
 */
  while ((*target++ = *src++)); // empty loop body;
}


void command_seq_change()
 {
     char sequence[] = "CSeq:";
     int result_ack,i, j, result_invite, k, l;
     char *ptrdest_ack, *ptrdest_invite, *ptrdest_sack;
     char  cseq_add[2];

        ptrdest_ack = strstr(ack_resume_invite_pkt, sequence);
        result_ack = (int)(ptrdest_ack - ack_resume_invite_pkt + 1);
        sscanf(ptrdest_ack + 6, "%d", &cseq);
        cseq ++;

        snprintf(cseq_add, 4, "%d", cseq);
        printf(" content of cseq_add is %s \n", cseq_add);
        for(i = result_ack + 5, j = 0; i <= result_ack + 6; i++)
          {
             ack_resume_invite_pkt[i] = cseq_add[j]; j++;
          }

        ptrdest_invite = strstr(same_invite_buff, "CSeq:");
        result_invite = (int)(ptrdest_invite - same_invite_buff + 1);

        for(k = result_invite + 5, l = 0; k <= result_invite + 6; k++)
          {
             same_invite_buff[k] = cseq_add[l]; l++;
          }

        ptrdest_sack = strstr(sack_seq, "CSeq:");
        result_ok = (int)(ptrdest_sack - sack_seq + 1);
        for(i = result_ok + 5, j = 0; i <= result_ok + 6; i++)
	  {
             sack_seq[i] = cseq_add[j]; j++;
          }
       puts(sack_seq);
 }

/* Subroutine for IP address change detection using IPC */

void networkchangedip()
{
    struct sockaddr_nl addr;
    struct nlmsghdr *nlh;
    int sock, len;
    char buffer[4096];

    if ((sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) == -1)
      {
          perror("couldn't open NETLINK_ROUTE socket");
//   return 1;

}

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_IPV4_IFADDR;

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
     {
        perror("couldn't bind");
        //return 1;
	 }

    nlh = (struct nlmsghdr *)buffer;
    while ((len = recv(sock, nlh, 4096, 0)) > 0) {
        while ((NLMSG_OK(nlh, len)) && (nlh->nlmsg_type != NLMSG_DONE)) {
            if (nlh->nlmsg_type == RTM_NEWADDR) {
                struct ifaddrmsg *ifa = (struct ifaddrmsg *) NLMSG_DATA(nlh);
                struct rtattr *rth = IFA_RTA(ifa);
                int rtl = IFA_PAYLOAD(nlh);

                while (rtl && RTA_OK(rth, rtl)) {
                    if (rth->rta_type == IFA_LOCAL) {
                        uint32_t ipaddr = htonl(*((uint32_t *)RTA_DATA(rth)));
                        char name[IFNAMSIZ];
                        if_indextoname(ifa->ifa_index, name);
                        networkchanged = 1;
                        printf(" netlink %d \n", networkchanged);

                        printf("%s is now %d.%d.%d.%d\n",name,
                               (ipaddr >> 24) & 0xff,
                               (ipaddr >> 16) & 0xff,
                               (ipaddr >> 8) & 0xff,
                               ipaddr & 0xff);
                    }
                    rth = RTA_NEXT(rth, rtl);
                }
            }
            nlh = NLMSG_NEXT(nlh, len);
        }
    }
}
