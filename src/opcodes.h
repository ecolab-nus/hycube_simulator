/*
 * opcodes.h
 *
 *  Created on: 6 Mar 2017
 *      Author: manupa
 */

#ifndef OPCODES_H_
#define OPCODES_H_

#define NOP 0
#define ADD 1
#define SUB 2
#define MUL 3
#define SEXT 4
#define DIV 5


#define LS 8
#define RS 9
#define ARS 10
#define AND 11
#define OR 12
#define XOR 13


#define SELECT 16
#define CMERGE 17
#define CMP 18
#define CLT 19
#define BR 20
#define CGT 21
#define LOADCL 22
#define MOVCL 23
#define LOAD 24
#define LOADH 25
#define LOADB 26
#define STORE 27
#define STOREH 28
#define STOREB 29
#define JUMPL 30
#define MOVC 31

#endif /* OPCODES_H_ */
