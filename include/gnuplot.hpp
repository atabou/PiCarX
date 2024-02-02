#ifndef GNUPLOT_HPP

    #define GNUPLOT_HPP

#include <cstdio>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>


    class Delay {
        
        private:
            std::vector<double> data;
            size_t oldest;

        public:

            // Constructor initializes the buffer with zeros
            Delay() {}

            // Constructor initializes the buffer with initial data
            Delay(const std::vector<double>& initialData) {

                data = initialData;
                oldest = 0;

            }

            // Copy constructor
            Delay(const Delay& other) {

                data = other.data;
                oldest = other.oldest;

            }

            // Assingment operator
            Delay& operator=(const std::vector<double>& initialData) {

                data = initialData;
                oldest = 0;

                return *this;

            }

            // Add a new data point to the buffer
            void add(double value) {

                data[oldest] = value;
                oldest = (oldest + 1) % data.size();

            }

            // Get the value at the specified index where the specified index is 0 for the oldest value
            float operator[](size_t index) const {

                return data[(oldest + index) % data.size()];

            }

            size_t size() const {

                return data.size();

            }
    
    };


    class GNUPlot {

        private:
            
            FILE* gnuplotPipe = nullptr;
            double start_time;
            double update_rate;
            Delay xData;
            Delay yData;

        public:
        
            GNUPlot(const std::string& title, const std::string& label, float min, float max, size_t buffer_size, double update_rate) {
        
                // Open pipe to GNUplot
                gnuplotPipe = popen("gnuplot -persist", "w");
        
                // Check if pipe was opened
                if (!gnuplotPipe) {
            
                    std::cerr << "Could not open pipe to GNUplot" << std::endl;
            
                } else {

                    // Initialize GNUplot settings
                    sendCommand("set title '" + title + "'");
                    sendCommand("set xlabel 'time (s)'");
                    sendCommand("set ylabel '" + label + "'");
                    sendCommand("set grid");
                    sendCommand("set term x11");
                    sendCommand("set style data linespoints");

                    // Initialize data buffer
                    xData = std::vector<double>(buffer_size, 0);
                    yData = std::vector<double>(buffer_size, 0);

                    // Get start time
                    this->start_time  = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
                    this->update_rate = update_rate;

                    std::vector<double> time(buffer_size, 0.0f);
                    std::vector<double> data(buffer_size, 0.0f);

                    // Fill buffer with initial data
                    for (size_t i = 0; i < buffer_size; ++i) {
                        
                        time[i] = - (float) (buffer_size - i) * update_rate;
                        data[i] = 0.0f;

                    }

                    // Initialize the delays
                    xData = Delay(time);
                    yData = Delay(data);

                    // Set the x range
                    sendCommand("set xrange [" + std::to_string(xData[0]) + ":" + std::to_string(xData[buffer_size - 1]) + "]");
                    sendCommand("set yrange [" + std::to_string(min) + ":" + std::to_string(max) + "]");

		}
            
            }


            void disconnect() {

                if (gnuplotPipe) {
                    sendCommand("quit");
                    pclose(gnuplotPipe);
                    gnuplotPipe = nullptr;
                }
            
            }

            ~GNUPlot() {
                
                this->disconnect();
            
            }

            // Prevent copy and assignment
            GNUPlot(const GNUPlot&) = delete;
            GNUPlot& operator=(const GNUPlot&) = delete;

            void sendCommand(const std::string& command) {
                
                if (gnuplotPipe) {
                
                    fprintf(gnuplotPipe, "%s\n", command.c_str());
                    fflush(gnuplotPipe);

                }
            
            }

            void add(double x) {

                // Get current time
                double current_time = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();

                // Add data point to buffer
                xData.add(current_time - start_time);
                yData.add(x);
		
            }

            void plot() {

                sendCommand("plot '-' with linespoints");

                for (size_t i = 0; i < xData.size(); ++i) {
                    fprintf(gnuplotPipe, "%f %f\n", xData[i], yData[i]);
                }

                sendCommand("e"); // End of dataset

            }

    };


#endif // GNUPLOT_HPP
