CC = mpicc
CFLAGS = -Wall -Wextra -g -O2

TARGETS = q1a q1b

q1a: q1a.c
	$(CC) $(CFLAGS) -o $@ $<

q1b: q1b.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
