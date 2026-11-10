#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#include <stdbool.h>
#include <stdint.h>

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

typedef struct
{
    bool Zero_Flag;
    bool Sign_Flag;
    bool Carry_Flag;

} flags;


#ifndef IS_STRING
#define IS_STRING(x) (__builtin_types_compatible_p(__typeof__(x), char*) || __builtin_types_compatible_p(__typeof__(x), const char*) || __builtin_types_compatible_p(__typeof__(x), char[]) || __builtin_types_compatible_p(__typeof__(x), const char[]))
#endif

#ifndef IS_INT
#define IS_INT(x) (__builtin_types_compatible_p(__typeof__(x), int)) || (__builtin_types_compatible_p(__typeof__(x), int*)) || (__builtin_types_compatible_p(__typeof__(x), int[])) || (__builtin_types_compatible_p(__typeof__(x), const int)) || (__builtin_types_compatible_p(__typeof__(x), const int[]))
#endif

#define MAX_MEM_SIZE 4096 // 4KB
#define STACK_SIZE 4096 

#define EQUALS 1
#define NEQUALS 0
#define BIGGER 2
#define SMALLER 3

uint8_t memory[MAX_MEM_SIZE];
int stack[STACK_SIZE];
int pc = 0; // program counter
int sp = -1; //stack counter
int storage[256];
int call_stack[256];
int csp = -1;



void start(){
    bool start = true;

    while (start)
    {
        uint8_t instruction = memory[pc++];

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
            pc = target;
        }
        //jump equals gelecek..
        flags f;
        if (instruction == OP_JMPE)
        {
            uint16_t target = memory[pc++];
            
            if (f.Zero_Flag)
            {
                pc = target; // inidisi en sonki seçilmiş indise eşitledik
            }
            else{ continue; }            
        }
        
        else if (instruction == OP_JNE) // jump not equals
        {
            uint16_t target = memory[pc++];
            
            
            if (!f.Zero_Flag)
            {
                pc = target;

            }else{ continue; }
            

        }
        else if (instruction == OP_JZ)
        {
            uint16_t target = memory[pc++];

            if(f.Zero_Flag){ // sondaki elemansa 0 ise komut çalışır
                pc = target; // en sonki gitmek istenen adrese ışınlanırız.
            }
            else // 0 değilse değiştirmeyiz.
            {
                continue;
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


int main()
{

}

