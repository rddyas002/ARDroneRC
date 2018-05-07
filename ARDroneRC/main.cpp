#include <stdio.h>
#include <pthread.h>
#include "inc/UDP_socket.h"
#include "inc/UARTComms.h"
#include "inc/SpektrumRX.h"

typedef enum ARDroneState{
	GROUNDED,
	TAKING_OFF,
	HOVERING,
	FLYING,
	LANDING,
	FLAT_TRIM,
	RESET_WATCHDOG
}ARDroneState_s;

ARDroneState_s droneState = GROUNDED;

int main(void){
	SpektrumRX * spektrumRX = new SpektrumRX();
	unsigned int count_takeoff = 0;

	sleep(1);
	if (!spektrumRX->initOkay()){
		printf("Radio initial stick positions are incorrect\r\n");
		spektrumRX->~SpektrumRX();
		return 0;
	}

	UDP_socket * droneConnection;
	droneConnection = new UDP_socket("127.0.0.1","5556");
	sleep(1);

	droneConnection->initialiseDrone();

	int i;
	for (i = 0; i < 1000; i++){
		usleep(100000);	// 30 ms delay
		switch (droneState){
			case GROUNDED:
//				printf("GND_");
				droneConnection->land();
				if (spektrumRX->getAuto() == 1.0){
					droneState = RESET_WATCHDOG;
					printf("CHANGE_STATE to RESET_WATCHDOG\r\n");
				}
				else{
					droneState = GROUNDED;
				}
				break;
			case RESET_WATCHDOG:
//				printf("RWD_");
				droneConnection->resetWatchdog();
				droneState = FLAT_TRIM;
				printf("CHANGE_STATE to FLAT_TRIM\r\n");
				break;
			case FLAT_TRIM:
//				printf("FTM_");
				droneConnection->flatTrim();
				usleep(100000);
				droneState = TAKING_OFF;
				printf("CHANGE_STATE to TAKING_OFF\r\n");
				count_takeoff = 0;
				break;
			case TAKING_OFF:
//				printf("TGF_");
				droneConnection->takeOff(spektrumRX->getRoll(), spektrumRX->getPitch(), spektrumRX->getThrottle(), spektrumRX->getYaw());
				if (count_takeoff++ > 100){	// stay in this state for 3 seconds
					droneState = HOVERING;
					printf("CHANGE_STATE to HOVERING\r\n");
				}
				if (spektrumRX->getAuto() == 0.0){
					droneState = GROUNDED;
					printf("CHANGE_STATE to GROUNDED\r\n");
				}
				break;
			case HOVERING:
//				printf("HOV_");
				droneConnection->hoverControl(spektrumRX->getRoll(), spektrumRX->getPitch(), spektrumRX->getThrottle(), spektrumRX->getYaw());
				if (spektrumRX->getAuto() == 0.0){
					droneState = GROUNDED;
					printf("CHANGE_STATE to GROUNDED\r\n");
				}
				if ((spektrumRX->getRoll() != 0.0) || (spektrumRX->getPitch()) != 0.0){
					droneState = FLYING;
					printf("CHANGE_STATE to FLYING\r\n");
				}
				break;
			case FLYING:
//				printf("FLY_");
				droneConnection->flying(spektrumRX->getRoll(), spektrumRX->getPitch(), spektrumRX->getThrottle(), spektrumRX->getYaw());
				if (spektrumRX->getAuto() == 0.0){
					droneState = GROUNDED;
					printf("CHANGE_STATE to GROUNDED\r\n");
				}
				if ((spektrumRX->getRoll() == 0.0) && (spektrumRX->getPitch()) == 0.0){
					droneState = HOVERING;
					printf("CHANGE_STATE to HOVERING\r\n");
				}
				break;
			default:
				printf("DEF_");
				droneState = GROUNDED;
				break;
		}
//		printf("%-7s%5.2f%7s%5.2f%7s%5.2f%7s%5.2f%7s%5.2f\r\n",
//				"THR:", spektrumRX->getThrottle(),
//				"ROL:", spektrumRX->getRoll(),
//				"PIT:", spektrumRX->getPitch(),
//				"YAW:", spektrumRX->getYaw(),
//				"CH4:", spektrumRX->getAuto());
//		fflush(stdout);
	}

	spektrumRX->~SpektrumRX();
	droneConnection->~UDP_socket();

	return 0;
}
