#include "../input_str.h"
#include <syslog.h>


long unsigned int last_t = 0;
float state = 0.0;
int direction = -1;
float step = 0.1f;


#define THRUST_CHANNEL 0
#define LEFT_FLAP_CHANNEL 1
#define RIGHT_FLAP_CHANNEL 2


void init(void){

}

void update(long unsigned int t, struct INPUT *input, float *states){

	if((t - last_t) >= 1000) {	
		last_t = t;

		if(direction > 0) {
			state = state + step;
		}else {
			state = state - step;
		}	
		
		
	}

	states[0] = state;
	states[1] = state;
	states[2] = state;
}
