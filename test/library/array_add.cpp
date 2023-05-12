#include <string.h>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <iostream>
#include "CGRA.h"



#define SIZE  20
int A[SIZE], B[SIZE], C[SIZE];

__attribute__((noinline))
void array_add(){

   
   for (int i=0;i<SIZE; i++){
      #ifdef CGRA_COMPILER
      please_map_me();
      #endif

      C[i] = A[i]+B[i];
   }


}

int main(){

int i,j;

for (i=0;i<SIZE; i++){
      A[i] = i * 2 + 5;
      B[i] = i * 3;
      C[i] = 0;
    }
    
//array_add();

//for (i=0;i<SIZE; i++) printf("%d\n", C[i]);

std::ifstream file("array_add_mem_alloc.txt");
std::map<std::string, int> base_addresses;
std::string line;
std::getline(file, line); // Skip the header line

while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string var_name;
    int base_addr;
    std::getline(iss, var_name, ',');
    iss >> base_addr;
    base_addresses[var_name] = base_addr;
}

HyCUBESim::CGRA cgraInstance(4, 4, 1, 4096); // Initialize CGRA instance 
cgraInstance.configCGRA("array_add_PartPredDFG.xml_DP1_XDim=4_YDim=4_II=4_MTP=1_binary.bin",4,4);


// Write data for each variable
for (const auto& pair : base_addresses) {
    std::vector<uint8_t> byteData;
    if(pair.first == "A"){
        for (int i = 0; i < SIZE; ++i) {
            for (int j = 0; j < 4; ++j) {
                byteData.push_back((A[i] >> (j * 8)) & 0xFF);
            }
        }
        cgraInstance.writeDMEM(cgraInstance, pair.second, byteData.data(), byteData.size());
    }
    if(pair.first == "B"){
        for (int i = 0; i < SIZE; ++i) {
            for (int j = 0; j < 4; ++j) {
                byteData.push_back((B[i] >> (j * 8)) & 0xFF);
            }
        }
        cgraInstance.writeDMEM(cgraInstance, pair.second, byteData.data(), byteData.size());
    }
    if(pair.first == "C"){
        for (int i = 0; i < SIZE; ++i) {
            for (int j = 0; j < 4; ++j) {
                byteData.push_back((C[i] >> (j * 8)) & 0xFF);
            }
        }
        cgraInstance.writeDMEM(cgraInstance, pair.second, byteData.data(), byteData.size());
    }
    
    if(pair.first == "loopend"){
        uint8_t byteD = 0;
        cgraInstance.writeDMEM(cgraInstance, pair.second, &byteD, 1);
    }
    
    if(pair.first == "loopstart"){
        uint8_t byteD = 1;
        cgraInstance.writeDMEM(cgraInstance, pair.second, &byteD, 1);
    }
    
}

// Invoke CGRA
cgraInstance.invokeCGRA(cgraInstance);

// Read and print data for each variable
for (const auto& pair : base_addresses) {
    if(pair.first == "loopstart" || pair.first == "loopend") continue;
    std::vector<uint8_t> byteData(SIZE * 4);
    cgraInstance.readDMEM(cgraInstance, pair.second, byteData.data(), byteData.size());

    std::cout << "Data for variable " << pair.first << ": ";
    for (int i = 0; i < SIZE; ++i) {
        int value = 0;
        for (int j = 0; j < 4; ++j) {
            value |= (byteData[i * 4 + j] << (j * 8));
        }
        std::cout << value << " ";
    }
    std::cout << std::endl;
}


    

return 0;

}
