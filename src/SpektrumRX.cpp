/*
 * SpektrumRX.cpp
 *
 *  Created on: 09 Dec 2013
 *      Author: yashren
 */

#include "../inc/SpektrumRX.h"
#include <stdlib.h>

extern "C"{
    // this C function will be used to receive the thread and pass it back to the Thread instance
    void* thread_catch_spektrumrx(void* arg)
    {
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

    spektrum_fd = open(devicename, O_RDWR | O_NOCTTY);

    if (spektrum_fd < 0)
        perror(devicename);
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
}

double SpektrumRX::timeSinceStart(void){
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	return ((tv.tv_sec)*1e6 + (tv.tv_nsec)/1e3 - time_t0);
}

void SpektrumRX::decodeData(void){
	unsigned short int spektrum_word;

	if ((read_buffer[0] == 0x03) &&
		(read_buffer[1] == 0x01)){

		spektrum_word = ((unsigned short int) read_buffer[2] << 8) & 0x3FF;
		channel[1] = (unsigned short int) (spektrum_word | (unsigned char) read_buffer[3]);

		spektrum_word = ((unsigned short int) read_buffer[4] << 8) & 0x3FF;
		channel[5] = (unsigned short int) (spektrum_word | (unsigned char) read_buffer[5]);

		spektrum_word = ((unsigned short int) read_buffer[6] << 8) & 0x3FF;
		channel[2] = (unsigned short int) (spektrum_word | (unsigned char) read_buffer[7]);

		spektrum_word = ((unsigned short int) read_buffer[8] << 8) & 0x3FF;
		channel[3] = (unsigned short int) (spektrum_word | (unsigned char) read_buffer[9]);

		spektrum_word = ((unsigned short int) read_buffer[10] << 8) & 0x3FF;
		channel[0] = (unsigned short int) (spektrum_word | (unsigned char) read_buffer[11]);

		spektrum_word = ((unsigned short int) read_buffer[12] << 8) & 0x3FF;
		channel[4] = (unsigned short int) (spektrum_word | (unsigned char) read_buffer[13]);
	}
}

void SpektrumRX::process(void){
	autoSampleThreadRun = true;

    std::cout << "Entering RX thread" << std::endl;

	while(autoSampleThreadRun){
        int bytes_available = 0;
        ioctl(spektrum_fd, FIONREAD, &bytes_available);

        if (bytes_available > 15)
        {
            int n = read(spektrum_fd,&read_buffer[0],256);
            read_buffer[n] = '\0';
            decodeData();
            //printBuffer();
            logData();
            usleep(20000);
        }
	}

	std::cout << "Exiting RX thread" << std::endl;
}

void SpektrumRX::printBuffer(void){
	printf("%-7s%1u%7s%5u%7s%5u%7s%5u%7s%5u%8s%5u\r",
			"CH0:", channel[0],
			"CH1:", channel[1],
			"CH2:", channel[2],
			"CH3:", channel[3],
			"CH4:", channel[4],
			"CH5:", channel[5]);
	fflush(stdout);
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

