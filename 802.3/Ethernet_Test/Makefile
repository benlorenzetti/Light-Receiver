all: broadcast_packet ethernet_listen

broadcast_packet: broadcast_packet.o
	gcc broadcast_packet.o -o broadcast_packet

broadcast_packet.o: broadcast_packet.c
	gcc -c broadcast_packet.c

ethernet_listen: ethernet_listen.o
	gcc ethernet_listen.o -o ethernet_listen

ethernet_listen.o: ethernet_listen.c
	gcc -c ethernet_listen.c
