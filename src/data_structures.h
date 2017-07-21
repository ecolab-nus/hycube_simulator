#ifndef DATA_STRUCTURE_H
#define DATA_STRUCTURE_H

#include <vector>
#include <map>
#include <queue>
#include <stdint.h>

enum Regs{Reg0,Reg1,Reg2,Reg3};
enum XBarInput{NORTH_I,EAST_I,WEST_I,SOUTH_I,RES_I,ALU_I,INV};
enum Dir{NORTH,EAST,WEST,SOUTH};

typedef uint32_t DataType;

typedef struct{
	XBarInput P;
	XBarInput I1;
	XBarInput I2;
	XBarInput NORTH_O;
	XBarInput EAST_O;
	XBarInput WEST_O;
	XBarInput SOUTH_O;
} XbarConfig;

typedef struct{
	XbarConfig xB;
	std::map<Regs,uint8_t> regwen;
	std::map<Regs,uint8_t> regbypass; //1 = get it from reg and 0 = bypass
	uint8_t tregwen;
	uint8_t opcode;
	uint32_t constant;
	bool constValid;
	bool NPB;
} HyIns;

#endif
