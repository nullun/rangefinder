make: main.c
	@gcc -o rangefinder main.c
clean:
	-@rm -f rangefinder packets.log 2>/dev/null

