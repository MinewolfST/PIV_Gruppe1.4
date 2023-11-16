#include <iostream>
#include <math.h>
#include <fstream>
#include <vector>
#include <string>

// version 0.8
// last edit 16NOV2023
// by kst

bool extra_output = false;

typedef std::vector<int> vInt;    //row of ints
typedef std::vector<vInt> v2Int;   //2d field
typedef std::vector<v2Int> v3Int;  //3d field or vector field

// When calling v2Ints Field[a][b] a is the row and b is the column so (x,y) would be calles Field[y][x]
// Same goes for v3Ints with Vector_Field[a][b][c] where it gives the you cth element of the vector at position x=b,y=a

v2Int read_file(std::string filename) {   //Function to read .dat file. Filename and path definded by input string
    std::ifstream file;                     //define file as reader
    file.open(filename);                        //open file
    if (!file) {                                //if the file doesnt exist
        std::cout << "File "  << filename << " not found" << std::endl;  //Error message
        return {{0}};
        }
    else {                      //if the file does exist
        int width,height;       //initialise ints for saving width and height
        file >> width;       //Reading first two values to determine image size
        file >> height;
        std::cout << "Reading file " << filename << " with height " << std::to_string(height) << " and width " << std::to_string(width) << std::endl;
        if(extra_output){std::cout << "Reading field: " << std::endl;}                                                                                  //Output for QA
        v2Int field;        //Defining actual field

        for(int h=0;h<height;h++) {         //iterate over file height
            vInt row;                       //initialise vector as row
            for(int w=0;w<width;w++) {            //Iterating over width of file
                int data = 0;                      
                file >> data;
                row.push_back(data);                             //Adding next value in file to row
                if(extra_output){std::cout << data << " ";}
            }
            field.push_back(row);                                  //Adding row to field
            if(extra_output){std::cout << std::endl;}
        }
        file.close();                      //Closing file
        return field;                       //Returning v2Int field
    }
}


v2Int create_window(v2Int field, int window_size, int x, int y) {  //extract window of given size as given coordinates
if(extra_output){std::cout << "Extracting window of size " << std::to_string(window_size) << " at coordinates " << std::to_string(x) << "," << std::to_string(y) << ":" << std::endl;}
    v2Int window;                           //initialising v2Int as "window"
        for(int i=0;i<window_size;i++) {  //window defined with i and j where j is the horizontal axis and i the vertical
            vInt row;                //iterate over columns and rows of window, initialising a new vInt "row" for each row
            for(int j=0;j<window_size;j++) {
                int abs_i = y-(window_size/2)+i;       //calculate position of the pixel in the original grid
                int abs_j = x-(window_size/2)+j;          
                int num = 0;
                if( !(abs_i < 0 || abs_j < 0 || abs_i >= field.size() ||  abs_j >= field.at(i).size()) ) {   //check if pixel is within given picture
                    num = field.at(abs_i).at(abs_j);                                                           //set num to value of pixel
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

void write_csv(v3Int field) {
    std::ofstream file;
        file.open ("output.csv");
        for (int row=0; row<field.size(); row++) {
            for (int e=0; e<field[row].size();e++) {
                std::string add =  "[" + std::to_string(field.at(row).at(e)[0]) + "," +  std::to_string(field.at(row).at(e)[1]) + "]";
                file << add;
                if (e < field[row].size() - 1) {
                    file << ";";
                }
            }
            file << "\n";
        }
        file.close();
}

v3Int cross_correlate_fields(v2Int field, v2Int field_prime, int window_size, int max_shift, int vec_res_y, int vec_res_x) {
    v3Int vec_field;
    for(int a=0; a<vec_res_y;a++) {
        v2Int vec_row;
        for(int b=0;b<vec_res_x;b++) {
            if(extra_output){std::cout << "Cross correlating for vector at (" << std::to_string(b) << "," << std::to_string(a) << ")" <<std::endl;}
            int x,y;
            x = b * (field.at(0).size()/vec_res_x);
            y = a * (field.size()/vec_res_y);
            v2Int window = create_window(field,window_size,x,y);
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
    v2Int field = read_file("Date_Messdaten_TVL_96.dat");
    v2Int field_prime = read_file("Date_Messdaten_TVL_97.dat");
    v3Int  vec_field = cross_correlate_fields(field,field_prime,50,10,30,40);

    write_csv(vec_field);

}

