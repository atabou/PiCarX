#ifndef GNUPLOT_HPP

    #define GNUPLOT_HPP

#include <cstdio>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

    class GNUPlot {

        private:
            
            FILE* gnuplotPipe = nullptr;
            double start_time;
            std::vector<double> xData;
            std::vector<double> yData;

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
                    sendCommand("set xdata time");
                    sendCommand("set timefmt '%s'");

                    // Initialize data buffer
                    xData = std::vector<double>(buffer_size, 0);
                    yData = std::vector<double>(buffer_size, 0);

                    // Get start time
                    start_time = std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();

                    // Fill buffer with initial data
                    // We start with 0 at the right end of the plot
                    for (size_t i = 0; i < buffer_size; ++i) {
                        
                        xData[i] = start_time - (buffer_size - i) * update_rate;
                        yData[i] = 0.0f;

                    }

                    // Set the x range
                    sendCommand("set xrange [" + std::to_string(xData.front()) + ":" + std::to_string(xData.back()) + "]");
                    sendCommand("set yrange [" + std::to_string(min) + ":" + std::to_string(max) + "]");
                    
                }
            
            }

            ~GNUPlot() {

                if (gnuplotPipe) {
                    pclose(gnuplotPipe);
                }
            
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

            void addDataPoint(double x) {

                // Remove oldest data point
                xData.erase(xData.begin());
                yData.erase(yData.begin()); 

                // Get timestamp
                auto now = std::chrono::system_clock::now();

                // Add new data point
                xData.push_back(std::chrono::duration<double>(now.time_since_epoch()).count());
                yData.push_back(x);

                // Set the new x range
                sendCommand("set xrange [" + std::to_string(xData.front()) + ":" + std::to_string(xData.back()) + "]");
                sendCommand("plot '-' with linespoints");

                // Send new data points
                for (size_t i = 0; i < xData.size(); ++i) {
                    fprintf(gnuplotPipe, "%lf %lf\n", xData[i], yData[i]);
                }

                sendCommand("e"); // End of dataset

            }

    };


#endif // GNUPLOT_HPP