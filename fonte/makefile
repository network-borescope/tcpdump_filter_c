#Makefile
#target: dependencies
#	action

all: main

main: ip_to_nome_lat_lon.o list.o dict.o filter_pcap_to_tc.c
	gcc filter_pcap_to_tc.c -o filter_c ip_to_nome_lat_lon.o list.o dict.o

ip_to_nome_lat_lon.o: ip_to_nome_lat_lon.c
	gcc -c ip_to_nome_lat_lon.c

list.o: list.c
	gcc -c list.c

dict.o: dict.c
	gcc -c dict.c

clean:
	rm *o