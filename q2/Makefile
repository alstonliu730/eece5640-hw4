CC = mpicc
CFLAGS = -Wall -Wextra -g -O2

TARGETS = q2a_two_nodes q2a_four_nodes q2b_two_nodes q2b_four_nodes
all: $(TARGETS)

q2a_two_nodes: q2a_two_nodes.c
	$(CC) $(CFLAGS) -o $@ $<

q2a_four_nodes: q2a_four_nodes.c
	$(CC) $(CFLAGS) -o $@ $<

q2b_two_nodes: q2b_two_nodes.c
	$(CC) $(CFLAGS) -o $@ $<

q2b_four_nodes: q2b_four_nodes.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
