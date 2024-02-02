#include "picarx.hpp"
#include "pid.hpp"
#include "filters.hpp"
#include "utilities.hpp"

#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>


#define FILTER_ALPHA_COEFF 1.0f
#define LOG_DIFF_BIAS 0.25f

#define KP 10.0f
#define KI 0.0f
#define KD 0.0f

#define SPEED 0.5f

/**
 * @brief Gracefully exits the program.
 * This function is called when the user presses Ctrl+C.
 * It disconnects from PiCarX and exits the program.
 * @param sig The signal number.
 * @return void
*/
void gracefulExit(int sig);


PiCarX* picarx = NULL;
PIDController* PID = NULL;
FIRFilter* filter = NULL;


int main() {

    int dt_us = 10000;
    float dt_s = dt_us / 1e-6;

    // Register graceful exit handler
    signal(SIGINT, gracefulExit);

    // Create and connect PiCarX object
    picarx = new PiCarX();

    picarx->connect();

    if (!picarx->isConnected()) {
    
        printf("Failed to connect to PiCarX\n");
        return 1;
    
   }

    // Initialize PID controller.
    PID = new PIDController(KP, KI, KD, dt_s);

    // Initialize FIR filter by calculating the mean of the first 100 samples
    float mu0 = 0.0f;

    for (int i = 0; i < 100; i++) {

        float left  = picarx->getAnalogVoltage(A0) * 5.7f;
        float right = picarx->getAnalogVoltage(A3) * 5.7f;
        
        mu0 += logdiff(left, right, LOG_DIFF_BIAS);
        usleep(dt_us); // TODO check the ability of the MCU to handle this sampling rate
    
    }

    mu0 /= 100.0f;

    filter = new FIRFilter(FILTER_ALPHA_COEFF, mu0);

    picarx->setMotorSpeed(SPEED);
    // Control loop
    while (true) {

        // Get battery voltage
        float battery_voltage = picarx->getBatteryVoltage();
	    
        // Get analog voltage from A0 and A1
        float left  = picarx->getAnalogVoltage(A0) * 5.7f;
        float right = picarx->getAnalogVoltage(A3) * 5.7f;

	    // Get log difference between left and right
        float diff = logdiff(left, right, LOG_DIFF_BIAS);

        // Verify the validity of the difference
        if (!isnan(diff) && !isinf(diff)) {
        
            // Filter the log difference
            float filtered = filter->pass(diff);

            // Get pid response
            float response = PID->pass(0.0f, filtered);

            printf("%.2f -> %.2f\n", filtered, response);

            // Set steering angle
            picarx->setSteeringAngle(response);
        
        }

        usleep(dt_us);

    }

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


void gracefulExit(int sig) {

    printf("Gracefully exiting...\n");

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
