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

        // Get analog voltage from A0 and A1
        float left  = picarx->getAnalogVoltage(A0) * 5.7f;
        float right = picarx->getAnalogVoltage(A1) * 5.7f;

        // Get log difference between left and right
        float diff = log(left + 0.25) - log(right + 0.25);

        // Verify the validity of the difference
        if (!isnan(diff) && !isinf(diff)) {
        
            // Get pid response
            float response = PID->pass(0.0f, diff);

            // Set steering angle
            picarx->setSteeringAngle(response);
        
        }

    }

}


void test() {

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
    PID    = new PIDController(0.5f, 0.0f, 0.0f, 0.1f);

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

    test();
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
