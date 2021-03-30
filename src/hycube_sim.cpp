//============================================================================
// Name        : hycube_sim.cpp
// Author      : Manupa Karunaratne
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;
#include "data_structures.h"
#include "CGRA.h"



int main(int argc, char* argv[]) {

	if(argc < 2){
		cout << "HyCUBE_Sim expect two(old version)/three(new version) arguments : instruction trc, data trace and base address allocation\n";
		return -1;
	}

	string cmemfileName(argv[1]);
	string dmemfileName(argv[2]);
	int x_ = 4;
	int y_ = 4;
	int memory_tile_size = 4096;
	if(argc>4){
		x_ = atoi(argv[4]);
		y_ = atoi(argv[5]);
	}
	if(argc>6){
		memory_tile_size = atoi(argv[6]);
	}
	
	HyCUBESim::CGRA cgraInstance(x_,y_, memory_tile_size);
	cgraInstance.parseCMEM(cmemfileName);
//	cout << "argc: "<< argc <<"\n";
	if(argc>=4){
		cout << "Parsing data file with base address pointers\n";
		string memallocfileName(argv[3]);
		cgraInstance.parseDMEM(dmemfileName,memallocfileName);
	}else{
		cgraInstance.parseDMEM(dmemfileName);
	}
	cgraInstance.dumpRawData();
	cgraInstance.printInterestedAddrOutcome();

	int count=0;
	while(cgraInstance.dmem[memory_tile_size/2-1]==0){
		cgraInstance.executeCycle(count);
		count++;
	}
	for(int i = 0; i<15; i++){// this is for additional latency
		cgraInstance.executeCycle(count);
		count++;
	}
	cgraInstance.printInterestedAddrOutcome();


	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
