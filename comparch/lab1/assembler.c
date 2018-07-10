/*
 * Name 1: Raphael De Los Santos
 * Name 2: Quinten Zambeck
 * UTEID 1: rd23353
 * UTEID 2: qaz62
 */
#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */
#include <stdlib.h>


#define MAX_LINE_LENGTH 255
#define OPCODE_NUM 28
#define FOREACH_OPCODE(OPCODE) \
    OPCODE(BR) \
    OPCODE(BRP) \
    OPCODE(BRZ) \
    OPCODE(BRZP) \
    OPCODE(BRN) \
    OPCODE(BRNP) \
    OPCODE(BRNZ) \
    OPCODE(BRNZP) \
    OPCODE(ADD) \
    OPCODE(LDB) \
    OPCODE(STB) \
    OPCODE(JSR) \
    OPCODE(JSRR) \
    OPCODE(AND) \
    OPCODE(LDW) \
    OPCODE(STW) \
    OPCODE(RTI) \
    OPCODE(XOR) \
    OPCODE(NOT) \
    OPCODE(JMP) \
    OPCODE(RET) \
    OPCODE(LSHF) \
    OPCODE(RSHFL) \
    OPCODE(RSHFA) \
    OPCODE(LEA) \
    OPCODE(TRAP) \
    OPCODE(NOP) \
    OPCODE(HALT) \
    OPCODE(NILL) \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,
#define GENERATE_MAP(X) {X, #X},


typedef enum error {
    no_error,
    error_label,
    error_opcode,
    error_constant,
    error_other
} error_code;

/* ansi c == c90, no bool */
typedef enum bool {
    false = 0,
    true = 1
} bool;

enum opcode_enum{
    FOREACH_OPCODE(GENERATE_ENUM)
} codes;

static const char *opcodes[] = {
    FOREACH_OPCODE(GENERATE_STRING)
    "\0"
};

struct mapping{
    enum opcode_enum value;
    char *name;
} enum_map[] = { FOREACH_OPCODE(GENERATE_MAP) };

enum opcode_enum str2enum(char *str){
    int i;
    for(i = 0; i < OPCODE_NUM; i++){
        if(strcmp(str, enum_map[i].name) == 0){
            return enum_map[i].value;
        }
    }
    return NILL;
}

const char* reserved[] = {
    "IN",
    "OUT",
    "GETC",
    "PUTS",
    ".FILL",
    "\0"
};

const char* registers[] = {
    "R0",
    "R1",
    "R2",
    "R3",
    "R4",
    "R5",
    "R6",
    "R7",
    "\0"
};



void remove_comment(char* pLine){
    char* lPtr = pLine;
    while(*lPtr != ';' && *lPtr != '\0' && *lPtr != '\n'){
        lPtr++;
    }
    *lPtr = '\0';
}


int symbolCount;
#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255
typedef struct {
    int address;
    char label[MAX_LABEL_LEN + 1];
} TableEntry;


/* makes a given string into uppercase
 * string must be NULL terminated
 */
void toUpper(char* str){
    int i;
    for(i = 0; i < strlen(str); i++){
        str[i] = toupper(str[i]);
    }
}

/* returns true if string is a reserved word
 * returns false otherwise
 */
bool is_opcode(const char* name){
    const char** word = opcodes;
    while(strcmp(*word, "\0") != 0){
        if(strcmp(name, *word) == 0){
            return true;
        }
        word++;
    }
    word = reserved;
    while(strcmp(*word, "\0") != 0){
        if(strcmp(name, *word) == 0){
            return true;
        }
        word++;
    }
    return false;
}


bool valid_label(const char* name){
    const char** word = registers;
    if(!isalpha(*name) || name[0] == 'X'){
        return false;
    }
    while(strcmp(*word, "\0") != 0){
        if(strcmp(name, *word) == 0){
            return false;
        }
        word++;
    }
    return true;
}


bool insert_symbol(TableEntry symbols[], int* symbol_count, const char* name, int value){
    int i = 0;
    while(i < *symbol_count){
        if(strcmp(name, symbols[i].label) == 0){
            exit(error_other);
        }
        i++;
    }
    (*symbol_count) = (*symbol_count) + 1;
    symbols[i].address = value;
    strcpy(symbols[i].label, name);
    return true;
}

/*returns -1 if symbol does not exist
  might be able to just exit(2)*/
int get_symbol_value(TableEntry symbols[], int symbol_count, const char* name){
    int i = 0;
    while(i < symbol_count){
        if(strcmp(name, symbols[i].label) == 0){
            return symbols[i].address;
        }
        i++;
    }
    return -1;
}



void setUpHexNum(char **hexNum){
    int i = 0;
    int num = 0;
    while(i < OPCODE_NUM){
        if(i <= TRAP && i != RET && i != RTI){
            hexNum[i] = (char *)malloc(2*sizeof(char));
            sprintf(hexNum[i], "%X", num);
        }
        else if(i == NOP){
            hexNum[i] = "0000\0";
        }
        else if(i == HALT){
            hexNum[i] = "F025\0";
        } else if(i == RET){
            hexNum[i] = "C1C0\0";
        } else if(i == RTI){
            hexNum[i] = "8000\0";
        }
        i++;
        num++;
        if(i <= BRNZP || i == JSRR || i == RET || i == NOT || i == RSHFL || i == RSHFA){
            num--;
        }
        if(i == JMP)
            num = 12;
    }
}


error_code phase1_parse(FILE* inFile, TableEntry symbols[], int* symbol_count, int* orig){
    char pLine[MAX_LINE_LENGTH + 1];
    char* lPtr;
    char* label;
    int i;

    *orig = -2;
    int address_offset; /* label address =  origin + address_offset */

    while (fgets(pLine, MAX_LINE_LENGTH, inFile)){
        /* convert to upper case */
        toUpper(pLine);
        /*------------------------*/

        /*ignore comments*/
        remove_comment(pLine);
        /*------------------------*/
        if(lPtr = strtok(pLine, "\t\n ,")){
            if((*orig == -2) && strcmp(lPtr, ".ORIG") == 0){
                if(!(lPtr = strtok(NULL, "\t\n ,"))){
                    return error_constant;
                }
                *orig = toNum(lPtr);
                if(*orig < 0 || *orig > 0xFFFF || *orig % 2 != 0){
                    return error_constant;
                }
                address_offset = 0;
                lPtr = strtok(NULL, "\t\n ,");
                if(lPtr != NULL){
                    return error_other;
                }
            }
            else if((*orig != -2) && strcmp(lPtr, ".END") == 0){
                lPtr = strtok(NULL, "\t\n ,");
                if(lPtr != NULL){
                    return error_other;
                }
                return no_error;
            }

            /* TODO might need to change orig check here */
            else if((*orig != -2) && !is_opcode(lPtr)){
                if(!valid_label(lPtr)){
                    return error_other;
                }
                if(!insert_symbol(symbols, symbol_count, lPtr, *orig + 2*address_offset)){
                    return error_other;
                }
                lPtr = strtok(NULL, "\t\n ,");
                if(lPtr == NULL){
                    exit(error_opcode);
                }
                address_offset += 1;
            }
            else if((*orig == -2 && (strtok(NULL, "\t\n ,") != NULL))){
                        return error_other;
            }
            else{
                address_offset += 1;
            }
            /*
            if(*orig >= 0){
                lPtr = strtok(NULL, "\t\n ,");
                if(lPtr != NULL){
                    address_offset += 1;
                }
            }
            */
        }
    }
    exit(error_other);
    return error_other; /*never hit .END */
}


int get_register_by_name(const char* reg){
    int num;
    if(strlen(reg) == 2){
        if(reg[0] == 'R'){
            if((reg[1] >= '0') && (reg[1] <= '7')){
                return (int) ((reg[1]) - '0');
            }
        }
    }
    exit(error_other);
}

bool immRange(int* check, int size){
    if(*check >= -(1<<(size-1)) && *check < (1<<(size-1))){
        *check = *check & ((1<<size) - 1);
        return true;
    }
    return false;
}

char* strtok_error(){
    char* ret = strtok(NULL, "\t\n ,");
    if(ret == NULL){
        exit(error_other);
    }
    return ret;
}

int next_register(){
    char *token = strtok_error();
    return get_register_by_name(token);
}


void parseCode(FILE* infile, FILE* outfile, int orig, int symbol_count, TableEntry *symbolTable, char** hexNum){
    int PC = orig;
    char codeLine[MAX_LINE_LENGTH + 1];
    int dr_reg;
    int sr_reg;
    int sr2_reg;
    int amount;
    int value;
    int label;
    while(fgets(codeLine, MAX_LINE_LENGTH, infile)){
        toUpper(codeLine);
        remove_comment(codeLine);
        char* token = strtok(codeLine, "\t\n ,");
        if(token == NULL){
            continue;
        }
        if(strcmp(token, ".END") == 0){
            if(strtok(NULL, "\t\n ,") != NULL)
               exit( error_other );
            return;
        }

        if(token == NULL)
            continue;
        if(strcmp(token, ".ORIG") == 0){
            fprintf(outfile, "0x%04X\n", orig);
            continue;
        } else if(get_symbol_value(symbolTable, symbol_count, token) != -1){
            token = strtok(NULL, "\t\n ,");
            if(token == NULL){
                continue;
            }
        } if(strcmp(token, ".FILL") == 0){
            token = strtok_error();
            amount = toNum(token);
            if(amount <= -32768 || amount > 65535)
                exit( error_constant );


            amount = amount & 0xFFFF;
            fprintf(outfile, "0x%04X\n", amount);

            token = strtok(NULL, "\t\n ,");
            if(token == NULL)
                continue;
            else
                exit( error_other );
        }
        enum opcode_enum opcode = str2enum(token);
        switch(opcode){
            case BR:
            case BRNZP:
            case BRN:
            case BRZ:
            case BRP:
            case BRNZ:
            case BRNP:
            case BRZP:
                if(opcode == BR){
                    opcode = BRNZP;
                }
                token = strtok_error();

                if(token[0] == '#' || token [0] == 'X')
                    exit(error_other);

                label = get_symbol_value(symbolTable, symbol_count, token);
                if(label != -1){
                    /*int offset = -((PC + 2) - label);*/
                    int offset = (label - (PC + 2)) >> 1;
                    if(!immRange(&offset, 9))
                        exit(error_other);

                    fprintf(outfile, "0x0%03X", ((opcode<<9) | offset));
                } else
                    exit(error_label);

                break;
            case ADD:
            case AND:
                dr_reg = next_register();
                sr_reg = next_register();

                token = strtok_error();

                if(token[0] == 'R'){ /* REGISTER mode */
                    sr2_reg = get_register_by_name(token);
                    fprintf(outfile, "0x%s%03X", hexNum[opcode], ((dr_reg << 9) | (sr_reg << 6) | (sr2_reg)));
                }
                else{
                    value = toNum(token);
                    if(!immRange(&value, 5)){
                        exit( error_constant);
                    }
                    fprintf(outfile, "0x%s%03X", hexNum[opcode], ((dr_reg << 9) | (sr_reg << 6) | 0x20 | (value)));
                }
                break;
            case JSR:
            case LEA:
                if(LEA == opcode){
                    dr_reg = next_register(token);
                }
                token = strtok_error();

                if(token[0] == '#' || token [0] == 'X')
                    exit(error_other);

                label = get_symbol_value(symbolTable, symbol_count, token);
                if(label == -1)
                    exit(error_label);

                /*amount = label - (PC + 2);*/
                amount = (label - (PC + 2))>>1;
                if(opcode == JSR){
                    if(!immRange(&amount, 11))
                        exit(error_other);
                    fprintf(outfile, "0x%s%03X", hexNum[opcode], (0x800 | amount));
                }
                else{
                    if(!immRange(&amount, 9))
                        exit(error_other);
                    fprintf(outfile, "0xE%03X", ((dr_reg<<9) | amount));
                }

                break;
            case JSRR:
            case JMP:
                dr_reg = next_register();
                fprintf(outfile, "0x%s%03X", hexNum[opcode], ((dr_reg << 6)));
                break;
            case LDB:
            case STB:
            case LDW:
            case STW:
                dr_reg = next_register();
                sr_reg = next_register();

                token = strtok_error();
                amount = toNum(token);

                if(!immRange(&amount, 6))
                    exit(error_constant);

                fprintf(outfile, "0x%s%03X", hexNum[opcode], ((dr_reg<<9) | (sr_reg<<6) | amount));
                break;
            case XOR:
            case NOT:
                dr_reg = next_register();
                sr_reg = next_register();

                if(NOT == opcode)
                    fprintf(outfile, "0x9%03X", ((dr_reg << 9) | (sr_reg << 6) | (0x3F)));
                else{
                    token = strtok_error();
                    if(token[0] == 'R'){ /* REGISTER mode */
                        sr2_reg = get_register_by_name(token);
                        fprintf(outfile, "0x9%03X", ((dr_reg << 9) | (sr_reg << 6) | (sr2_reg)));
                    }
                    else{
                        value = toNum(token);
                        if(!immRange(&value, 5)){
                            exit(error_constant);
                        }

                        fprintf(outfile, "0x9%03X", ((dr_reg << 9) | (sr_reg << 6) | 0x20 | (value)));
                    }
                }
                break;

            case LSHF:
            case RSHFL:
            case RSHFA:
                dr_reg = next_register();
                sr_reg = next_register();

                token = strtok_error();
                amount = toNum(token);
                if(amount < 0 || (amount >= (1 << 4))){
                    exit(error_constant);
                }
                if(opcode == LSHF)
                    fprintf(outfile, "0xD%02X%01X", ((dr_reg << 5) | (sr_reg << 2)), amount);
                else if(opcode == RSHFL)
                    fprintf(outfile, "0xD%02X%01X", ((dr_reg << 5) | (sr_reg << 2) | 0x1), amount);
                else if(opcode == RSHFA)
                    fprintf(outfile, "0xD%02X%01X", ((dr_reg << 5) | (sr_reg << 2) | 0x3), amount);
                break;
            case TRAP:
                token = strtok_error();
                if(token[0] == 'X' && strlen(token) == 3){
                    int num;
                    num = toNum(token);
                    if(num < 0){
                        exit(error_constant);
                    }
                    fprintf(outfile, "0xF0%02X", num);
                }
                else
                    exit( error_constant );
                break;
            case RTI:
            case RET:
            case NOP:
            case HALT:
                fprintf(outfile, "0x%s", hexNum[opcode]);
                break;
            case NILL:
                exit( error_opcode );
        }
        fprintf(outfile, "\n");
        PC += 2;

        token = strtok(NULL, "\t\n ,");
        if(token != NULL)
            exit(error_other);
    }
}

int
main(int argc, char* argv[]) {

    FILE* infile = NULL;
    FILE* outfile = NULL;

    /* open the source file */
    infile = fopen(argv[1], "r");
    outfile = fopen(argv[2], "w");

    if (!infile) {
        printf("Error: Cannot open file %s\n", argv[1]);
        exit(4);
    }
    if (!outfile) {
        printf("Error: Cannot open file %s\n", argv[2]);
        exit(4);
    }

    /* Do stuff with files */
    TableEntry symbolTable[MAX_SYMBOLS];
    int symbol_count = 0;
    int origin;
    error_code exitcode = phase1_parse(infile, symbolTable, &symbol_count, &origin);
    if(exitcode != no_error){
        exit(exitcode);
    }

    /*    int i;
          for(i = 0; i < symbol_count; i++){
          printf("The Label %s is at memory address %x\n", symbolTable[i].label, symbolTable[i].address);
          }*/
    char **hexNum = (char**)malloc(OPCODE_NUM*sizeof(char**));
    setUpHexNum(hexNum);

    rewind(infile);

    parseCode(infile, outfile, origin, symbol_count, symbolTable, hexNum);

    fclose(infile);
    fclose(outfile);
}

/*Convert a String To a Number*/
int toNum( char * pStr )
{
    char * t_ptr;
    char * orig_pStr;
    int t_length,k;
    int lNum, lNeg = 0;
    long int lNumLong;

    orig_pStr = pStr;
    if( *pStr == '#' )   /* decimal */
    {
        pStr++;
        if( *pStr == '-' ) /* dec is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for(k=0;k < t_length;k++)
        {
            if (!isdigit(*t_ptr))
            {
                printf("Error: invalid decimal operand, %s\n",orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNum = atoi(pStr);
        if (lNeg)
            lNum = -lNum;

        return lNum;
    }
    else if( *pStr == 'X' )  /* hex     */
    {
        pStr++;
        if( *pStr == '-' )    /* hex is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for(k=0;k < t_length;k++)
        {
            if (!isxdigit(*t_ptr))
            {
                printf("Error: invalid hex operand, %s\n",orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNumLong = strtol(pStr, NULL, 16); /* convert hex string into integer */
        lNum = (lNumLong > INT_MAX)? INT_MAX : lNumLong;
        if( lNeg )
            lNum = -lNum;
        return lNum;
    }
    else
    {
        printf( "Error: invalid operand, %s\n", orig_pStr);
        exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
    }
}
