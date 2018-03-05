#include "reciver.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>

#include "proto.h"

#define LINE_BUFFER_SIZE  250

const char * servoFilename = "/dev/servoblaster";
int servo_fd;

void initRecive(void){
	servo_fd = open(servoFilename, O_WRONLY);
	if(servo_fd < 0) {
		syslog(LOG_NOTICE, "failed to open file %s, reason: %s\n", servoFilename, strerror(errno));
	}
}

void readPacket(struct PCK * packet){

	char buffer[LINE_BUFFER_SIZE] = {0};
	long int len = 0;
	int i = 0;
	for(i = 0; i < CHANNEL_COUNT; i++){
		
		int value = packet->channels[i];
		if(value == UNUSED_SERVO){
			continue;
		}

		len = sprintf(buffer, "%d=%d\n", i, value);
		if(len <= 0) {
			continue;
		}

		if(write(servo_fd, buffer, len) <= 0){
			syslog(LOG_NOTICE, "write to %s, reason: %s\n", servoFilename, strerror(errno));
		}
	}
}

void recive(char * pckptr, int size){
	int pckCount =  size / sizeof(struct PCK);

	struct PCK * pck_ptr = (struct PCK*)pckptr;

	int i = 0;
	for (i = 0; i < pckCount; i++){
		readPacket(pck_ptr);
		pck_ptr++;
	}
}

