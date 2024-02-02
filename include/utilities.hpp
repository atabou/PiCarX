#ifndef UTILITIES_HPP

    #define UTILITIES_HPP

    /**
     * @brief Saturates a value to the range [min, max].
     * @param value The value to saturate.
     * @param min The minimum value.
     * @param max The maximum value.
     * @return The saturated value.
     */
    float saturate(float value, float min, float max);
    
    /**
     * @brief Maps a value from one range [min1, max1] to another range [min2, max2].
     * @param value The value to map.
     * @param min1 The minimum value of the input range.
     * @param max1 The maximum value of the input range.
     * @param min2 The minimum value of the output range.
     * @param max2 The maximum value of the output range.
     * @return The mapped value.
    */
    float map(float value, float min1, float max1, float min2, float max2);

#endif // UTILITIES_HPP
