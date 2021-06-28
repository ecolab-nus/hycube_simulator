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
#define ARCHI_16BIT 
#define MULTI_DATA

int main(int argc, char* argv[]) {

	if(argc < 2){
		cout << "HyCUBE_Sim expect two(old version)/three(new version) arguments : instruction trc, data trace and base address allocation\n";
		return -1;
	}

	string cmemfileName(argv[1]);
	string dmemfileName(argv[2]);
	int x_ = 4;
	int y_ = 4;
#ifdef ARCHI_16BIT
	int memory_tile_size = 16384;
#else
	int memory_tile_size = 4096;
#endif
	if(argc>4){
		x_ = atoi(argv[4]);
		y_ = atoi(argv[5]);
	}
	if(argc>6){
		memory_tile_size = atoi(argv[6]);
	}
	int c;

	HyCUBESim::CGRA cgraInstance(x_,y_, memory_tile_size);
	cgraInstance.parseCMEM(cmemfileName);
//	cout << "argc: "<< argc <<"\n";
	if(argc>=4){
		cout << "Parsing data file with base address pointers\n";
		string memallocfileName(argv[3]);
#ifdef MULTI_DATA
		cgraInstance.parseDMEM(dmemfileName,memallocfileName,0,memory_tile_size);
		if(argc>7){
			string dmemfileNamek2(argv[7]);
			cgraInstance.parseDMEM(dmemfileNamek2,memallocfileName,1,memory_tile_size);
		}
		if(argc>8){
			string dmemfileNamek3(argv[8]);
			cgraInstance.parseDMEM(dmemfileNamek3,memallocfileName,2,memory_tile_size);

		}
		if(argc>9){
			string dmemfileNamek4(argv[9]);
			cgraInstance.parseDMEM(dmemfileNamek4,memallocfileName,3,memory_tile_size);
		}	

#else
		cgraInstance.parseDMEM(dmemfileName,memallocfileName);
#endif
	}else{
		cgraInstance.parseDMEM(dmemfileName);
	}
	cgraInstance.dumpRawData();
	cgraInstance.printInterestedAddrOutcome();

	int count=0;
#ifdef ARCHI_16BIT
	while(cgraInstance.dmem[16382]==0){
		cgraInstance.executeCycle(count);
		count++;
	}
#else
	while(cgraInstance.dmem[2047]==0){
		cgraInstance.executeCycle(count);
		count++;
	}
#endif
	for(int i = 0; i<20; i++){// this is for additional latency
		cgraInstance.executeCycle(count);
		count++;
	}
	cgraInstance.printInterestedAddrOutcome();


	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
