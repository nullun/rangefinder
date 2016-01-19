make: main.c
	@clang -O3 -Wall -o rangefinder main.c
clean:
	-@rm -f rangefinder packets.log 2>/dev/null

