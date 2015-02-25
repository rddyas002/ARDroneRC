/*
 * UDPsocket.cpp
 *
 *  Created on: 23 Jun 2014
 *      Author: yashren
 */

#include "../inc/UDP_socket.h"

UDP_socket::UDP_socket(const char * ip_address, const char * ip_port) {
	serverPort = atoi(ip_port);
	socketId = socket(AF_INET,SOCK_DGRAM,0);

	sockaddr &serverAddrCast = (sockaddr &)serverAddr;

	//specify server address, port
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	struct hostent * hp = gethostbyname(ip_address);

	memcpy(( char *) &serverAddr.sin_addr, (char *) hp->h_addr, hp->h_length);
	size = sizeof(serverAddr);

	seq = 1;

	// Immediately reset the trim of the drone
	bufferSize = sprintf(buffer, "AT*FTRIM=%d,\r", seq);
	sendto(socketId, &buffer, bufferSize, 0, &serverAddrCast, size);
	seq++;
	bufferSize = 0;
	usleep(100000);

	std::cout << "Sending AR Drone flat trim" << std::endl;
}

void UDP_socket::takeOff(void){
	sockaddr &serverAddrCast = (sockaddr &)serverAddr;
	bufferSize = sprintf(buffer, "AT*REF=%d,%d\r", seq, 290718208);
	seq++;
	sendto(socketId, &buffer, bufferSize, 0, &serverAddrCast, size);
	bufferSize = sprintf(buffer, "AT*COMWDG=%d\r", seq);
	sendto(socketId, &buffer, bufferSize, 0, &serverAddrCast, size);
	seq++;
	usleep(100000);
}

void UDP_socket::land(void){
	sockaddr &serverAddrCast = (sockaddr &)serverAddr;
	bufferSize = sprintf(buffer, "AT*REF=%d,%d\r", seq, 290717696);
	seq++;
	sendto(socketId, &buffer, bufferSize, 0, &serverAddrCast, size);
	bufferSize = sprintf(buffer, "AT*COMWDG=%d\r", seq);
	sendto(socketId, &buffer, bufferSize, 0, &serverAddrCast, size);
	seq++;
	usleep(100000);
}

UDP_socket::~UDP_socket() {
	close(socketId);
}

