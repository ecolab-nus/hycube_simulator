/*
 * CGRA.cpp
 *
 *  Created on: 6 Mar 2017
 *      Author: manupa
 */

#include "CGRA.h"
#include <sstream>
#include <string>
#include <iostream>
#include <cstdlib>
#include <assert.h>

namespace HyCUBESim {

CGRA::CGRA(int SizeX, int SizeY) {
	// TODO Auto-generated constructor stub
	sizeX = SizeX;
	sizeY = SizeY;

	for (int y = 0; y < SizeY; ++y) {
		for (int x = 0; x < SizeX; ++x) {
			CGRATiles[y][x] = new CGRATile(x,y,(x==0),&dmem);
		}
	}

	for (int y = 0; y < SizeY; ++y) {
		for (int x = 0; x < SizeX; ++x) {

			if(x+1 < SizeX){
				CGRATiles[y][x]->connectedTiles[EAST]=CGRATiles[y][x+1];
			}

			if(y+1 < SizeY){
				CGRATiles[y][x]->connectedTiles[SOUTH]=CGRATiles[y+1][x];
			}

			if(x-1 >= 0){
				CGRATiles[y][x]->connectedTiles[WEST]=CGRATiles[y][x-1];
			}

			if(y-1 >= 0){
				CGRATiles[y][x]->connectedTiles[NORTH]=CGRATiles[y-1][x];
			}

		}
	}

}

int CGRA::parseCMEM(std::string CMEMFileName) {

	std::ifstream cmemfile(CMEMFileName.c_str());
	std::string line;

	assert(cmemfile.is_open() && 'Error opening cmem file');

	//ignoring the first line
	std::getline(cmemfile,line);

	while(1){
		if(!std::getline(cmemfile,line)){
			break;
		}

		if(line.empty()){
			break;
		}

	    std::istringstream iss(line);
	    int t;

	    iss >> t;
	    for (int i = 0; i < 16; ++i) {
			std::getline(cmemfile,line);
		    std::istringstream iss(line);
		    std::string phyloc;
		    std::string op;

		    std::getline(iss,phyloc,',');
		    std::getline(iss,op,',');

		    int y=i/4;
		    int x=i%4;

		    std::cout << "T=" << t << ",Y=" << y << ",X=" << x << "," << op << "\n";

		    HyIns currIns;
		    if(atoi(op.substr(0,1).c_str())==1){
		    	currIns.NPB=true;
		    }
		    else{
		    	currIns.NPB=false;
		    }

		    if(atoi(op.substr(1,1).c_str())==1){
		    	currIns.constValid=true;
		    }
		    else{
		    	currIns.constValid=false;
		    }

			currIns.constant = std::stoi(/*op.substr(46,3) +*/ op.substr(2,27),nullptr,2);
			if((currIns.constant >> 26) == 1){ //negative number identification
				currIns.constant = currIns.constant | 0b11111000000000000000000000000000;
			}


			currIns.opcode = std::stoi(op.substr(29,5),nullptr,2);

		    currIns.regwen[Reg0] = std::stoi(op.substr(34,1),nullptr,2);
		    currIns.regwen[Reg2] = std::stoi(op.substr(35,1),nullptr,2);
		    currIns.regwen[Reg3] = std::stoi(op.substr(36,1),nullptr,2);
		    currIns.regwen[Reg1] = std::stoi(op.substr(37,1),nullptr,2);

		    currIns.tregwen = std::stoi(op.substr(38,1),nullptr,2);

		    currIns.regbypass[Reg3] = std::stoi(op.substr(39,1),nullptr,2);
		    currIns.regbypass[Reg0] = std::stoi(op.substr(40,1),nullptr,2);
		    currIns.regbypass[Reg2] = std::stoi(op.substr(41,1),nullptr,2);
		    currIns.regbypass[Reg1] = std::stoi(op.substr(42,1),nullptr,2);

		    std::cout << "XbConfig : " << op.substr(43,21) << "\n";
		    std::cout << "xB.I2 : " << op.substr(49,3) << "\n";

		    currIns.xB.P = convertStrtoXBI(op.substr(43,3));

//		    if(currIns.constValid){
//		    	currIns.xB.I2 = INV;
//		    }
//		    else{
		    	currIns.xB.I2 = convertStrtoXBI(op.substr(46,3));
//		    }
		    currIns.xB.I1 = convertStrtoXBI(op.substr(49,3));
		    currIns.xB.NORTH_O = convertStrtoXBI(op.substr(52,3));
		    currIns.xB.WEST_O = convertStrtoXBI(op.substr(55,3));
		    currIns.xB.SOUTH_O = convertStrtoXBI(op.substr(58,3));
		    currIns.xB.EAST_O = convertStrtoXBI(op.substr(61,3));

		    CGRATiles[y][x]->printIns(currIns);

		    CGRATiles[y][x]->configMem.push_back(currIns);
		}
	    std::getline(cmemfile,line);
	}
}

int CGRA::parseDMEM(std::string DMEMFileName) {

	std::ifstream dmemfile(DMEMFileName.c_str());
	std::string line;

	assert(dmemfile.is_open() && 'Error opening dmem file');

	//ignore the first line
	std::getline(dmemfile,line);

	for (int i = 0; i < 4096; ++i) {
		dmem[i]=0;
	}

	while(std::getline(dmemfile,line)){
		std::istringstream iss(line);

		std::string addr;
		std::string pre;
		std::string post;

		std::getline(iss,addr,',');
		std::getline(iss,pre,',');
		std::getline(iss,post,',');

		std::cout << addr << "," << pre << "\n";
		InterestedAddrList.push_back(atoi(addr.c_str()));

		dmem[(DataType)atoi(addr.c_str())]=atoi(pre.c_str());
	}
}

int CGRA::executeCycle(int kII) {
	for (int y = 0; y < sizeY; ++y) {
		for (int x = 0; x < sizeX; ++x) {
			CGRATiles[y][x]->execute(kII);
		}
	}

	for (int y = 0; y < sizeY; ++y) {
		for (int x = 0; x < sizeX; ++x) {
			CGRATiles[y][x]->updateRegisters();
			CGRATiles[y][x]->updatePC();
		}
	}
}

void CGRA::printInterestedAddrOutcome() {
	std::cout << "Interested Addresses :: \n";
	for(int addr : InterestedAddrList){
		std::cout << addr << "," << (int)dmem[addr] << "\n";
	}
}

XBarInput CGRA::convertStrtoXBI(std::string str) {

	std::cout << "convertStr called : " << std::stoi(str,nullptr,2) << "\n";

	switch(std::stoi(str,nullptr,2)){
		case 0:
			return EAST_I;
		case 1:
			return SOUTH_I;
		case 2:
			return WEST_I;
		case 3:
			return NORTH_I;
		case 4:
			return ALU_I;
		case 5:
			return RES_I;
		case 7:
			return INV;
		default:
			std::cout << str;
			assert(false);
	}
}

} /* namespace HyCUBESim */
