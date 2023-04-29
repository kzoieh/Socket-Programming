#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netpacket/packet.h>
#include<net/ethernet.h>
#include<arpa/inet.h>
#include<sys/ioctl.h>
#include<linux/if.h>
//#include<linux/if_packet.h>

int main()
{
	int sockfd, len;
	char buffer[2048];
	struct sockaddr_ll pla;
	
	sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(sockfd<0)
	{
		perror("packet_socket");
		exit(0);
	} 

	int io, b, s;
	struct ifreq req;
	strcpy(req.ifr_name,"enp2s0");

	io = ioctl(sockfd,SIOCGIFINDEX,&req);
	if(io<0)
	{
		printf("The interface enp2s0 does not exist");
	}

	//struct sockaddr_ll pla;
	pla.sll_family=PF_PACKET;
	pla.sll_protocol=htons(ETH_P_ALL);
	pla.sll_ifindex=req.ifr_ifindex;
	b=bind(sockfd,(struct sockaddr *)&pla,sizeof(pla));
	if(b<0)
	{
		perror("bind");
	}

	struct packet_mreq mr;
	mr.mr_ifindex=req.ifr_ifindex;
	mr.mr_type=PACKET_MR_PROMISC;
	s = setsockopt(sockfd,SOL_PACKET,PACKET_ADD_MEMBERSHIP,(void *)&mr,sizeof(mr));
	if(s<0)
	{
		perror("Promiscuous Mode");
	}

	printf("Types of the captured packets are...\n");
	
	while(1)
	{
		len = sizeof(struct sockaddr_ll);
		recvfrom(sockfd,buffer,sizeof(buffer),0,(struct sockaddr *)&pla,&len);
		printf("%x\t", ntohs(pla.sll_protocol));

                switch(ntohs(pla.sll_protocol))
                {
                        case 0x0800:
                                printf("Internet Protocol Packet\t");
                                break;
                        case 0x0806:
                                printf("Address resolution Packet\t");
                                break;
                        case 0x86dd:
                                printf("IPv6 over bluebook\t\t");
                                break;
                        case 0x0004:
                                printf("802.2 frames\t\t\t");
                                break;
                        default:
                                printf("xprotocol\t\t\t");
                                break;
                }

		switch(pla.sll_pkttype)
		{
			// these constant values are taken from linux/if_packet.h
			case 0://PACKET_HOST
				printf("For_Me\n"); 
				break;
			case 1://PACKET_BROADCAST
				printf("Broadcast\n");
				break;
			case 2://PACKET_MULTICAST
				printf("Multicast\n");
				break;
			case 3://PACKET_OTHERHOST
				printf("Other_Host\n");
				break;
			case 4://PACKET_OUTGOING
				printf("Outgoing\n");
				break;
			case 5://PACKET_LOOPBACK
				printf("Loop_Back\n");
				break;
			case 6://PACKET_FASTROUTE
				printf("Fast_Route\n");
				break;
		}
	}
}
