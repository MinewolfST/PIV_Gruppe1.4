#include <iostream>
#include <math.h>
#include <fstream>
#include <vector>
#include <string>



std::vector<std::vector<int>> read_file(std::string filename) {
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
        std::cout << "Reading field: " << std::endl;
        std::vector<std::vector<int>> field;

        for(int h=0;h<height;h++) {
            std::vector<int> row;
            for(int w=0;w<width;w++) {            //Iterating over height and width while reading the remaining file and adding them to the output field
                int data = 0;
                file >> data;
                row.push_back(data);
                std::cout << data << " ";
            }
            field.push_back(row);
            std::cout << std::endl;
        }
        file.close();                      //Closing file and returning field
        return field;
    }
}


int main() {          //test code
    std::vector<std::vector<int>> window = read_file("test.dat");
    std::cout << std::endl;
    for(int x=0;x<window.size();x++) {
        for(int y=0;y<window[0].size();y++) {
            std::cout << std::to_string(window[x][y]) << " ";
        }
    std::cout << std::endl;
    }
}

