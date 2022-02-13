#include "sctp_common.h"

#define member_size(type, member) sizeof(((type *)0)->member)
#define sizeof_up_to(type, member) (offsetof(type, member) + member_size(type, member))

void print_addr_info(struct sockaddr *sin, char *text)
{
	struct sockaddr_in *addr4;
	struct sockaddr_in6 *addr6;
	char addr_str[INET6_ADDRSTRLEN + 1];

	switch (sin->sa_family) {
	case AF_INET:
		addr4 = (struct sockaddr_in *)sin;
		inet_ntop(sin->sa_family,
			  (void *)&addr4->sin_addr,
			  addr_str, INET6_ADDRSTRLEN + 1);
		printf("%s IPv4 addr %s\n", text, addr_str);
		break;
	case AF_INET6:
		addr6 = (struct sockaddr_in6 *)sin;
		if (IN6_IS_ADDR_V4MAPPED(&addr6->sin6_addr)) {
			inet_ntop(AF_INET,
				  (void *)&addr6->sin6_addr.s6_addr32[3],
				  addr_str, INET6_ADDRSTRLEN + 1);
			printf("%s IPv6->IPv4 MAPPED addr %s\n",
			       text, addr_str);
		} else if (IN6_IS_ADDR_LINKLOCAL(&addr6->sin6_addr)) {
			inet_ntop(sin->sa_family,
				  (void *)&addr6->sin6_addr,
				  addr_str, INET6_ADDRSTRLEN + 1);
			printf("%s IPv6 local link addr %s scope_id %d\n",
			       text, addr_str,
			       ((struct sockaddr_in6 *)addr6)->sin6_scope_id);
		} else {
			inet_ntop(sin->sa_family,
				  (void *)&addr6->sin6_addr,
				  addr_str, INET6_ADDRSTRLEN + 1);
			printf("%s IPv6 addr %s\n", text,
			       addr_str);
		}
		break;
	default:
		printf("%s Unknown IP family %d\n", text, sin->sa_family);
		break;
	}
}

int set_subscr_events(int fd, int data_io, int assoc, int addr, int shutd)
{
	struct sctp_event_subscribe subscr_events;

	memset(&subscr_events, 0, sizeof(subscr_events));
	subscr_events.sctp_data_io_event = data_io;
	subscr_events.sctp_association_event = assoc;
	subscr_events.sctp_address_event = addr;
	subscr_events.sctp_shutdown_event = shutd;

	/*
	 * Truncate optlen to just the fields we touch to avoid errors when
	 * the uapi headers are newer than the running kernel.
	 */
	return setsockopt(fd, IPPROTO_SCTP, SCTP_EVENTS, &subscr_events,
			  sizeof_up_to(struct sctp_event_subscribe,
				       sctp_shutdown_event));
}

/*
 * Currently only SCTP_ASSOC_CHANGE, SCTP_PEER_ADDR_CHANGE and
 * SCTP_SHUTDOWN_EVENT are enabled via set_subscr_events().
 */
int handle_event(void *buf, char *cmp_addr, sctp_assoc_t *assoc_id,
		 bool verbose, char *text)
{
	union sctp_notification *snp = buf;
	char addrbuf[INET6_ADDRSTRLEN];
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;
	const char *ap;
	struct sctp_paddr_change *spc;
	struct sctp_assoc_change *sac;
	struct sctp_remote_error *sre;
	struct sctp_send_failed *ssf;
	struct sctp_authkey_event *auth_event;

	switch (snp->sn_header.sn_type) {
	case SCTP_ASSOC_CHANGE:
		sac = &snp->sn_assoc_change;

		if (verbose)
			printf("%s SCTP_ASSOC_CHANGE event for assoc_id: %d ERR: 0x%x\n",
			       text, sac->sac_assoc_id, sac->sac_error);

		if (assoc_id)
			*assoc_id = sac->sac_assoc_id;
		break;
	case SCTP_PEER_ADDR_CHANGE:
		spc = &snp->sn_paddr_change;

		if (verbose)
			/*
			 * Not all spc_error codes are errors - linux/sctp.h
			 * (e.g. SCTP_HEARTBEAT_SUCCESS = 0x02)
			 */
			printf("%s SCTP_PEER_ADDR_CHANGE event for assoc_id: %d ERR: 0x%x\n",
			       text, spc->spc_assoc_id, spc->spc_error);

		if (spc->spc_aaddr.ss_family == AF_INET) {
			sin = (struct sockaddr_in *) &spc->spc_aaddr;
			ap = inet_ntop(AF_INET, &sin->sin_addr, addrbuf,
				       INET6_ADDRSTRLEN);
		} else {
			sin6 = (struct sockaddr_in6 *) &spc->spc_aaddr;
			ap = inet_ntop(AF_INET6, &sin6->sin6_addr, addrbuf,
				       INET6_ADDRSTRLEN);
		}
		if (verbose) /* Print additional address details */
			print_addr_info((struct sockaddr *)&spc->spc_aaddr,
					"Peer Address change:\n\t");

		switch (spc->spc_state) {
		case SCTP_ADDR_AVAILABLE:
			if (verbose)
				printf("\t%s is available\n", text);
			break;
		case SCTP_ADDR_UNREACHABLE:
			if (verbose)
				printf("\t%s is not available - Error: 0x%x\n",
				       text, spc->spc_error);
			break;
		case SCTP_ADDR_REMOVED:
			if (verbose)
				printf("\t%s was removed\n", text);
			break;
		case SCTP_ADDR_ADDED:
			if (verbose)
				printf("\t%s was added\n", text);
			break;
		case SCTP_ADDR_MADE_PRIM:
			if (verbose)
				printf("\t%s is primary\n", text);
			if (cmp_addr) {
				if (!strcmp(ap, cmp_addr)) {
					if (verbose)
						printf("\t%s and is now the new primary\n", text);

					return EVENT_ADDR_MATCH;
				}
			}
			break;
		case SCTP_ADDR_CONFIRMED:
			if (verbose)
				printf("\t%s is confirmed\n", text);
			break;
		default:
			if (verbose)
				printf("%s unknown state: %d\n", text,
				       spc->spc_state);
			break;
		}
		break;
	case SCTP_SEND_FAILED:
		ssf = &snp->sn_send_failed;

		if (verbose)
			printf("%s SCTP_SEND_FAILED event assoc_id: %d ERR: 0x%x\n",
			       text, ssf->ssf_assoc_id, ssf->ssf_error);
		break;
	case SCTP_REMOTE_ERROR:
		sre = &snp->sn_remote_error;
		if (verbose) /* Error in network byte order - linux/sctp.h */
			printf("%s SCTP_REMOTE_ERROR event ERR: 0x%x\n",
			       text, ntohs(sre->sre_error));
		break;
	case SCTP_SHUTDOWN_EVENT:
		if (verbose)
			printf("%s SCTP_SHUTDOWN_EVENT\n", text);

		return EVENT_SHUTDOWN;
	case SCTP_PARTIAL_DELIVERY_EVENT:
		if (verbose)
			printf("%s SCTP_PARTIAL_DELIVERY_EVENT\n", text);
		break;
	case SCTP_ADAPTATION_INDICATION:
		if (verbose)
			printf("%s SCTP_ADAPTATION_INDICATION event\n", text);
		break;
	case SCTP_AUTHENTICATION_INDICATION:
		auth_event = &snp->sn_authkey_event;

		if (verbose) {
			printf("%s SCTP_AUTHENTICATION_INDICATION event\n"
			       "\tauth_event->auth_type:       0x%x\n"
			       "\tauth_event->auth_flags:      0x%x\n"
			       "\tauth_event->auth_length:     0x%x\n"
			       "\tauth_event->auth_keynumber:  0x%x\n"
			       "\tauth_event->auth_indication: 0x%x\n"
			       "\tauth_event->auth_assoc_id:   %d\n",
			       text, auth_event->auth_type,
			       auth_event->auth_flags,
			       auth_event->auth_length,
			       auth_event->auth_keynumber,
			       auth_event->auth_indication,
			       auth_event->auth_assoc_id);
		}
		/* SCTP_AUTH_NO_AUTH defined in linux/sctp.h */
		if (auth_event->auth_indication == SCTP_AUTH_NO_AUTH)
			return EVENT_NO_AUTH;
		break;
	case SCTP_SENDER_DRY_EVENT:
		if (verbose)
			printf("%s SCTP_SENDER_DRY_EVENT\n", text);
		break;
	case SCTP_STREAM_RESET_EVENT:
		if (verbose)
			printf("%s SCTP_STREAM_RESET_EVENT\n", text);
		break;
	case SCTP_ASSOC_RESET_EVENT:
		if (verbose)
			printf("%s SCTP_ASSOC_RESET_EVENT\n", text);
		break;
	case SCTP_STREAM_CHANGE_EVENT:
		if (verbose)
			printf("%s SCTP_STREAM_CHANGE_EVENT\n", text);
		break;
	case SCTP_SEND_FAILED_EVENT:
		if (verbose)
			printf("%s SCTP_SEND_FAILED_EVENT\n", text);
		break;
	default:
		fprintf(stderr, "%s unknown event: 0x%x\n", text,
			snp->sn_header.sn_type);
		break;
	}

	return EVENT_OK;
}

int receive_assoc(int sock, struct sockaddr_storage *sin, socklen_t *sinlen,
		  int verbose)
{
	int result, peeloff_sk = 0, flags;
	sctp_assoc_t assoc_id = 0;
	char *peerlabel, msglabel[256];

	/* Get assoc_id for sctp_peeloff() */
	result = set_subscr_events(sock, off, on, off, off);
	if (result < 0) {
		perror("Server setsockopt: SCTP_EVENTS");
		return 1;
	}
	*sinlen = sizeof(*sin);
	flags = 0;

	result = sctp_recvmsg(sock, msglabel, sizeof(msglabel),
			      (struct sockaddr *)sin, sinlen,
			      NULL, &flags);
	if (result < 0) {
		perror("Server sctp_recvmsg-1");
		return 1;
	}

	if (verbose)
		print_addr_info((struct sockaddr *)sin,
				"Server SEQPACKET recvmsg");

	if (flags & MSG_NOTIFICATION && flags & MSG_EOR) {
		handle_event(msglabel, NULL, &assoc_id,
			     verbose, "Peeloff Server");
		if (assoc_id <= 0) {
			printf("Server Invalid association ID: %d\n",
			       assoc_id);
			return 1;
		}
		/* No more notifications */
		result = set_subscr_events(sock, off, off, off, off);
		if (result < 0) {
			perror("Server setsockopt: SCTP_EVENTS");
			return 1;
		}

		peeloff_sk = sctp_peeloff(sock, assoc_id);
		if (peeloff_sk < 0) {
			perror("Server sctp_peeloff");
			return 1;
		}
		if (verbose) {
			printf("Server sctp_peeloff(3) on sk: %d with association ID: %d\n",
			       peeloff_sk, assoc_id);
		}

		/* Now get the client msg on peeloff socket */
		result = sctp_recvmsg(peeloff_sk, msglabel, sizeof(msglabel),
				      (struct sockaddr *)sin, sinlen,
				      NULL, &flags);
		if (result < 0) {
			perror("Server sctp_recvmsg-2");
			close(peeloff_sk);
			return 1;
		}

		if (verbose) {
			print_addr_info((struct sockaddr *)sin,
					"Server SEQPACKET peeloff recvmsg");
			printf("peeloff association ID: %d\n",
			       assoc_id);
		}
	} else {
		printf("Invalid sctp_recvmsg response FLAGS: %x\n",
		       flags);
		close(peeloff_sk);
		return 1;
	}

	peerlabel = strdup("nopeer");

	printf("Server PEELOFF peer label: %s\n", peerlabel);

	result = sctp_sendmsg(peeloff_sk, peerlabel,
			      strlen(peerlabel),
			      (struct sockaddr *)sin,
			      *sinlen, 0, 0, 0, 0, 0);
	if (result < 0) {
		perror("Server sctp_sendmsg");
		close(peeloff_sk);
		close(sock);
		exit(1);
	}

	if (verbose)
		printf("Server PEELOFF sent: %s\n", peerlabel);

	free(peerlabel);

	close(peeloff_sk);
	return 0;
}

int open_assoc(int sock, struct sockaddr *sin, socklen_t sinlen,
	       int verbose)
{
	int result, peeloff_sk = 0, flags;
	sctp_assoc_t assoc_id = 0;
	char byte = 0x41, label[1024];

	/* Subscribe to assoc_id events */
	result = set_subscr_events(sock, off, on, off, off);
	if (result < 0) {
		perror("Client setsockopt: SCTP_EVENTS");
		return 1;
	}

	/* First send a message to get an association. */
	result = sctp_sendmsg(sock, &byte, 1, sin, sinlen,
			      0, 0, 0, 0, 0);
	if (result < 0) {
		perror("Client sctp_sendmsg");
		return 1;
	}

	/* Get assoc_id for sctp_peeloff() */
	flags = 0;
	result = sctp_recvmsg(sock, label, sizeof(label),
			      NULL, 0, NULL, &flags);
	if (result < 0) {
		perror("Client sctp_recvmsg-1");
		return 1;
	}

	if ((flags & (MSG_NOTIFICATION | MSG_EOR)) != (MSG_NOTIFICATION | MSG_EOR)) {
		printf("Invalid sctp_recvmsg response FLAGS: %x\n", flags);
		return 1;
	}
	handle_event(label, NULL, &assoc_id, verbose, "Peeloff Client");
	if (assoc_id <= 0) {
		printf("Client Invalid association ID: %d\n", assoc_id);
		return 1;
	}
	/* No more notifications */
	result = set_subscr_events(sock, off, off, off, off);
	if (result < 0) {
		perror("Client setsockopt: SCTP_EVENTS");
		return 1;
	}

	peeloff_sk = sctp_peeloff(sock, assoc_id);
	if (peeloff_sk < 0) {
		perror("Client sctp_peeloff");
		return 1;
	}

	result = sctp_recvmsg(peeloff_sk, label, sizeof(label),
			      NULL, 0, NULL, NULL);
	if (result < 0) {
		perror("Client sctp_recvmsg");
		close(peeloff_sk);
		return 1;
	}

	close(peeloff_sk);
	return 0;
}
