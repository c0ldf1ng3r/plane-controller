#include "input.h"

#include <pthread.h>
#include <linux/joystick.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <syslog.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <syslog.h>
#include <string.h>

const int AXIS_MAX = 32767;
const char * joystickFilename = "/dev/input/js0";

int js_fd = 0;
struct INPUT input;

pthread_t inputThread;
pthread_mutex_t inputMutex = PTHREAD_MUTEX_INITIALIZER;


void *handleInput(void *ptr);


void getInput(struct INPUT *dstInput){
	pthread_mutex_lock(&inputMutex);
	memcpy(&input, dstInput, sizeof(struct INPUT));
	pthread_mutex_unlock(&inputMutex);
}

int filter(struct js_event * e){
	int type = e->type &= ~JS_EVENT_INIT;
	if(type == JS_EVENT_AXIS && e->number > 0 && e->number < 4){
		return 1;
	}

	if(type == JS_EVENT_BUTTON && e->number > 0 && e->number < 17){
		return 1;
	}

	return 0;
}

double normalizeAxis(int value) {
	return (double)value / (double)AXIS_MAX;
}

void interpret(struct js_event * e, struct INPUT * input){
	int type = e->type &= ~JS_EVENT_INIT;

	if(type == JS_EVENT_AXIS){		
		switch (e->number) {
			case 0: 
				input->lx = normalizeAxis(-e->value);
			break;
			case 1: 
				input->ly = normalizeAxis(-e->value);
			break;
			case 2: 
				input->rx = normalizeAxis(-e->value);
			break;
			case 3: 
				input->ry = normalizeAxis(-e->value);
			break;
		}
	}

	if(type == JS_EVENT_BUTTON){
		switch (e->number) {
			case 11: 
				input->r1 = e->value > 0 ? 1 : 0;
			break;
			case 9: 
				input->r2 = e->value > 0 ? 1 : 0;
			break;

			case 10: 
				input->l1 = e->value > 0 ? 1 : 0;
			break;
			case 8: 
				input->l2 = e->value > 0 ? 1 : 0;
			break;

			case 4: 
				input->up = e->value == 1 ? 1 : 0;
			break;
			case 6: 
				input->down = e->value == 1 ? 1 : 0;
			break;
			case 7: 
				input->left = e->value == 1 ? 1 : 0;
			break;
			case 5: 
				input->right = e->value == 1 ? 1 : 0;
			break;

			case 15: 
				input->sqr = e->value == 1 ? 1 : 0;
			break;
			case 13: 
				input->crc = e->value == 1 ? 1 : 0;
			break;
			case 14: 
				input->crs = e->value == 1 ? 1 : 0;
			break;
			case 12: 
				input->tri = e->value == 1 ? 1 : 0;
			break;

			case 1: 
				input->lpad = e->value == 1 ? 1 : 0;
			break;
			case 2: 
				input->rpad = e->value == 1 ? 1 : 0;
			break;

			case 0: 
				input->sel = e->value == 1 ? 1 : 0;
			break;
			case 3: 
				input->start = e->value == 1 ? 1 : 0;
			break;

			case 16: 
				input->ps = e->value == 1 ? 1 : 0;
			break;

		}
	}
}


void *handleInput(void *ptr){
	struct js_event e;
	
	while(1){
		if(js_fd == 0){
			if(access(joystickFilename, R_OK) != -1){
				js_fd = open(joystickFilename, O_RDONLY);	
			}else{
				sleep(1);
				continue;
			}
		}

		if(read(js_fd, &e, sizeof(e)) == -1){
			close(js_fd);
			js_fd = 0;
		}

		if(filter(&e) == 0){
			continue;
		}

		pthread_mutex_lock(&inputMutex);
		interpret(&e, &input);
		pthread_mutex_unlock(&inputMutex);
	}
}

void initInput(void) {
	memset(&input, 0, sizeof(struct INPUT));

	pthread_create(&inputThread, NULL, handleInput, NULL);
}
