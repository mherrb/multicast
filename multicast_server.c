/*
 * multicast_server.c
 *
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

#define _GNU_SOURCE		/* for asprintf() */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <err.h>
#include <limits.h>
#include <netdb.h>
#include <stdio.h>      /* for fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef HAVE_LIBBSD
# include <bsd/stdlib.h>	/* for strtonum() */
#endif

void
usage(const char *name)
{
	errx(2, "Usage: %s [-i interval(ms)] <Multicast Address> <Port> "
	    "<Send String> [<TTL>]", name);
}


int
main(int argc, char *argv[])
{
	int sock;		      /* Socket */
	int ch;
	unsigned long interval = 3000;
	const char *errmsg;
	char *multicastIP;            /* Arg: IP Multicast address */
	char *multicastPort;          /* Arg: Server port */
	char *sendString;             /* Arg: String to multicast */
	size_t sendStringLen;         /* Length of string to multicast */
	int multicastTTL;             /* Arg: TTL of multicast packets */
	struct addrinfo* multicastAddr;          /* Multicast address */
	struct addrinfo hints = { 0 }; /* Hints for name lookup */

	while ((ch = getopt(argc, argv, "i:")) != -1) {
		switch (ch) {
		case 'i':
			interval = strtonum(optarg, 0, INT_MAX,
			    &errmsg);
			if (errmsg != NULL)
				err(2, "-i: %s", errmsg);
			break;
		default:
			usage(argv[0]);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 3 || argc > 4)
		errx(2, "Usage: %s <Multicast Address> <Port> <Send String>"
		    " [<TTL>]", argv[0]);

	multicastIP = argv[0]; /* First arg:   multicast IP address */
	multicastPort = argv[1]; /* Second arg:  multicast port */
	sendString = argv[2];	/* Third arg:   String to multicast */
       /* Fourth arg:  If supplied, use command-line */
	/* specified TTL, else use default TTL of 1 */
	multicastTTL = (argc == 4 ? atoi(argv[3]) : 1);

	/* Resolve destination address for multicast datagrams */
	hints.ai_family   = PF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags    = AI_NUMERICHOST;
	if (getaddrinfo(multicastIP, multicastPort,
		&hints, &multicastAddr) != 0)
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
		struct timespec tv;
		char *buffer;

		clock_gettime(CLOCK_REALTIME, &tv);
		sendStringLen = asprintf(&buffer, "%ld.%.09ld %s",
		    tv.tv_sec, tv.tv_nsec, sendString);
		if (sendStringLen < 0)
			err(2, "asprintf");
		sendStringLen++; /* nul byte */
		if (sendto(sock, buffer, sendStringLen, 0,
			multicastAddr->ai_addr, multicastAddr->ai_addrlen)
		    != sendStringLen )
			err(2, "sendto() failed");
		free(buffer);

		usleep(interval*1000); /* Multicast sendString in datagram to
					  clients every interval ms */
	}

	/* NOT REACHED */
	freeaddrinfo(multicastAddr);
	close(sock);
	return 0;
}
