#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<netinet/ip_icmp.h>
#include<string.h>

unsigned short checksum(unsigned short *addr, int len);
int main(int argc, char* argv[]) 
{
	int len, ip_len, n;
	char buff[1024];
	struct ip *iphdr;
	struct icmp *icmphdr;
	int sockfd;
	struct addrinfo hints, *res;
	struct sockaddr_in raddr;
	
	/* check args */	
	if (argc < 2) 
	{
		fprintf(stderr, "Usage: sping <ip_address>\n");
		exit(EXIT_SUCCESS);
	}

	/* get socket info */
	if ( (sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) 
	{
		fprintf(stderr, "Cannot create raw socket. You must be root.\n");
		exit(EXIT_FAILURE);
	}
	
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_CANONNAME;
	hints.ai_flags = 0;
	hints.ai_socktype = 0;
	if ( (n = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) 
	{
		fprintf(stderr, "Error in getaddrinfo()\n");
		exit(EXIT_FAILURE);
	}
	
	/* fill in icmp header info */
	icmphdr = (struct icmp *) buff;
	icmphdr->icmp_type = ICMP_ECHO;
	icmphdr->icmp_code = 0;
	icmphdr->icmp_id = 0;
	icmphdr->icmp_seq = 0;
	len = 8;
	icmphdr->icmp_cksum = 0;
	icmphdr->icmp_cksum = checksum((u_short*) icmphdr, len);
	
	/* send the packet! */
	sendto(sockfd, buff, len, 0, res->ai_addr, res->ai_addrlen);

	/* wait for reply packet and set timeout */
	memset(buff, 0, 1024);
	n = sizeof(struct sockaddr_in);
	recvfrom(sockfd, buff, 1024, 0, (struct sockaddr *) &raddr, &n);

	/* print results to screen */
	iphdr = (struct ip *)buff;
	ip_len = iphdr->ip_hl << 2;
	icmphdr = (struct icmp *) (ip_len + buff);
	
	if (icmphdr->icmp_type == ICMP_ECHOREPLY)
		printf("%s Alive.\n", argv[1]);
	else
		printf("%s is not responding.\n", argv[1]);
	
	printf("message received from %s\n",inet_ntoa(raddr.sin_addr));
	close(sockfd);
	return 0;
}


/*---------------------------------------------------------------- CHECKSUM() --
* Internet checksum function. Taken from Steven’s UNIX Network
* Programming book.
*/

unsigned short checksum(unsigned short *addr, int len) 
{
	int nleft = len;
	int sum = 0;
	unsigned short *w = addr;
	unsigned short answer = 0;

	/*
	* Our algorithm is simple, using a 32 bit accumulator (sum), we add
	* seqential 16 bit words to it, and at the end, fold back all the
	* carry bits from the top 16 bits into the lower 16 bits.
	*/

	while (nleft > 1) 
	{
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */

	if (nleft == 1) 
	{
		*(unsigned char *) (&answer) = *(unsigned char *) w;
		sum += answer;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
	sum += (sum >> 16);
	/* add carry */
	answer =  ̃sum;
	/* truncate to 16 bits */
	return answer;
}
