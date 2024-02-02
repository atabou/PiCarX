#ifndef PID_HPP

    #define PID_HPP

    class PIDController {

        private:

            float kp;       /** Proportional gain.  */
            float ki;       /** Integral gain.      */
            float kd;       /** Derivative gain.    */
            float dt;       /** Time step.          */
            float ierr;     /** Integral error.     */
            float derr;     /** Derivative error.   */

        public:

            /**
             * @brief Construct a new PIDController object.
             * @param kp The proportional gain.
             * @param ki The integral gain.
             * @param kd The derivative gain.
             * @param dt The time step.
            */
            PIDController() : PIDController(0.0f, 0.0f, 0.0f, 0.0f) {}
            
            /**
             * @brief Construct a new PIDController object.
             * 
             * @param kp The proportional gain.
             * @param ki The integral gain.
             * @param kd The derivative gain.
             * @param dt The time step.
            */
            PIDController(float kp, float ki, float kd, float dt);

            /**
             * @brief Updates the PID controller using the setpoint and the current input value.
             * 
             * @param setpoint The desired value.
             * @param value The current value.
             * @return The output of the PID controller. 
             */
            float pass(float setpoint, float value);

            /**
             * @brief Resets the state of the PID controller.
            */
            void reset();

            ~PIDController();


    };

#endif // PID_HPP
