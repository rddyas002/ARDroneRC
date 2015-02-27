#include <stdio.h>
#include <pthread.h>
#include "inc/UDP_socket.h"
#include "inc/UARTComms.h"
#include "inc/SpektrumRX.h"

int main(void){
	//UDP_socket droneConnection = UDP_socket("127.0.0.1","5556");
	//sleep(1);
	//droneConnection.takeOff();
	//sleep(5);
	//droneConnection.land();

	SpektrumRX * spektrumRX = new SpektrumRX();

	sleep(10);
	spektrumRX->~SpektrumRX();

	return 0;
}
