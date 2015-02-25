/* 
 * File:   UARTComms.cpp
 * Author: reddi
 * 
 * Created on 18 February 2015, 6:54 AM
 */

#include "../inc/UARTComms.h"

extern "C"{
    // this C function will be used to receive the thread and pass it back to the Thread instance
    void* thread_catch_rx(void* arg)
    {
        UARTComms* t = static_cast<UARTComms*>(arg);
        t->receiveThread();
        return 0;
    }      
}

UARTComms::UARTComms() {
    sprintf(devicename, "%s", MODEMDEVICE);
    uart_fd = open(devicename, O_RDWR | O_NOCTTY);

    if (uart_fd < 0)
        perror(devicename);

    tcgetattr(uart_fd, &options);
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    options.c_oflag &= ~(ONLCR | OCRNL);
    tcsetattr(uart_fd, TCSANOW, &options);
    
    // create a read thread
    quit_rx_thread = false;
    if (pthread_create(&receive_thread, 0, &thread_catch_rx, this) != 0){
        perror("Thread creation");
    }
    
    setlocale(LC_ALL,"C");
    
    //initialiseLog();
}

void UARTComms::openLog(char * file_name){
    plant_data_log.open(file_name);
}

void UARTComms::closeLog(void){
    if (plant_data_log.is_open())
        plant_data_log.close();
}

bool UARTComms::writeLog(void){
    if (plant_data_log.is_open()){
        plant_data_log << log_buffer;
        return true;
    }
    else
        return false;
}

void UARTComms::initialiseLog(void){
    struct tm * sTm;
    char file_buffer[128];
    char time_buffer[128];
    time_t now = time(0);
    sTm = localtime(&now);
    strftime(time_buffer,sizeof(time_buffer),"%Y-%m-%d_%H-%M", sTm);
    sprintf(file_buffer,"log/Plant_data_%s.txt", time_buffer);
    openLog(&file_buffer[0]);
}

void UARTComms::receiveThread(void){
    // setup nonblocking read here
    struct pollfd fds;
    fds.fd = uart_fd;
    fds.events = POLLIN;
    
    int read_timeout = 1000;
           
    while(!quit_rx_thread){
        int poll_ret = poll(&fds,1,read_timeout);
//        if(poll_ret > 0){
//            int n = read(uart_fd,rx_buffer,256);
//            rx_buffer[n] = '\0';
//            if (n >= 21)
//                decodePacket();
//            else
//                std::cout << "DBG: More than 25 bytes" << std::endl;
//        }
    }
}

void UARTComms::writeCommand(void){
    /*
     * byte 0: STATUS
     * byte 1: REF_TEMP
     * 
     * byte n: checksum     
     */
    
    writePacket(&tx_buffer[0], 19);
}

int UARTComms::writePacket(char * buffer, char length){
    if (write(uart_fd, buffer, length) == -1){
        perror("error writing");
        return -1;
    }
    return 1;
}

int UARTComms::writeByte(char buffer){
    if (write(uart_fd, &buffer, 1) == -1){
        perror("error writing");
        return -1;
    }
    return 1;
}

void UARTComms::decodePacket(void){
    int i = 0;
    char checksum = 0;
    
    for (i = 0; i < 20; i++){
        checksum ^= rx_buffer[i];
    }
    
    if (checksum == rx_buffer[20]){

    }
    else{
        std::cout << "DBG: Checksum failed" << std::endl;
    }
}


void UARTComms::stopReceiveThread(void){
    if (!quit_rx_thread){
        quit_rx_thread = true;
        // wait for thread to end
        pthread_join(receive_thread, NULL);            
    }
}

void UARTComms::closeUART(void){
    stopReceiveThread();
    if (!(uart_fd < 0)){
        close(uart_fd);
    }    
}

UARTComms::~UARTComms() {
    closeUART();
    closeLog();
}

