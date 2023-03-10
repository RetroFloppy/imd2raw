CFLAGS = -O2 -Wall
CMP = cmp

imd2raw: imd2raw.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean

clean:
	$(RM) imd2raw imd2raw.exe *.o

.PHONY: test
test: %test: %imd2raw
	$(RM) test/test.img
	./imd2raw test/DIAG_2.04.IMD test/test.img
	$(CMP) test/DIAG_2.04.IMG test/test.img
	$(RM) test/test.img
	./imd2raw test/BLANK.IMD test/test.img
	$(CMP) test/BLANK.IMG test/test.img
	$(RM) test/test.img
	./imd2raw test/VEDIT86_1.36.IMD test/test.img
	$(CMP) test/VEDIT86_1.36.IMG test/test.img
