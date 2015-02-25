#include <stdio.h>
#include <pthread.h>
#include "inc/UDP_socket.h"
#include "inc/UARTComms.h"

int main(void){
	//UDP_socket droneConnection = UDP_socket("127.0.0.1","5556");
	//sleep(1);
	//droneConnection.takeOff();
	//sleep(5);
	//droneConnection.land();

	UARTComms * uartComms = new UARTComms();

	int i;
	for (i = 0; i < 100; i++){
		usleep(10000);
		uartComms->writeByte(0xAA);
	}

	uartComms->~UARTComms();

	return 0;
}
