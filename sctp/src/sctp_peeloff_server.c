#include "sctp_common.h"

static void usage(char *progname)
{
	fprintf(stderr,
		"usage:  %s [-4] [-f file] [-v] port\n"
		"\nWhere:\n\t"
		"-4      Listen on IPv4 addresses only.\n\t"
		"-f      Write a line to the file when listening starts.\n\t"
		"        \"nopeer\" message to client, otherwise the peer context\n\t"
		"        will be retrieved and sent to client.\n\t"
		"-v      Print context and ip options information.\n\t"
		"port    Listening port.\n", progname);
	exit(1);
}

int main(int argc, char **argv)
{
	int opt, sock, result;
	socklen_t sinlen;
	struct sockaddr_storage sin;
	struct addrinfo hints, *res;
	char *flag_file = NULL;
	bool verbose = false, ipv4 = false;
	unsigned short port;

	while ((opt = getopt(argc, argv, "4f:v")) != -1) {
		switch (opt) {
		case '4':
			ipv4 = true;
			break;
		case 'f':
			flag_file = optarg;
			break;
		case 'v':
			verbose = true;
			break;
		default:
			usage(argv[0]);
		}
	}

	if ((argc - optind) != 1)
		usage(argv[0]);

	port = atoi(argv[optind]);
	if (!port)
		usage(argv[0]);

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = IPPROTO_SCTP;

	if (ipv4)
		hints.ai_family = AF_INET;
	else
		hints.ai_family = AF_INET6;

	/* sctp_peeloff(3) must be from 1 to Many style socket */
	hints.ai_socktype = SOCK_SEQPACKET;

	result = getaddrinfo(NULL, argv[optind], &hints, &res);
	if (result < 0) {
		fprintf(stderr, "Server getaddrinfo: %s\n",
			gai_strerror(result));
		exit(1);
	}

	sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sock < 0) {
		perror("Server socket");
		exit(1);
	}

	result = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (result < 0) {
		perror("Server setsockopt: SO_REUSEADDR");
		close(sock);
		exit(1);
	}

	result = bind(sock, res->ai_addr, res->ai_addrlen);
	if (result < 0) {
		perror("Server bind");
		close(sock);
		exit(1);
	}

	if (listen(sock, SOMAXCONN)) {
		perror("Server listen");
		close(sock);
		exit(1);
	}

	if (flag_file) {
		FILE *f = fopen(flag_file, "w");
		if (!f) {
			perror("Flag file open");
			exit(1);
		}
		fprintf(f, "listening\n");
		fclose(f);
	}

	/* Subscribe to assoc_id events */
	result = set_subscr_events(sock, off, on, off, off);
	if (result < 0) {
		perror("Client setsockopt: SCTP_EVENTS");
		return 1;
	}

	do {
		result = receive_assoc(sock, &sin, &sinlen, verbose);
		if (result) {
			close(sock);
			exit(result);
		}
	} while (1);

	close(sock);
	exit(0);
}
