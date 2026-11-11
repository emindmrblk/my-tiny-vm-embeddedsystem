#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include <stdbool.h>
#include <stdint.h>

#ifndef IS_STRING
#define IS_STRING(x) (__builtin_types_compatible_p(__typeof__(x), char*) || __builtin_types_compatible_p(__typeof__(x), const char*) || __builtin_types_compatible_p(__typeof__(x), char[]) || __builtin_types_compatible_p(__typeof__(x), const char[]))
#endif

#ifndef IS_INT
#define IS_INT(x) (__builtin_types_compatible_p(__typeof__(x), int)) || (__builtin_types_compatible_p(__typeof__(x), int*)) || (__builtin_types_compatible_p(__typeof__(x), int[])) || (__builtin_types_compatible_p(__typeof__(x), const int)) || (__builtin_types_compatible_p(__typeof__(x), const int[]))
#endif

//#define debug_mode

#define MAX_MEM_SIZE 4096 // 4KB
#define STACK_SIZE 4096
#pragma optimization strict

#define EQUALS 1
#define NEQUALS 0


typedef enum {
    OP_PUSH = 0x01, 
    OP_ADD  = 0x02, 
    OP_PRINT= 0x03, 
    OP_SUM_ALL = 0x04,
    OP_SUB = 0x05,
    OP_DIV = 0x06,
    OP_MUL = 0x07,
    OP_CMP = 0x08,
    OP_JMP = 0x09,
    OP_JMPE = 0x010,
    OP_JNE = 0x11,
    OP_JZ = 0x12,
    OP_STORE = 0x13,
    OP_LOAD = 0x14,
    OP_CALL = 0x15,
    OP_RET = 0x16,
    OP_DUP = 0x17,
    OP_SWAP = 0x18,
    OP_DROP = 0x19,
    OP_POP = 0x20,
    OP_HALT = 0x00 

} Opcode;

typedef enum{

    EX_NONE = 0,
    EX_DIV_ZERO,
    EX_STACK_OVERFLOW,
    EX_STACK_UNDERFLOW,
    EX_INVALID_OPCODE,
    EX_SEGMENTATION_FAULT,
    EX_PAGE_FAULT
    
}err_Handler;

typedef struct 
{
    int pc;
    int sp;
    bool running;
    err_Handler current_ERR;
} vm;


typedef struct
{
    bool Zero_Flag;
    bool Sign_Flag;
    bool Carry_Flag;

} flags;

typedef struct
{
    bool has_argument;
    Opcode op;
    int64_t arg;
} Instruction;


typedef struct 
{
    const char *name;
    Opcode op;
    bool argincld;

} InstructionInfo;


int64_t memory[MAX_MEM_SIZE];

uint8_t physical_memory[16][64];

int stack[STACK_SIZE];
int pc = 0; // program counter
int sp = -1; //stack counter
int storage[256];
int call_stack[256];
int csp = -1;
static uint32_t total_INSTRUCTION = 0;


void count_total_instruction(const char* filename){
    FILE *file = fopen(filename,"r");
    
    while (fgetc(file) == '\n' && fgetc(file) != EOF)
    {
        total_INSTRUCTION++;
    }
    
}

typedef struct 
{
    int frame_num;
    bool valid;
    bool readable;
    bool writable;
    bool executable;

} pageTable;

pageTable page[16];

int64_t tlb[16][64];

int64_t memory_management_unit(int16_t target, int64_t value, char mode){ // basic mmu 
    int16_t vpn = target/64;
    int16_t offset = target%64;
    pageTable p = page[vpn];

    if (vpn < 0 || vpn >=16)
    {
        errHANDLER(EX_SEGMENTATION_FAULT);
         return -1;
    }
    if (!p.valid)
    {
        err_Handler(EX_PAGE_FAULT);
        return -1;
    }

    if (mode == 'w')
    {
        
        if (!p.writable)
        {
            errHANDLER(EX_SEGMENTATION_FAULT);
            return -1;
        }

        return physical_memory[p.frame_num][offset];
        
    }
    else if (mode == 'r')
    {
        if (!p.writable)
        {
            errHANDLER(EX_SEGMENTATION_FAULT);
            return -1;
        }

        physical_memory[p.frame_num][offset] = value;
        return 0;
    }
    
}
    

void errHANDLER(err_Handler ex){

    printf("Hardware mistake, exception code: %d\n",ex);
    
    if (ex == EX_DIV_ZERO)
    {
        printf("Error: Div-by-zero-fault\n");
    }
    else if (ex == EX_STACK_OVERFLOW)
    {
        printf("Error: Stack-Over-Flow\n");
    }
    else if (ex == EX_SEGMENTATION_FAULT)
    {   
        printf("Error: Segmentation fault\n");
    }
    else if (ex == EX_STACK_UNDERFLOW)
    {
        printf("Error: Stack-Under-Flow\n");
    }
    else if (ex == EX_INVALID_OPCODE)
    {
        printf("Error: Invalid machine code!\n");
    }
    
}

void start(){
    bool start = true;
    err_Handler ex;
    InstructionInfo* ins;
    #pragma warnings(disable:4996);
    while (start)
    {
        
        uint8_t instruction = memory[pc++];

        if (reverse_the_command(instruction,10,ins) == NULL)// if command doesnt exist in the dict.
        {
            errHANDLER(EX_INVALID_OPCODE);
            start = false;
            break;
        }
        if (pc < 0 || pc >= 4096)
        {
            errHANDLER(EX_SEGMENTATION_FAULT);
            start = false;
            break;
        }
                
        

        #ifdef debug_mode
            
            while (true)
            {
                printf("Debug Mode >>> komut : %i, program counter : %i , stack counter : %i callstackcounter : %i\n",reverse_the_command(instruction,10, Instruction*),pc,sc,csp);
                printf("Do you want to exit debug mode? (Y/N)")
                char eh;
                if (fgets(eh,sizeof(eh),stdin) == 'Y')
                {
                    #undef debug_mode
                    break;
                }
                
            }
            
        #endif
        
        if(sp >= STACK_SIZE){
            errHANDLER(EX_STACK_OVERFLOW);
            start = false;
        }
        if(instruction == OP_PUSH){
            // pushdan sonra yığına değer eklenir PUSH ,5 5 alınır.
            uint8_t value = memory[pc++]; // memory pc++ 5 olur memort pc-1 push olur
            stack[++sp] = value;
    
        }
        else if(instruction == OP_PRINT){
            // en üstteki değer 
            printf(">> %i\n",stack[sp]);
            
        }

        // basic ALU modules
        else if (instruction == OP_ADD){   
            int a = stack[sp--];
            int b = stack[sp--];

            stack[++sp] = b+a; // yine başa ekle azatlt

        }
        else if (instruction == OP_SUB && sp >= 1)
        {
            int a = stack[sp--];
            int b = stack[sp--];

            stack[++sp] = b-a;
        }
        else if (instruction == OP_CMP)
        {
            if (sp >= 1)
            {
                flags f;
                size_t a = (size_t)stack[sp--];
                size_t b = (size_t)stack[sp--];

                /*if (IS_STRING(a) && IS_STRING(b) && strcmp(a,b)) // elde ettiğimiz şeye göre aslında her fonksiyona yazılması gereken kontrol
                { continue; }*/
                if (a == b)
                {
                    f.Zero_Flag = EQUALS;
                }
                else if (a > b)
                {
                    f.Carry_Flag = EQUALS;

                }
                else if (b > a)
                {
                    f.Sign_Flag = EQUALS;
                }
                        
            }
            
        }
        else if (instruction == OP_DIV && sp >= 1)
        {
            int a = stack[sp--]; // son girilen sayi
            int b = stack[sp--]; // ilk girilen sayi
            if (a == NEQUALS)
            {
                errHANDLER(EX_DIV_ZERO);
                start = false;
            }
            
            stack[sp++] = b/a;
            
        }
        else if (instruction == OP_MUL && sp >= 1)
        {
            int a = stack[sp--];
            int b = stack[sp--];

            stack[++sp] = a*b;  
        }
        else if (instruction == OP_SUM_ALL)
        {
            if (sp >= 0) // for safety
            {
                int64_t sum = 0;
                for (uint16_t i = 0; i <= sp; i++)
                {
                    sum += stack[i];
                }
                sp = 0;
                stack[sp] = sum;
            }
            
        }

        //checking modules
        else if (instruction == OP_JMP)
        {
            uint16_t target = memory[pc++];
            
            if (target <= total_INSTRUCTION)
            {
                pc = target;
            }else if (target > total_INSTRUCTION){
                errHANDLER(EX_SEGMENTATION_FAULT);
            }
            else {continue;}
        }
        //jump equals gelecek..
        flags f;
        if (instruction == OP_JMPE)
        {
            uint16_t target = memory[pc++];
            
            if (f.Zero_Flag && target<=total_INSTRUCTION)
            {
                pc = target; // inidisi en sonki seçilmiş indise eşitledik
            }
            else if (target > total_INSTRUCTION)
            {
                errHANDLER(EX_SEGMENTATION_FAULT);
            }
            
            else{ continue; }            
        }
        
        else if (instruction == OP_JNE) // jump not equals
        {
            uint16_t target = memory[pc++];
            
            
            if (!f.Zero_Flag && target <= total_INSTRUCTION)
            {
                pc = target;

            }
            else if (target > total_INSTRUCTION)
            {
                errHANDLER(EX_SEGMENTATION_FAULT);
            }
            
            else{ continue; }
            

        }
        else if (instruction == OP_JZ)
        {
            uint16_t target = memory[pc++];

            if(f.Zero_Flag && target <= total_INSTRUCTION){ // sondaki elemansa 0 ise komut çalışır
                pc = target; // en sonki gitmek istenen adrese ışınlanırız.
            }
            else if (target > total_INSTRUCTION)
            {
                errHANDLER(EX_SEGMENTATION_FAULT);
            }
        }
        
        
        // load store
        else if (instruction == OP_STORE && sp >= 0)
        { // when used "Push" and "Store" it will push to the indicated point where storage is held
            
            uint8_t address = memory[pc++];

            int64_t value = stack[sp--];

            storage[address] = value;  
        }
        else if (instruction == OP_LOAD) 
        {
            uint8_t address = memory[pc++];

            stack[++sp] = storage[address];
                        
        }


        else if (instruction == OP_CALL)
        {
            size_t hedef = 0;
            hedef = memory[pc];   // call 50 mesela 50 olur hedef 
            call_stack[++csp] = pc; // 0'ıncı indis 51'e eşit olur

            pc = hedef; // hedef
        }
        else if (instruction == OP_RET)
        {
            pc = call_stack[csp--];// o sayının indisini verir yani geriye döneriz.
        }
        
        // stack manipulation

        else if (instruction == OP_DUP)
        {
            size_t duplicator = stack[sp++];
            stack[sp++] = duplicator;
        }
        else if (instruction == OP_POP || instruction == OP_DROP)
        {
            if (sp < 0)
            {
                err_Handler(EX_STACK_UNDERFLOW);
                start = false;
            }
            
            sp--;
        }
        else if (instruction == OP_SWAP)
        {
            size_t swap1 = stack[sp];
            size_t swap2 = stack[sp-1];

            stack[sp] = swap2;
            stack[sp-1] = swap1;
        }
        


    
        else if (instruction == OP_HALT)
        {
            start = false;
            printf(">>> program has been stopped.\n");
            break;
        }
        
        else{
            printf(">>> Unrecognized command in the user shell? [%u]",memory[pc-1]);
        }
        
        
    }
    
    
}
    
InstructionInfo lookup_table[] = {
    {"PUSH", OP_PUSH, true},
    {"STORE", OP_STORE, true},
    {"LOAD", OP_LOAD, true},
    {"JZ", OP_JZ, true},
    {"JNE", OP_JNE, true},
    {"ADD", OP_ADD, true},
    {"SUB", OP_SUB, true},
    {"CMP", OP_CMP, true},
    {"PRINT", OP_PRINT, true},
    {"HALT", OP_HALT, true}
};

int lookup_size = sizeof(lookup_table) / sizeof(lookup_table[0]);

Instruction* parse_assembly_file(const char *filename, int *out_count){

    
    FILE *f = fopen(filename,"r"); // or any text anything that it will need

    if (!f)
    {
        printf("the text couldnt open!");
        return;
    }
    
    char command[256];

    int capacity = 64;
    int count = 0;

    Instruction *instructions = malloc(capacity*sizeof(instructions));

    while (fgets(command,sizeof(command),f))
    {   
        char metin[32] = "";
        int64_t arg = 0;
        int parsed = sscanf(command,"%s %d",metin,&arg);

        if (parsed <= 0) return;
        
        Opcode op = 0x01;
        bool need = false;
        bool match = false;

        for (uint16_t i = 0; i < lookup_size; i++)
        {
            if (strcmp(metin,lookup_table[i].name) == NEQUALS) // eğer bulursan
            {
                op = lookup_table[i].op;
                need = lookup_table[i].argincld;
                match = true;
                break;
            }
            
        }
        if (!match)
        {
            continue;
        }

        if (count >= capacity)
        {   
            capacity*=2;
            instructions = realloc(instructions,capacity*sizeof(Instruction));

        }
        
        instructions[count].op = op;
        instructions[count].arg = need ? arg : 0;
        instructions[count].has_argument = need;
        count++;
    
    }
    fclose(f);
    *out_count = count;
    return instructions;
}

void copy_to_bin(const char *filename,Instruction *instructions, int count){

    FILE *f = fopen(filename,"wb");
    #pragma warnings(disable:4996)
    if (!f)
    {
        return;
    }
    

    for (int i = 0; i < count; i++)
    {
        uint8_t op = (uint8_t)instructions[i].op; // hangi komut olduğunu buluyoruz
        fwrite(&op,sizeof(uint8_t),1,f);

        if (instructions[i].has_argument)
        {
            fwrite(&instructions[i].arg, sizeof(int64_t), 1, f);
        }

    }
    fclose(f);    
    
}

char* reverse_the_command(const uint8_t *command, int count, InstructionInfo *instructions){



    for (uint8_t i = 0; i < count; i++)
    {
        if (instructions[i].op == command)
        {
            return instructions[i].name;
        }
        else continue; // komutu bulana kadar devam et
        
    }
    return NULL;
    
}

int main()
{

}

