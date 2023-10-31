#include <iostream>
#include <math.h>
#include <fstream>
#include <vector>
#include <string>

// version 0.6
// last edit 31OCT2023
// by kst

//All coordinate systems are flipped. x is down and y is to the right

bool extra_output = true;

typedef std::vector<int> vInt;    //row of ints
typedef std::vector<vInt> v2Int;   //2d field
typedef std::vector<v2Int> v3Int;  //3d field or vector field

v2Int read_file(std::string filename) {
    std::ifstream file;
    file.open(filename);
    if (!file) {
        std::cout << "File "  << filename << " not found" << std::endl;  //Error message if filename not found
        }
    else {
        int width,height;
        file >> width;       //Reading first to values to determine image size
        file >> height;
        std::cout << "Reading file " << filename << " with height " << std::to_string(height) << " and width " << std::to_string(width) << std::endl;
        if(extra_output){std::cout << "Reading field: " << std::endl;}
        v2Int field;

        for(int h=0;h<height;h++) {
            vInt row;
            for(int w=0;w<width;w++) {            //Iterating over height and width while reading the remaining file and adding them to the output field
                int data = 0;
                file >> data;
                row.push_back(data);
                if(extra_output){std::cout << data << " ";}
            }
            field.push_back(row);
            if(extra_output){std::cout << std::endl;}
        }
        file.close();                      //Closing file and returning field
        return field;
    }
}

v2Int create_window(v2Int field, int window_size, int x, int y) {  //extract window of given size as given coordinates
if(extra_output){std::cout << "Extracting window of size " << std::to_string(window_size) << " at coordinates " << std::to_string(x) << "," << std::to_string(y) << ":" << std::endl;}
    v2Int window;
        for(int i=0;i<window_size;i++) {
            vInt row;                //iterate over columns and rows of window
            for(int j=0;j<window_size;j++) {
                int abs_i = x-(window_size/2)+i;       //calculate position of the pixel in the overall grid
                int abs_j = y-(window_size/2)+j;
                int num = 0;
                if( !(abs_i < 0 || abs_j < 0 || abs_i >= field.size() ||  abs_j >= field.at(i).size()) ) {   //check if pixel is within given picture
                    num = field.at(abs_i).at(abs_j);
                }
                if(extra_output){std::cout << std::to_string(num) << " ";}
                row.push_back(num);
            }
            if(extra_output){std::cout << std::endl;}
            window.push_back(row);
            }
    return window;
    if(extra_output){std::cout << std::endl;}
}

int correlate(v2Int window, v2Int window_prime) {  //calculate the correlation value for two given windows
    //check for dimensions here
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

v3Int cross_correlate(v2Int field, v2Int field_prime, int window_size, int max_shift) {
    v3Int  vec_field;
    int vec_field_height = std::floor(field.size()/window_size);                        //Initialising vector field to fit the given data based on window size
    
    int vec_field_width = std::floor(field[0].size()/window_size);
    for(int a=0;a<vec_field_height;a++) {                                           //iterating over vector positions
        v2Int vec_row;
        for(int b=0;b<vec_field_width;b++)  {
            if(extra_output){std::cout << "Cross correlating for vector at " << std::to_string(a) << "," << std::to_string(b) <<std::endl;}
            int x,y;
            x = (a*window_size)+(window_size/2);            //calculating centre of window
            y = (b*window_size)+(window_size/2);

            v2Int window = create_window(field,window_size,x,y);  //extracting window from field
            std::vector<int*> R;
            for(int s_x=-max_shift;s_x<max_shift;s_x++) {                //iterating over shifts until maximum allowed is reached comparing to field prime
                for(int s_y=-max_shift; s_y<max_shift; s_y++) {
                    v2Int window_prime = create_window(field_prime,window_size,x+s_x,y+s_y);
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
    return vec_field;
}

int main() {          //test code
    v2Int field = read_file("test.dat");
    v2Int field_prime = read_file("test3.dat");
    v3Int  vec_field = cross_correlate(field,field_prime,3,3);
    std::cout << std::endl << "Calculations complete, printing vector field: " << std::endl;

    v2Int vel_field;

    for(int i=0;i<vec_field.size();i++) {
        vInt vel_row;
        for(int j=0;j<vec_field[i].size();j++) {
            std::cout << std::to_string(vec_field.at(i).at(j)[0]) << "," << std::to_string(vec_field.at(i).at(j)[1]) << " ";
            vel_row.push_back(floor(sqrt( (vec_field.at(i).at(j)[0]*vec_field.at(i).at(j)[0]) + (vec_field.at(i).at(j)[1]*vec_field.at(i).at(j)[1]) )));
        }
    std::cout << std::endl;
    }
    std::cout << std::endl;
    for (const auto& row : vel_field) {
        for (int value : row) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }


}