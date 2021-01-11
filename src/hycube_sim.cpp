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

	HyCUBESim::CGRA cgraInstance(4,4);
	cgraInstance.parseCMEM(cmemfileName);
//	cout << "argc: "<< argc <<"\n";
	if(argc==4){
		cout << "Parsing data file with base address pointers\n";
		string memallocfileName(argv[3]);
		cgraInstance.parseDMEM(dmemfileName,memallocfileName);
	}else{
		cgraInstance.parseDMEM(dmemfileName);
	}
	cgraInstance.printInterestedAddrOutcome();

	int count=0;
	while(cgraInstance.dmem[2047]==0){
		cgraInstance.executeCycle(count);
		count++;
	}
	cgraInstance.printInterestedAddrOutcome();


	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
