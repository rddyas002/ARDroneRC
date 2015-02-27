/*
 * SpektrumRX.h
 *
 *  Created on: 09 Dec 2013
 *      Author: yashren
 */

#ifndef SPEKTRUMRX_H_
#define SPEKTRUMRX_H_

#include <string.h>
#include <termios.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <tr1/stdbool.h>
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <fstream>
#include <sys/time.h>

#define BAUD B115200
#define DATABITS CS8
#define STOPBITS 0
#define PARITY 0
#define PARITYON 0
#define MODEMDEVICE "/dev/ttyO3"
#define _POSIX_SOURCE 1         //POSIX compliant source
#define SPEKTRUM_RX_LOGFILE "log.txt"

#define SPEKTRUMRX_m0 (1.1721)
#define SPEKTRUMRX_c0 (940)
#define SPEKTRUM2PULSEWIDTH(x) ((unsigned short int) (SPEKTRUMRX_m0*(float)x + SPEKTRUMRX_c0))

class SpektrumRX {
public:
	SpektrumRX(double t0 = 0);
	void printBuffer(void);
	void stopReceiveThread(void);
	void decodeData(void);
	void logData(void);
	double timeSinceStart(void);
	// log data for test
    std::ofstream logFile;
    void openLogFile(void);
    void closeLogFile(void);
    bool writeLogFile(void);

	virtual void process(void);
	virtual ~SpektrumRX();
private:
private:
    int spektrum_fd;
    struct termios options;       //place for old and new port settings for serial port
    struct sigaction saio;               //definition of signal action
    char read_buffer[256];                       //buffer for where data is put
    int read_len;
    char devicename[80];
    double time_t0;

    pthread_t autoSample_thread;
    int autoSampleDelay;
    volatile bool autoSampleThreadRun;
    pthread_mutex_t count_mutex;

    unsigned short int channel[6];

    char write_buffer[512];
};

#endif /* SPEKTRUMRX_H_ */
