all: ftserver ftclient ftterminate tftserver tftclient

ftserver: ftserver.c 
	gcc -o ftserver ftserver.c -lrt

ftclient: ftclient.c 
	gcc -o ftclient ftclient.c -lrt

ftterminate: ftterminate.c 
	gcc -o ftterminate ftterminate.c

tftserver: tftserver.c 
	gcc -o tftserver tftserver.c -lrt

tftclient: tftclient.c 
	gcc -o tftclient tftclient.c -lrt

clean:
	rm -f ftserver ftclient tftserver tftclient ftterminate
