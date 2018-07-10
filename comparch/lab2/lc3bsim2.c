/*
    REFER TO THE SUBMISSION INSTRUCTION FOR DETAILS

    Name 1: Raphael De Los Santos
    Name 2: Quinten Zambeck
    UTEID 1: rd23353
    UTEID 2: qaz62
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

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
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A
*/

#define WORDS_IN_MEM    0x08000
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
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
/* Purpose   : Simulate the LC-3b until HALTed                 */
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

  printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
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
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
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

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) {
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;
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
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/
#define FOREACH_OPCODE(OPCODE) \
    OPCODE(BR) \
    OPCODE(ADD) \
    OPCODE(LDB) \
    OPCODE(STB) \
    OPCODE(JSR) \
    OPCODE(AND) \
    OPCODE(LDW) \
    OPCODE(STW) \
    OPCODE(RTI) \
    OPCODE(XOR) \
	OPCODE(NOT_OPCODE) \
	OPCODE(UNUSED) \
    OPCODE(JMP) \
    OPCODE(SHF) \
    OPCODE(LEA) \
    OPCODE(TRAP) \
    OPCODE(NILL) \

#define GENERATE_ENUM(ENUM) ENUM,
#define SIGNEX(v, sb) ((v) | (((v) & (1 << (sb))) ? ~((1 << (sb))-1) : 0))

enum opcode{
	FOREACH_OPCODE(GENERATE_ENUM)
};


void updateLatch(){
    NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;
    NEXT_LATCHES.N = CURRENT_LATCHES.N;
    NEXT_LATCHES.Z = CURRENT_LATCHES.Z;
    NEXT_LATCHES.P = CURRENT_LATCHES.P;

    int i;
    for(i = 0; i < LC_3b_REGS; i++)
        NEXT_LATCHES.REGS[i] = CURRENT_LATCHES.REGS[i];
}

void setcc(int N, int Z, int P){
    NEXT_LATCHES.N = N;
    NEXT_LATCHES.Z = Z;
    NEXT_LATCHES.P = P;
}

void fetch_decode(int IR){
    int instruction = (IR & 0x0F000 ) >> 12;

    int DR;
    int SR1;
    int SR2;
    int offset;
    int n, z, p;

    switch(instruction){
        case BR:
            n = ((IR >> 11) & 0x01);
            z = (IR >> 10) & 0x01;
            p = (IR >> 9) & 0x01;

            if((n && NEXT_LATCHES.N) || (z && NEXT_LATCHES.Z) || (p && NEXT_LATCHES.P))
                NEXT_LATCHES.PC = NEXT_LATCHES.PC + (SIGNEX((IR & 0x01FF), 8) << 1);
            break;
        case ADD:
        case AND:
        case XOR:
            DR = (IR >> 9) & 0x07;                                                  /*Destination Register number */
            SR1 = NEXT_LATCHES.REGS[(IR >> 6) & 0x07];                              /*contents of Source Register 1 */

            if(((IR >> 5) & 0x01) == 1)                                             /*check addressing mode */
                SR2 = SIGNEX((IR & 0x1F), 4);                                       /*SR2 is the immediate number */
            else
                SR2 = NEXT_LATCHES.REGS[IR & 0x07];                                 /*SR2 is the contents of Source Register 2 */

            if(instruction == ADD)
                NEXT_LATCHES.REGS[ DR ] = Low16bits(SR1 + SR2);                     /*ADD SR! and SR2 */
            else if(instruction == AND)
                NEXT_LATCHES.REGS[ DR ] = Low16bits(SR1 & SR2);                     /*AND SR1 and SR2 */
            else
                NEXT_LATCHES.REGS[ DR ] = Low16bits(SR1 ^ SR2);                     /*XOR SR1 and SR2 */

            if(NEXT_LATCHES.REGS[DR] == 0)                                          /*set condition codes */
                setcc(0, 1, 0);
            else if(SIGNEX(NEXT_LATCHES.REGS[DR], 15) < 0)
                setcc(1, 0, 0);
            else
                setcc(0, 0, 1);
            break;
        case LDB:
            DR = (IR >> 9) & 0x07;                                                  /*Destination Register Number */
            offset = SIGNEX((IR & 0x3F), 5);                                        /*Sign extended offset */
            SR1 = NEXT_LATCHES.REGS[ (IR >> 6) & 0x07 ];                            /*contents of Source Register 1 */
            offset = SR1 + offset;                                                  /*add contents of Source Register 1 to offset */

            if(offset % 2 == 0)                                                     /*if alligned acces then contents of low bits */
                NEXT_LATCHES.REGS[DR] = Low16bits( SIGNEX(MEMORY[offset>>1][0], 7) );
            else                                                                    /*if unalligned acces, then get contents of high bits */
                NEXT_LATCHES.REGS[DR] = Low16bits( SIGNEX(MEMORY[offset>>1][1], 7) );

            if(NEXT_LATCHES.REGS[DR] == 0)                                          /*set condition codes */
                setcc(0, 1, 0);
            else if(SIGNEX(NEXT_LATCHES.REGS[DR], 15) < 0)
                setcc(1, 0, 0);
            else
                setcc(0, 0, 1);
            break;
        case STB:
            DR = NEXT_LATCHES.REGS[ (IR >> 6) & 0x07 ];                             /*get contents of the Base Register */
            offset = SIGNEX((IR & 0x3F), 5);                                        /*sign extend the offset */
            SR1 = NEXT_LATCHES.REGS[ (IR >> 9) & 0x07];                             /*get the contents of the Source Register*/

            if((DR + offset) % 2 == 0)                                              /*if alligned access then store contents of source into low bits */
                MEMORY[ (DR + offset) >> 1 ][0] =  SR1  & 0xFF;
            else                                                                    /*if unalligned acces then store contents of source into high bits */
                MEMORY[ (DR + offset) >> 1 ][1] =  SR1  & 0xFF;
            break;
        case JSR:
            NEXT_LATCHES.REGS[7] = Low16bits(NEXT_LATCHES.PC);                      /*set Register 7 to be the incremented PC */

            if(((IR >> 11) & 0x01) == 0){                                           /*check addressing mode */
                DR = NEXT_LATCHES.REGS[ (IR >> 6) & 0x07 ];                         /*get contents of the Base Register */
                NEXT_LATCHES.PC = DR;                                               /*set PC to be the contents of Base Register */
            } else{
                NEXT_LATCHES.PC = NEXT_LATCHES.PC + (SIGNEX((IR & 0x07FF), 10) << 1);  /*set PC = PC† + Sign extended offset << 1*/
            }
            break;
        case LDW:
            offset = SIGNEX((IR & 0x3F), 5);                                        /*sign extend the offset */
            offset = offset << 1;                                                   /*left shift the offset */

            SR1 = NEXT_LATCHES.REGS[ (IR >> 6) & 0x07 ];                            /*get the contents of the Source Register */
            DR = (IR >> 9) & 0x07;                                                  /*get the Destination Register Number */

            NEXT_LATCHES.REGS[DR] = Low16bits( (MEMORY[(SR1 + offset) >> 1][1] << 8) | MEMORY[(SR1 + offset) >> 1][0]); /*set Contents of Destination Register */

            if(NEXT_LATCHES.REGS[DR] == 0)                                         /*set Condition codes */
                setcc(0, 1, 0);
            else if(SIGNEX(NEXT_LATCHES.REGS[DR], 15) < 0)
                setcc(1, 0, 0);
            else
                setcc(0, 0, 1);
            break;
        case STW:
            DR = NEXT_LATCHES.REGS[ (IR >> 6) & 0x07 ];                             /*get the Contents of the Base Register */
            offset = (SIGNEX((IR & 0x3F), 5) << 1);                                 /*get the offset and sign extend */
            SR1 = Low16bits(NEXT_LATCHES.REGS[ (IR >> 9) & 0x07]);                  /*get the contents of the Source Register */

            MEMORY[ (DR + offset)>>1 ][1] = (SR1 & 0xFF00) >> 8;                    /*store the contents of Source Register into memory location of Base Register */
            MEMORY[ (DR + offset)>>1 ][0] = SR1 & 0x00FF;
            break;
        case RTI:                                                                   /*doc says to ignore RTI */
            break;
        case JMP:
            DR = (IR >> 6) & 0x07;                                                  /*get the Base Register Number */
            NEXT_LATCHES.PC = NEXT_LATCHES.REGS[DR];                                /*set the PC to be the contents of Base Register*/
            break;
        case SHF:
            DR = (IR >> 9) & 0x07;                                                  /*get the Destination Register Number */
            SR1 = NEXT_LATCHES.REGS[ (IR >> 6) & 0x07 ];                            /*get the contents of Source Register */
            SR2 = IR & 0x0F;                                                        /*get the immediate offset */
            if(((IR >> 4) & 0x01) == 0){                                            /*if left shift */
                NEXT_LATCHES.REGS[DR] = Low16bits((SR1 << SR2));
            } else if (((IR >> 5) & 0x01) == 0){                                    /*if right shift logical */
                NEXT_LATCHES.REGS[DR] = Low16bits((SR1 >> SR2));
            } else{                                                                 /*if right shift arithmetic */
                SR1 = SIGNEX(SR1, 15);
                NEXT_LATCHES.REGS[DR] = Low16bits( (signed) SR1 >> SR2);
            }

            if(NEXT_LATCHES.REGS[DR] == 0)                                          /*set condition codes */
                setcc(0, 1, 0);
            else if(SIGNEX(NEXT_LATCHES.REGS[DR], 15) < 0)
                setcc(1, 0, 0);
            else
                setcc(0, 0, 1);
            break;
        case LEA:
            DR = (IR >> 9) & 0x07;                                                  /*get Destination Register Number*/
            offset = SIGNEX(IR & 0x01FF, 8) << 1;                                   /*Sign extend and left shift the offset */
            NEXT_LATCHES.REGS[DR] = Low16bits(offset + NEXT_LATCHES.PC);            /*set the contents of Destination register to the memory location PC + offset*/
            break;
        case TRAP:
            NEXT_LATCHES.REGS[7] = Low16bits(NEXT_LATCHES.PC);
            NEXT_LATCHES.PC = 0;
            break;
    }
}

void process_instruction(){
    /*  function: process_instruction
     *
     *    Process one instruction at a time
     *       -Fetch one instruction
     *       -Decode
     *       -Execute
     *       -Update NEXT_LATCHES
     */
    int MAR = CURRENT_LATCHES.PC >> 1;
    int MDR = (MEMORY[MAR][1] << 8) | MEMORY[MAR][0];

    updateLatch();
    fetch_decode(MDR);

}

