#include "picarx.hpp"

#include <stdexcept>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>

extern "C" {
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
}

#include "utilities.hpp"

#define RPI_I2C_FILE "/dev/i2c-1"
#define RPI_GPIO_CHIP 0

#define MCU_I2C_ADDR 0x14
#define MCU_CLK_FREQ 72000000
#define MCU_PWM_TICK 4095
#define MCU_RST_GPIO 5

#define MOTOR_PWM_TIMER_PRESCL_REG 0x43
#define MOTOR_PWM_TIMER_PERIOD_REG 0x47
#define MOTOR_PWM_TIMER_PRESCL_VAL 10

#define MOTOR1_PWM_CHAN 0x2D
#define MOTOR2_PWM_CHAN 0x2C
#define MOTOR1_DIR_GPIO 23
#define MOTOR2_DIR_GPIO 24

#define STEERING_PWM_TIMER_PRESCL_REG 0x40
#define STEERING_PWM_TIMER_PERIOD_REG 0x44
#define STEERING_PWM_TIMER_FREQCY_VAL 50.0
#define STEERING_PWM_TIMER_PRESCL_VAL (uint16_t) ((float) MCU_CLK_FREQ / ((float) MCU_PWM_TICK * (float) STEERING_PWM_TIMER_FREQCY_VAL)) - 1

#define STEERING_PWM_CHAN 0x22
#define STEERING_MAX_ANGLE 30
#define STEERING_MIN_ANGLE -30

#define SERVO_MAX_ANGLE 90.0
#define SERVO_MIN_ANGLE -90.0
#define SERVO_LEFT  500
#define SERVO_RIGHT 2500

#define BATT 0x13

#define ADC_VREF 3.3
#define ADC_RESO 4095
#define BAT_VDIV 3.0

PiCarX::PiCarX() {

    this->i2cfd = -1;
    this->gpio = NULL;
    this->mot1_dir_line = NULL;
    this->mot2_dir_line = NULL;
    this->mcu_rst_line  = NULL;

}


void write_to_chip(uint8_t i2cfd, uint8_t reg, uint16_t data) {


    uint16_t reversed = ((data & 0xff) << 8) + (data >> 8);
    i2c_smbus_write_word_data(i2cfd, reg, reversed);
    //printf("i2c sent: [%#04x] [%#06x]\n\n", reg, reversed);

}


void PiCarX::connect() {

    // Connect to GPIO chip
    this->gpio = gpiod_chip_open_by_number(RPI_GPIO_CHIP);

    if (gpio == NULL) {

        perror("gpio chip failed to open");
        return;

    }

    // Claim GPIO lines
    this->mot1_dir_line = gpiod_chip_get_line(gpio, MOTOR1_DIR_GPIO);

    if (this->mot1_dir_line == NULL) {

        perror("motor 1 direction line failed to open");
        this->disconnect();
        return;

    }

    this->mot2_dir_line = gpiod_chip_get_line(gpio, MOTOR2_DIR_GPIO);

    if (this->mot2_dir_line == NULL) {

        perror("motor 2 direction line failed to open");
        this->disconnect();
        return;

    }

    this->mcu_rst_line = gpiod_chip_get_line(gpio, MCU_RST_GPIO);

    if (this->mcu_rst_line == NULL) {

        perror("mcu reset line failed to open");
        this->disconnect();
        return;

    }

    // Set GPIO lines to output
    if (gpiod_line_request_output(this->mot1_dir_line, "motor 1 direction", 0) < 0) {

        perror("motor 1 direction line failed to set as output");
        this->disconnect();
        return;

    }

    if (gpiod_line_request_output(this->mot2_dir_line, "motor 2 direction", 0) < 0) {

        perror("motor 2 direction line failed to set as output");
        this->disconnect();
        return;

    }

    if (gpiod_line_request_output(this->mcu_rst_line, "mcu reset", 0) < 0) {

        perror("mcu reset line failed to set as output");
        this->disconnect();
        return;

    }

    // Reset MCU
    gpiod_line_set_value(this->mcu_rst_line, 1);
    usleep(10000);
    gpiod_line_set_value(this->mcu_rst_line, 0);
    usleep(10000);
    gpiod_line_set_value(this->mcu_rst_line, 1);
    usleep(10000);

    // Open I2C file descriptor
    this->i2cfd = open(RPI_I2C_FILE, O_RDWR);

    if (this->i2cfd < 0) {

        return;

    }

    // Set I2C slave address
    if (ioctl(this->i2cfd, I2C_SLAVE, MCU_I2C_ADDR) < 0) {

        // Close I2C file descriptor
        close(this->i2cfd);
        this->i2cfd = -1;

        // Release GPIO lines
        gpiod_line_release(this->mot1_dir_line);
        gpiod_line_release(this->mot2_dir_line);
        gpiod_line_release(this->mcu_rst_line);

        // Set GPIO lines to NULL
        this->mot1_dir_line = NULL;
        this->mot2_dir_line = NULL;
        this->mcu_rst_line  = NULL;

        return;

    }

    // Iniialze steering
    write_to_chip(i2cfd, STEERING_PWM_TIMER_PRESCL_REG, STEERING_PWM_TIMER_PRESCL_VAL);
    write_to_chip(i2cfd, STEERING_PWM_TIMER_PERIOD_REG, MCU_PWM_TICK);
    this->setSteeringAngle(0);

    // Initialize motors
    write_to_chip(i2cfd, MOTOR_PWM_TIMER_PRESCL_REG, MOTOR_PWM_TIMER_PRESCL_VAL);
    write_to_chip(i2cfd, MOTOR_PWM_TIMER_PERIOD_REG, MCU_PWM_TICK);
    this->setMotorSpeed(0);

}


void PiCarX::setMotorSpeed(float speed) {

    // Get direction and duty cycle
    uint16_t direction = (speed >= 0) ? 0 : 1;
    float duty_cycle = fabs(speed);

    // Set direction
    gpiod_line_set_value(this->mot1_dir_line, direction);
    gpiod_line_set_value(this->mot2_dir_line, !direction);

    // Calculate pulse width in number of ticks
    uint16_t pulse_width = duty_cycle * MCU_PWM_TICK;

    // Set PWM of the motors
    write_to_chip(i2cfd, MOTOR1_PWM_CHAN, pulse_width);
    write_to_chip(i2cfd, MOTOR2_PWM_CHAN, pulse_width);

}


void PiCarX::setSteeringAngle(float angle) {

    printf("angle: %f\n", angle);

    // Saturate angle between min and max
    angle = saturate(angle, STEERING_MIN_ANGLE, STEERING_MAX_ANGLE);
    printf("saturated: %f\n", angle);

    // Convert angle to ms
    float millis = map(angle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE, SERVO_LEFT, SERVO_RIGHT);
    printf("millisec: %f\n", millis);

    // Convert ms to duty cycle
    float duty_cycle = millis / 20000;
    printf("duty cycle: %f\n", duty_cycle); 

    // Calculate pulse width in number of ticks
    uint16_t pulse_width = duty_cycle * MCU_PWM_TICK;
    printf("pulse width: %u\n", (uint32_t) pulse_width);

    // Set PWM of the steering servo
    write_to_chip(i2cfd, STEERING_PWM_CHAN, pulse_width);

}


bool PiCarX::isConnected() {
    
    return this->i2cfd >= 0 && this->mot1_dir_line != NULL && this->mot2_dir_line != NULL && this->mcu_rst_line != NULL;

}


float PiCarX::getAnalogVoltage(uint8_t channel) {

    if (channel != A0 && channel != A1 && channel != A2 && channel != A3) {

        throw std::invalid_argument("Invalid analog channel: must be A0 A1 A2 or A3");

    }

    uint16_t raw = i2c_smbus_read_word_data(this->i2cfd, channel);

    return raw * ADC_VREF / ADC_RESO;

}


float PiCarX::getBatteryVoltage() {

    uint16_t raw = i2c_smbus_read_word_data(this->i2cfd, BATT);

    float divided = raw * ADC_VREF / ADC_RESO;

    float batterVoltage = divided * BAT_VDIV;

    return batterVoltage;

}


void PiCarX::disconnect() {

    if (this->i2cfd >= 0) {

        close(this->i2cfd);
        this->i2cfd = -1;

    }

    if (this->mot1_dir_line != NULL) {

        // Check that GPIO line is set to output and pull GPIO line low
        if (gpiod_line_direction(this->mot1_dir_line) == GPIOD_LINE_DIRECTION_OUTPUT) {
            gpiod_line_set_value(this->mot1_dir_line, 0);
        }

        // Release GPIO line
        gpiod_line_release(this->mot1_dir_line);

        // Set GPIO line to NULL
        this->mot1_dir_line = NULL;

    }

    if (this->mot2_dir_line != NULL) {

        // Check that GPIO line is set to output and pull GPIO line low
        if (gpiod_line_direction(this->mot2_dir_line) == GPIOD_LINE_DIRECTION_OUTPUT) {
            gpiod_line_set_value(this->mot2_dir_line, 0);
        }

        // Release GPIO line
        gpiod_line_release(this->mot2_dir_line);

        // Set GPIO line to NULL
        this->mot2_dir_line = NULL;

    }

    if (this->mcu_rst_line != NULL) {

        // Check that GPIO line is set to output and reset MCU if it is
        if (gpiod_line_direction(this->mcu_rst_line) == GPIOD_LINE_DIRECTION_OUTPUT) {

            // Reset MCU and keep it in reset state
            gpiod_line_set_value(this->mcu_rst_line, 1);
            usleep(10000);
            gpiod_line_set_value(this->mcu_rst_line, 0);
            usleep(10000);
            gpiod_line_set_value(this->mcu_rst_line, 1);
            usleep(10000);

        }

        // Release GPIO line
        gpiod_line_release(this->mcu_rst_line);

        // Set GPIO line to NULL
        this->mcu_rst_line = NULL;

    }

    if (this->gpio != NULL) {

        gpiod_chip_close(gpio);
        this->gpio = NULL;

    }

}


PiCarX::~PiCarX() {

    this->disconnect();

}
