#ifndef PICARX_HPP

    #define PICARX_HPP

    #include <stdint.h>
    #include <gpiod.h>

    #define A0 0x17
    #define A1 0x16
    #define A2 0x15
    #define A3 0x14

    class PiCarX {
            
        private:

            int i2cfd;                          /** I2C file descriptor.                           */
            struct gpiod_chip *gpio;            /** GPIO chip file connection.                     */
            struct gpiod_line *mot1_dir_line;   /** GPIO line to control the direction of motor 1. */
            struct gpiod_line *mot2_dir_line;   /** GPIO line to control the direction of motor 2. */
            struct gpiod_line *mcu_rst_line;    /** GPIO line to reset the MCU.                    */

        public:

            /**
             * @brief Construct a new PiCarX object (does nothing, use connect() to connect to the PiCar-X
             */
            PiCarX();

            /**
             * @brief Connects to the PiCar-X.
            */
            void connect();

            /**
             * @brief Sets the speed of the motors.
             * @param speed The speed of the motors in the range [-1, 1].
            */
            void setMotorSpeed(float speed);

            /**
             * @brief Sets the steering angle.
             * @param angle The steering angle in the range [-30; 30].
            */
            void setSteeringAngle(float angle);

            /**
             * @brief Reads the analog voltage from the specified channel.
             * @param channel The channel to read from (A0, A1, A2, A3).
             * @return The analog voltage scaled to the range [0, 3.3] V.
            */
            float getAnalogVoltage(uint8_t channel);

            /**
             * @brief Reads the battery voltage.
             * @return The battery voltage in the range [0, 9.9] V.
            */
            float getBatteryVoltage();

            /**
             * @brief Checks if the PiCar-X is connected.
             * @return True if the PiCar-X is connected, false otherwise.
            */
            bool isConnected();

            /**
             * @brief Disconnects from the PiCar-X.
            */
            void disconnect();

            ~PiCarX();

    };


#endif // PICARX_HPP
