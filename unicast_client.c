/*
 * unicast_client.c
 *
 * Derived from :
 *
 * This sample demonstrates a Windows multicast client that works with either
 * IPv4 or IPv6, depending on the multicast address given.
 *
 * Usage:
 *     ucastc port
 *
 * Examples:
 *     $ ucastc 9210
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
	fprintf(stderr, "Usage: %s <Multicast Port>\n", name);
	exit(1);
}

int
main(int argc, char* argv[])
{
	int sock;		/* Socket */
	int result;
	char *progname;
	char *port;	/* Arg: Port */
	struct addrinfo *localAddr;	/* Local address to bind to */
	struct addrinfo hints = { 0 };	/* Hints for name lookup */

	progname = argv[0];
	if (argc != 2)
		usage(progname);

	port = argv[1]; /* arg: port */

	/* Get a local address to listen to */
	hints.ai_family   = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags    = AI_PASSIVE; /* Return an address we can bind to */
	result = getaddrinfo(NULL, port, &hints, &localAddr);
	if (result != 0)
		errx(2, "getaddrinfo() failed: %s", gai_strerror(result));

	/* Create socket for receiving datagrams */
	if ((sock = socket(localAddr->ai_family,
		    localAddr->ai_socktype, 0)) == -1)
		err(2, "socket() failed");

	/* Bind to the  port */
	if (bind(sock, localAddr->ai_addr, localAddr->ai_addrlen) != 0)
		err(2, "bind() failed");


	freeaddrinfo(localAddr);

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
