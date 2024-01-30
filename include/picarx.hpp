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

            int i2cfd; // File descriptor for I2C
            
            struct gpiod_chip *gpio;
            struct gpiod_line *mot1_dir_line;
            struct gpiod_line *mot2_dir_line;
            struct gpiod_line *mcu_rst_line;

        public:

            PiCarX();

            void connect();

            void setMotorSpeed(float speed);
            void setSteeringAngle(float angle);

            float getAnalogVoltage(uint8_t channel);
            float getBatteryVoltage();

            bool isConnected();
            void disconnect();

            ~PiCarX();

    };


#endif // PICARX_HPP
