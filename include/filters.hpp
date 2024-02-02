#ifndef FILTERS_HPP

    #define FILTERS_HPP

    /**
     * @brief A first-order low-pass FIR filter.
     */
    struct FIRFilter {

        float alpha;    /** Filter coefficient. */
        float xp;       /** Filter output.      */

        /**
         * @brief Construct a new FIRFilter object.
         * @param alpha The filter coefficient.
         * @param x0 The initial output value.
         */
        FIRFilter(float alpha=1.0f, float x0=0.0f);

        /**
         * @brief Updates the filter using the new input value.
         * @param x The new input value.
         * @return The filter output.
         */
        float pass(float x);


        /**
         * @brief Resets the filter state.
         */
        void reset(float x0=0.0f);

    };


#endif