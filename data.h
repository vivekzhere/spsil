/*
  Contains basic declarations for register etc.
*/

#ifndef DATA_H
#define DATA_H
#define R0		0
#define R1		1
#define R2		2
#define R3		3
#define R4		4
#define R5		5
#define R6		6
#define R7		7

#define S0		8
#define S1		9
#define S2		10
#define S3		11
#define S4		12
#define S5		13
#define S6		14
#define S7		15
#define S8		16
#define S9		17
#define S10		18
#define S11		19
#define S12		20
#define S13		21
#define S14		22
#define S15		23

#define T0		24
#define T1		25
#define T2		26
#define T3		27

#define BP_REG		28
#define IP_REG		29
#define SP_REG		30
#define PTBR_REG	31
#define PTLR_REG	32
#define EFR_REG		33

#define NO_USER_REG		8
#define NO_SYS_REG		16
#define NO_TEMP_REG  	4
#define NO_SPECIAL_REG  6

#define NUM_REGS		(NO_USER_REG + NO_SPECIAL_REG + NO_SYS_REG + NO_TEMP_REG)

#endif
