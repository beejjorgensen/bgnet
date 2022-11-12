/*
** telnot.c -- Not telnet, but can be used in place of telnet for
**             the guide demos.
**
** This doesn't implement the telnet protocol in the least.
**
** Usage: telnot hostname port
**
** Then type things and hit RETURN to send them. (It uses the current
** terminal line discipline, which is probably line-buffered so nothing
** will get sent until you hit RETURN.) It will print things to
** standard output as it receives them.
**
** Hit ^C to break out.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>

#include <arpa/inet.h>

#define BUFSIZE 1024

/**
 * Get a sockaddr, IPv4 or IPv6
 */
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/**
 * Main
 */
int main(int argc, char *argv[])
{
	int sockfd;  
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 3) {
	    fprintf(stderr,"usage: telnot hostname port\n");
	    exit(1);
	}

	char *hostname = argv[1];
	char *port = argv[2];

	// Try to connect

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			//perror("telnot: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			//perror("telnot: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	// Connected!

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);

	printf("Connected to %s port %s\n", s, port);
	printf("Hit ^C to exit\n");

	freeaddrinfo(servinfo); // All done with this structure

	// Poll stdin and sockfd for incoming data (ready-to-read)
	struct pollfd fds[2];

	fds[0].fd = 0;
	fds[0].events = POLLIN;

	fds[1].fd = sockfd;
	fds[1].events = POLLIN;

	// Main loop
	for(;;) {
		if (poll(fds, 2, -1) == -1) {
			perror("poll");
			exit(1);
		}

		for (int i = 0; i < 2; i++) {

			// Check for ready-to-read
			if (fds[i].revents & POLLIN) {

				int readbytes, writebytes;
				char buf[BUFSIZE];

				// Compute where to write data. If we're stdin (0),
				// we'll write to the sockfd. If we're the sockfd, we'll
				// write to stdout (1).
				int outfd = fds[i].fd == 0? sockfd: 1;

				// We use read() and write() in here since those work on
				// all fds, not just sockets. send() and recv() would
				// fail on stdin and stdout since they're not sockets.
				if ((readbytes = read(fds[i].fd, buf, BUFSIZE)) == -1) {
					perror("read");
					exit(2);
				}

				char *p = buf;
				int remainingbytes = readbytes;

				// Write all data out
				while (remainingbytes > 0) {
					if ((writebytes = write(outfd, p, remainingbytes)) == -1) {
						perror("write");
						exit(2);
					}

					p += writebytes;
					remainingbytes -= writebytes;
				}
			}
		}
	}

	// Not reached--use ^C to exit.

	return 0;
}

