TARGETS = sctp_peeloff_client sctp_peeloff_server \
	sctp_bz2048251_client sctp_bz2048251_server
DEPS = sctp_common.c sctp_common.h
CFLAGS ?= -Wall

LDLIBS += -lsctp

all: $(TARGETS)

clean:
	rm -f $(TARGETS) reproduce_bz2048251.tar.gz

reproduce_bz2048251.tar.gz: $(DEPS) $(addsuffix .c,$(TARGETS)) reproduce.sh README Makefile
	tar -czvf $@ $^

tarball: reproduce_bz2048251.tar.gz

$(TARGETS): $(DEPS)
