#include "pid.hpp"


PIDController::PIDController(float kp, float ki, float kd, float dt) {

    this->kp = kp;
    this->ki = ki;
    this->kd = kd;
    this->dt = dt;

    this->ierr = 0.0f;
    this->derr = 0.0f;

}


float PIDController::pass(float setpoint, float value) {

    float err = setpoint - value;

    this->ierr += ki * err * dt;

    float response = kp * err + this->ierr + kd * (err - this->derr) / dt;

    this->derr = err;

    return response;
 
}


void PIDController::reset() {

    this->ierr = 0.0f;
    this->derr = 0.0f;

}


PIDController::~PIDController() {


}

