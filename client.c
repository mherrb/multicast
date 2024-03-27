/*
 * multicast_client.c
 *
 * Derived from :
 *
 * This sample demonstrates a Windows multicast client that works with either
 * IPv4 or IPv6, depending on the multicast address given.
 *
 * Usage:
 *     mcastc [multicastip] port
 *
 * Examples:
 *     $ mcastc 224.0.22.1 9210
 *     $ mcastc ff15::1 2001
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

void
usage(const char *name)
{
	fprintf(stderr, "Usage: %s [<Multicast IP>] <Port>\n", name);
	exit(1);
}

int
multicastSock(char *multicastIP, char *port)
{
	struct addrinfo *multicastAddr;	/* Multicast Address */
	struct addrinfo *localAddr;	/* Local address to bind to */
	struct addrinfo hints = { 0 };	/* Hints for name lookup */
	int result, sock;

	/* Resolve the multicast group address */
	hints.ai_family = PF_UNSPEC;
	hints.ai_flags  = AI_NUMERICHOST;
	result = getaddrinfo(multicastIP, NULL, &hints, &multicastAddr);
	if (result != 0) {
		warnx("getaddrinfo(%s) failed: %s", multicastIP,
		    gai_strerror(result));
		return -1;
	}

	printf("Using %s\n", multicastAddr->ai_family == PF_INET6 ?
	    "IPv6" : "IPv4");

	/* Get a local address with the same family (IPv4 or IPv6)
	   as our multicast group */
	hints.ai_family   = multicastAddr->ai_family;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags    = AI_PASSIVE; /* Return an address we can bind to */
	result = getaddrinfo(NULL, port, &hints, &localAddr);
	if (result != 0) {
		warnx("getaddrinfo() failed: %s", gai_strerror(result));
		return -1;
	}

	/* Create socket for receiving datagrams */
	if ((sock = socket(localAddr->ai_family,
		    localAddr->ai_socktype, 0)) == -1)
		return -1;

	/* Bind to the multicast port */
	if (bind(sock, localAddr->ai_addr, localAddr->ai_addrlen) != 0)
		return -1;

	/* Join the multicast group.
	 * We do this seperately depending on whether we
	 * are using IPv4 or IPv6.  */
	if (multicastAddr->ai_family  == PF_INET &&
	    multicastAddr->ai_addrlen == sizeof(struct sockaddr_in)) {
		/* IPv4 */
		struct ip_mreq multicastRequest;  /* Multicast address
						     join structure */

		/* Specify the multicast group */
		memcpy(&multicastRequest.imr_multiaddr,
		    &((struct sockaddr_in*)(multicastAddr->ai_addr))->sin_addr,
		    sizeof(struct in_addr));

		/* Accept multicast from any interface */
		multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);

		/* Join the multicast address */
		if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
			(char *)&multicastRequest, sizeof(multicastRequest))
		    != 0 ) {
			warn("setsockopt(IP_ADD_MEMBERSHIP) failed");
			return -1;
		}
	} else if (multicastAddr->ai_family  == PF_INET6 &&
	    multicastAddr->ai_addrlen == sizeof(struct sockaddr_in6)) {
		/* IPv6 */
		struct ipv6_mreq multicastRequest;  /* Multicast address
						       join structure */

		/* Specify the multicast group */
		memcpy(&multicastRequest.ipv6mr_multiaddr,
		    &((struct sockaddr_in6*)(multicastAddr->ai_addr))->sin6_addr,
		    sizeof(struct in6_addr));

		/* Accept multicast from any interface */
		multicastRequest.ipv6mr_interface = 0;

		/* Join the multicast group */
		if (setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP,
			(char *) &multicastRequest, sizeof(multicastRequest))
		    != 0) {
			warn("setsockopt(IPV6_JOIN_GROUP) failed");
			return -1;
		}
	} else {
		warnx("Neither IPv4 or IPv6");
		return -1;
	}

	freeaddrinfo(localAddr);
	freeaddrinfo(multicastAddr);
	return sock;
}

int
unicastSock(char *port)
{
	struct addrinfo *localAddr;	/* Local address to bind to */
	struct addrinfo hints = { 0 };	/* Hints for name lookup */
	int result, sock;
	
	/* Get a local address to listen to */
	hints.ai_family   = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags    = AI_PASSIVE; /* Return an address we can bind to */
	result = getaddrinfo(NULL, port, &hints, &localAddr);
	if (result != 0) {
		warn("getaddrinfo() failed: %s", gai_strerror(result));
		return -1;
	}

	/* Create socket for receiving datagrams */
	if ((sock = socket(localAddr->ai_family,
		    localAddr->ai_socktype, 0)) == -1) 
		return -1;

	/* Bind to the  port */
	if (bind(sock, localAddr->ai_addr, localAddr->ai_addrlen) != 0)
		return -1;

	freeaddrinfo(localAddr);
	return(sock);
}

int
main(int argc, char* argv[])
{
	int sock;		/* Socket */
	char *progname;
	char *multicastIP;	/* Arg: IP Multicast Address */
	char *port;	/* Arg: Port */

	progname = argv[0];
	switch (argc) {
	case 2: 
		multicastIP = NULL;
		port = argv[1];
		break;
	case 3:
		multicastIP = argv[1];
		port = argv[2];
		break;
	default:
		usage(progname);
	}

	if (multicastIP != NULL)
		sock = multicastSock(multicastIP, port);
	else
		sock = unicastSock(port);
	if (sock == -1)
		err(2, "socket");

	for (;;) {
		struct timespec tv;
#ifdef __MACH__
		clock_serv_t cclock;
		mach_timespec_t mts;
#endif
		char   recvString[1500];      /* Buffer for received string */
		int    recvStringLen;        /* Length of received string */

		/* Receive a single datagram from the server */
		if ((recvStringLen = recvfrom(sock, recvString,
			    sizeof(recvString) - 1, 0, NULL, 0)) < 0 )
			err(2, "recvfrom() failed");

		recvString[recvStringLen] = '\0';

		/* Print the received string */
#ifdef __MACH__
		host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
		clock_get_time(cclock, &mts);
		mach_port_deallocate(mach_task_self(), cclock);
		tv.tv_sec = mts.tv_sec;
		tv.tv_nsec = mts.tv_nsec;
#else
		clock_gettime(CLOCK_REALTIME, &tv);
#endif
		printf("%ld.%09ld %s\n", tv.tv_sec, tv.tv_nsec,
		    recvString);
	}

	/* NOT REACHED */
	close(sock);
	exit(0);
}
