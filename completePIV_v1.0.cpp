#include <iostream>
#include <math.h>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <chrono>
#include <thread>
#include <omp.h>


// version 1.0
// last edit 04DEC2023
// by kst

bool extra_output = false;

typedef std::vector<int> vInt;    //row of ints
typedef std::vector<vInt> v2Int;   //2d field
typedef std::vector<v2Int> v3Int;  //3d field or vector field
typedef std::vector<double> vDub;
typedef std::vector<vDub> v2Dub;
typedef std::vector<v2Dub> v3Dub;

// When calling v2Ints Field[a][b] a is the row and b is the column so (x,y) would be calles Field[y][x]
// Same goes for v3Ints with Vector_Field[a][b][c] where it gives the you cth element of the vector at position x=b,y=a


//Fancy console Progress bar
void printProgressBar(int progress, int total, int barWidth = 70) {
    float percentage = static_cast<float>(progress) / total;
    int progressBarWidth = static_cast<int>(percentage * barWidth);

    std::cout << "[";

    for (int i = 0; i < progressBarWidth; ++i) {
        std::cout << "#";
    }

    std::cout << ">";

    for (int i = progressBarWidth + 1; i < barWidth; ++i) {
        std::cout << "-";
    }

    std::cout << "] " << std::setprecision(3) << percentage * 100.0 << "%" << "\r";
    std::cout.flush();
}

// Class representing a step in the PIV (transition between two data sets)
class Step {
private:
    std::string file, file_prime;
    int max_shift, window_size, vec_res_x, vec_res_y;
    double cutoff;
    v2Int field, field_prime;

     // Function to calculate the mean vector of neighbors for a given position
    vInt calculate_neighbor_mean(int x, int y) {
        int sum_x = 0, sum_y = 0;
        int count = 0;

        // Loop through neighbors in a 3x3 window
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                int new_x = x + i;
                int new_y = y + j;

                // Check if the neighbor is within bounds
                if (new_x >= 0 && new_x < vec_field[0].size() && new_y >= 0 && new_y < vec_field.size()) {
                    sum_x += vec_field[new_y][new_x][0];
                    sum_y += vec_field[new_y][new_x][1];
                    count++;
                }
            }
        }

        // Calculate the mean vector
        vInt mean_vector;
        if (count > 0) {
            mean_vector.push_back(sum_x / count);
            mean_vector.push_back(sum_y / count);
        }

        return mean_vector;
    }

public:
    // Constructor to initialize Step object with input parameters
    Step(std::string file, std::string file_prime, int max_shift, int window_size, int vec_res_x, int vec_res_y, double cutoff)
    : file(file), file_prime(file_prime), max_shift(max_shift), window_size(window_size), vec_res_x(vec_res_x), vec_res_y(vec_res_y), cutoff(cutoff) {
    read_file(true);
    read_file(false);
}


    v3Int vec_field;

    // Function to read .dat file
    void read_file(bool is_prime) {
        std::string filename;

        // Set filename based on whether the current field is prime or not
        if (is_prime) {
            filename = file_prime;
        } else {
            filename = file;
        }

        std::ifstream rfile;
        rfile.open(filename);

        // Check if the file is successfully opened
        if (!rfile) {
            std::cout << "File " << filename << " not found" << std::endl;
        } else {
            int width, height;
            rfile >> width;
            rfile >> height;

            v2Int rfield;

            // Loop through each row and column of the file
            for (int h = 0; h < height; h++) {
                vInt row;

                for (int w = 0; w < width; w++) {
                    int data = 0;
                    rfile >> data;
                    row.push_back(data);
                }

                rfield.push_back(row);
            }

            rfile.close();

            // Set the appropriate field based on whether it is prime or not
            if (is_prime) {
                field_prime = rfield;
            } else {
                field = rfield;
            }
        }
    }

    // Function to create a window of given size at specified coordinates
    v2Int create_window(v2Int wfield, int x, int y) {
        if (extra_output) {
            std::cout << "Extracting window of size " << window_size << " at coordinates " << x << "," << y << ":" << std::endl;
        }

        v2Int window;

        // Loop through each row and column of the window
        for (int i = 0; i < window_size; i++) {
            vInt row;

            for (int j = 0; j < window_size; j++) {
                int abs_i = y - (window_size / 2) + i;
                int abs_j = x - (window_size / 2) + j;
                int num = 0;

                // Check if the pixel is within the bounds of the original picture
                if (!(abs_i < 0 || abs_j < 0 || abs_i >= wfield.size() || abs_j >= wfield.at(i).size())) {
                    num = wfield.at(abs_i).at(abs_j);
                }

                row.push_back(num);
            }

            window.push_back(row);
        }

        return window;

        if (extra_output) {
            std::cout << std::endl;
        }
    }

    // Function to calculate the correlation value for two given windows
    int correlate(v2Int window, v2Int window_prime) {
        int sum = 0;
        int window_size = window.size();

        // Loop through each row and column of the window
        for (int k = 0; k < window_size; k++) {
            for (int l = 0; l < window_size; l++) {
                sum += window[k][l] * window_prime[k][l];
            }
        }

        if (extra_output) {
            std::cout << "Calculated R to " << sum << std::endl;
        }

        return sum;
    }

    // Function to perform cross-correlation between fields with a progress bar
void cross_correlate_fields() {
    int counter = 1;
    const int totalIterations = (vec_res_x * vec_res_y) - 1;
    v3Int temp_vec_field(vec_res_y, v2Int(vec_res_x, vInt(2, 0)));
    int a = 0;

    // Loop through each row of the vector field
    #pragma omp parallel for private(a)
    for (int a = 0; a < vec_res_y; a++) {
        v2Int vec_row;

        // Loop through each element in the row
        for (int b = 0; b < vec_res_x; b++) {

            #pragma omp critical
            { 
                if(!extra_output) {
                    // Print progress bar
                    printProgressBar(counter, totalIterations);
                    counter++;
                }
            }

            if (extra_output) {
                std::cout << "Cross correlating for vector at (" << b << "," << a << ")" << std::endl;
            }

            int x, y;
            x = b * (field.at(0).size() / vec_res_x);
            y = a * (field.size() / vec_res_y);

            v2Int window = create_window(field, x, y);
            std::vector<int*> R;

            // Loop through possible shifts for cross-correlation
            for (int s_x = -max_shift; s_x < max_shift; s_x++) {
                for (int s_y = -max_shift; s_y < max_shift; s_y++) {
                    v2Int window_prime = create_window(field_prime, x + s_x, y + s_y);
                    int* R_list = new int[3];
                    R_list[0] = correlate(window, window_prime);
                    R_list[1] = x + s_x;
                    R_list[2] = y + s_y;

                    R.push_back(R_list);
                }
            }

            int r_max_index = 0;


                // Find the maximum correlation value
                for (int e = 0; e < R.size(); e++) {
                    if (extra_output) {
                        std::cout << "comparing " << R.at(e)[0] << " and " << R.at(r_max_index)[0] << std::endl;
                    }

                    if (R.at(e)[0] > R.at(r_max_index)[0]) {
                        r_max_index = e;
                    }
                }

                if (extra_output) {
                    std::cout << "Maximum R of " << R.at(r_max_index)[0] << " at coordinates " << R.at(r_max_index)[1] << "," << R.at(r_max_index)[2] << std::endl;
                }

                int vec_x = R.at(r_max_index)[1] - x;
                int vec_y = R.at(r_max_index)[2] - y;

                vInt r_max_vec;
                r_max_vec.push_back(vec_x);
                r_max_vec.push_back(vec_y);

                if (extra_output) {
                    std::cout << "Adding vector " << vec_x << "," << vec_y << std::endl;
                }

                vec_row.push_back(r_max_vec);

                // Clean up memory allocated for R_list
                for (int i = 0; i < R.size(); i++) {
                    delete[] R[i];
                }
            }
            
            for (int e = 0; e<vec_row.size();e++) {
                temp_vec_field[a][e][0] = vec_row[e][0];
                temp_vec_field[a][e][1] = vec_row[e][1];
            }
        }

        vec_field = temp_vec_field;

        if (!extra_output) {
            // Complete the progress bar when done
            printProgressBar(totalIterations, totalIterations);
            std::cout << std::endl;
        }
    }

     // Function to smooth vectors in vec_field based on the cutoff value
    void smooth_vectors() {
        int counter = 0;

        // Loop through each row and column of vec_field
        for (int y = 0; y < vec_field.size(); y++) {
            for (int x = 0; x < vec_field[y].size(); x++) {
                vInt current_vector = vec_field[y][x];

                // Calculate the mean vector of neighbors
                vInt neighbor_mean = calculate_neighbor_mean(x, y);

                // Check if the deviation is above the cutoff
                double deviation = sqrt(pow(current_vector[0] - neighbor_mean[0], 2) + pow(current_vector[1] - neighbor_mean[1], 2));

                if (deviation > cutoff) {
                    // Replace the vector with the mean vector of neighbors
                    vec_field[y][x] = neighbor_mean;
                    counter++;

                    if (extra_output) {
                        std::cout << "Smoothed vector at (" << std::to_string(x) << "," << std::to_string(y) << ")" << std::endl;
                    }
                }
            }
        }

        if (extra_output) {
            std::cout << "Smoothed " << std::to_string(counter) << " vectors" << std::endl;
        }
    }
};

// Function to write vector field to CSV file with debug output
void write_vec_csv(v3Int vec_field, std::string extra) {
    std::string filename = "output_" + extra + ".csv";
    std::ofstream wfile;
    wfile.open(filename);

    if (true) {
        std::cout << "Writing vector field to CSV: " << filename << std::endl;
        std::cout << "Vector field dimensions: " << vec_field.size() << " rows x " << vec_field[0].size() << " elements per row" << std::endl;
    }

    // Loop through each row of the vector field
    for (int row = 0; row < vec_field.size(); row++) {
        // Loop through each element of the vector in the row
        for (int e = 0; e < vec_field[row].size(); e++) {
            std::string add = "[" + std::to_string(vec_field.at(row).at(e)[0]) + "," + std::to_string(vec_field.at(row).at(e)[1]) + "]";
            wfile << add;

            // Add a separator if it's not the last element in the row
            if (e < vec_field[row].size() - 1) {
                wfile << ";";
            }
        }

        wfile << "\n";
    }
}
    // Function to write vector field to CSV file with debug output
void write_vDub_csv(v3Dub vec_field, std::string extra) {
    std::string filename = "output_" + extra + ".csv";
    std::ofstream wfile;
    wfile.open(filename);

    if (extra_output) {
        std::cout << "Writing vector field to CSV: " << filename << std::endl;
        std::cout << "Vector field dimensions: " << vec_field.size() << " rows x " << vec_field[0].size() << " elements per row" << std::endl;
    }

    // Loop through each row of the vector field
    for (int row = 0; row < vec_field.size(); row++) {
        // Loop through each element of the vector in the row
        for (int e = 0; e < vec_field[row].size(); e++) {
            std::string add = "[" + std::to_string(vec_field.at(row).at(e)[0]) + "," + std::to_string(vec_field.at(row).at(e)[1]) + "]";
            wfile << add;

            // Add a separator if it's not the last element in the row
            if (e < vec_field[row].size() - 1) {
                wfile << ";";
            }
        }

        wfile << "\n";
    }

    wfile.close();

    if (extra_output) {
        std::cout << "Write complete." << std::endl;
    }
}

// Function to write field to CSV file with debug output
void write_field_csv(v2Dub field, std::string extra) {
    std::string filename = "output_field_" + extra + ".csv";
    std::ofstream wfile;
    wfile.open(filename);

    if (extra_output) {
        std::cout << "Writing field to CSV: " << filename << std::endl;
        std::cout << "Field dimensions: " << field.size() << " rows x " << field[0].size() << " columns" << std::endl;
    }

    // Loop through each column of the field
    for (int y = 0; y < field.size(); y++) {
        // Loop through each row of the field
        for (int x = 0; x < field[0].size(); x++) {
            wfile << field[y][x];

            // Add a comma if it's not the last element in the row
            if (x < field[0].size() - 1) {
                wfile << ",";
            }
        }

        wfile << "\n";
    }

    wfile.close();

    if (extra_output) {
        std::cout << "Write complete." << std::endl;
    }
}


int main() {
    // Parameters
    int max_shift = 15;
    int window_size = 50;
    int vec_res_x = 130;
    int vec_res_y = 100;
    double cutoff = 5.0;

    int start = 96;
    int end = 115;

    // Initialize velocity map and mean vector field
    v2Dub vel_map(vec_res_y, vDub(vec_res_x, 0));
    v3Dub mean_field(vec_res_y, v2Dub(vec_res_x, vDub(2, 0)));

    std::vector<Step> steps;

    // Loop through each step
    for (int s = 0; s < ((end - start)/2 + 1); s++) {
        std::string filename = "Rohdaten/Date_Messdaten_TVL_" + std::to_string(start+(s*2) ) + ".dat";
        std::string filename_prime = "Rohdaten/Date_Messdaten_TVL_" + std::to_string(start + (s*2) + 1) + ".dat";

        std::cout << "Now processing step " << s+1 << " of " << ((end - start)/2 + 1) << std::endl;
        std::cout << "Cross correlating between " << filename << " and " << filename_prime << std::endl;

        // Create Step object and perform correlation
        steps.push_back(Step(filename, filename_prime, max_shift, window_size, vec_res_x, vec_res_y, cutoff));
        steps.back().cross_correlate_fields();
        steps.back().smooth_vectors();

        write_vec_csv(steps.back().vec_field, std::to_string(s));

        
    }

    std::cout << "Finished cross correlation, now constructing combined outputs" << std::endl;
    const int totalIterations = steps.size() * (vec_res_x * vec_res_y);
    int counter = 0;

    double divisor = (steps.size());

    for (int s = 0; s < steps.size(); s++) {
        // Accumulate values for velocity map and mean vector field
        for (int y = 0; y < steps[s].vec_field.size(); y++) {
            for (int x = 0; x < steps[s].vec_field[y].size(); x++) {

                double magnitude = sqrt(pow(steps[s].vec_field[y][x][0], 2) + pow(steps[s].vec_field[y][x][1], 2)) / divisor;
                vel_map[y][x] += magnitude;

                double x_mean = steps[s].vec_field[y][x][0] / divisor;
                double y_mean = steps[s].vec_field[y][x][1] / divisor;

                if (extra_output) {
                std::cout << "For Vector: (" << steps[s].vec_field[y][x][0] << "," << steps[s].vec_field[y][x][1] << ")" << std::endl;
                std::cout << "Adding to mean: (" << x_mean << "," << y_mean << ")" << std::endl;
                }

                mean_field[y][x][0] += x_mean;
                mean_field[y][x][1] += y_mean;
                

                // Print progress bar
                printProgressBar(counter, totalIterations, 1);
                counter++;
            }
        }
    }

    // Complete the progress bar when done
    printProgressBar(totalIterations, totalIterations);
    std::cout << std::endl;

    // Write velocity map and mean vector field to CSV
        write_field_csv(vel_map, "vel");
        write_vDub_csv(mean_field, "mean");

    return 0;
}