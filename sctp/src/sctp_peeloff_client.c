#include "sctp_common.h"

static void usage(char *progname)
{
	fprintf(stderr,
		"usage:  %s [-v] addr port\n"
		"\nWhere:\n\t"

		"-v      Print context and ip options information.\n\t"
		"addr    IPv4 or IPv6 address (e.g. 127.0.0.1 or ::1).\n\t"
		"port    Port for accessing server.\n", progname);
	exit(1);
}

int main(int argc, char **argv)
{
	int opt, sock, result;
	struct addrinfo hints, *serverinfo;
	bool verbose = false;
	struct timeval tm;

	while ((opt = getopt(argc, argv, "v")) != -1) {
		switch (opt) {
		case 'v':
			verbose = true;
			break;
		default:
			usage(argv[0]);
		}
	}

	if ((argc - optind) != 2)
		usage(argv[0]);

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_protocol = IPPROTO_SCTP;
	hints.ai_socktype = SOCK_SEQPACKET;

	result = getaddrinfo(argv[optind], argv[optind + 1], &hints,
			     &serverinfo);
	if (result < 0) {
		fprintf(stderr, "Client getaddrinfo: %s\n",
			gai_strerror(result));
		exit(2);
	}

	sock = socket(serverinfo->ai_family, serverinfo->ai_socktype,
		      serverinfo->ai_protocol);
	if (sock < 0) {
		perror("Client socket");
		exit(3);
	}

	/*
	 * These timeouts are set to test whether the peer { recv } completes
	 * or not when the permission is denied.
	 */
	tm.tv_sec = 4;
	tm.tv_usec = 0;
	result = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tm, sizeof(tm));
	if (result < 0) {
		perror("Client setsockopt: SO_SNDTIMEO");
		exit(4);
	}

	result = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tm, sizeof(tm));
	if (result < 0) {
		perror("Client setsockopt: SO_RCVTIMEO");
		exit(5);
	}

	result = open_assoc(sock, serverinfo->ai_addr, serverinfo->ai_addrlen,
			    verbose);
	if (result) {
		close(sock);
		exit(result);
	}

	close(sock);
	exit(0);
}
