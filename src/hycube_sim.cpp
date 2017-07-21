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
		cout << "HyCUBE_Sim expect two arguments : instruction trc and data trace\n";
		return -1;
	}

	string cmemfileName(argv[1]);
	string dmemfileName(argv[2]);

	HyCUBESim::CGRA cgraInstance(4,4);
	cgraInstance.parseCMEM(cmemfileName);
	cgraInstance.parseDMEM(dmemfileName);
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
