CC = mpiicc
MPI_FLAGS

TARGETS = q1a q1b

q1a: q1a.c
	$(CC) -o $@ $<

q1b: q1b.c
	$(CC) -o $@ $<

clean:
	rm -f $(TARGETS)