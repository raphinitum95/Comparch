/*
        Name: Raphael De Los Santos
        UTEID: rd23353
 */
/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {
    IRD,
    COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    LD_PSR,
    LD_USP,
    LD_SSP,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    GATE_PSR,
    GATE_USP,
    GATE_SSP,
    GATE_VECT,
    PSRMUX1, PSRMUX0,
    R6MUX1, R6MUX0,
    PCMUX1, PCMUX0,
    DRMUX1, DRMUX,
    SR1MUX1, SR1MUX,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
/* MODIFY: you have to add all your new control signals */
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
				      (x[J3] << 3) + (x[J2] << 2) +
				      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); }
int GetLSHF1(int *x)         { return(x[LSHF1]); }
/* MODIFY: you can add more Get functions for your new control signals */

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A
   There are two write enable signals, one for each byte. WE0 is used for
   the least significant byte of a word. WE1 is used for the most significant
   byte of a word. */

#define WORDS_IN_MEM    0x08000
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */

int READY;	/* ready bit */
  /* The ready bit is also latched as you don’t want the memory system to assert it
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microinstruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */

/* For lab 4 */
int INTV; /* Interrupt vector register */
int EXCV; /* Exception vector register */
int SSP; /* Initial value of system stack pointer */
int USP; /* initial value of user stack pointer */
int PSR; /* the process status register */
int EX_IN_Flag;
/* MODIFY: You may add system latches that are required by your implementation */

} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

  eval_micro_sequencer();
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {
    int i;

    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
	if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
	}
	cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {
    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
	cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
    int address; /* this is a byte address */

    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
    int k;

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
    case 'G':
    case 'g':
	go();
	break;

    case 'M':
    case 'm':
	scanf("%i %i", &start, &stop);
	mdump(dumpsim_file, start, stop);
	break;

    case '?':
	help();
	break;
    case 'Q':
    case 'q':
	printf("Bye.\n");
	exit(0);

    case 'R':
    case 'r':
	if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
	else {
	    scanf("%d", &cycles);
	    run(cycles);
	}
	break;

    default:
	printf("Invalid Command\n");
	break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {
    FILE *ucode;
    int i, j, index;
    char line[200];

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	    index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
	MEMORY[i][0] = 0;
	MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {
    FILE * prog;
    int ii, word, program_base;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
	printf("Error: Can't open program file %s\n", program_filename);
	exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
	program_base = word >> 1;
    else {
	printf("Error: Program file is empty\n");
	exit(-1);
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
	/* Make sure it fits. */
	if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
		   program_filename, ii);
	    exit(-1);
	}

	/* Write the word to memory array. */
	MEMORY[program_base + ii][0] = word & 0x00FF;
	MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
	ii++;
    }

    if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *program_filename, int num_prog_files) {
    int i;
    init_control_store(ucode_filename);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
        load_program(program_filename);
        while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */
    CURRENT_LATCHES.PSR = 0x8002;

    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 3) {
        printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
                argv[0]);
        exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argv[2], argc - 2);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
        printf("Error: Can't open dumpsim file\n");
        exit(-1);
    }

    while (1)
        get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated
   with a "MODIFY:" comment.

   Do not modify the rdump and mdump functions.

   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS
   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/

#define SIGNEX(v, sb) ((v) | (((v) & (1 << (sb))) ? ~((1 << (sb))-1) : 0))
#define CL(X) (CURRENT_LATCHES.X)
#define NL(X) (NEXT_LATCHES.X)
#define INST(x) (CL(MICROINSTRUCTION)[x])
#define ADDER(sr1) ((addr2mux == 0) ? 0 : (addr2mux == 1) ? (SIGNEX((ir & 0x3F), 5) << INST(LSHF1)) : (addr2mux == 2) ? (SIGNEX((ir & 0x1FF), 8) << INST(LSHF1)) : (SIGNEX((ir & 0x7FF), 10) << INST(LSHF1))) + ((INST( ADDR1MUX ) == 0) ? CL(PC) : sr1)

int interrupt = 0;
int exception = 0;
void eval_micro_sequencer() {
    /*
     * Evaluate the address of the next state according to the
     * micro sequencer logic. Latch the next microinstruction.
     */

    /* Determining next state */
    int cur_state = CL(STATE_NUMBER);
    int j5 = INST(J5) << 5;
    int j4 = INST(J4) << 4;
    int j3 = INST(J3) << 3;
    int j2 = INST(J2) << 2;
    int j1 = INST(J1) << 1;
    int j0 = INST(J0);

    int state = Low16bits( j5 | j4 | j3 | j2 | j1 | j0 );
    int s = CL(STATE_NUMBER);
    exception = ( ( (0 <= CL(MAR)) && (CL(MAR) < 0x3000) && (((CL(PSR) >> 15) & 0x01) == 1)) && ((s == 50) || (s == 45) || (s == 33) || (s == 23) || (s == 35)) ) ? 1 : ((CL(MAR) & 0x01) && ((s == 45) || (s == 23) || (s == 50))) ? 1 :  exception;
    CL(EXCV) = ( ( (0 <= CL(MAR)) && (CL(MAR) < 0x3000) && (((CL(PSR) >> 15) & 0x01) == 1)) && ((s == 50) || (s == 45) || (s == 33) || (s == 23) || (s == 35)) ) ? 0x02 : ((CL(MAR) & 0x01) && ((s == 45) || (s == 23) || (s == 50))) ? 0x03 :  CL(EXCV);

    if(s == 48)
        printf("Clock number is %i\n", CYCLE_COUNT);

    /* choosing j bits or IR[15 ... 12] */
    if(INST(IRD) == 1){
        state = ((CL(IR) & 0xF000) >> 12);
    } else{
        /* adding to state depending on condition variable */
        if(INST(COND1) == 0 && INST(COND0) == 1)          /* memory ready */
            state += (CL(READY) << 1);

        else if(INST(COND1) == 1 && INST(COND0) == 1)     /* addressing mode */
            state += ( (CL(IR) >> 11 ) & 0x01);

        else if(INST(COND1) == 1 && INST(COND0) == 0)     /* Branch mode */
            state += (CL(BEN) << 2);
    }

    state = exception || interrupt ? 48 : state;
    NL(EX_IN_Flag) = exception ? 1 : CL(EX_IN_Flag);

    exception = CL(STATE_NUMBER) == 10 || CL(STATE_NUMBER) == 11 ? 0 : exception;
    exception = interrupt = 0;
    if(CYCLE_COUNT == 299){
        interrupt = 1;
        NL(EX_IN_Flag) = interrupt ? 0 : CL(EX_IN_Flag);
    }

    int i;
    for(i = 0; i < CONTROL_STORE_BITS; i++){
        NL(MICROINSTRUCTION)[i] = CONTROL_STORE[state][i];
    }
    NL(STATE_NUMBER) = state;
}

static int ready = 0;
void cycle_memory() {
    /*
     * This function emulates memory and the WE logic.
     * Keep track of which cycle of MEMEN we are dealing with.
     * If fourth, we need to latch Ready bit at the end of
     * cycle to prepare microsequencer for the fifth cycle.
     */
    if(INST(COND1) == 0 && INST(COND0) == 1){
        ready++;

        if(ready == 4){
            NL(READY) = 1;
            NL(MDR) = Low16bits( (MEMORY[CL(MAR) >> 1][1] << 8) | MEMORY[CL(MAR) >> 1][0]);
        }
        if(ready == 5)
            ready = 0;
    } else{
        NL(READY) = 0;
    }
}

int tristate_driver[9];
int mod_val = GATE_PC;
#define TD(X) (tristate_driver[X % mod_val])

void eval_bus_drivers() {
    /*
     * Datapath routine emulating operations before driving the bus.
     * Evaluate the input of tristate drivers
     *       Gate_MARMUX,
     *		 Gate_PC,
     *		 Gate_ALU,
     *		 Gate_SHF,
     *		 Gate_MDR.
     */
    int addr2mux = (INST(ADDR2MUX1) << 1) | INST(ADDR2MUX0);
    int ir = CL(IR);
    int sr1mux = (INST(SR1MUX1) << 1) | INST(SR1MUX);
    int sr1 = CL(REGS)[ sr1mux == 0 ? (ir >> 9) & 0x07 : ( sr1mux == 1 ? ir >> 6 & 0x07 : 6)];
    int r6mux = (INST(R6MUX1) << 1) | INST(R6MUX0);
    sr1 = r6mux == 0 ? sr1 : (r6mux == 1 ? sr1 + 2 : sr1 - 2);

    int adder = Low16bits(ADDER(sr1));
    int zext = Low16bits( (ir & 0x0FF) << 1 );

    int aluk = (INST( ALUK1 ) << 1) | INST( ALUK0 );
    int sr2mux = (((ir >> 5) & 0x01) == 0 ?  CL(REGS)[ ir & 0x07 ] : SIGNEX( (ir & 0x1F), 4));

    int alu = Low16bits( (aluk == 0) ? sr1 + sr2mux :
            (aluk == 1) ? sr1 & sr2mux :
            (aluk == 2) ? sr1 ^ sr2mux : sr1 );

    int mdr = ((INST(DATA_SIZE) == 0) ? ( ((CL(MAR) & 0x01) == 1) ? SIGNEX((CL(MDR) >> 8) & 0x0FF, 7) : SIGNEX(CL(MDR) & 0x0FF , 7)) : CL(MDR));

    int shifter = (ir >> 4) & 0x03;
    int shift_amt = ir & 0x0F;
    int shift = ( (shifter == 0) ? (sr1 << shift_amt) :
            (shifter == 1) ? (sr1 >> shift_amt) : Low16bits( SIGNEX( sr1, 15 ) >> shift_amt ));

    TD( GATE_PC ) = CL(PC);
    TD( GATE_MARMUX) = (INST(MARMUX) == 0 ? zext : adder);
    TD( GATE_ALU ) = alu;
    TD( GATE_SHF ) = shift;
    TD( GATE_MDR ) = mdr;
    TD( GATE_PSR ) = CL(PSR);
    TD( GATE_USP ) = CL(USP);
    TD( GATE_SSP ) = CL(SSP);
    TD( GATE_VECT ) = CL(EX_IN_Flag) == 0 ? (CL(INTV) << 1) + 0x0200 : (CL(EXCV) << 1) + 0x0200;
}

void drive_bus() {
    /*
     * Datapath routine for driving the bus from one of the 5 possible
     * tristate drivers.
     */
    BUS = ( (INST(GATE_PC) == 1) ? TD(GATE_PC) :
            (INST(GATE_MARMUX) == 1) ? TD(GATE_MARMUX) :
            (INST(GATE_ALU) == 1) ? TD(GATE_ALU) :
            (INST(GATE_SHF) == 1) ? TD(GATE_SHF) :
            (INST(GATE_MDR) == 1) ? TD(GATE_MDR) :
            (INST(GATE_USP) == 1) ? TD(GATE_USP) :
            (INST(GATE_SSP) == 1) ? TD(GATE_SSP) :
            (INST(GATE_PSR) == 1) ? TD(GATE_PSR) :
            (INST(GATE_VECT) == 1) ? TD(GATE_VECT) : 0);
}

void latch_datapath_values() {
    /*
     * Datapath routine for computing all functions that need to latch
     * values in the data path at the end of this cycle.  Some values
     * require sourcing the bus; therefore, this routine has to come
     * after drive_bus.
     */
    int ir = CL(IR);
    NL(MAR) = ( INST(LD_MAR) == 1) ? BUS : CL(MAR);
    NL(MDR) = ( INST(LD_MDR) == 1) ? (
            (INST(MIO_EN) == 0) ? (
                ( (CL(MAR) & 0x01) == 1) ? ( (BUS & 0x0FF) << 8) | (BUS & 0x0FF) : BUS) :
            ( ready == 0 && (INST(R_W) == 0) ? (MEMORY[CL(MAR)>>1][1] << 8) | MEMORY[CL(MAR)>>1][0] : CL(MDR))) : CL(MDR);

    MEMORY[CL(MAR) >> 1][0] = ( INST(MIO_EN) == 1 && INST(R_W) == 1) ?
        ( (INST(DATA_SIZE) == 0 && (CL(MAR) & 0x01) == 0) || INST(DATA_SIZE) == 1 ? CL(MDR) & 0xFF : MEMORY[CL(MAR) >> 1][0]) : MEMORY[CL(MAR) >> 1][0];

    MEMORY[CL(MAR) >> 1][1] = ( INST(MIO_EN) == 1 && INST(R_W) == 1) ?
        ( (INST(DATA_SIZE) == 0 && (CL(MAR) & 0x01) == 1) || INST(DATA_SIZE) == 1 ? (CL(MDR) >> 8) & 0xFF : MEMORY[CL(MAR) >> 1][1]) : MEMORY[CL(MAR) >> 1][1];

    NL(IR) = ( (INST(LD_IR) == 1) ? BUS : CL(IR));
    NL(BEN) = ( (INST(LD_BEN) == 1) ? ( (((ir >> 11) & 0x01) && CL(N)) || (((ir >> 10) & 0x01 ) && CL(Z)) || (((ir >> 9) & 0x01)  && CL(P)) ) : CL(BEN) );

    int drmux = (INST(DRMUX1) << 1) | INST(DRMUX);
    NL(REGS)[ (drmux == 0 ? (ir >> 9) & 0x07 : drmux == 1 ? 7 : 6) ] = Low16bits( (INST( LD_REG ) == 0) ? CL(REGS)[ drmux == 0 ? (ir >> 9) & 0x07 : drmux == 1 ? 7 : 6  ] : BUS );

    int pcmux = (INST(PCMUX1) << 1) | INST(PCMUX0);
    int sr1mux = (INST(SR1MUX1) << 1) | INST(SR1MUX);
    int r6mux = (INST(R6MUX1) << 1) | INST(R6MUX0);
    int psrmux = (INST(PSRMUX1) << 1) | INST(PSRMUX0);

    int sr1 = CL(REGS)[ sr1mux == 0 ? (ir >> 9) & 0x07 : ( sr1mux == 1 ? ir >> 6 & 0x07 : 6)];
    sr1 = r6mux == 0 ? sr1 : (r6mux == 1 ? sr1 + 2 : sr1 - 2);
    int addr2mux = (INST(ADDR2MUX1) << 1) | INST(ADDR2MUX0);


    NL(USP) = (INST(LD_USP) == 1 ? sr1 : CL(USP));
    NL(SSP) = (INST(LD_SSP) == 1 ? sr1 : CL(SSP));

    NL(PC) = ( (INST(LD_PC) == 1) ? ( (pcmux == 0) ? CL(PC) + 2 : ( (pcmux == 1) ? BUS : (pcmux == 2 ? Low16bits(ADDER(sr1) ) : CL(PC) - 2))) : CL(PC) );
    NL(N) = ( (INST(LD_CC) == 1) ? ( SIGNEX(BUS, 15) < 0 ? 1 : 0) : CL(N));
    NL(Z) = ( (INST(LD_CC) == 1) ? ( SIGNEX(BUS, 15) == 0 ? 1 : 0) : CL(Z));
    NL(P) = ( (INST(LD_CC) == 1) ? ( SIGNEX(BUS, 15) > 0 ? 1 : 0) : CL(P));

    int CC = (NL(N) << 2) | (NL(Z) << 1) | NL(P);
    NL(PSR) = (INST(LD_PSR) == 1 ?
              (psrmux == 0 ? BUS : (psrmux == 1 ? (CL(PSR) & 0xFFF8) | CC : (psrmux == 2 ? CL(PSR) & 0x7FFF : CL(PSR)))) : CL(PSR));
    if(NL(PSR) == BUS && INST(LD_PSR)){
        NL(N) = (NL(PSR) & 0x04) >> 2;
        NL(Z) = (NL(PSR) & 0x02) >> 1;
        NL(P) = (NL(PSR) & 0x01);
    }
    NL(INTV) = CYCLE_COUNT == 299 ? 0x01 : CL(INTV);

    int num = NL(STATE_NUMBER);
    int psr = (NL(PSR) >> 15 ) & 0x01;
    NL(EXCV) = num == 10 || num == 11 ? 0x04 : CL(EXCV);
    exception = (num != 10 && num != 11) ?  exception : 1 ;

}





