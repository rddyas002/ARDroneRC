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

#define YAW_RATE 4
#define THROTTLE_RATE 15000
#define PITCH_ROLL_RATE 1
#define ALTITUDE_MAX 10000
#define FLY_OUTDOOR "FALSE"

class UDP_socket {
public:
	UDP_socket(const char * ip_address, const char * ip_port);
	void initialiseDrone(void);
	void sendData(void);
	void resetWatchdog(void);
	void flatTrim(void);
	void takeOff(float roll, float pitch, float heave_rate, float yaw_rate);
	void hoverControl(float roll, float pitch, float heave_rate, float yaw_rate);
	void flying(float roll, float pitch, float heave_rate, float yaw_rate);
	void land(void);
	virtual ~UDP_socket();

private:
	int socketId, serverPort, bufferSize, size;
	sockaddr_in serverAddr, clientAddr;
	char buffer[200];
	int seq;
};


#endif /* UDPSOCKET_H_ */
