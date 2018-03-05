#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <syslog.h>
#include <stdarg.h>
#include <time.h>

#include "reciver.h"
#include "transmitter.h"
#include "proto.h"

const int MAX_PACKET_SIZE = sizeof(struct PCK)*10;

const int TRANSMITTER_RESOLUTION = 20;

const int MIN_ARG_COUNT = 2;
const char * reciveFileName = "/tmp/rtx-recive";
const char * sendFileName = "/tmp/rtx-send";

#define daemonMode

int send_fd;
int recive_fd;

void usage(void);
void daemonize(void);
void run(char * mode, char * modName);

void sendLoop(char *modName);
void reciveLoop(void);

void cleanup(void);
static void sighandler(int signum);
void msg(char * fmt, ...);

int main(int argc, char **argv) {

	if(argc - 1 < MIN_ARG_COUNT) {
        usage();
        exit(0);
    }

    char * mode = argv[1];

    signal(SIGPIPE, SIG_IGN);
    signal(SIGTERM, sighandler);
    
#ifdef daemonMode
        daemonize();
        openlog(NULL, LOG_PID, LOG_DAEMON);
#endif
	
	char *configFilename = NULL;
	if( argc -1 > 1){
		configFilename = argv[2];
	}    

    run(mode, configFilename);
	return 0;
}

void usage(void) {    
    printf("Usage: control <mode r/t> <config filename>\n");
}

void daemonize(void) {
	pid_t pid;

	pid = fork();
    if (pid < 0){
        exit(0);
    }

    if (pid > 0){
        exit(0);
    }

    if(setsid() < 0){
        exit(0);
    }

    pid = fork();

    if (pid < 0) {
        exit(0);
    }

    
    if (pid > 0) {
        exit(0);
    }


    umask(0);
    chdir("/");

    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--){
        close(x);
    }
}

void run(char *mode, char * configName) {

	if (mode[0] == 'r') {
        msg("starting reciving\n");
		reciveLoop();
	}else if (mode[0] == 't' && configName != NULL) {
        msg("starting transmiting\n");
		sendLoop(configName);
	}else {
		usage();
		exit(0);
	}	
}

void reciveLoop(){

    recive_fd = open(reciveFileName, O_RDONLY);
    if(recive_fd < 0) {
    	msg("failed to open fifo %s, reason: %s\n", reciveFileName, strerror(errno));
        exit(1);
    }

    initRecive();

    char packet[sizeof(struct PCK)];
    while(1){
        memset(packet, 0, sizeof(struct PCK));
        int readed = read(recive_fd, packet, sizeof(struct PCK));

        recive(packet, readed);
    }
}

void sendLoop(char * modName){

    send_fd = open(sendFileName, O_WRONLY);
    if(send_fd < 0) {
    	msg("failed to open fifo %s, reason: %s\n", sendFileName, strerror(errno));
        exit(1);
    }

    initTransmit(modName);

    char packet[MAX_PACKET_SIZE];
    int packetSize = 0;


    time_t desiredDuration = 100000000 / TRANSMITTER_RESOLUTION;
    struct timespec start, stop, waitDuration;
    while(1){

        clock_gettime(CLOCK_REALTIME, &start);

    	memset(packet, 0, MAX_PACKET_SIZE);
        
        packetSize = transmit(packet, MAX_PACKET_SIZE);
        
        write(send_fd, packet, packetSize);

        clock_gettime(CLOCK_REALTIME, &stop);

        time_t taken = stop.tv_nsec - start.tv_nsec;

        waitDuration.tv_sec = 0;
        waitDuration.tv_nsec = desiredDuration - taken;
        nanosleep(&waitDuration, NULL);
    }
}


void cleanup(void) {

    if(send_fd){
        close(send_fd);
    }
    if(recive_fd){
        close(recive_fd);
    }
}

static void sighandler(int signum){
    msg("SIGTERM recived");
    cleanup();
    exit(0);
}

void msg(char * fmt, ...){
    va_list args;
    va_start(args, fmt);
    va_end(args);

#ifdef daemonMode
    vsyslog(LOG_NOTICE, fmt, args);
#else
    vprintf(fmt, args);
#endif

}
