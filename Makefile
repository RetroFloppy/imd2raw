CFLAGS = -O2 -Wall
CMP = cmp

imd2raw: imd2raw.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean

clean:
	$(RM) imd2raw *.o

.PHONY: test
test: %test:
	$(RM) test/test.img
	./imd2raw test/BLANK.IMD test/test.img
	$(CMP) test/BLANK.IMG test/test.img
