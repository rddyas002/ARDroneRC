/* 
 * File:   UARTComms.h
 * Author: reddi
 *
 * Created on 18 February 2015, 6:54 AM
 */

#ifndef UARTCOMMS_H
#define	UARTCOMMS_H

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <tr1/stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <locale.h>

#define BAUD B115200
#define DATABITS CS8
#define STOPBITS 0
#define PARITY 0
#define PARITYON 0
#define MODEMDEVICE "/dev/ttyO3"
#define _POSIX_SOURCE 1         //POSIX compliant source

#define MAV_FILTER_N    10

#define STEP_ON_BIT         (1<<0)
#define LOOP_CLOSED_BIT     (1<<1)

class UARTComms {
public:
    UARTComms();
    void stopReceiveThread(void);
    void closeUART(void);
    void decodePacket(void);
    
    int writePacket(char * buffer, char length);
    int writeByte(char buffer);
    void writeCommand(void);
    
    std::ofstream plant_data_log;
    void initialiseLog();
    void openLog(char * str);
    void closeLog(void);
    bool writeLog(void);      
    
    virtual void receiveThread(void);
    virtual ~UARTComms();
private:
    int uart_fd;
    struct termios options;       //place for old and new port settings for serial port
    struct sigaction saio;               //definition of signal action
    char devicename[80];
    
    bool quit_rx_thread;
    pthread_t receive_thread;

    char tx_buffer[256];
    char rx_buffer[256];    

    char log_buffer[512];
};

#endif	/* UARTCOMMS_H */

