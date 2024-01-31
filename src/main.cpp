#include "picarx.hpp"
#include "pid.hpp"

#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>


PiCarX* picarx = NULL;
PIDController* PID = NULL;


void gracefulExit(int sig) {

    if (picarx != NULL) {
        picarx->disconnect();
        delete picarx;
        picarx = NULL;
    }

    if (PID != NULL) {
        delete PID;
        PID = NULL;
    }

    exit(0);

}


void run() {

    while (true) {

        // Get battery voltage
        float battery_voltage = picarx->getBatteryVoltage();
	printf("battery: %f V\n", battery_voltage);

        // Get analog voltage from A0 and A1
        float left  = picarx->getAnalogVoltage(A0) * 5.7f;
        float right = picarx->getAnalogVoltage(A3) * 5.7f;

	printf("left: %f V\n", left);
	printf("right: %f V\n", right);
        
	// Get log difference between left and right
        //float diff = log(left + 0.25) - log(right + 0.25);
        float diff = log(left) - log(right);
	//float diff = left - right;

	printf("log diff: %f\n\n", diff);

        // Verify the validity of the difference
        if (!isnan(diff) && !isinf(diff)) {
        
            // Get pid response
            float response = PID->pass(0.0f, diff);

            // Set steering angle
            picarx->setSteeringAngle(response);
        
        }

	usleep(100000);

    }

}


void test() {

    picarx->setMotorSpeed(0.5f);
    sleep(2);
    picarx->setMotorSpeed(1.0f);
    sleep(2);
    picarx->setMotorSpeed(-0.5f);
    sleep(2);
    picarx->setMotorSpeed(-1.0f);
    sleep(2);
    picarx->setMotorSpeed(0.0f);
 
    for (float i=-30; i<=30; i+=10) {

	    picarx->setSteeringAngle(i);
	    sleep(2);

    }

}


int main() {

    // Register graceful exit handler
    signal(SIGINT, gracefulExit);

    // Create PiCarX object
    picarx = new PiCarX();
    PID    = new PIDController(15.0f, 0.0f, 0.0f, 0.1f);

    // Connect to PiCarX
    picarx->connect();

    if (!picarx->isConnected()) {
    
        printf("Failed to connect to PiCarX\n");
        return 1;
    
   }

    // Set motor speed
    picarx->setMotorSpeed(0.0f);
    
    // Set steering angle
    picarx->setSteeringAngle(0.0f);


    //test();
    run();

    if (picarx != NULL) {
        picarx->disconnect();
        delete picarx;
        picarx = NULL;
    }

    if (PID != NULL) {
        delete PID;
        PID = NULL;
    }


    return 0;


} 
