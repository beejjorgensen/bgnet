TARGETS=broadcaster client getip ghbn ieee754 listener pack pack2 \
	select selectserver server showip talker

CC=gcc
CCOPTS=-Wall -Wextra

.PHONY: all clean

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

%: %.c
	$(CC) $(CCOPTS) -o $@ $<
