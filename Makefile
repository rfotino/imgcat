all:
	gcc -Wall -O2 imgcat.c convert.c environment.c -o imgcat
clean:
	rm -rf *.o imgcat

