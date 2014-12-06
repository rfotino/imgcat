all:
	gcc -o imgcat imgcat.c environment.c
clean:
	rm -rf *.o imgcat

