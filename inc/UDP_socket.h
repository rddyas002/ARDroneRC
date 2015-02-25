/*
 * UDPsocket.h
 *
 *  Created on: 23 Jun 2014
 *      Author: yashren
 */

#ifndef UDPSOCKET_H_
#define UDPSOCKET_H_

#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

class UDP_socket {
public:
	UDP_socket(const char * ip_address, const char * ip_port);
	void takeOff(void);
	void land(void);
	virtual ~UDP_socket();

private:
	int socketId, serverPort, bufferSize, size;
	sockaddr_in serverAddr, clientAddr;
	char buffer[200];
	int seq;
};


#endif /* UDPSOCKET_H_ */
