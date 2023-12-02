#include <iostream>
#include <math.h>
#include <fstream>
#include <vector>
#include <string>

// version 0.2
// last edit 02DEC2023
// by kst

bool extra_output = false;

typedef std::vector<int> vInt;    //row of ints
typedef std::vector<vInt> v2Int;   //2d field
typedef std::vector<v2Int> v3Int;  //3d field or vector field

// When calling v2Ints Field[a][b] a is the row and b is the column so (x,y) would be calles Field[y][x]
// Same goes for v3Ints with Vector_Field[a][b][c] where it gives the you cth element of the vector at position x=b,y=a


class Step {
    private:
        std::string file,file_prime;
        int max_shift,window_size,vec_res_x,vec_res_y;
        v2Int field,field_prime;

    public:
       Step(std::string file, std::string file_prime, int max_shift, int window_size, int vec_res_x, int vec_res_y)
       : file(file), file_prime(file_prime), max_shift(max_shift), window_size(window_size), vec_res_x(vec_res_x), vec_res_y(vec_res_y) {
            read_file(true);
            read_file(false);
    }

    v3Int vec_field;

        void read_file(bool is_prime) {   //Function to read .dat file. Filename and path definded by input string
            std::string filename;
            if (is_prime) {
                filename = file_prime;
            }else {
                filename = file;
            }

            std::ifstream rfile;                     //define file as reader
            rfile.open(filename);                        //open file
            if (!rfile) {                                //if the file doesnt exist
                std::cout << "File "  << filename << " not found" << std::endl;  //Error message
                }
            else {                      //if the file does exist
                int width,height;       //initialise ints for saving width and height
                rfile >> width;       //Reading first two values to determine image size
                rfile >> height;
                std::cout << "Reading file " << filename << " with height " << std::to_string(height) << " and width " << std::to_string(width) << std::endl;
                if(extra_output){std::cout << "Reading field: " << std::endl;}                                                                                  //Output for QA
                v2Int rfield;        //Defining actual field

                for(int h=0;h<height;h++) {         //iterate over file height
                    vInt row;                       //initialise vector as row
                    for(int w=0;w<width;w++) {            //Iterating over width of file
                        int data = 0;                      
                        rfile >> data;
                        row.push_back(data);                             //Adding next value in file to row
                        if(extra_output){std::cout << data << " ";}
                    }
                    rfield.push_back(row);                                  //Adding row to field
                    if(extra_output){std::cout << std::endl;}
                }
                rfile.close();  
                                    //Closing file
                if (is_prime) {
                    field_prime = rfield;
                }else {
                    field = rfield;
                }                      //Returning v2Int field
            }
        }


        v2Int create_window(v2Int wfield, int x, int y) {  //extract window of given size as given coordinates
        if(extra_output){std::cout << "Extracting window of size " << std::to_string(window_size) << " at coordinates " << std::to_string(x) << "," << std::to_string(y) << ":" << std::endl;}
            v2Int window;                           //initialising v2Int as "window"
                for(int i=0;i<window_size;i++) {  //window defined with i and j where j is the horizontal axis and i the vertical
                    vInt row;                //iterate over columns and rows of window, initialising a new vInt "row" for each row
                    for(int j=0;j<window_size;j++) {
                        int abs_i = y-(window_size/2)+i;       //calculate position of the pixel in the original grid
                        int abs_j = x-(window_size/2)+j;          
                        int num = 0;
                        if( !(abs_i < 0 || abs_j < 0 || abs_i >= wfield.size() ||  abs_j >= wfield.at(i).size()) ) {   //check if pixel is within given picture
                            num = wfield.at(abs_i).at(abs_j);                                                           //set num to value of pixel
                        }                                                                                   //if pixel is outside of original field num stays 0
                        if(extra_output){std::cout << std::to_string(num) << " ";}
                        row.push_back(num);                                                             //appending num to the row
                    }
                    if(extra_output){std::cout << std::endl;}
                    window.push_back(row);                                  //adding the row to the field
                    }
            return window;                                      //returning the v2Int "field"
            if(extra_output){std::cout << std::endl;}
        }

        int correlate(v2Int window, v2Int window_prime) {  //calculate the correlation value for two given windows
            //TODO: check for dimensions here
            int sum = 0;
            int window_size = window.size();
            for(int k=0;k<window_size;k++) {
                for(int l=0;l<window_size;l++) {                //iterating over the window size adding the values to the sum
                    sum += window[k][l]*window_prime[k][l];
                }
            }
            if(extra_output){std::cout << "Calculated R to " << std::to_string(sum) << std::endl;}
            return sum;
        }

        void write_vec_csv(std::string extra) {
            std::string filename = "output_" + extra + ".csv";     //Open a new csv file for output
            std::ofstream wfile;
                wfile.open (filename);
                for (int row=0; row<vec_field.size(); row++) {                          
                    for (int e=0; e<vec_field[row].size();e++) {
                        std::string add =  "[" + std::to_string(vec_field.at(row).at(e)[0]) + "," +  std::to_string(vec_field.at(row).at(e)[1]) + "]";
                        wfile << add;
                        if (e < vec_field[row].size() - 1) {
                            wfile << ";";
                        }
                    }
                    wfile << "\n";
                }
                wfile.close();
        }

        void cross_correlate_fields() {
            for(int a=0; a<vec_res_y;a++) {
                v2Int vec_row;
                for(int b=0;b<vec_res_x;b++) {
                    if(extra_output){std::cout << "Cross correlating for vector at (" << std::to_string(b) << "," << std::to_string(a) << ")" <<std::endl;}
                    int x,y;
                    x = b * (field.at(0).size()/vec_res_x);
                    y = a * (field.size()/vec_res_y);
                    v2Int window = create_window(field,x,y);
                    std::vector<int*> R;
                    for(int s_x=-max_shift;s_x<max_shift;s_x++) {                //iterating over shifts until maximum allowed is reached comparing to field prime
                        for(int s_y=-max_shift; s_y<max_shift; s_y++) {
                            v2Int window_prime = create_window(field_prime,x+s_x,y+s_y);
                            int* R_list = new int[3];                                           //collecting correlation values and corresponding coordinates in a vector type list
                            R_list[0] = correlate(window, window_prime);
                            R_list[1] = x + s_x;
                            R_list[2] = y + s_y;

                            R.push_back(R_list);
                        }
                    }
                    int r_max_index=0;
                    for(int e=0; e<R.size();e++) {              //Iterating through list of correlation values to find maximum
                        if(extra_output){std::cout << "comparing " << std::to_string(R.at(e)[0]) << " and " << std::to_string(R.at(r_max_index)[0]) << std::endl;}
                        if(R.at(e)[0] > R.at(r_max_index)[0]) {
                            r_max_index = e;
                        }
                    }
                    if(extra_output){std::cout << "Maximum R of " << std::to_string(R.at(r_max_index)[0]) << " at coordinates " << std::to_string(R.at(r_max_index)[1]) << "," << std::to_string(R.at(r_max_index)[2]) << std::endl;}
                    int vec_x = R.at(r_max_index)[1] - x;
                    int vec_y = R.at(r_max_index)[2] - y;

                    vInt r_max_vec ;
                    r_max_vec.push_back(vec_x);      
                    r_max_vec.push_back(vec_y);           //calculating vector from field position to highest correlated position and adding it to the position in the vector field
                    if(extra_output){std::cout << "Adding vector " << std::to_string(vec_x) << "," << std::to_string(vec_y) << std::endl;}
                    vec_row.push_back(r_max_vec);
                }
                vec_field.push_back(vec_row);
            }

        }
};

void write_field_csv(v2Int field, std::string extra) {
            std::string filename = "output_field_" + extra + ".csv";
            std::ofstream wfile;
                wfile.open (filename);
                for (int col = 0; col < field[0].size(); col++) {
                    for (int row = 0; row < field.size(); row++) {
                        wfile << field[row][col];
                        wfile << ",";
                    }
                    wfile << "\n";
                }
                wfile.close();
        }

int main() {
    int max_shift = 5;
    int window_size = 3;
    int vec_res_x = 10;
    int vec_res_y = 5;

    int start = 4;
    int end = 7;

    v2Int vel_map(vec_res_x, vInt(vec_res_y, 0));

    std::vector<Step> steps;
    for (int s = 0; s < end-start; s++) {
        std::string filename =  "examples/test" +std::to_string(s+start) + ".dat";
        std::string filename_prime = "examples/test" +std::to_string(s+start+1) + ".dat";

        steps.push_back(Step(filename, filename_prime,max_shift,window_size,vec_res_x,vec_res_y));
        
        steps[s].cross_correlate_fields();
        steps[s].write_vec_csv(std::to_string(s));

        for(int y = 0; y< steps[s].vec_field.size();y++) {
            for(int x = 0; x< steps[s].vec_field.at(y).size();x++) {
                double magnitude = sqrt(pow(steps[s].vec_field[y][x][0], 2) + pow(steps[s].vec_field[y][x][1], 2));
                vel_map[x][y] += magnitude / (end - start);
            }
        }

    }

    write_field_csv(vel_map, "vel");


}

