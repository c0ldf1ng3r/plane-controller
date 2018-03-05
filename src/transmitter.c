#include "transmitter.h"
#include "proto.h"
#include "input.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <syslog.h>
#include <sys/time.h>
#include <pthread.h>
#include <libconfig.h>


typedef void (*init_f) ();
init_f init;

typedef void (*update_f) (long unsigned int t, struct INPUT *input, float *states);
update_f update;

struct INPUT input;
int js_fd;



int enabledChannels = 0;

#define MAX_MOD_NAME_LEN 256
char modName[MAX_MOD_NAME_LEN] = {0};

int channelMinValues[CHANNEL_COUNT] = {0};
int channelMaxValues[CHANNEL_COUNT] = {0};
int channelPolarityValues[CHANNEL_COUNT] = {0};

float states[CHANNEL_COUNT] = {0};
int normalizedStates[CHANNEL_COUNT] = {0};

void *module;

int loadConfig(char * configName) {
	config_t cfg;
	config_t *config = &cfg;

	config_init(config);

	if(config_read_file(config, configName) == CONFIG_FALSE){
		syslog(LOG_NOTICE, "Cannot read config file: %s", config_error_text(config));
		exit(1);
	}

	char * driverName = NULL;
	
	if(config_lookup_string(config, "driver", (const char**)&driverName) == CONFIG_FALSE){
		syslog(LOG_NOTICE, "Cannot find driver name: %s", config_error_text(config));
		exit(1);
	}
	
	if(strlen(driverName) < MAX_MOD_NAME_LEN){
		memcpy(modName, driverName, strlen(driverName) + 1);
	}else {
		syslog(LOG_NOTICE, "driver name > %d", MAX_MOD_NAME_LEN);
		exit(1);
	}
	
	int channelNumber;
	for (channelNumber = 0; channelNumber < CHANNEL_COUNT; channelNumber++){
		char minValueKey[255] = {0};
		char maxValueKey[255] = {0};
		char polarityKey[255] = {0};
		
		sprintf(minValueKey, "ch%d.minValue", channelNumber);
		sprintf(maxValueKey, "ch%d.maxValue", channelNumber);
		sprintf(polarityKey, "ch%d.polarity", channelNumber);

		int value = 0;
		if(config_lookup_int(config, minValueKey, &value) == CONFIG_FALSE) {
			syslog(LOG_NOTICE, "Cannot read minValue path: %s error: %s", minValueKey, config_error_text(config));
			break;
		}
		channelMinValues[channelNumber] = value;

		if(config_lookup_int(config, maxValueKey, &value) == CONFIG_FALSE) {
			syslog(LOG_NOTICE, "Cannot read maxValue path: %s error: %s", maxValueKey, config_error_text(config));
			break;
		}
		channelMaxValues[channelNumber] = value;

		if(config_lookup_int(config, polarityKey, &value) == CONFIG_FALSE) {
			syslog(LOG_NOTICE, "Cannot read polarity path: %s error: %s", polarityKey, config_error_text(config));
			break;
		}
		channelPolarityValues[channelNumber] = value;
	}

	enabledChannels = channelNumber;
	
	config_destroy(config);
	return 0;
}

void initTransmit(char *configName){

	if(loadConfig(configName) == -1){
		syslog(LOG_NOTICE, "Cannot load config: %s", configName);
		exit(1);
	}

	char  dlName[300] = {0};
	sprintf(dlName, "%s.so", modName);

    module = dlopen(dlName, RTLD_NOW);
    if (!module){
        syslog(LOG_NOTICE, "Cannot load %s", dlerror ());
        exit(1);
    }

    char* result;
    
    init = dlsym(module, "init");
    result = dlerror();
    if (result){
        syslog(LOG_NOTICE, "Cannot find init in %s: %s", modName, result);
        exit(1);
    }

    update = dlsym(module, "update");
    result = dlerror();
    if (result){
        syslog(LOG_NOTICE, "Cannot find update in %s: %s", modName, result);
        exit(1);
    }

    init();
    initInput();
}

void normalize(void){

    int i = 0;
    for(i = 0; i < enabledChannels; i++){

		float servoMax = (float)channelMaxValues[i];
        float servoMin = (float)channelMinValues[i];

        float sval = states[i];

		if(channelPolarityValues[i] < 0){
			sval = -sval;
		}
        
        sval = sval > 1.0f ? 1.0f : sval;

        sval = sval < -1.0f ? -1.0f : sval;

        sval = (sval + 1.0f) / 2.0f;


        normalizedStates[i] = (int)(servoMin + (sval * (servoMax - servoMin)));
    }   
}

void writePacket(char * pck, int maxSize){

    struct PCK * packet = (struct PCK*)pck;
    int i = 0;
    for(i = 0; i < enabledChannels; i++){
            packet->channels[i] = normalizedStates[i];
    }

    for(i = enabledChannels; i < CHANNEL_COUNT; i++){
        packet->channels[i] = UNUSED_SERVO;
    }
}

long unsigned int currentTime() {
    struct timeval te; 
    gettimeofday(&te, NULL);
    long unsigned int milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

int transmit(char * pckptr, int maxSize){
	
    getInput(&input);
    update(currentTime(), &input, states);
    normalize();
    writePacket(pckptr, maxSize);

    return sizeof(struct PCK);
}

