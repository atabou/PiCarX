#ifndef PID_HPP

    #define PID_HPP

    class PIDController {

        private:

            float kp;
            float ki;
            float kd;
            float dt;

            float ierr;
            float derr;

        public:

            PIDController() : PIDController(0.0f, 0.0f, 0.0f, 0.0f) {}
            PIDController(float kp, float ki, float kd, float dt);

            float pass(float setpoint, float value);

            void reset();

            ~PIDController();


    };

#endif // PID_HPP
