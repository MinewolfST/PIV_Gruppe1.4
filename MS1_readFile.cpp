#include <iostream>
#include <math.h>
#include <fstream>
#include <vector>
#include <string>


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

int main() {          //test code
    v2Int window = read_file("examples/test.dat");
    std::cout << std::endl;
    for(int x=0;x<window.size();x++) {
        for(int y=0;y<window[0].size();y++) {
            std::cout << std::to_string(window[x][y]) << " ";
        }
    std::cout << std::endl;
    }
    std::cin.get();
}