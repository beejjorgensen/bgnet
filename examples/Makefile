TARGETS=broadcaster client getip ghbn ieee754 listener pack pack2 \
	poll pollserver select selectserver server showip talker telnot

CC=gcc
CCOPTS=-Wall -Wextra

.PHONY: all clean pristine

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

pristine: clean

%: %.c
	$(CC) $(CCOPTS) -o $@ $<
