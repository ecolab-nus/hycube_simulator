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

namespace HyCUBESim {

class CGRA {

	public:
		CGRA(int SizeX, int SizeY);
		std::map<int,std::map<int,CGRATile*> > CGRATiles;

		int parseCMEM(std::string CMEMFileName);
		int parseDMEM(std::string DMEMFileName);
		int parseDMEM(std::string DMEMFileName,std::string memallocFileName);
		int executeCycle(int kII);

		std::map<DataType,uint8_t> dmem;
		std::map<DataType,uint8_t> dmem_post;
		void printInterestedAddrOutcome();


	private:

		int sizeX;
		int sizeY;
		std::vector<int> InterestedAddrList;
		XBarInput convertStrtoXBI(std::string str);


		int tile_size = 4;
		int mem_each_tile = 4096;
		int overall_mem =  16384;

	};









} /* namespace HyCUBESim */

#endif /* CGRA_H_ */
