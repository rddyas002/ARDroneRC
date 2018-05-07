/*
 * SpektrumRX.cpp
 *
 *  Created on: 09 Dec 2013
 *      Author: yashren
 */

#include "../inc/SpektrumRX.h"
#include <stdlib.h>
#include <bitset>
#include <iostream>
#include <climits>

extern "C"{
    // this C function will be used to receive the thread and pass it back to the Thread instance
    void* thread_catch_spektrumrx(void* arg){
    	SpektrumRX* t = static_cast<SpektrumRX*>(arg);
        t->process();
        return 0;
    }
}

SpektrumRX::SpektrumRX(double t0) {
    read_len = -1;
    sprintf(devicename, "%s", MODEMDEVICE);
    time_t0 = t0;
    time_t0 = timeSinceStart();
    memset(&reference_command[0], 10, sizeof(float)*6);

    spektrum_fd = open(devicename, O_RDWR | O_NOCTTY);

    if (spektrum_fd < 0){
        perror(devicename);
        exit(1);
    }
    else
        std::cout << "Port " << devicename << " successfully opened." << std::endl;

    tcgetattr(spektrum_fd, &options);
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    options.c_oflag &= ~(ONLCR | OCRNL);
    // set baud rate
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    tcsetattr(spektrum_fd, TCSANOW, &options);

    openLogFile();

    if (pthread_create(&autoSample_thread, 0, &thread_catch_spektrumrx, this) != 0){
        std::cout << "Receive thread creation error." << std::endl;
    }

//    // Make thread priority high
//    struct sched_param sp;
//    sp.sched_priority = 95;
//    pthread_setschedparam(autoSample_thread, SCHED_FIFO, &sp);
}

double SpektrumRX::timeSinceStart(void){
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	return ((tv.tv_sec)*1e6 + (tv.tv_nsec)/1e3 - time_t0);
}

void SpektrumRX::decodePacket(char bytes){
    unsigned short int spektrum_word;
    short int temp = 0;

    // rough error checking
	if ((read_buffer[0] == 0x03) && (read_buffer[1] == 0x01) && (read_buffer[bytes-2] == 0x18)){
        if ((read_buffer[2] & 0xFC) == SPEKTRUM_CH1){
            spektrum_word = ((unsigned short int)read_buffer[2] << 8) & 0x03FF;
            channel[1] = (unsigned short int) (spektrum_word | (unsigned char)read_buffer[3]);
            reference_command[1] = ((float)channel[1] - 511)/360;
        }
        if ((read_buffer[4] & 0xFC) == SPEKTRUM_CH5){
            spektrum_word = ((unsigned short int)read_buffer[4] << 8) & 0x03FF;
            channel[5] = (unsigned short int) (spektrum_word | (unsigned char)read_buffer[5]);
            if (channel[5] > 180){
            	reference_command[5] = 0;
            }
            else{
            	reference_command[5] = 1;
            }

        }
        if ((read_buffer[6] & 0xFC) == SPEKTRUM_CH2){
            spektrum_word = ((unsigned short int)read_buffer[6] << 8) & 0x03FF;
            channel[2] = (unsigned short int) (spektrum_word | (unsigned char)read_buffer[7]);
            reference_command[2] = -((float)channel[2] - 511)/360;
        }
        if ((read_buffer[8] & 0xF8) == SPEKTRUM_CH3){
            spektrum_word = ((unsigned short int)read_buffer[8] << 8) & 0x07FF;
            channel[3] = (unsigned short int) (spektrum_word | (unsigned char)read_buffer[9]);
            // +ve signal
            if ((channel[3] & (0b11 << 9)) == (0b11 << 9)){
            	temp = (short int)(channel[3] & 0x01FF);
            }
            else if ((channel[3] & (0b01 << 9)) == (0b01 << 9)){// -ve signal
            	temp = (signed short int) (-255 + (channel[3] & 0xFF));
            }
            else if ((channel[3] & (0b10 << 9)) == (0b10 << 9)){
            	temp = -255 + (signed short int) (-255 + (channel[3] & 0xFF));
            }
            reference_command[3] = -(float)temp/360;
        }
        if ((read_buffer[10] & 0xFC) == SPEKTRUM_CH0){
            spektrum_word = ((unsigned short int)read_buffer[10] << 8) & 0x03FF;
            channel[0] = (unsigned short int) (spektrum_word | (unsigned char)read_buffer[11]);
            reference_command[0] = (float)channel[0]/400 - 1.06;
        }
        if (bytes == 16){
            	reference_command[4] = 0;
        }
        else{
        	reference_command[4] = 1;

        }
    }
}

bool SpektrumRX::initOkay(void){
	// check if all sticks are centered and land selected all is okay
	if (getThrottle() == 0){
		if (getRoll() == 0){
			if (getPitch() == 0){
				if (getYaw() == 0){
					if (getAuto() == 0)
						return true;
					else
						return false;
				}
				else
					return false;
			}
			else
				return false;
		}
		else
			return false;
	}
	else
		return false;
}

void SpektrumRX::process(void){
	autoSampleThreadRun = true;

    std::cout << "Entering RX thread" << std::endl;
    // Flush read buffer
    tcflush(spektrum_fd,TCIFLUSH);

	while(autoSampleThreadRun){
        int bytes_available = 0;
        ioctl(spektrum_fd, FIONREAD, &bytes_available);
        if (bytes_available > 14){
            int n = read(spektrum_fd,&read_buffer[0],64);
            read_buffer[n] = '\0';
            decodePacket(n);
//            printBuffer();
//            fflush(stdout);
          //  logData();
        }
        usleep(10000);
	}

	std::cout << "Exiting RX thread" << std::endl;
}

void SpektrumRX::printBuffer(void){
	printf("%-7s%5.2f%7s%5.2f%7s%5.2f%7s%5.2f%7s%5.2f%8s%5.2f\r\n",
			"THR:", reference_command[0],
			"ROL:", reference_command[1],
			"PIT:", reference_command[2],
			"YAW:", reference_command[3],
			"CH4:", reference_command[4],
			"CH5:", reference_command[5]);

}

float SpektrumRX::deadband(float reference, float half_width){
    if ((reference > -half_width) && (reference < half_width)){
    	reference = 0;
    }
    else{
    	if (reference > half_width)
    		reference -= half_width;
    	if (reference < -half_width)
    		reference += half_width;
    }
    return reference;
}

float SpektrumRX::getThrottle(void){
	return deadband(reference_command[0], 0.2);
}

float SpektrumRX::getRoll(void){
	return deadband(reference_command[1], 0.05);
}

float SpektrumRX::getPitch(void){
	return deadband(reference_command[2], 0.05);
}

float SpektrumRX::getYaw(void){
	return deadband(reference_command[3], 0.05);
}

float SpektrumRX::getAuto(void){
	return reference_command[5];
}

void SpektrumRX::logData(void){
	sprintf(write_buffer,"%u,%u,%u,%u,%u,%u,%.0f\r\n",
			channel[0],
			channel[1],
			channel[2],
			channel[3],
			channel[4],
			channel[5],
			timeSinceStart());
	writeLogFile();
}

void SpektrumRX::openLogFile(void){
	logFile.open(SPEKTRUM_RX_LOGFILE);
}

void SpektrumRX::closeLogFile(void){
	if (logFile.is_open())
		logFile.close();
}

bool SpektrumRX::writeLogFile(void){
	if (logFile.is_open())	{
		logFile << write_buffer;
		return true;
	}
	else
		return false;
}

void SpektrumRX::stopReceiveThread(void){
	autoSampleThreadRun = false;
    // wait for thread to end
    pthread_join(autoSample_thread, NULL);
}

SpektrumRX::~SpektrumRX() {
	stopReceiveThread();
	closeLogFile();
	close(spektrum_fd);
}

