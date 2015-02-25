/*
 * SpektrumRX.cpp
 *
 *  Created on: 09 Dec 2013
 *      Author: yashren
 */

#include "../inc/SpektrumRX.h"
#include <stdlib.h>

extern "C"
{
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

    if (pthread_create(&autoSample_thread, 0, &thread_catch_spektrumrx, this) != 0){
        std::cout << "Receive thread creation error." << std::endl;
    }
}

void SpektrumRX::decodeData(void){
	unsigned short int spektrum_word;

	if ((read_buffer[0] == 0x03) &&
		(read_buffer[1] == 0x01)){

		spektrum_word = ((unsigned short int) read_buffer[2] << 8) & 0x3FF;
		channel[1] = (unsigned short int) (spektrum_word | (unsigned char) read_buffer[3]);
		left = SPEKTRUM2PULSEWIDTH(channel[1]);

		spektrum_word = ((unsigned short int) read_buffer[4] << 8) & 0x3FF;
		channel[5] = (unsigned short int) (spektrum_word | (unsigned char) read_buffer[5]);
		right = SPEKTRUM2PULSEWIDTH(channel[5]);

		spektrum_word = ((unsigned short int) read_buffer[6] << 8) & 0x3FF;
		channel[2] = (unsigned short int) (spektrum_word | (unsigned char) read_buffer[7]);
		front = SPEKTRUM2PULSEWIDTH(channel[2]);

		spektrum_word = ((unsigned short int) read_buffer[8] << 8) & 0x3FF;
		channel[3] = (unsigned short int) (spektrum_word | (unsigned char) read_buffer[9]);
		rudder = SPEKTRUM2PULSEWIDTH(channel[3]);

		spektrum_word = ((unsigned short int) read_buffer[10] << 8) & 0x3FF;
		channel[0] = (unsigned short int) (spektrum_word | (unsigned char) read_buffer[11]);
		esc = SPEKTRUM2PULSEWIDTH(channel[0]);

		spektrum_word = ((unsigned short int) read_buffer[12] << 8) & 0x3FF;
		channel[4] = (unsigned short int) (spektrum_word | (unsigned char) read_buffer[13]);
		gain = SPEKTRUM2PULSEWIDTH(channel[4]);
	}
}

void SpektrumRX::process(void){
	autoSampleThreadRun = true;

    // setup nonblocking read here
    //struct pollfd fds;
    //fds.fd = spektrum_fd;
    //fds.events = POLLIN;

    std::cout << "Entering RX thread" << std::endl;

	while(autoSampleThreadRun)
	{
        //int poll_ret = poll(&fds,1,read_timeout);

        int bytes_available = 0;
        ioctl(spektrum_fd, FIONREAD, &bytes_available);

        if (bytes_available > 15)
        {
            int n = read(spektrum_fd,&read_buffer[0],256);
            read_buffer[n] = '\0';
            decodeData();
            printBuffer();
            usleep(20000);
        }
	}

	std::cout << "Exiting RX thread" << std::endl;
}

void SpektrumRX::printBuffer(void){
	//clearScreen();

	printf("%-7s%1u%7s%5u%7s%5u%7s%5u%7s%5u%8s%5u\r\n",
			"RIGHT:", right,
			"LEFT:", left,
			"FRONT:", front,
			"ESC:", esc,
			"GAIN:", gain,
			"RUDDER:", rudder);
			/*
	printf("%-7s%1u%7s%5u%7s%5u%7s%5u%7s%5u%8s%5u\r\n",
			"CH0:", channel[0],
			"CH1:", channel[1],
			"CH2:", channel[2],
			"CH3:", channel[3],
			"CH4:", channel[4],
			"CH5:", channel[5]);
			*/
}

void SpektrumRX::clearScreen(void){
	printf("\n\n\n\n\n\n\n\n\n\n");
}

void SpektrumRX::stopReceiveThread(void){
	autoSampleThreadRun = false;
    // wait for thread to end
    pthread_join(autoSample_thread, NULL);
}

SpektrumRX::~SpektrumRX() {
	stopReceiveThread();
	close(spektrum_fd);
}

