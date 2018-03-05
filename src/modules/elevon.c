#include "../input_str.h"
#include <syslog.h>
#include <math.h>

float lFlap = 0;
float rFlap = 0;
float thrust = 0;

float rollTrim = 0;
float elevatorTrim = 0;
float trimStep = 0.05;

char trimUpState = 0;
char trimDownState = 0;
char trimLeftState = 0;
char trimRightState = 0;
char resetTrimsState = 0;
char exponentialsState = 0;
char halfThrustState = 0;
char fullThrustState = 0;

char halfThrust = 0;
char fullThrust = 0;
char exponential = 0;

#define THRUST_CHANNEL 0
#define LEFT_FLAP_CHANNEL 2
#define RIGHT_FLAP_CHANNEL 1


void init(void){

}

void update(long unsigned int t, struct INPUT *input, float *states){

	if(input->r2 == 1){
		halfThrust = 1;
	}else {
		halfThrust = 0;
	}

	if(input->r1 == 1){
		fullThrust = 1;
	}else {
		fullThrust = 0;
	}

	if(trimUpState != input->up && input->up == 0){
		elevatorTrim += trimStep;
	}
	trimUpState = input->up;

	if(trimDownState != input->down && input->down == 0){
		elevatorTrim -= trimStep;
	}
	trimDownState = input->down;

	if(trimLeftState != input->left && input->left == 0){
		rollTrim -= trimStep;
	}
	trimLeftState = input->left;

	if(trimRightState != input->right && input->right == 0){
		rollTrim += trimStep;
	}
	trimRightState = input->right;

	if(resetTrimsState != input->crs && input->crs == 0){
		rollTrim = 0;
		elevatorTrim = 0;
	}
	resetTrimsState = input->crs;

	if(exponentialsState != input->tri && input->tri == 0){
		exponential = (exponential == 0 ? 1 : 0);
	}
	exponentialsState = input->tri;

	
	if(fullThrust == 1){
		thrust = 1.0f;
	}else if(halfThrust == 1){
		thrust = 0.5f;
	}else{
		if(input->ly <= 0.0f) {
			thrust = -1.0f;
		}else {
			thrust = (input->ly * 2.0f) - 1.0f;
		}
	} 

	float roll = 0;
	float pitch = 0;
	
	if(exponential != 0) {
		float pitchFactor = input->ry;
		float rollFactor = input->rx;
		float exp = 2;
		pitch = (pitchFactor/exp + exp*pow(pitchFactor, 5))/3.0f;
		roll = (rollFactor/exp + exp*pow(rollFactor, 5))/3.0f;
	}else {
		pitch = input->ry;
		roll = input->rx;
	}

	lFlap = -roll + pitch + rollTrim + elevatorTrim;
	rFlap = roll + pitch - rollTrim + elevatorTrim;

	states[THRUST_CHANNEL] = thrust;
	states[LEFT_FLAP_CHANNEL] = lFlap;
	states[RIGHT_FLAP_CHANNEL] = rFlap;
}

