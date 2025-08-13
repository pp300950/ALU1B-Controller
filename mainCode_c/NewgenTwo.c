#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h> // --- อัปเกรด ---: เพิ่มเข้ามาเพื่อช่วยในการตัดช่องว่าง

#include <locale.h>

#define COM_PORT "COM3"
#define BAUD_RATE CBR_9600
#define UPLOAD_WAIT_MS 3000
#define READ_TIMEOUT_MS 1000
#define MAX_READ_BUFFER 256
#define NUM_BITS 32
#define MAX_MEMORY 50 // --- อัปเกรด ---: เพิ่มขนาดหน่วยความจำ
#define MAX_INSTRUCTIONS 256 // --- อัปเกรด ---: เพิ่มจำนวนคำสั่งสูงสุดที่สร้างได้
#define MAX_VARIABLES 50 // --- อัปเกรด ---: เพิ่มจำนวนตัวแปรสูงสุด
#define JUMP_STACK_SIZE 20 // --- อัปเกรด ---: ขนาดของ Stack สำหรับจัดการ if-else

HANDLE hSerial = INVALID_HANDLE_VALUE;

// --- Global Registers and Memory ---
long long REG_A = 0;
long long REG_B = 0;
long long MEMORY[MAX_MEMORY]; // --- อัปเกรด ---: ใช้ขนาดใหม่
bool CARRY_FLAG = false; 

// (ส่วนของฟังก์ชันพื้นฐาน เช่น decimalToBinary, openAndSetupSerialPort, ฯลฯ ยังคงเดิม...
//  ...เพื่อความกระชับ จึงขอข้ามไปแสดงส่วนที่อัปเกรด)

// --- โค้ดเดิม (ย่อ) ---
char *decimalToBinary(long long decimal){ char *binaryString = (char *)malloc(NUM_BITS + 1); if (!binaryString) return NULL; binaryString[NUM_BITS] = '\0'; long long value = decimal; for (int i = NUM_BITS - 1; i >= 0; i--) { binaryString[i] = (value & 1) ? '1' : '0'; value >>= 1; } return binaryString; }
long long binaryToDecimal(const char *binaryString){ long long decimalValue = 0; int length = strlen(binaryString); long long powerOfTwo = 1; for (int i = length - 1; i >= 0; i--) { if (binaryString[i] == '1') { decimalValue += powerOfTwo; } powerOfTwo *= 2; } return decimalValue; }
// ... (ฟังก์ชันอื่นๆ ที่ไม่เปลี่ยนแปลง)


// -----------------------------------------------------------
// คำสั่งจำลองภาษา Assembly
// -----------------------------------------------------------
typedef struct
{
    char instruction[10];
    char operand1[100]; // --- อัปเกรด ---: เพิ่มขนาดรองรับข้อความใน print
    char operand2[50];
    char label[20];
} Instruction;

// Global Flags
bool ZERO_FLAG = false;
bool SIGN_FLAG = false; 

long long getRegisterValue(const char *regName) { if (strcmp(regName, "REG_A") == 0) return REG_A; if (strcmp(regName, "REG_B") == 0) return REG_B; return atoll(regName); } // --- อัปเกรด ---: ให้รองรับค่าตัวเลขโดยตรง
void setRegisterValue(const char *regName, long long value) { if (strcmp(regName, "REG_A") == 0) REG_A = value; if (strcmp(regName, "REG_B") == 0) REG_B = value; }
long long getOperandValue(const char *operand) { if (strcmp(operand, "REG_A") == 0) return REG_A; if (strcmp(operand, "REG_B") == 0) return REG_B; return atoll(operand); }


void executeInstructions(Instruction *instructions, int numInstructions)
{
    // สร้างแผนที่ Label -> Index เพื่อเพิ่มความเร็วในการ Jump
    struct LabelMap { char label[20]; int index; };
    struct LabelMap labelMap[numInstructions];
    int labelCount = 0;

    // --- อัปเกรด ---: สร้าง Label Map ก่อนการทำงานจริง
    for (int i = 0; i < numInstructions; i++) {
        if (strlen(instructions[i].label) > 0) {
            if (labelCount < numInstructions) {
                strcpy(labelMap[labelCount].label, instructions[i].label);
                labelMap[labelCount].index = i;
                labelCount++;
            }
        }
    }

    int pc = 0; // Program Counter
    while (pc < numInstructions)
    {
        Instruction current = instructions[pc];
        bool shouldJump = false;

        // ไม่แสดงผล Label ว่าเป็นคำสั่ง
        if (strlen(current.instruction) == 0 && strlen(current.label) > 0) {
             printf("\n[PC:%02d] พบ Label: %s\n", pc, current.label);
             pc++;
             continue;
        }

        printf("\n[PC:%02d] กำลังรัน: %s", pc, current.instruction);
        if (strlen(current.operand1) > 0) printf(" %s", current.operand1);
        if (strlen(current.operand2) > 0) printf(", %s", current.operand2);
        printf("\n");

        if (strcmp(current.instruction, "DEF") == 0) {
            int mem_addr = atoi(current.operand1);
            long long value = atoll(current.operand2);
            if (mem_addr >= 0 && mem_addr < MAX_MEMORY) MEMORY[mem_addr] = value;
            printf("      [INFO] DEFINE: MEMORY[%d] = %lld\n", mem_addr, value);
        }
        else if (strcmp(current.instruction, "LOAD") == 0) {
            int mem_addr = atoi(current.operand2);
            if (mem_addr >= 0 && mem_addr < MAX_MEMORY) setRegisterValue(current.operand1, MEMORY[mem_addr]);
             printf("      [INFO] LOAD: %s = MEMORY[%d] (ค่า: %lld)\n", current.operand1, mem_addr, getRegisterValue(current.operand1));
        }
        else if (strcmp(current.instruction, "STORE") == 0) {
            int mem_addr = atoi(current.operand1);
            long long val = getRegisterValue(current.operand2);
            if (mem_addr >= 0 && mem_addr < MAX_MEMORY) MEMORY[mem_addr] = val;
            printf("      [INFO] STORE: MEMORY[%d] = %s (ค่า: %lld)\n", mem_addr, current.operand2, val);
        }
        else if (strcmp(current.instruction, "MOV") == 0) {
            long long value = getOperandValue(current.operand2);
            setRegisterValue(current.operand1, value);
            printf("      [INFO] MOV: %s = %lld\n", current.operand1, value);
        }
        // --- อัปเกรด: เพิ่มคำสั่งคณิตศาสตร์ ---
        else if (strcmp(current.instruction, "ADD") == 0) {
            long long val1 = getRegisterValue(current.operand1);
            long long val2 = getRegisterValue(current.operand2);
            setRegisterValue(current.operand1, val1 + val2);
            printf("      [INFO] ADD: %s = %lld + %lld (ผลลัพธ์: %lld)\n", current.operand1, val1, val2, getRegisterValue(current.operand1));
        }
         else if (strcmp(current.instruction, "SUB") == 0) {
            long long val1 = getRegisterValue(current.operand1);
            long long val2 = getRegisterValue(current.operand2);
            setRegisterValue(current.operand1, val1 - val2);
            printf("      [INFO] SUB: %s = %lld - %lld (ผลลัพธ์: %lld)\n", current.operand1, val1, val2, getRegisterValue(current.operand1));
        }
         else if (strcmp(current.instruction, "MUL") == 0) {
            long long val1 = getRegisterValue(current.operand1);
            long long val2 = getRegisterValue(current.operand2);
            setRegisterValue(current.operand1, val1 * val2);
            printf("      [INFO] MUL: %s = %lld * %lld (ผลลัพธ์: %lld)\n", current.operand1, val1, val2, getRegisterValue(current.operand1));
        }
         else if (strcmp(current.instruction, "DIV") == 0) {
            long long val1 = getRegisterValue(current.operand1);
            long long val2 = getRegisterValue(current.operand2);
            if(val2 == 0) {
                printf("      [ERROR] Division by zero!\n");
                break;
            }
            setRegisterValue(current.operand1, val1 / val2);
            printf("      [INFO] DIV: %s = %lld / %lld (ผลลัพธ์: %lld)\n", current.operand1, val1, val2, getRegisterValue(current.operand1));
        }
        else if (strcmp(current.instruction, "PRINT") == 0) {
            long long valueToPrint = getRegisterValue(current.operand2);
            printf("\n>>> [OUTPUT] %s %lld <<<\n", current.operand1, valueToPrint);
        }
        else if (strcmp(current.instruction, "CMP") == 0) {
            long long op1_val = getRegisterValue(current.operand1);
            long long op2_val = getRegisterValue(current.operand2);
            long long result = op1_val - op2_val;
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            CARRY_FLAG = (op1_val < op2_val); // สำหรับ <
            printf("      [INFO] CMP: %lld vs %lld. Z=%d, S=%d, C=%d\n", op1_val, op2_val, ZERO_FLAG, SIGN_FLAG, CARRY_FLAG);
        }
        else if (strcmp(current.instruction, "JMP") == 0 || strcmp(current.instruction, "JZ") == 0 || strcmp(current.instruction, "JNZ") == 0 || strcmp(current.instruction, "JS") == 0 || strcmp(current.instruction, "JNS") == 0 || strcmp(current.instruction, "JC") == 0 || strcmp(current.instruction, "JNC") == 0) {
            bool do_the_jump = false;
            if (strcmp(current.instruction, "JMP") == 0) do_the_jump = true;
            if (strcmp(current.instruction, "JZ") == 0 && ZERO_FLAG) do_the_jump = true;   // Jump if Equal
            if (strcmp(current.instruction, "JNZ") == 0 && !ZERO_FLAG) do_the_jump = true; // Jump if Not Equal
            if (strcmp(current.instruction, "JC") == 0 && CARRY_FLAG) do_the_jump = true;   // Jump if Less Than
            if (strcmp(current.instruction, "JNC") == 0 && !CARRY_FLAG && !ZERO_FLAG) do_the_jump = true; // Jump if Greater Than
            if (strcmp(current.instruction, "JNC") == 0 && !CARRY_FLAG) do_the_jump = true; // Jump if Greater or Equal
            
            if(do_the_jump){
                char *label = current.operand1;
                for (int i = 0; i < labelCount; i++) {
                    if (strcmp(labelMap[i].label, label) == 0) {
                        pc = labelMap[i].index;
                        shouldJump = true;
                        printf("      [INFO] JUMP to '%s' (PC -> %d)\n", label, pc);
                        break;
                    }
                }
            } else {
                 printf("      [INFO] JUMP condition false. No jump.\n");
            }
        }
        else if (strcmp(current.instruction, "HLT") == 0) {
            printf("      [INFO] HLT. Program terminated.\n");
            break;
        }

        if (!shouldJump) {
            pc++;
        }
    }
}


// --- อัปเกรด: ส่วนของคอมไพเลอร์ภาษาขั้นสูง (REBUILT) ---

// โครงสร้างสำหรับเก็บข้อมูลตัวแปร (Symbol Table)
typedef struct { char name[50]; int mem_addr; } Variable;

static int label_id_counter = 0; // ตัวนับสำหรับสร้าง Label ที่ไม่ซ้ำกัน
void generate_new_label(char* buffer) { sprintf(buffer, "L%d", label_id_counter++); }

// ฟังก์ชันสำหรับค้นหาตัวแปรใน Symbol Table
int findVariable(const char* name, Variable* table, int count) { for (int i = 0; i < count; i++) if (strcmp(name, table[i].name) == 0) return table[i].mem_addr; return -1; }

// ฟังก์ชันช่วย trim whitespace
char *trim(char *str) { char *end; while(isspace((unsigned char)*str)) str++; if(*str == 0) return str; end = str + strlen(str) - 1; while(end > str && isspace((unsigned char)*end)) end--; end[1] = '\0'; return str; }


Instruction* parseAndGenerateInstructions(const char** highLevelCode, int numLines, int* outNumInstructions) {
    Instruction* instructions = (Instruction*)malloc(sizeof(Instruction) * MAX_INSTRUCTIONS);
    Variable symbolTable[MAX_VARIABLES];
    int variableCount = 0;
    int nextMemAddr = 0;
    int instructionCount = 0;

    // Stack สำหรับจัดการการ Jump ของ if-else
    int jump_fix_stack[JUMP_STACK_SIZE];
    int stack_ptr = -1;

    printf("\n--- Compiler: Stage 1 - Parsing and Generating Instructions ---\n");

    for (int i = 0; i < numLines; i++) {
        char line[256];
        strcpy(line, highLevelCode[i]); // ทำสำเนาเพื่อแก้ไข
        char* trimmed_line = trim(line);

        if (strlen(trimmed_line) == 0) continue; // ข้ามบรรทัดว่าง

        char varName[50], rightHandSide[200], message[100];
        char lhs[50], op[10], rhs[50];

        // 1. Declaration: int A; or int A = 10;
        if (strncmp(trimmed_line, "int ", 4) == 0) {
            int value = 0; // Default to 0
            if (sscanf(trimmed_line, "int %s = %d", varName, &value) == 2 || sscanf(trimmed_line, "int %s", varName) == 1) {
                char* clean_var_name = strtok(varName, ";");
                printf("[Compiler] Found declaration: int %s = %d\n", clean_var_name, value);
                if (findVariable(clean_var_name, symbolTable, variableCount) != -1) { printf("ERROR: Variable '%s' already declared.\n", clean_var_name); free(instructions); return NULL; }

                strcpy(symbolTable[variableCount].name, clean_var_name);
                symbolTable[variableCount].mem_addr = nextMemAddr;
                variableCount++;

                // GEN: DEF <mem_addr> <value>
                sprintf(instructions[instructionCount].instruction, "DEF");
                sprintf(instructions[instructionCount].operand1, "%d", nextMemAddr);
                sprintf(instructions[instructionCount].operand2, "%d", value);
                strcpy(instructions[instructionCount].label, "");
                instructionCount++;
                nextMemAddr++;
            }
        }
        // 2. Assignment: A = B + C; or A = 10;
        else if (sscanf(trimmed_line, "%s = %[^\n]", varName, rightHandSide) == 2) {
             char* clean_var_name = trim(varName);
             char* clean_rhs = trim(strtok(rightHandSide, ";"));
             printf("[Compiler] Found assignment: %s = %s\n", clean_var_name, clean_rhs);

             int dest_addr = findVariable(clean_var_name, symbolTable, variableCount);
             if (dest_addr == -1) { printf("ERROR: Variable '%s' not declared.\n", clean_var_name); free(instructions); return NULL; }
            
             // ลองแยก RHS: operand1 op operand2
             if(sscanf(clean_rhs, "%s %s %s", lhs, op, rhs) == 3) {
                 // Case: A = B + C
                 char* clean_lhs = trim(lhs); char* clean_op = trim(op); char* clean_rhs_op = trim(rhs);
                 int addr1 = findVariable(clean_lhs, symbolTable, variableCount);
                 int addr2 = findVariable(clean_rhs_op, symbolTable, variableCount);

                 // GEN: Load/Move operand 1 to REG_A
                 if(addr1 != -1) { sprintf(instructions[instructionCount].instruction, "LOAD"); sprintf(instructions[instructionCount].operand1, "REG_A"); sprintf(instructions[instructionCount].operand2, "%d", addr1); } 
                 else { sprintf(instructions[instructionCount].instruction, "MOV"); sprintf(instructions[instructionCount].operand1, "REG_A"); sprintf(instructions[instructionCount].operand2, "%s", clean_lhs); }
                 instructionCount++;

                 // GEN: Load/Move operand 2 to REG_B
                 if(addr2 != -1) { sprintf(instructions[instructionCount].instruction, "LOAD"); sprintf(instructions[instructionCount].operand1, "REG_B"); sprintf(instructions[instructionCount].operand2, "%d", addr2); } 
                 else { sprintf(instructions[instructionCount].instruction, "MOV"); sprintf(instructions[instructionCount].operand1, "REG_B"); sprintf(instructions[instructionCount].operand2, "%s", clean_rhs_op); }
                 instructionCount++;

                 // GEN: The operation
                 if(strcmp(clean_op, "+") == 0) sprintf(instructions[instructionCount].instruction, "ADD");
                 else if(strcmp(clean_op, "-") == 0) sprintf(instructions[instructionCount].instruction, "SUB");
                 else if(strcmp(clean_op, "*") == 0) sprintf(instructions[instructionCount].instruction, "MUL");
                 else if(strcmp(clean_op, "/") == 0) sprintf(instructions[instructionCount].instruction, "DIV");
                 sprintf(instructions[instructionCount].operand1, "REG_A");
                 sprintf(instructions[instructionCount].operand2, "REG_B");
                 instructionCount++;
             } else {
                 // Case: A = B; or A = 10;
                 int src_addr = findVariable(clean_rhs, symbolTable, variableCount);
                 if (src_addr != -1) { sprintf(instructions[instructionCount].instruction, "LOAD"); sprintf(instructions[instructionCount].operand1, "REG_A"); sprintf(instructions[instructionCount].operand2, "%d", src_addr); }
                 else { sprintf(instructions[instructionCount].instruction, "MOV"); sprintf(instructions[instructionCount].operand1, "REG_A"); sprintf(instructions[instructionCount].operand2, "%s", clean_rhs); }
                 instructionCount++;
             }

             // GEN: Store result from REG_A back to variable
             sprintf(instructions[instructionCount].instruction, "STORE");
             sprintf(instructions[instructionCount].operand1, "%d", dest_addr);
             sprintf(instructions[instructionCount].operand2, "REG_A");
             instructionCount++;
        }
        // 3. Print: print("Message", A);
        else if (sscanf(trimmed_line, "print(\"%[^\"]\", %[^)]);", message, varName) == 2) {
             char* clean_var_name = trim(varName);
             printf("[Compiler] Found print: \"%s\", %s\n", message, clean_var_name);
             int mem_addr = findVariable(clean_var_name, symbolTable, variableCount);
             if (mem_addr == -1) { printf("ERROR: Variable '%s' not declared.\n", clean_var_name); free(instructions); return NULL; }
             
             // GEN: LOAD, PRINT
             sprintf(instructions[instructionCount].instruction, "LOAD"); sprintf(instructions[instructionCount].operand1, "REG_A"); sprintf(instructions[instructionCount].operand2, "%d", mem_addr); instructionCount++;
             sprintf(instructions[instructionCount].instruction, "PRINT"); sprintf(instructions[instructionCount].operand1, "%s", message); sprintf(instructions[instructionCount].operand2, "REG_A"); instructionCount++;
        }
        // 4. If statement: if (A == 10) {
        else if (sscanf(trimmed_line, "if (%s %s %s)", lhs, op, rhs) == 3) {
             char* clean_lhs = trim(lhs); char* clean_op = trim(op); char* clean_rhs_if = trim(strtok(rhs, ")"));
             printf("[Compiler] Found if: %s %s %s\n", clean_lhs, clean_op, clean_rhs_if);

             int addr1 = findVariable(clean_lhs, symbolTable, variableCount);
             int addr2 = findVariable(clean_rhs_if, symbolTable, variableCount);

             if(addr1 != -1) { sprintf(instructions[instructionCount].instruction, "LOAD"); sprintf(instructions[instructionCount].operand1, "REG_A"); sprintf(instructions[instructionCount].operand2, "%d", addr1); } 
             else { sprintf(instructions[instructionCount].instruction, "MOV"); sprintf(instructions[instructionCount].operand1, "REG_A"); sprintf(instructions[instructionCount].operand2, "%s", clean_lhs); }
             instructionCount++;

             if(addr2 != -1) { sprintf(instructions[instructionCount].instruction, "LOAD"); sprintf(instructions[instructionCount].operand1, "REG_B"); sprintf(instructions[instructionCount].operand2, "%d", addr2); } 
             else { sprintf(instructions[instructionCount].instruction, "MOV"); sprintf(instructions[instructionCount].operand1, "REG_B"); sprintf(instructions[instructionCount].operand2, "%s", clean_rhs_if); }
             instructionCount++;

             // GEN: CMP
             sprintf(instructions[instructionCount].instruction, "CMP"); sprintf(instructions[instructionCount].operand1, "REG_A"); sprintf(instructions[instructionCount].operand2, "REG_B"); instructionCount++;

             // GEN: Conditional Jump (jump if condition is FALSE)
             if(strcmp(clean_op, "==") == 0) sprintf(instructions[instructionCount].instruction, "JNZ"); // if not equal, jump over
             else if(strcmp(clean_op, "!=") == 0) sprintf(instructions[instructionCount].instruction, "JZ");  // if equal, jump over
             else if(strcmp(clean_op, "<") == 0) sprintf(instructions[instructionCount].instruction, "JNC"); // if not less (>=), jump
             else if(strcmp(clean_op, ">") == 0) sprintf(instructions[instructionCount].instruction, "JC"); // if not greater (<=), jump
             
             char label_buffer[20];
             generate_new_label(label_buffer); // e.g., "L0"
             sprintf(instructions[instructionCount].operand1, "%s", label_buffer);
             
             // Push the instruction index to the stack to be patched later
             jump_fix_stack[++stack_ptr] = instructionCount;
             instructionCount++;
        }
        // 5. Closing brace for if
        else if (strcmp(trimmed_line, "}") == 0) {
            printf("[Compiler] Found closing brace '}'\n");
            int jump_instruction_index = jump_fix_stack[stack_ptr--];
            char* label_to_set = instructions[jump_instruction_index].operand1;

            // GEN: The label for the jump to land on
            strcpy(instructions[instructionCount].label, label_to_set);
            strcpy(instructions[instructionCount].instruction, ""); // This is just a label, not an instruction
            instructionCount++;
        }
    }
    
    // GEN: HLT
    sprintf(instructions[instructionCount].instruction, "HLT"); instructionCount++;

    printf("--- Compiler: Stage 1 Finished. Generated %d instructions. ---\n", instructionCount);
    *outNumInstructions = instructionCount;
    return instructions;
}


// -----------------------------------------------------------
// Main
// -----------------------------------------------------------
int main()
{
    SetConsoleOutputCP(CP_UTF8);
    // ส่วนของการเชื่อมต่อ Arduino ถูกคอมเมนต์ไว้เพื่อทดสอบ Compiler
    
    printf("\n--- Advanced Language Simulator ---\n");

    // --- อัปเกรด: โปรแกรมทดสอบชุดใหญ่ ---
    const char* highLevelProgram[] = {
        "int A;",                   // A=0
        "int B = 10;",              // B=10
        "int C = 5;",               // C=5
        "print(\"Initial A is\", A);",
        "A = B - C;",               // A = 10 - 5 = 5
        "print(\"A after B-C is\", A);",
        "",                         // Test empty line
        "if (A == 5) {",
        "   print(\"IF_BLOCK: A is indeed 5!\", A);",
        "   A = A * 2;",             // A = 5 * 2 = 10
        "}",
        "print(\"A after IF is\", A);",
        "",
        "if (A > 20) {",
        "   print(\"This should NOT print\", A);",
        "}",
        "print(\"Program finished.\", A);"
    };
    int numHighLevelLines = sizeof(highLevelProgram) / sizeof(const char*);
    
    int numGeneratedInstructions = 0;
    Instruction* program = parseAndGenerateInstructions(highLevelProgram, numHighLevelLines, &numGeneratedInstructions);

    if (program != NULL) {
        executeInstructions(program, numGeneratedInstructions);
        free(program); 
    }

    printf("\n--- Simulation Complete ---\n");
    return 0;
}
