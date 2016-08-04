#include "vrms.h"
#define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */

void* ambulet_demo_thread(){

	int i, iResult, app_sock, recv_len, num_ppp, out_socks[MAX_PPP];
        char *buffer, ** ip_addr;
        socklen_t length;
	struct packet *recvd_pkt;
	struct sockaddr_in client_addr, server_addr, app_addr, out_addrs[MAX_PPP];
	
	buffer = malloc(BUFFERSIZE);
	ip_addr = malloc (sizeof(char *) * 3);

        app_sock = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
        if(app_sock <= INVALID_SOCK) {
	    perror("Could not open n/w change sock \n"); 
	    close(app_sock);
        }

        memset(&app_addr, 0 , sizeof(struct sockaddr_in));
        app_addr.sin_family = AF_INET;
        app_addr.sin_addr.s_addr = INADDR_ANY;
        app_addr.sin_port = htons(60000);

        memset(&server_addr, 0 , sizeof(struct sockaddr_in));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr("202.41.124.36");
        server_addr.sin_port = htons(61000);

        iResult = bind(app_sock,(struct sockaddr *)&app_addr, sizeof(struct sockaddr_in));
        if(iResult < 0) {
           perror(" Bind failed in network-change \n");
 	}

        length = sizeof(struct sockaddr_in);

	num_ppp = get_available_ifs(ip_addr);
	if(num_ppp < 0){
		printf("No PPP interfaces available to send!\n");
	}
	
	for(i=0; i<num_ppp; i++){
		printf("ip_addr[%d] : %s \n", i, ip_addr[i]);
#if 1
        	out_socks[i] = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
        	if(out_socks[i] <= INVALID_SOCK) {
	    		perror("Could not open outgoing sock ambulet_demo.c\n"); 
	    		close(app_sock);
		}

        	memset(&out_addrs[i], 0 , sizeof(struct sockaddr_in));
	        out_addrs[i].sin_family = AF_INET;
        	out_addrs[i].sin_addr.s_addr = inet_addr(ip_addr[i]);

        	iResult = bind(out_socks[i],(struct sockaddr *)&out_addrs[i], sizeof(struct sockaddr_in));
	        if(iResult < 0) {
	        	perror(" Bind failed in ambule_demo.c \n");
 		}
		printf("bind successful\n");
#endif

        }

        while(1){ 
	 
		bzero(buffer, BUFFERSIZE);

          	recv_len = recvfrom(app_sock, buffer, BUFFERSIZE, 0, (struct sockaddr *)&client_addr, &length);
	        if(recv_len <= 0){
	 	       perror(" Error recv data from n/w thread \n");
	       	} 
		printf("packet received\n");	
		recvd_pkt = (struct packet*) buffer;

		for(i=0; i<num_ppp; i++){

			recvd_pkt->interface = i;

			iResult = sendto(out_socks[i], recvd_pkt, recv_len,
                                                0, (struct sockaddr *)&server_addr, length);
			if(iResult < 0){
				perror("sendto of out failed");
			}
			printf("packet no: %d  sent through interface %d\n",recvd_pkt->packet_number,i);	
		
		}
       	}	

}

#if 1

int get_available_ifs(char **ip_addr){

        int i, family, iResult;
	struct ifaddrs *ifaddr, *ifa;
	
	for(i=0;i<3;i++){
		ip_addr[i] = malloc(64);
	}

        if (getifaddrs(&ifaddr) == -1) {
            perror("getifaddrs");
            exit(EXIT_FAILURE);
        }

	i = 0;

        /* Walk through linked list, maintaining head pointer so we can free list later */

        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
             if (ifa->ifa_addr == NULL)
                 continue;

             family = ifa->ifa_addr->sa_family;

        /* If the element is a ppp link then store the address */

            if (ifa->ifa_flags & IFF_POINTOPOINT) {
		printf("ppp found!!");
	
               	iResult = getnameinfo(ifa->ifa_addr,
                                      (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                      sizeof(struct sockaddr_in6), ip_addr[i], NI_MAXHOST, 
			              NULL, 0, NI_NUMERICHOST);
                if (iResult != 0) {
                    printf("getnameinfo() failed: %s\n", gai_strerror(iResult));
                    exit(EXIT_FAILURE);
		}
		printf("ip_addr : %s\n",ip_addr[i]);
		i++;
	    }

        }
	if(i == 0){
		freeifaddrs(ifaddr);
		return -1;
        } 

        freeifaddrs(ifaddr);

	return i;

}
#endif 
