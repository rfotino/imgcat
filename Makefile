all:
	gcc -Wall -O2 imgcat.c convert.c environment.c -ljpeg -lpng -o imgcat
clean:
	rm -rf *.o imgcat
