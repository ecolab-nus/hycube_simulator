/*
 * CGRA.h
 *
 *  Created on: 6 Mar 2017
 *      Author: manupa
 */

#ifndef CGRA_H_
#define CGRA_H_

#include "data_structures.h"
#include "CGRATile.h"
#include <fstream>


//Uncomment this for 16-bit full chip
//#define ARCHI_16BIT

namespace HyCUBESim {

class CGRA {

	public:
		CGRA(int SizeX, int SizeY, int type);
		std::map<int,std::map<int,CGRATile*> > CGRATiles;

		int parseCMEM(std::string CMEMFileName,int xdim,int ydim);
		int parseDMEM(std::string DMEMFileName);
		int parseDMEM(std::string DMEMFileName,std::string memallocFileName,int memsize);
		int executeCycle(int kII);

		std::map<DataType,uint8_t> dmem;
		std::map<DataType,uint8_t> dmem_post;
		std::map<DataType,uint8_t> dmem_pre;
		
		void printInterestedAddrOutcome();
		void dumpRawData();


	private:

		int sizeX;
		int sizeY;
		std::vector<int> InterestedAddrList;
		XBarInput convertStrtoXBI(std::string str);

	};









} /* namespace HyCUBESim */

#endif /* CGRA_H_ */
