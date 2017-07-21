/*
 * CGRATile.h
 *
 *  Created on: 6 Mar 2017
 *      Author: manupa
 */

#ifndef CGRATILE_H_
#define CGRATILE_H_

#include "data_structures.h"
#include "opcodes.h"

namespace HyCUBESim {

	class CGRATile {
		public:
			CGRATile(int x, int y, bool mem, std::map<DataType,uint8_t>* dmemPtr);
			std::map<Dir,std::pair<bool,DataType>> inputs;
			std::map<Dir,CGRATile*> connectedTiles;

			void execute(int kII);
			void executeFinish(HyIns currIns, bool ALU_valid, DataType ALU_Data);
			void updatePC();
			void updateRegisters();

			void DistrubuteALUData(HyIns currIns);
			void DistrubuteRegs(HyIns currIns);
			bool passData(Dir incomingDir, std::pair<bool,DataType> val);

			bool insertIns(HyIns newIns);
			void clearCMEM();
			void runLoop(int LS_i, int LE_i);

			void printIns(HyIns ins);
			std::string getNameXBarInput(XBarInput xinp);
			void printRegisterInfo();

			std::string str(Dir dir);
			std::string str(XBarInput xbin);
			std::string str(Regs reg);

			std::vector<HyIns> configMem;
			std::map<DataType,uint8_t>* dmemPtr;

		private:
			int X;
			int Y;
			bool MEM;

			std::map<Dir,std::pair<bool,DataType>> inputsXBar;

			std::pair<bool,DataType> R0;
			std::pair<bool,DataType> R1;
			std::pair<bool,DataType> R2;
			std::pair<bool,DataType> R3;
			std::pair<bool,DataType> RES;

			std::pair<bool,DataType> R0_tobewritten;
			std::pair<bool,DataType> R1_tobewritten;
			std::pair<bool,DataType> R2_tobewritten;
			std::pair<bool,DataType> R3_tobewritten;
			std::pair<bool,DataType> RES_tobewritten;

//			std::queue<DataType> R0;
//			std::queue<DataType> R1;
//			std::queue<DataType> R2;
//			std::queue<DataType> R3;
//			std::queue<DataType> RES;

			std::queue<DataType> P;
			std::queue<DataType> I1;
			std::queue<DataType> I2;

			bool Pisvalid=false;
			bool I1isvalid=false;
			bool I2isvalid=false;

			std::pair<bool,DataType> P_tobewritten;
			std::pair<bool,DataType> I1_tobewritten;
			std::pair<bool,DataType> I2_tobewritten;

			DataType PC=0;
			DataType LER=0;
			DataType LSR=0;
			DataType CL=0;

//			std::queue<DataType> ALU_O;
			std::pair<bool,DataType> ALU_O;
			std::pair<bool,DataType> ALU_R; // this is only there in MEM PEs.

			HyIns prevIns;

			DataType ars(DataType op1, DataType op2);
			DataType sext(DataType op1, DataType op2);
			DataType select(DataType op1, DataType op2);
			DataType cmerge(DataType op1, DataType op2);
			DataType loadcl(DataType op1, DataType op2);
			DataType movcl(DataType op1, DataType op2);
			DataType load(DataType op2, int size);
			DataType store(DataType op1, DataType op2,int size);
			DataType jumpl(DataType op2);
		};

} /* namespace HyCUBESim */

#endif /* CGRATILE_H_ */
