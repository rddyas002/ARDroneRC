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

	//specify server address, port
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	struct hostent * hp = gethostbyname(ip_address);

	memcpy(( char *) &serverAddr.sin_addr, (char *) hp->h_addr, hp->h_length);
	size = sizeof(serverAddr);

	seq = 1;
}

void UDP_socket::initialiseDrone(void){

	bufferSize = sprintf(buffer, "AT*CONFIG=%d\"general:navdata_demo\",\"TRUE\"\r", seq++);
	sendData();
	usleep(30000);

	bufferSize = sprintf(buffer, "AT*CONFIG=%d,\"control:outdoor\",\"%s\"\rAT*CONFIG=%d,\"control:flight_without_shell\",\"%s\"\r",
            seq++,
            FLY_OUTDOOR,
            seq++,
            FLY_OUTDOOR);
	sendData();
	usleep(30000);

	bufferSize = sprintf(buffer, "AT*CONFIG=%d,\"control:control_yaw\",\"%d\"\r", seq++, YAW_RATE);
	sendData();
	usleep(30000);

	bufferSize = sprintf(buffer, "AT*CONFIG=%d,\"control:control_vz_max\",\"%d\"\r", seq++, THROTTLE_RATE);
	sendData();
	usleep(30000);

	bufferSize = sprintf(buffer, "AT*CONFIG=%d,\"control:euler_angle_max\",\"0.%d\"\r", seq++, PITCH_ROLL_RATE);
	sendData();
	usleep(30000);

	bufferSize = sprintf(buffer, "AT*CONFIG=%d,\"control:altitude_max\",\"%d\"\r", seq++, ALTITUDE_MAX);
	sendData();
	usleep(30000);

	bufferSize = sprintf(buffer, "AT*LED=%d,2,1073741824,5\r", seq++);
	sendData();
	usleep(30000);
}

void UDP_socket::sendData(void){
	sockaddr &serverAddrCast = (sockaddr &)serverAddr;
	sendto(socketId, &buffer, bufferSize, 0, &serverAddrCast, size);
}

void UDP_socket::land(void){
	sockaddr &serverAddrCast = (sockaddr &)serverAddr;
	bufferSize = sprintf(buffer, "AT*PCMD=%d,0,0,0,0,0\rAT*REF=%d,%d\r", seq++, seq++, 290717696);
	sendto(socketId, &buffer, bufferSize, 0, &serverAddrCast, size);
}

void UDP_socket::resetWatchdog(void){
	seq = 1;
	sockaddr &serverAddrCast = (sockaddr &)serverAddr;
	bufferSize = sprintf(buffer, "AT*COMWDG=%d\r", seq++);
	sendto(socketId, &buffer, bufferSize, 0, &serverAddrCast, size);
}

void UDP_socket::flatTrim(void){
	sockaddr &serverAddrCast = (sockaddr &)serverAddr;
	bufferSize = sprintf(buffer, "AT*FTRIM=%d\rAT*LED=%d,2,1073741824,1\r", seq++, seq++);
	sendto(socketId, &buffer, bufferSize, 0, &serverAddrCast, size);
}

void UDP_socket::takeOff(float roll, float pitch, float heave_rate, float yaw_rate){
	sockaddr &serverAddrCast = (sockaddr &)serverAddr;
	bufferSize = sprintf(buffer,"AT*REF=%d,290718208\rAT*PCMD=%d,0,%ld,%ld,%ld,%ld\r",
			seq++, seq++, roll, pitch, heave_rate, yaw_rate);
	sendto(socketId, &buffer, bufferSize, 0, &serverAddrCast, size);
}

void UDP_socket::hoverControl(float roll, float pitch, float heave_rate, float yaw_rate){
	sockaddr &serverAddrCast = (sockaddr &)serverAddr;
	bufferSize = sprintf(buffer,"AT*PCMD=%d,0,%ld,%ld,%ld,%ld\r",
			seq++, roll, pitch, heave_rate, yaw_rate);
	sendto(socketId, &buffer, bufferSize, 0, &serverAddrCast, size);
}

void UDP_socket::flying(float roll, float pitch, float heave_rate, float yaw_rate){
	sockaddr &serverAddrCast = (sockaddr &)serverAddr;
	bufferSize = sprintf(buffer,"AT*PCMD=%d,1,%ld,%ld,%ld,%ld\r",
			seq++, roll, pitch, heave_rate, yaw_rate);
	sendto(socketId, &buffer, bufferSize, 0, &serverAddrCast, size);
}

UDP_socket::~UDP_socket() {
	land();
	sleep(1);
	close(socketId);
}

