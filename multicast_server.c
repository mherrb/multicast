/* multicast_server.c
 * This sample demonstrates a Linux multicast server that works with either
 * IPv4 or IPv6, depending on the multicast address given.
 *
 * Usage:
 *     mcasts multicastip port data [ttl]
 *
 * Examples:
 *     $ mcasts 224.0.22.1 9210 HelloIPv4World
 *     $ mcasts ff15::1 2001 HelloIPv6World
 *
 * Written by tmouse, July 2005
 * http://cboard.cprogramming.com/showthread.php?t=67469
 *
 * Ported to Linux/BSD by Matthieu Herrb, December 2012
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <err.h>
#include <netdb.h>
#include <stdio.h>      /* for fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
	int sock;                   /* Socket */
	char *multicastIP;            /* Arg: IP Multicast address */
	char *multicastPort;          /* Arg: Server port */
	char *sendString;             /* Arg: String to multicast */
	size_t sendStringLen;          /* Length of string to multicast */
	int multicastTTL;           /* Arg: TTL of multicast packets */
	struct addrinfo* multicastAddr;          /* Multicast address */
	struct addrinfo hints = { 0 }; /* Hints for name lookup */


	if (argc < 4 || argc > 5)
		errx(2, "Usage: %s <Multicast Address> <Port> <Send String>"
		    " [<TTL>]", argv[0]);

	multicastIP = argv[1]; /* First arg:   multicast IP address */
	multicastPort = argv[2]; /* Second arg:  multicast port */
	sendString = argv[3];	/* Third arg:   String to multicast */
       /* Fourth arg:  If supplied, use command-line */
	/* specified TTL, else use default TTL of 1 */
	multicastTTL = (argc == 5 ? atoi(argv[4]) : 1);
	sendStringLen = strlen(sendString);

	/* Resolve destination address for multicast datagrams */
	hints.ai_family   = PF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags    = AI_NUMERICHOST;
	if (getaddrinfo(multicastIP, multicastPort, &hints, &multicastAddr) != 0)
		err(2, "getaddrinfo() failed");

	printf("Using %s\n", multicastAddr->ai_family == PF_INET6 ?
	    "IPv6" : "IPv4");

	/* Create socket for sending multicast datagrams */
	if ((sock = socket(multicastAddr->ai_family,
		    multicastAddr->ai_socktype, 0)) == -1)
		err(2, "socket() failed");

	/* Set TTL of multicast packet */
	if (setsockopt(sock,
		multicastAddr->ai_family == PF_INET6 ? IPPROTO_IPV6 : IPPROTO_IP,
		multicastAddr->ai_family == PF_INET6 ? IPV6_MULTICAST_HOPS
		: IP_MULTICAST_TTL,
		(char*) &multicastTTL, sizeof(multicastTTL)) != 0)
		err(2, "setsockopt() failed");

	for (;;) {
		if (sendto(sock, sendString, sendStringLen, 0,
			multicastAddr->ai_addr, multicastAddr->ai_addrlen)
		    != sendStringLen )
			err(2, "sendto() failed");


		usleep(3000000); /* Multicast sendString in datagram to
				    clients every 3 seconds */
	}

	/* NOT REACHED */
	freeaddrinfo(multicastAddr);
	close(sock);
	return 0;
}
