CC = mpicc
CFLAGS = -Wall -Wextra -g -O2

TARGETS = q2a_two_nodes

q2a_two_nodes: q2a_two_nodes.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)