/*
 * CGRATile.cpp
 *
 *  Created on: 6 Mar 2017
 *      Author: manupa
 */

#include "CGRATile.h"
#include <assert.h>
#include <iostream>
#include "data_structures.h"

namespace HyCUBESim {

	CGRATile::CGRATile(int x, int y, bool mem, std::map<DataType,uint8_t>* dmemPtr) {
		// TODO Auto-generated constructor stub
		this->X = x;
		this->Y = y;
		this->MEM = mem;
		this->dmemPtr = dmemPtr;

		prevIns.opcode=NOP;
		prevIns.xB.P=INV;
		prevIns.xB.I1=INV;
		prevIns.xB.I2=INV;
		prevIns.xB.NORTH_O=INV;
		prevIns.xB.EAST_O=INV;
		prevIns.xB.WEST_O=INV;
		prevIns.xB.SOUTH_O=INV;
	}

	void CGRATile::execute(int kII) {
		LOG(SIMULATOR) << "---------------execute new instruction ----------------------------\n";
		LOG(SIMULATOR) << "current PC = " << PC << ",LER=" << LER << ",LSR=" << LSR << "\n";
		LOG(SIMULATOR) << "kII =" << kII << ",PC =" << PC << ",y=" << Y << ",x=" << X << "\n";
		LOG(SIMULATOR) << "RegInfo ::\n";
		this->printRegisterInfo();

		HyIns currIns = configMem[PC];
		LOG(SIMULATOR) << "currIns ::\n";
		printIns(currIns);
		DataType result;

		bool predicate;
		DataType operand1;
		DataType operand2;

		if(prevIns.xB.P != INV){
			if(P.empty()) {
				LOG(SIMULATOR) << "expected Predicate is not there!\n";
				executeFinish(currIns,false,0);
				return;
			}
			Pisvalid=true;

			if(P.front() != 0){
				predicate = true;
			}
			else{
				predicate = false;
			}
			if(currIns.NPB){
				predicate = !predicate;
			}
			P.pop();
		}
		else{
			Pisvalid=false;
		}

		if(prevIns.xB.I1 != INV){
			if(I1.empty() && (currIns.opcode!=SELECT)){
				LOG(SIMULATOR) << "expected I1 is not there!\n";
				executeFinish(currIns,false,0);
				return;
			}

			if(!I1.empty()){
				I1isvalid=true;
				operand1 = I1.front();
				I1.pop();
			}
			else{
				I1isvalid=false;
			}
		}
		else{
			I1isvalid=false;
		}

		if(currIns.constValid){
			I2isvalid=true;
			operand2 = currIns.constant;
		}
		else{
			if(prevIns.xB.I2 != INV){
				if(I2.empty() && (currIns.opcode!=SELECT)) {
					LOG(SIMULATOR) << "expected I2 is not there!\n";
					executeFinish(currIns,false,0);
					return;
				}

				if(!I2.empty()){
					I2isvalid=true;
					operand2 = I2.front();
					I2.pop();
				}
				else{
					I2isvalid=false;
				}
			}
			else{
				I2isvalid=false;
			}
		}

		DataType ALUTempOut;

		switch(currIns.opcode){
			case NOP:
				break;
			case ADD :
				LOG(SIMULATOR) << ": ADD," << operand1 << "," << operand2 << "\n";
				ALUTempOut =  operand1 + operand2;
				break;
			case SUB :
				LOG(SIMULATOR) << ": SUB," << operand1 << "," << operand2 << "\n";
				ALUTempOut = operand1 - operand2;
				break;
			case MUL :
				LOG(SIMULATOR) << ": MUL," << operand1 << "," << operand2 << "\n";
				ALUTempOut = operand1 * operand2;
				break;
			case SEXT :
				LOG(SIMULATOR) << ": SEXT," << operand1 << "," << operand2 << "\n";
				ALUTempOut = sext(operand1,operand2);
				break;
			case DIV :
				LOG(SIMULATOR) << ": DIV," << operand1 << "," << operand2 << "\n";
				ALUTempOut = operand1 / operand2;
				break;
			case LS :
				LOG(SIMULATOR) << ": LS," << operand1 << "," << operand2 << "\n";
				ALUTempOut = operand1 << operand2;
				break;
			case RS :
				LOG(SIMULATOR) << ": RS," << operand1 << "," << operand2 << "\n";
				ALUTempOut = operand1 >> operand2;
				break;
			case ARS :
				LOG(SIMULATOR) << ": ARS," << operand1 << "," << operand2 << "\n";
				ALUTempOut = ars(operand1,operand2);
				break;
			case AND :
				LOG(SIMULATOR) << ": AND," << operand1 << "," << operand2 << "\n";
				ALUTempOut = operand1 & operand2;
				break;
			case OR :
				LOG(SIMULATOR) << ": OR," << operand1 << "," << operand2 << "\n";
				ALUTempOut = operand1 | operand2;
				break;
			case XOR :
				LOG(SIMULATOR) << ": XOR," << operand1 << "," << operand2 << "\n";
				ALUTempOut = operand1 ^ operand2;
				break;
			case SELECT :
				LOG(SIMULATOR) << ": SELECT," << operand1 << "," << operand2 << "\n";
				assert(!(I1isvalid && I2isvalid));
				if(I1isvalid){
					ALUTempOut=operand1;
				}
				else if(I2isvalid){
					ALUTempOut=operand2;
				}
				else{
					LOG(SIMULATOR) << "expected I2 or I1 is not there!\n";
					executeFinish(currIns,false,0);
					return;
				}
//				ALUTempOut = I1isvalid ? operand1 : operand2;
				break;
			case CMERGE :
				LOG(SIMULATOR) << ": CMERGE," << operand1 << "," << operand2 << "\n";
				//dmd ALUTempOut = operand1;
				//dmd:
				if(I2isvalid){
					ALUTempOut=operand2;
				}
				else{
					ALUTempOut = operand1;
				}
				break;
			case CMP :
				LOG(SIMULATOR) << ": CMP," << operand1 << "," << operand2 << "\n";
				ALUTempOut = operand1 == operand2;
				break;
			case CLT :
				LOG(SIMULATOR) << ": CLT," << operand1 << "," << operand2 << "\n";
				ALUTempOut = operand1 < operand2;
				break;
			case BR :
				LOG(SIMULATOR) << ": BR," << predicate << "," << I1isvalid << "," << I2isvalid << "\n";
				if((predicate)||(I1isvalid)||(I2isvalid)){
					ALUTempOut = 1;
				}
				else{
					ALUTempOut = 0;
				}
				break;
			case CGT :
				LOG(SIMULATOR) << ": CGT," << operand1 << "," << operand2 << "\n";
				ALUTempOut = operand1 > operand2;
				break;
			case LOADCL :
				LOG(SIMULATOR) << ": LOADCL," << operand1 << "," << operand2 << "\n";
				ALUTempOut = loadcl(operand1,operand2);
				break;
			case MOVCL :
				LOG(SIMULATOR) << ": MOVCL," << operand1 << "," << operand2 << "\n";
				ALUTempOut = movcl(operand1,operand2);
				break;
			case LOAD :
				LOG(SIMULATOR) << ": LOAD," << operand1 << "," << operand2 << "\n";
#ifdef ARCHI_16BIT
				ALUTempOut = load(operand2,2);
#else
				ALUTempOut = load(operand2,4);
#endif
				break;
			case LOADH :
				LOG(SIMULATOR) << ": LOADH," << operand1 << "," << operand2 << "\n";
				ALUTempOut = load(operand2,2);
				break;
			case LOADB :
				LOG(SIMULATOR) << ": LOADB," << operand1 << "," << operand2 << "\n";
				ALUTempOut = load(operand2,1);
				break;
			case STORE :
				LOG(SIMULATOR) << ": STORE," << operand1 << "," << operand2 << "\n";
				if(!Pisvalid || predicate){
					//only store after checking predicate
					//other operations dont care as the output is not routed.
#ifdef ARCHI_16BIT
					ALUTempOut = store(operand1,operand2,2);
#else
					ALUTempOut = store(operand1,operand2,4);
#endif
				}
				break;
			case STOREH :
				LOG(SIMULATOR) << ": STOREH," << operand1 << "," << operand2 << "\n";
				if(!Pisvalid || predicate){
					//only store after checking predicate
					//other operations dont care as the output is not routed.
					ALUTempOut = store(operand1,operand2,2);
				}
				break;
			case STOREB :
				LOG(SIMULATOR) << ": STOREB," << operand1 << "," << operand2 << "\n";
				if(!Pisvalid || predicate){
					//only store after checking predicate
					//other operations dont care as the output is not routed.
					ALUTempOut = store(operand1,operand2,1);
				}
				break;
			case JUMPL :
				LOG(SIMULATOR) << ": JUMPL," << operand1 << "," << operand2 << "\n";
				ALUTempOut = jumpl(operand2);
				executeFinish(currIns,false,0);
				return;
				break;
			case MOVC :
				LOG(SIMULATOR) << ": MOVC," << operand1 << "," << operand2 << "\n";
				ALUTempOut = operand2;
				break;
			default :
				LOG(SIMULATOR) << "unknown opcode = " << currIns.opcode << "\n";
				assert(false);
				LOG(SIMULATOR) << ": ????," << operand1 << "," << operand2 << "\n";
				break;
		}
		LOG(SIMULATOR) << "OUTPUT=" << ALUTempOut << "\n";

		//stop exeuction after poping the queues.
		if(!predicate && Pisvalid){
			LOG(SIMULATOR) << "Predicate is not valid!\n";
			executeFinish(currIns,false,0);
			return;
		}

		executeFinish(currIns,true,ALUTempOut);
	}

	void CGRATile::executeFinish(HyIns currIns, bool ALU_valid, DataType ALU_Data) {

//		if(MEM){
//			while(!ALU_O.empty()) ALU_O.pop();
//			ALU_O.push(ALU_R);
//		}

		while(!I1.empty()){
			I1.pop();
		}
		while(!I2.empty()){
			I2.pop();
		}
		while(!P.empty()){
			P.pop();
		}

		if(MEM){
			ALU_O = ALU_R;
			ALU_R = std::make_pair(ALU_valid,ALU_Data);
		}
		else{
			ALU_O = std::make_pair(ALU_valid,ALU_Data);
		}

		if(currIns.tregwen==1){
			RES_tobewritten=ALU_O;
		}

		DistrubuteALUData(currIns);
		DistrubuteRegs(currIns);
		prevIns = currIns;
	}

	void CGRATile::updatePC() {

//		HyIns currIns = configMem[PC];

		if(prevIns.opcode != JUMPL){
			if((PC < LSR)||(PC >= LER)){
				PC = LSR;
			}
			else{
				PC = (PC+1)%32;
			}
		}
	}

	void CGRATile::updateRegisters() {

		if(P_tobewritten.first){
			assert(P.empty());
			P.push(P_tobewritten.second);
			P_tobewritten.first=false;
		}

		if(I1_tobewritten.first){
			assert(I1.empty());
			I1.push(I1_tobewritten.second);
			I1_tobewritten.first=false;
		}

		if(I2_tobewritten.first){
			assert(I2.empty());
			I2.push(I2_tobewritten.second);
			I2_tobewritten.first=false;
		}

		R0 = R0_tobewritten;
		R1 = R1_tobewritten;
		R2 = R2_tobewritten;
		R3 = R3_tobewritten;
		RES = RES_tobewritten;
	}

	bool CGRATile::insertIns(HyIns newIns) {
		if(configMem.size() == 32){
			return false;
		}
		this->configMem.push_back(newIns);
		return true;
	}

	void CGRATile::clearCMEM(){
		this->configMem.clear();
	}

	void CGRATile::runLoop(int LS_i, int LE_i){
		this->LSR = LS_i;
		this->LER = LE_i;
	}

	void CGRATile::DistrubuteALUData(HyIns currIns) {
		switch (currIns.xB.NORTH_O) {
			case RES_I:
//				if(!RES.empty()){
//					assert(RES.size()==1);
//					assert(connectedTiles[NORTH]->passData(SOUTH,RES.front()));
//					RES.pop();
//				}
				connectedTiles[NORTH]->passData(SOUTH,RES);
				break;
			case ALU_I:
//				if(!ALU_O.empty()){
//					assert(ALU_O.size()==1);
//					assert(connectedTiles[NORTH]->passData(SOUTH,ALU_O.front()));
//					ALU_O.pop();
//				}
				connectedTiles[NORTH]->passData(SOUTH,ALU_O);
				break;
			default:
				break;
		}

		switch (currIns.xB.EAST_O) {
			case RES_I:
//				if(!RES.empty()){
//					assert(RES.size()==1);
//					assert(connectedTiles[EAST]->passData(WEST,RES.front()));
//					RES.pop();
//				}
				connectedTiles[EAST]->passData(WEST,RES);
				break;
			case ALU_I:
//				if(!ALU_O.empty()){
//					assert(ALU_O.size()==1);
//					assert(connectedTiles[EAST]->passData(WEST,ALU_O.front()));
//					ALU_O.pop();
//				}
				connectedTiles[EAST]->passData(WEST,ALU_O);
				break;
			default:
				break;
		}

		switch (currIns.xB.SOUTH_O) {
			case RES_I:
//				if(!RES.empty()){
//					assert(RES.size()==1);
//					assert(connectedTiles[SOUTH]->passData(NORTH,RES.front()));
//					RES.pop();
//				}
				connectedTiles[SOUTH]->passData(NORTH,RES);
				break;
			case ALU_I:
//				if(!ALU_O.empty()){
//					assert(ALU_O.size()==1);
//					assert(connectedTiles[SOUTH]->passData(NORTH,ALU_O.front()));
//					ALU_O.pop();
//				}
				connectedTiles[SOUTH]->passData(NORTH,ALU_O);
				break;
			default:
				break;
		}

		switch (currIns.xB.WEST_O) {
			case RES_I:
//				if(!RES.empty()){
//					assert(RES.size()==1);
//					assert(connectedTiles[WEST]->passData(EAST,RES.front()));
//					RES.pop();
//				}
				connectedTiles[WEST]->passData(EAST,RES);
				break;
			case ALU_I:
//				if(!ALU_O.empty()){
//					assert(ALU_O.size()==1);
//					assert(connectedTiles[WEST]->passData(EAST,ALU_O.front()));
//					ALU_O.pop();
//				}
				connectedTiles[WEST]->passData(EAST,ALU_O);
				break;
			default:
				break;
		}

		switch (currIns.xB.P) {
			case RES_I:
//				if(!RES.empty()){
//					assert(RES.size()==1);
//					P.push(RES.front());
//					RES.pop();
//				}
//				if(RES.first){
//					assert(P.empty());
//					P.push(RES.second);
//				}
				P_tobewritten=RES;
				break;
			case ALU_I:
//				if(!ALU_O.empty()){
//					assert(ALU_O.size()==1);
//					P.push(ALU_O.front());
//					ALU_O.pop();
//				}
//				if(ALU_O.first){
//					assert(P.empty());
//					P.push(ALU_O.second);
//				}
				P_tobewritten=ALU_O;
				break;
			default:
				break;
		}

		switch (currIns.xB.I1) {
			case RES_I:
//				if(!RES.empty()){
//					assert(RES.size()==1);
//					LOG(SIMULATOR) << "Pushed I1\n";
//					I1.push(RES.front());
//					RES.pop();
//				}
//				if(RES.first){
//					assert(I1.empty());
//					I1.push(RES.second);
//				}
				I1_tobewritten=RES;
				break;
			case ALU_I:
//				if(!ALU_O.empty()){
//					assert(ALU_O.size()==1);
//					LOG(SIMULATOR) << "Pushed I1\n";
//					I1.push(ALU_O.front());
//					ALU_O.pop();
//				}
//				if(ALU_O.first){
//					assert(I1.empty());
//					I1.push(ALU_O.second);
//				}
				I1_tobewritten=ALU_O;
				break;
			default:
				break;
		}

		switch (currIns.xB.I2) {
			case RES_I:
//				if(!RES.empty()){
//					assert(RES.size()==1);
//					I2.push(RES.front());
//					RES.pop();
//				}
//				if(RES.first){
//					assert(I2.empty());
//					I2.push(RES.second);
//				}
				I2_tobewritten=RES;
				break;
			case ALU_I:
//				if(!ALU_O.empty()){
//					assert(ALU_O.size()==1);
//					I2.push(ALU_O.front());
//					ALU_O.pop();
//				}
//				if(ALU_O.first){
//					assert(I2.empty());
//					I2.push(ALU_O.second);
//				}
				I2_tobewritten=ALU_O;
				break;
			default:
				break;
		}

	}

	void CGRATile::DistrubuteRegs(HyIns currIns) {

		if(currIns.regbypass[Reg0]==1){
//			if(!R0.empty()){
//				assert(R0.size()==1);
				inputsXBar[NORTH]=R0;
//				R0.pop();
				XBarInput xbarIncomingDir=NORTH_I;
				Dir incomingDir = NORTH;
				if(currIns.xB.NORTH_O == xbarIncomingDir){
					assert(connectedTiles[NORTH]);
					assert(connectedTiles[NORTH]->passData(SOUTH,inputsXBar[incomingDir]));
				}
				if(currIns.xB.EAST_O == xbarIncomingDir){
					assert(connectedTiles[EAST]);
					assert(connectedTiles[EAST]->passData(WEST,inputsXBar[incomingDir]));
				}
				if(currIns.xB.WEST_O == xbarIncomingDir){
					assert(connectedTiles[WEST]);
					assert(connectedTiles[WEST]->passData(EAST,inputsXBar[incomingDir]));
				}
				if(currIns.xB.SOUTH_O == xbarIncomingDir){
					assert(connectedTiles[SOUTH]);
					assert(connectedTiles[SOUTH]->passData(NORTH,inputsXBar[incomingDir]));
				}
				if(currIns.xB.P == xbarIncomingDir){
					P_tobewritten = inputsXBar[incomingDir];
				}
				if(currIns.xB.I1 == xbarIncomingDir){
					I1_tobewritten = inputsXBar[incomingDir];
				}
				if(currIns.xB.I2 == xbarIncomingDir){
					I2_tobewritten = inputsXBar[incomingDir];
				}
//			}
		}

		if(currIns.regbypass[Reg1]==1){
//			if(!R1.empty()){
//				assert(R1.size()==1);
				inputsXBar[EAST]=R1;
//				R1.pop();
				XBarInput xbarIncomingDir=EAST_I;
				Dir incomingDir = EAST;
				if(currIns.xB.NORTH_O == xbarIncomingDir){
					assert(connectedTiles[NORTH]);
					assert(connectedTiles[NORTH]->passData(SOUTH,inputsXBar[incomingDir]));
				}
				if(currIns.xB.EAST_O == xbarIncomingDir){
					assert(connectedTiles[EAST]);
					assert(connectedTiles[EAST]->passData(WEST,inputsXBar[incomingDir]));
				}
				if(currIns.xB.WEST_O == xbarIncomingDir){
					assert(connectedTiles[WEST]);
					assert(connectedTiles[WEST]->passData(EAST,inputsXBar[incomingDir]));
				}
				if(currIns.xB.SOUTH_O == xbarIncomingDir){
					assert(connectedTiles[SOUTH]);
					assert(connectedTiles[SOUTH]->passData(NORTH,inputsXBar[incomingDir]));
				}
				if(currIns.xB.P == xbarIncomingDir){
					P_tobewritten = inputsXBar[incomingDir];
				}
				if(currIns.xB.I1 == xbarIncomingDir){
					I1_tobewritten = inputsXBar[incomingDir];
				}
				if(currIns.xB.I2 == xbarIncomingDir){
					I2_tobewritten = inputsXBar[incomingDir];
				}
//			}
		}

		if(currIns.regbypass[Reg2]==1){
//			if(!R2.empty()){
//				assert(R2.size()==1);
				inputsXBar[WEST]=R2;
//				R2.pop();
				XBarInput xbarIncomingDir=WEST_I;
				Dir incomingDir = WEST;
				if(currIns.xB.NORTH_O == xbarIncomingDir){
					assert(connectedTiles[NORTH]);
					assert(connectedTiles[NORTH]->passData(SOUTH,inputsXBar[incomingDir]));
				}
				if(currIns.xB.EAST_O == xbarIncomingDir){
					assert(connectedTiles[EAST]);
					assert(connectedTiles[EAST]->passData(WEST,inputsXBar[incomingDir]));
				}
				if(currIns.xB.WEST_O == xbarIncomingDir){
					assert(connectedTiles[WEST]);
					assert(connectedTiles[WEST]->passData(EAST,inputsXBar[incomingDir]));
				}
				if(currIns.xB.SOUTH_O == xbarIncomingDir){
					assert(connectedTiles[SOUTH]);
					assert(connectedTiles[SOUTH]->passData(NORTH,inputsXBar[incomingDir]));
				}
				if(currIns.xB.P == xbarIncomingDir){
					P_tobewritten = inputsXBar[incomingDir];
				}
				if(currIns.xB.I1 == xbarIncomingDir){
					I1_tobewritten = inputsXBar[incomingDir];
				}
				if(currIns.xB.I2 == xbarIncomingDir){
					I2_tobewritten = inputsXBar[incomingDir];
				}
//			}
		}

		if(currIns.regbypass[Reg3]==1){
//			if(!R3.empty()){
//				assert(R3.size()==1);
				inputsXBar[SOUTH]=R3;
//				R3.pop();
				XBarInput xbarIncomingDir=SOUTH_I;
				Dir incomingDir = SOUTH;
				if(currIns.xB.NORTH_O == xbarIncomingDir){
					assert(connectedTiles[NORTH]);
					assert(connectedTiles[NORTH]->passData(SOUTH,inputsXBar[incomingDir]));
				}
				if(currIns.xB.EAST_O == xbarIncomingDir){
					assert(connectedTiles[EAST]);
					assert(connectedTiles[EAST]->passData(WEST,inputsXBar[incomingDir]));
				}
				if(currIns.xB.WEST_O == xbarIncomingDir){
					assert(connectedTiles[WEST]);
					assert(connectedTiles[WEST]->passData(EAST,inputsXBar[incomingDir]));
				}
				if(currIns.xB.SOUTH_O == xbarIncomingDir){
					assert(connectedTiles[SOUTH]);
					assert(connectedTiles[SOUTH]->passData(NORTH,inputsXBar[incomingDir]));
				}
				if(currIns.xB.P == xbarIncomingDir){
					P_tobewritten = inputsXBar[incomingDir];
				}
				if(currIns.xB.I1 == xbarIncomingDir){
					I1_tobewritten = inputsXBar[incomingDir];
				}
				if(currIns.xB.I2 == xbarIncomingDir){
					I2_tobewritten = inputsXBar[incomingDir];
				}
//			}
		}

	}

	bool CGRATile::passData(Dir incomingDir,  std::pair<bool,DataType> val) {
		LOG(SIMULATOR) << "passData :: PC=" << PC << ",Y=" << Y << ",X=" << X << "\n";

		if(val.first){
			LOG(SIMULATOR) << "Incoming Dir : " << str(incomingDir) << ",val = " << val.second;
		}
		else{
			LOG(SIMULATOR) << "Incoming Dir : " << str(incomingDir) << ",val = " << "INV";
		}


		HyIns currIns = configMem[PC];
		XBarInput xbarIncomingDir;
		Regs correspondingReg;
		inputs[incomingDir]=val;
		bool sinkedtoReg=false;

		switch (incomingDir) {
			case NORTH:
				xbarIncomingDir=NORTH_I;
				correspondingReg=Reg0;
				if(currIns.regwen[correspondingReg]==1){
//					assert(R0.empty());
//					R0.push(inputs[incomingDir]);
					LOG(SIMULATOR) << ",writtenR0";
//					R0 = inputs[incomingDir];
					R0_tobewritten = inputs[incomingDir];
					sinkedtoReg=true;
				}
				break;
			case EAST:
				xbarIncomingDir=EAST_I;
				correspondingReg=Reg1;
				if(currIns.regwen[correspondingReg]==1){
//					assert(R1.empty());
//					R1.push(inputs[incomingDir]);
					LOG(SIMULATOR) << ",writtenR1";
//					R1 = inputs[incomingDir];
					R1_tobewritten = inputs[incomingDir];
					sinkedtoReg=true;
				}
				break;
			case WEST:
				xbarIncomingDir=WEST_I;
				correspondingReg=Reg2;
				if(currIns.regwen[correspondingReg]==1){
//					assert(R2.empty());
//					R2.push(inputs[incomingDir]);
					LOG(SIMULATOR) << ",writtenR2";
//					R2 = inputs[incomingDir];
					R2_tobewritten = inputs[incomingDir];
					sinkedtoReg=true;
				}
				break;
			case SOUTH:
				xbarIncomingDir=SOUTH_I;
				correspondingReg=Reg3;
				if(currIns.regwen[correspondingReg]==1){
//					assert(R3.empty());
//					R3.push(inputs[incomingDir]);
					LOG(SIMULATOR) << ",writtenR3";
//					R3 = inputs[incomingDir];
					R3_tobewritten = inputs[incomingDir];
					sinkedtoReg=true;
				}
				break;
			default:
				assert(false);
				break;
		}


		if(currIns.regbypass[correspondingReg]==0){
			// only need to store the inputXbar and because prior to the execution
			// inputXBar values will be loaded P,O1 and O2
			inputsXBar[incomingDir]=inputs[incomingDir];
		}
		else{
			//no need recursively call if regbypass is not happenning
			return true;
		}

		if(currIns.xB.P == xbarIncomingDir){
//			P.push(val);
//			return true;

			if(val.first){
//				assert(P.empty());
//				P.push(val.second);
				P_tobewritten = val;
				LOG(SIMULATOR) << ",writtenP";
				sinkedtoReg=true;
			}

		}
		if(currIns.xB.I1 == xbarIncomingDir){
//			I1.push(val);
//			return true;

			if(val.first){
//				assert(I1.empty());
//				I1.push(val.second);
				I1_tobewritten = val;
				LOG(SIMULATOR) << ",writtenI1";
				sinkedtoReg=true;
			}
		}
		if(currIns.xB.I2 == xbarIncomingDir){
//			I2.push(val);
//			return true;

			if(val.first){
//				assert(I2.empty());
//				I2.push(val.second);
				I2_tobewritten = val;
				LOG(SIMULATOR) << ",writtenI2";
				sinkedtoReg=true;
			}
		}



		if(currIns.xB.NORTH_O == xbarIncomingDir){
			connectedTiles[NORTH]->passData(SOUTH,val);
		}
		if(currIns.xB.EAST_O == xbarIncomingDir){
			connectedTiles[EAST]->passData(WEST,val);
		}
		if(currIns.xB.WEST_O == xbarIncomingDir){
			LOG(SIMULATOR) << "[CGRATile.cpp][passData]West passdata\n" ;
			connectedTiles[WEST]->passData(EAST,val);
			LOG(SIMULATOR) << "[CGRATile.cpp][passData]West passdata done\n" ;
		}
		if(currIns.xB.SOUTH_O == xbarIncomingDir){
			connectedTiles[SOUTH]->passData(NORTH,val);
		}

//		assert(sinkedtoReg);
		LOG(SIMULATOR) << "\n";

		return true;
	}

	DataType CGRATile::ars(DataType op1, DataType op2){
		DataType signbitmask = 1 << (sizeof(DataType)*8-1);
		DataType signbit = op1 & signbitmask;
		DataType result = op1 >> op2;

		if(signbit!=0){
			DataType signextendedbitmask = (1<<op2) - 1;
			signextendedbitmask = signextendedbitmask << (sizeof(DataType)*8-op2);
			result = result | signextendedbitmask;
		}

		return result;
	}

	DataType CGRATile::sext(DataType op1, DataType op2) {
		uint16_t srcByteWidth = (op2 >> 32);
		uint16_t dstByteWidth = op2 & (0x0000ffff);

		assert(dstByteWidth >= srcByteWidth);

		DataType mask;

		bool msb = ((op1 >> (srcByteWidth*8-1)) != 0);
		if(msb){
			assert(op1 >> (srcByteWidth*8) == 0);
			mask = (1 << ((dstByteWidth-srcByteWidth)*8))-1;
			mask = mask << (srcByteWidth*8);
			assert((op1 & mask) == 0);
			return op1 | mask;
		}
		return op1;
	}

	DataType CGRATile::select(DataType op1, DataType op2) {
		//only one of them should be 1
		assert(I1isvalid != I2isvalid);

		if(I1isvalid){
			return op1;
		}
		else{
			return op2;
		}

	}

	DataType CGRATile::cmerge(DataType op1, DataType op2) {
		return 0;
	}

	DataType CGRATile::loadcl(DataType op1, DataType op2) {
		assert(MEM);
		DataType newcl = (*dmemPtr)[op2];
		CL = op1;
		return newcl;
	}

	DataType CGRATile::movcl(DataType op1, DataType op2) {
		CL = op1;
		return op1;
	}

	DataType CGRATile::load(DataType op2, int size) {
		assert(size == 1 || size == 2 || size == 4);
		DataType res = 0;
		if(size == 1){
			res = res | (*dmemPtr)[op2];
		}
		else if(size == 2){
			assert(op2 % 2 == 0);
			res = res | (*dmemPtr)[op2] | ((*dmemPtr)[op2+1] << 8);
		}
		else{ //size == 4
			//assert(op2 % 4 == 0);//dmd
			res = res | (*dmemPtr)[op2] | ((*dmemPtr)[op2+1] << 8) | ((*dmemPtr)[op2+2] << 16) | ((*dmemPtr)[op2+3] << 24);
		}
		return res;
	}

	DataType CGRATile::store(DataType op1, DataType op2, int size) {
		assert(size == 1 || size == 2 || size == 4);
		DataType res = 0;
		if(size == 1){
			assert( (op1 & 0xffffff00) == 0);
			uint8_t stdata = op1 & 0x000000ff;
			(*dmemPtr)[op2] = stdata;
		}
		else if(size == 2){
#ifndef ARCHI_16BIT
			assert( (op1 & 0xffff0000) == 0);
#endif
			assert(op2 % 2 == 0);
			uint8_t stdata0 = op1 & 0x000000ff;
			uint8_t stdata1 = (op1 & 0x0000ff00) >> 8;
			(*dmemPtr)[op2] = stdata0;
			(*dmemPtr)[op2+1] = stdata1;
		}
		else{ //size == 4
			assert(op2 % 4 == 0);
			uint8_t stdata0 = op1 & 0x000000ff;
			uint8_t stdata1 = (op1 & 0x0000ff00) >> 8;
			uint8_t stdata2 = (op1 & 0x00ff0000) >> 16;
			uint8_t stdata3 = (op1 & 0xff000000) >> 24;

			(*dmemPtr)[op2] = stdata0;
			(*dmemPtr)[op2+1] = stdata1;
			(*dmemPtr)[op2+2] = stdata2;
			(*dmemPtr)[op2+3] = stdata3;
		}
		return 1;
	}

void CGRATile::printIns(HyIns ins) {
	LOG(SIMULATOR) << "NPB=" << ins.NPB;

	switch(ins.opcode){
				case NOP:
					LOG(SIMULATOR) << ",OP=" << "NOP";
					break;
				case ADD :
					LOG(SIMULATOR) << ",OP=" << "ADD";
					break;
				case SUB :
					LOG(SIMULATOR) << ",OP=" << "SUB";
					break;
				case MUL :
					LOG(SIMULATOR) << ",OP=" << "MUL";
					break;
				case SEXT :
					LOG(SIMULATOR) << ",OP=" << "MUL";
					break;
				case DIV :
					LOG(SIMULATOR) << ",OP=" << "DIV";
					break;
				case LS :
					LOG(SIMULATOR) << ",OP=" << "LS";
					break;
				case RS :
					LOG(SIMULATOR) << ",OP=" << "RS";
					break;
				case ARS :
					LOG(SIMULATOR) << ",OP=" << "ARS";
					break;
				case AND :
					LOG(SIMULATOR) << ",OP=" << "AND";
					break;
				case OR :
					LOG(SIMULATOR) << ",OP=" << "OR";
					break;
				case XOR :
					LOG(SIMULATOR) << ",OP=" << "XOR";
					break;
				case SELECT :
					LOG(SIMULATOR) << ",OP=" << "SELECT";
					break;
				case CMERGE :
					LOG(SIMULATOR) << ",OP=" << "CMERGE";
					break;
				case CMP :
					LOG(SIMULATOR) << ",OP=" << "CMP";
					break;
				case CLT :
					LOG(SIMULATOR) << ",OP=" << "CLT";
					break;
				case BR :
					LOG(SIMULATOR) << ",OP=" << "BR";
					break;
				case CGT :
					LOG(SIMULATOR) << ",OP=" << "CGT";
					break;
				case LOADCL :
					LOG(SIMULATOR) << ",OP=" << "LOADCL";
					break;
				case MOVCL :
					LOG(SIMULATOR) << ",OP=" << "MOVCL";
					break;
				case LOAD :
					LOG(SIMULATOR) << ",OP=" << "LOAD";
					break;
				case LOADH :
					LOG(SIMULATOR) << ",OP=" << "LOADH";
					break;
				case LOADB :
					LOG(SIMULATOR) << ",OP=" << "LOADB";
					break;
				case STORE :
					LOG(SIMULATOR) << ",OP=" << "STORE";
					break;
				case STOREH :
					LOG(SIMULATOR) << ",OP=" << "STOREH";
					break;
				case STOREB :
					LOG(SIMULATOR) << ",OP=" << "STOREB";
					break;
				case JUMPL :
					LOG(SIMULATOR) << ",OP=" << "JUMPL";
					break;
				case MOVC :
					LOG(SIMULATOR) << ",OP=" << "MOVC";
					break;
				default :
					LOG(SIMULATOR) << ",OP=" << "UNKNOWN";
					break;
			}

			LOG(SIMULATOR) << ",CONSTVALID=" << ins.constValid;
			LOG(SIMULATOR) << ",CONST=" << ins.constant;
			LOG(SIMULATOR) << "\n";
			LOG(SIMULATOR) << ",BYP_R0=" << (int)ins.regbypass[Reg0];
			LOG(SIMULATOR) << ",BYP_R1=" << (int)ins.regbypass[Reg1];
			LOG(SIMULATOR) << ",BYP_R2=" << (int)ins.regbypass[Reg2];
			LOG(SIMULATOR) << ",BYP_R3=" << (int)ins.regbypass[Reg3];
			LOG(SIMULATOR) << "\n";
			LOG(SIMULATOR) << ",REN_R0=" << (int)ins.regwen[Reg0];
			LOG(SIMULATOR) << ",REN_R1=" << (int)ins.regwen[Reg1];
			LOG(SIMULATOR) << ",REN_R2=" << (int)ins.regwen[Reg2];
			LOG(SIMULATOR) << ",REN_R3=" << (int)ins.regwen[Reg3];

			LOG(SIMULATOR) << ",tregwen=" << (int)ins.tregwen;

			LOG(SIMULATOR) << "\n";
			LOG(SIMULATOR) << ",XB_P=" << getNameXBarInput(ins.xB.P);
			LOG(SIMULATOR) << ",XB_I1=" << getNameXBarInput(ins.xB.I1);
			LOG(SIMULATOR) << ",XB_I2=" << getNameXBarInput(ins.xB.I2);
			LOG(SIMULATOR) << ",XB_NORTH=" << getNameXBarInput(ins.xB.NORTH_O);
			LOG(SIMULATOR) << ",XB_EAST=" << getNameXBarInput(ins.xB.EAST_O);
			LOG(SIMULATOR) << ",XB_WEST=" << getNameXBarInput(ins.xB.WEST_O);
			LOG(SIMULATOR) << ",XB_SOUTH=" << getNameXBarInput(ins.xB.SOUTH_O);

			LOG(SIMULATOR) << "\n**print ins end**\n";
}

std::string CGRATile::getNameXBarInput(XBarInput xinp) {

	if(xinp==NORTH_I){
		return "NORTH_I";
	}
	else if(xinp==EAST_I){
		return "EAST_I";
	}
	else if(xinp==WEST_I){
		return "WEST_I";
	}
	else if(xinp==SOUTH_I){
		return "SOUTH_I";
	}
	else if(xinp==RES_I){
		return "RES_I";
	}
	else if(xinp==ALU_I){
		return "ALU_I";
	}
	else if(xinp==INV){
		return "INV";
	}
	else{
		assert(false);
		return "";
	}
}

std::string CGRATile::str(Dir dir) {

	if(dir == NORTH){
		return "NORTH";
	}
	else if(dir == EAST){
		return "EAST";
	}
	else if(dir == WEST){
		return "WEST";
	}
	else if(dir == SOUTH){
		return "SOUTH";
	}
	else{
		assert(false);
		return "";
	}

}

std::string CGRATile::str(XBarInput xbin) {

	if(xbin == NORTH_I){
		return "NORTH_I";
	}
	else if(xbin == EAST_I){
		return "EAST_I";
	}
	else if(xbin == WEST_I){
		return "WEST_I";
	}
	else if(xbin == SOUTH_I){
		return "SOUTH_I";
	}
	else if(xbin == ALU_I){
		return "ALU_I";
	}
	else if(xbin == RES_I){
		return "RES_I";
	}
	else {
		assert(false);
		return "";
	}

}

std::string CGRATile::str(Regs reg) {

	if(reg == Reg0){
		return "Reg0";
	}
	else if(reg == Reg1){
		return "Reg1";
	}
	else if(reg == Reg2){
		return "Reg2";
	}
	else if(reg == Reg3){
		return "Reg3";
	}
	else {
		assert(false);
		return "";
	}

}

void CGRATile::printRegisterInfo() {
	if(!P.empty()){
		LOG(SIMULATOR) << "P=" << P.front();
	}
	else{
		LOG(SIMULATOR) << "P=INV";
	}

	if(!I1.empty()){
		LOG(SIMULATOR) << ",I1=" << I1.front();
	}
	else{
		LOG(SIMULATOR) << ",I1=INV";
	}

	if(!I2.empty()){
		LOG(SIMULATOR) << ",I2=" << I2.front();
	}
	else{
		LOG(SIMULATOR) << ",I2=INV";
	}


	if(R0.first){
		LOG(SIMULATOR) << ",R0=" << R0.second;
	}
	else{
		LOG(SIMULATOR) << ",R0=INV";
	}

	if(R1.first){
		LOG(SIMULATOR) << ",R1=" << R1.second;
	}
	else{
		LOG(SIMULATOR) << ",R1=INV";
	}

	if(R2.first){
		LOG(SIMULATOR) << ",R2=" << R2.second;
	}
	else{
		LOG(SIMULATOR) << ",R2=INV";
	}

	if(R3.first){
		LOG(SIMULATOR) << ",R3=" << R3.second;
	}
	else{
		LOG(SIMULATOR) << ",R3=INV";
	}

	if(RES.first){
		LOG(SIMULATOR) << ",RES=" << RES.second;
	}
	else{
		LOG(SIMULATOR) << ",RES=INV";
	}
	LOG(SIMULATOR) << "\n";
}



	DataType CGRATile::jumpl(DataType op2) {
		while(!P.empty()){
			P.pop();
		}
		while(!I1.empty()){
			I1.pop();
		}
		while(!I2.empty()){
			I2.pop();
		}

		LSR = op2 & 0b11111;
		LER = (op2 >> 5) & 0b11111;
		PC = (op2 >> 10) & 0b11111;

		LOG(SIMULATOR) << ": JUMPL,LSR=" << LSR << ",LER=" << LER << ",PC=" << PC << "\n";

		assert(LSR < 32);
		assert(LER < 32);
		assert(PC < 32);
		return 1;
	}


} /* namespace HyCUBESim */


