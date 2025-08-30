#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include <locale.h>

#define COM_PORT "COM3"
#define BAUD_RATE CBR_9600
#define UPLOAD_WAIT_MS 3000
#define READ_TIMEOUT_MS 1000
#define MAX_READ_BUFFER 256
#define NUM_BITS 32
#define MAX_MEMORY 50
#define MAX_INSTRUCTIONS 256
#define MAX_VARIABLES 50
#define JUMP_STACK_SIZE 20
#define STACK_SIZE 1024

// --- Global Handles and Flags ---
HANDLE hSerial = INVALID_HANDLE_VALUE;
bool ZERO_FLAG = false;
bool SIGN_FLAG = false;
bool CARRY_FLAG = false;

// --- Simulated CPU Components ---
long long REG_A = 0;
long long REG_B = 0;
long long MEMORY[MAX_MEMORY];
long long STACK[STACK_SIZE];
int sp = -1; // Stack Pointer

// --- Forward Declarations ---
typedef struct
{
    char instruction[10];
    char operand1[100];
    char operand2[50];
    char label[20];
} Instruction;

void clearSerialBuffer();
long long getOperandValue(const char *operand);
void setRegisterValue(const char *regName, long long value);

// ===================================================================================
//
// SECTION: Serial Communication & Arduino Interaction
//
// ===================================================================================

/**
 * @brief Sends a command string to the Arduino and receives a two-integer response.
 * @param dataToSend The command string, must end with '\n'.
 * @param resultOutput Pointer to store the first integer from the response.
 * @param carryOutput Pointer to store the second integer from the response.
 * @return true on success, false on failure.
 */
bool sendAndReceiveData(const char *dataToSend, int *resultOutput, int *carryOutput)
{
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        printf("[ERROR] Invalid serial handle.\n");
        return false;
    }

    DWORD bytesWritten = 0;
    DWORD bytesRead = 0;
    char readBuffer[MAX_READ_BUFFER] = {0};

    // Ensure command is properly terminated
    size_t len = strlen(dataToSend);
    if (len == 0 || dataToSend[len - 1] != '\n')
    {
        printf("[ERROR] Data to send must end with a newline character (\\n).\n");
        return false;
    }

    clearSerialBuffer(); // Clear buffers before new transaction

    // Write data to serial port
    if (!WriteFile(hSerial, dataToSend, (DWORD)len, &bytesWritten, NULL))
    {
        printf("[ERROR] WriteFile failed. Error code: %lu\n", GetLastError());
        return false;
    }
    printf("[SERIAL TX] -> %s", dataToSend);

    // Wait for response from Arduino
    Sleep(5);

    // Read response from serial port
    if (ReadFile(hSerial, readBuffer, MAX_READ_BUFFER - 1, &bytesRead, NULL) && bytesRead > 0)
    {
        readBuffer[bytesRead] = '\0';

        // Trim newline characters from the end of the buffer
        for (int i = (int)bytesRead - 1; i >= 0; --i)
        {
            if (readBuffer[i] == '\n' || readBuffer[i] == '\r')
                readBuffer[i] = '\0';
            else
                break;
        }

        printf("[SERIAL RX] <- %s\n", readBuffer);
        if (sscanf(readBuffer, "%d %d", resultOutput, carryOutput) == 2)
        {
            return true;
        }
        else
        {
            printf("[ERROR] Invalid data format received: \"%s\"\n", readBuffer);
            return false;
        }
    }
    else
    {
        printf("[ERROR] No data received or read timed out.\n");
        return false;
    }
}

/**
 * @brief **CORE FUNCTION** Performs a bitwise hardware operation using the Arduino ALU.
 * This function iterates through each bit of the operands, sends them to the Arduino for processing,
 * and reconstructs the final result.
 *
 * @param op1 The first operand (e.g., REG_A's value).
 * @param op2 The second operand.
 * @param muxCode The 3-bit MUX code string (e.g., "001" for ADD/SUB, "100" for AND).
 * @param subAddFlag The flag for the SUB/ADD pin (0 for ADD/Logic, 1 for SUB).
 * @param final_carry Pointer to store the final carry out of the operation.
 * @return The final calculated result as a long long.
 */
long long executeAluOperation(long long op1, long long op2, const char *muxCode, int subAddFlag, bool *final_carry)
{
    long long result = 0;
    int carry_in = (subAddFlag == 1) ? 1 : 0; // For subtraction, initial carry-in is 1

    for (int i = 0; i < NUM_BITS; i++)
    {
        int bitA = (op1 >> i) & 1;
        int bitB = (op2 >> i) & 1;

        // For NOT operation (mux 111), the Arduino sketch uses input A for the value to be inverted.
        if (strcmp(muxCode, "111") == 0)
        {
            bitA = bitB; // Move B into A's slot for the hardware
        }

        char command[32];
        snprintf(command, sizeof(command), "%s %d %d %d\n", muxCode, subAddFlag, bitA, bitB);

        int alu_result_sum = 0, alu_carry_sum = 0;
        if (!sendAndReceiveData(command, &alu_result_sum, &alu_carry_sum))
        {
            printf("[FATAL] Hardware communication failed at bit %d. Aborting operation.\n", i);
            return 0; // Return 0 on failure
        }

        // Full adder logic implemented in software using two half-adder results from hardware
        int final_sum_bit = alu_result_sum ^ carry_in;
        int carry_out = alu_carry_sum | (alu_result_sum & carry_in);

        // Set the corresponding bit in the result
        if (final_sum_bit)
        {
            result |= (1LL << i);
        }

        carry_in = carry_out;
    }

    *final_carry = (carry_in == 1);
    return result;
}

// ===================================================================================
//
// SECTION: Assembly Language Simulation
//
// ===================================================================================

// Helper to get value from register name or literal
long long getOperandValue(const char *operand)
{
    if (strcmp(operand, "REG_A") == 0)
        return REG_A;
    if (strcmp(operand, "REG_B") == 0)
        return REG_B;
    if (strncmp(operand, "MEM[", 4) == 0)
    {
        int addr = atoi(operand + 4);
        if (addr >= 0 && addr < MAX_MEMORY)
            return MEMORY[addr];
        return 0; // Return 0 if address is invalid
    }
    return atoll(operand);
}

// Helper to set a register's value by name
void setRegisterValue(const char *regName, long long value)
{
    if (strcmp(regName, "REG_A") == 0)
        REG_A = value;
    if (strcmp(regName, "REG_B") == 0)
        REG_B = value;
}

void executeInstructions(Instruction *instructions, int numInstructions)
{
    // Build a label map for faster jumps
    struct LabelMap
    {
        char label[20];
        int index;
    };
    struct LabelMap labelMap[MAX_INSTRUCTIONS];
    int labelCount = 0;
    for (int i = 0; i < numInstructions; i++)
    {
        if (strlen(instructions[i].label) > 0)
        {
            if (labelCount < MAX_INSTRUCTIONS)
            {
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

        // Skip blank lines that are just labels
        if (strlen(current.instruction) == 0 && strlen(current.label) > 0)
        {
            pc++;
            continue;
        }

        printf("\n[PC:%02d] Executing: %s", pc, current.instruction);
        if (strlen(current.operand1) > 0)
            printf(" %s", current.operand1);
        if (strlen(current.operand2) > 0)
            printf(", %s", current.operand2);
        printf("\n");

        // --- Data and Memory Operations ---
        if (strcmp(current.instruction, "DEF") == 0)
        {
            int mem_addr = atoi(current.operand1);
            long long value = atoll(current.operand2);
            if (mem_addr >= 0 && mem_addr < MAX_MEMORY)
                MEMORY[mem_addr] = value;
            printf("      [INFO] DEFINE: MEMORY[%d] = %lld\n", mem_addr, value);
        }
        else if (strcmp(current.instruction, "LOAD") == 0)
        {
            int mem_addr = atoi(current.operand2);
            if (mem_addr >= 0 && mem_addr < MAX_MEMORY)
            {
                setRegisterValue(current.operand1, MEMORY[mem_addr]);
                printf("      [INFO] LOAD: %s = MEMORY[%d] (Value: %lld)\n", current.operand1, mem_addr, getOperandValue(current.operand1));
            }
        }
        else if (strcmp(current.instruction, "STORE") == 0)
        {
            int mem_addr = atoi(current.operand1);
            long long val = getOperandValue(current.operand2);
            if (mem_addr >= 0 && mem_addr < MAX_MEMORY)
                MEMORY[mem_addr] = val;
            printf("      [INFO] STORE: MEMORY[%d] = %s (Value: %lld)\n", mem_addr, current.operand2, val);
        }
        else if (strcmp(current.instruction, "MOV") == 0)
        {
            long long value = getOperandValue(current.operand2);
            setRegisterValue(current.operand1, value);
            printf("      [INFO] MOV: %s = %lld\n", current.operand1, value);
        }

        // --- Hardware ALU Arithmetic Operations ---
        else if (strcmp(current.instruction, "ADD") == 0)
        {
            long long val1 = getOperandValue(current.operand1);
            long long val2 = getOperandValue(current.operand2);
            long long result = executeAluOperation(val1, val2, "001", 0, &CARRY_FLAG);
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            printf("      [INFO] HW_ADD: %s = %lld + %lld -> %lld. Flags: Z=%d S=%d C=%d\n", current.operand1, val1, val2, result, ZERO_FLAG, SIGN_FLAG, CARRY_FLAG);
        }
        else if (strcmp(current.instruction, "SUB") == 0)
        {
            long long val1 = getOperandValue(current.operand1);
            long long val2 = getOperandValue(current.operand2);
            long long result = executeAluOperation(val1, val2, "001", 1, &CARRY_FLAG);
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            printf("      [INFO] HW_SUB: %s = %lld - %lld -> %lld. Flags: Z=%d S=%d C=%d\n", current.operand1, val1, val2, result, ZERO_FLAG, SIGN_FLAG, CARRY_FLAG);
        }

        // --- Hardware ALU Logic Operations ---
        else if (strcmp(current.instruction, "AND") == 0)
        {
            long long val1 = getOperandValue(current.operand1);
            long long val2 = getOperandValue(current.operand2);
            long long result = executeAluOperation(val1, val2, "100", 0, &CARRY_FLAG); // Assuming MUX 100 is AND
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            printf("      [INFO] HW_AND: %s = %lld & %lld -> %lld. Flags: Z=%d S=%d\n", current.operand1, val1, val2, result, ZERO_FLAG, SIGN_FLAG);
        }
        else if (strcmp(current.instruction, "OR") == 0)
        {
            long long val1 = getOperandValue(current.operand1);
            long long val2 = getOperandValue(current.operand2);
            long long result = executeAluOperation(val1, val2, "101", 0, &CARRY_FLAG); // Assuming MUX 101 is OR
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            printf("      [INFO] HW_OR: %s = %lld | %lld -> %lld. Flags: Z=%d S=%d\n", current.operand1, val1, val2, result, ZERO_FLAG, SIGN_FLAG);
        }
        else if (strcmp(current.instruction, "XOR") == 0)
        {
            long long val1 = getOperandValue(current.operand1);
            long long val2 = getOperandValue(current.operand2);
            long long result = executeAluOperation(val1, val2, "110", 0, &CARRY_FLAG); // Assuming MUX 110 is XOR
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            printf("      [INFO] HW_XOR: %s = %lld ^ %lld -> %lld. Flags: Z=%d S=%d\n", current.operand1, val1, val2, result, ZERO_FLAG, SIGN_FLAG);
        }
        else if (strcmp(current.instruction, "NOT") == 0)
        {
            long long val = getOperandValue(current.operand1);
            long long result = executeAluOperation(0, val, "111", 0, &CARRY_FLAG); // MUX 111 is NOT, op1 is ignored
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            printf("      [INFO] HW_NOT: %s = ~%lld -> %lld. Flags: Z=%d S=%d\n", current.operand1, val, result, ZERO_FLAG, SIGN_FLAG);
        }

        // --- PC-Simulated Complex Operations ---
        else if (strcmp(current.instruction, "MUL") == 0)
        {
            printf("      [INFO] กำลังประมวลผล MUL ด้วยหลักการ Shift-and-Add บน ALU...\n");
            long long val1 = getOperandValue(current.operand1); // ตัวตั้งคูณ
            long long val2 = getOperandValue(current.operand2); // ตัวคูณ

            long long result = 0;

            // วนลูปตามจำนวนบิต (long long คือ 64 บิต)
            for (int i = 0; i < 64; i++)
            {
                // ตรวจสอบบิตที่ i ของตัวคูณ (val2)
                if ((val2 >> i) & 1)
                {
                    // ถ้าบิตเป็น 1, ให้บวกตัวตั้งคูณ (val1) ที่ Shift ไป i ครั้งเข้ากับผลลัพธ์
                    // ใช้ ALU ในการบวก
                    result = executeAluOperation(result, val1 << i, "001", 0, &CARRY_FLAG);
                }
                printf("      [STEP] i=%d, val2 bit=%lld, Current result=%lld\n", i, (val2 >> i) & 1, result);
            }

            // ตั้งค่า register ด้วยผลลัพธ์สุดท้าย
            setRegisterValue(current.operand1, result);

            // ตั้งค่า Flags ตามผลลัพธ์สุดท้าย
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            CARRY_FLAG = false; // Carry flag มักจะถูกรีเซ็ตหลังการคูณ

            printf("      [INFO] MUL: %s = %lld * %lld -> ผลลัพธ์ %lld\n", current.operand1, val1, val2, result);
            printf("      [INFO] Flags: Z=%d, S=%d, C=%d\n", ZERO_FLAG, SIGN_FLAG, CARRY_FLAG);
        }
        else if (strcmp(current.instruction, "DIV") == 0)
        {
            printf("      [INFO] กำลังประมวลผล DIV ด้วยหลักการ Shift-and-Subtract บน ALU...\n");
            long long val1 = getOperandValue(current.operand1);
            long long val2 = getOperandValue(current.operand2);
            if (val2 == 0)
            {
                printf("      [ERROR] Division by zero!\n");
                break;
            }

            long long remainder = 0;
            long long quotient = 0;

            for (int i = 63; i >= 0; i--)
            {
                // Shift remainder to the left by 1 bit
                remainder = (remainder << 1) | ((val1 >> i) & 1);

                // Use ALU to compare remainder with divisor (val2)
                // CMP is essentially a subtraction (remainder - val2)
                long long cmp_result = executeAluOperation(remainder, val2, "001", 1, &CARRY_FLAG);

                printf("      [STEP] i=%d, Current Remainder=%lld, Divisor=%lld, CARRY_FLAG=%d\n", i, remainder, val2, CARRY_FLAG);

                // If remainder is greater than or equal to divisor (no borrow)
                if (!CARRY_FLAG)
                {
                    // Use ALU to subtract
                    remainder = cmp_result;
                    // Use ALU to set the quotient bit
                    quotient = executeAluOperation(quotient, 1LL << i, "001", 0, &CARRY_FLAG);
                }
            }

            // Set the register with the final quotient
            setRegisterValue(current.operand1, quotient);

            // Set flags based on the final result
            ZERO_FLAG = (quotient == 0);
            SIGN_FLAG = (quotient < 0);
            CARRY_FLAG = false; // Carry flag is usually reset after division

            printf("      [INFO] DIV: %s = %lld / %lld -> ผลลัพธ์ %lld, เศษ %lld\n", current.operand1, val1, val2, quotient, remainder);
            printf("      [INFO] Flags: Z=%d, S=%d, C=%d\n", ZERO_FLAG, SIGN_FLAG, CARRY_FLAG);
        }

        // --- Control Flow and I/O ---
        else if (strcmp(current.instruction, "PRINT") == 0)
        {
            long long valueToPrint = getOperandValue(current.operand2);
            printf("\n>>> [OUTPUT] %s %lld <<<\n", current.operand1, valueToPrint);
        }
        else if (strcmp(current.instruction, "CMP") == 0)
        {
            long long op1_val = getOperandValue(current.operand1);
            long long op2_val = getOperandValue(current.operand2);

            // ใช้ ALU ในการลบแทนการใช้โอเปอเรเตอร์ '-' ของภาษา C
            // executeAluOperation ในโหมด SUB ("001", 1) จะทำการลบและตั้งค่า CARRY_FLAG
            long long result = executeAluOperation(op1_val, op2_val, "001", 1, &CARRY_FLAG);

            // ตั้งค่า ZERO_FLAG และ SIGN_FLAG ตามผลลัพธ์ที่ได้จาก ALU
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);

            // CARRY_FLAG ถูกตั้งค่าโดย executeAluOperation อยู่แล้ว

            printf("      [INFO] CMP: %lld vs %lld. ผลลัพธ์จากการลบ=%lld. Flags: Z=%d, S=%d, C=%d\n", op1_val, op2_val, result, ZERO_FLAG, SIGN_FLAG, CARRY_FLAG);
        }
        else if (strcmp(current.instruction, "JMP") == 0 || strcmp(current.instruction, "JZ") == 0 || strcmp(current.instruction, "JNZ") == 0 || strcmp(current.instruction, "JS") == 0 || strcmp(current.instruction, "JNS") == 0 || strcmp(current.instruction, "JC") == 0 || strcmp(current.instruction, "JNC") == 0)
        {
            bool do_the_jump =
                (strcmp(current.instruction, "JMP") == 0) ||
                (strcmp(current.instruction, "JZ") == 0 && ZERO_FLAG) ||   // Jump if Zero (Equal)
                (strcmp(current.instruction, "JNZ") == 0 && !ZERO_FLAG) || // Jump if Not Zero (Not Equal)
                (strcmp(current.instruction, "JS") == 0 && SIGN_FLAG) ||   // Jump if Sign (Negative)
                (strcmp(current.instruction, "JNS") == 0 && !SIGN_FLAG) || // Jump if Not Sign (Positive)
                (strcmp(current.instruction, "JC") == 0 && CARRY_FLAG) ||  // Jump if Carry (Less than)
                (strcmp(current.instruction, "JNC") == 0 && !CARRY_FLAG);  // Jump if No Carry (Greater or Equal)

            if (do_the_jump)
            {
                for (int i = 0; i < labelCount; i++)
                {
                    if (strcmp(labelMap[i].label, current.operand1) == 0)
                    {
                        pc = labelMap[i].index;
                        shouldJump = true;
                        printf("      [INFO] JUMP to '%s' (PC -> %d)\n", current.operand1, pc);
                        break;
                    }
                }
            }
            else
            {
                printf("      [INFO] JUMP condition false. No jump.\n");
            }
        }
        else if (strcmp(current.instruction, "HLT") == 0)
        {
            printf("      [INFO] HLT. Program terminated.\n");
            break;
        }

        if (!shouldJump)
        {
            pc++;
        }
    }
}

// ===================================================================================
//
// SECTION: High-Level Language Compiler
//
// ===================================================================================

typedef struct
{
    char name[50];
    int mem_addr;
} Variable;
static int label_id_counter = 0;
void generate_new_label(char *buffer) { sprintf(buffer, "L%d", label_id_counter++); }
int findVariable(const char *name, Variable *table, int count)
{
    for (int i = 0; i < count; i++)
        if (strcmp(name, table[i].name) == 0)
            return table[i].mem_addr;
    return -1;
}
char *trim(char *str)
{
    char *end;
    while (isspace((unsigned char)*str))
        str++;
    if (*str == 0)
        return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;
    end[1] = '\0';
    return str;
}

Instruction *parseAndGenerateInstructions(const char **highLevelCode, int numLines, int *outNumInstructions)
{
    Instruction *instructions = (Instruction *)malloc(sizeof(Instruction) * MAX_INSTRUCTIONS);
    Variable symbolTable[MAX_VARIABLES];
    int variableCount = 0;
    int nextMemAddr = 0;
    int instructionCount = 0;

    int jump_fix_stack[JUMP_STACK_SIZE];
    int stack_ptr = -1;

    printf("\n--- Compiler: Stage 1 - Parsing and Generating Instructions ---\n");

    for (int i = 0; i < numLines; i++)
    {
        char line[256];
        strcpy(line, highLevelCode[i]);
        char *trimmed_line = trim(line);

        if (strlen(trimmed_line) == 0)
            continue;

        char varName[50], rightHandSide[200], message[100];
        char lhs[50], op[10], rhs[50];

        // 1. Declaration: int A; or int A = 10;
        if (strncmp(trimmed_line, "int ", 4) == 0)
        {
            int value = 0;
            if (sscanf(trimmed_line, "int %s = %d", varName, &value) == 2 || sscanf(trimmed_line, "int %s", varName) == 1)
            {
                char *clean_var_name = strtok(varName, ";");
                if (findVariable(clean_var_name, symbolTable, variableCount) != -1)
                {
                    printf("ERROR: Variable '%s' already declared.\n", clean_var_name);
                    free(instructions);
                    return NULL;
                }

                strcpy(symbolTable[variableCount].name, clean_var_name);
                symbolTable[variableCount].mem_addr = nextMemAddr;
                variableCount++;

                sprintf(instructions[instructionCount].instruction, "DEF");
                sprintf(instructions[instructionCount].operand1, "%d", nextMemAddr);
                sprintf(instructions[instructionCount].operand2, "%d", value);
                instructionCount++;
                nextMemAddr++;
            }
        }
        // 2. Assignment: A = B + C; or A = 10;
        else if (sscanf(trimmed_line, "%s = %[^\n]", varName, rightHandSide) == 2)
        {
            char *clean_var_name = trim(varName);
            char *clean_rhs = trim(strtok(rightHandSide, ";"));
            int dest_addr = findVariable(clean_var_name, symbolTable, variableCount);
            if (dest_addr == -1)
            {
                printf("ERROR: Variable '%s' not declared.\n", clean_var_name);
                free(instructions);
                return NULL;
            }

            if (sscanf(clean_rhs, "%s %s %s", lhs, op, rhs) == 3)
            {
                char *clean_lhs = trim(lhs);
                char *clean_op = trim(op);
                char *clean_rhs_op = trim(rhs);
                int addr1 = findVariable(clean_lhs, symbolTable, variableCount);
                int addr2 = findVariable(clean_rhs_op, symbolTable, variableCount);

                if (addr1 != -1)
                {
                    sprintf(instructions[instructionCount].instruction, "LOAD");
                    sprintf(instructions[instructionCount].operand1, "REG_A");
                    sprintf(instructions[instructionCount].operand2, "%d", addr1);
                }
                else
                {
                    sprintf(instructions[instructionCount].instruction, "MOV");
                    sprintf(instructions[instructionCount].operand1, "REG_A");
                    sprintf(instructions[instructionCount].operand2, "%s", clean_lhs);
                }
                instructionCount++;

                if (addr2 != -1)
                {
                    sprintf(instructions[instructionCount].instruction, "LOAD");
                    sprintf(instructions[instructionCount].operand1, "REG_B");
                    sprintf(instructions[instructionCount].operand2, "%d", addr2);
                }
                else
                {
                    sprintf(instructions[instructionCount].instruction, "MOV");
                    sprintf(instructions[instructionCount].operand1, "REG_B");
                    sprintf(instructions[instructionCount].operand2, "%s", clean_rhs_op);
                }
                instructionCount++;

                if (strcmp(clean_op, "+") == 0)
                    sprintf(instructions[instructionCount].instruction, "ADD");
                else if (strcmp(clean_op, "-") == 0)
                    sprintf(instructions[instructionCount].instruction, "SUB");
                else if (strcmp(clean_op, "*") == 0)
                    sprintf(instructions[instructionCount].instruction, "MUL");
                else if (strcmp(clean_op, "/") == 0)
                    sprintf(instructions[instructionCount].instruction, "DIV");
                sprintf(instructions[instructionCount].operand1, "REG_A");
                sprintf(instructions[instructionCount].operand2, "REG_B");
                instructionCount++;
            }
            else
            {
                int src_addr = findVariable(clean_rhs, symbolTable, variableCount);
                if (src_addr != -1)
                {
                    sprintf(instructions[instructionCount].instruction, "LOAD");
                    sprintf(instructions[instructionCount].operand1, "REG_A");
                    sprintf(instructions[instructionCount].operand2, "%d", src_addr);
                }
                else
                {
                    sprintf(instructions[instructionCount].instruction, "MOV");
                    sprintf(instructions[instructionCount].operand1, "REG_A");
                    sprintf(instructions[instructionCount].operand2, "%s", clean_rhs);
                }
                instructionCount++;
            }
            sprintf(instructions[instructionCount].instruction, "STORE");
            sprintf(instructions[instructionCount].operand1, "%d", dest_addr);
            sprintf(instructions[instructionCount].operand2, "REG_A");
            instructionCount++;
        }
        // 3. Print: print("Message", A);
        else if (sscanf(trimmed_line, "print(\"%[^\"]\", %[^)]);", message, varName) == 2)
        {
            char *clean_var_name = trim(varName);
            int mem_addr = findVariable(clean_var_name, symbolTable, variableCount);
            if (mem_addr == -1)
            {
                printf("ERROR: Variable '%s' not declared.\n", clean_var_name);
                free(instructions);
                return NULL;
            }

            sprintf(instructions[instructionCount].instruction, "LOAD");
            sprintf(instructions[instructionCount].operand1, "REG_A");
            sprintf(instructions[instructionCount].operand2, "%d", mem_addr);
            instructionCount++;
            sprintf(instructions[instructionCount].instruction, "PRINT");
            sprintf(instructions[instructionCount].operand1, "%s", message);
            sprintf(instructions[instructionCount].operand2, "REG_A");
            instructionCount++;
        }
        // 4. If statement: if (A == 10) {
        else if (sscanf(trimmed_line, "if (%s %s %s)", lhs, op, rhs) == 3)
        {
            char *clean_lhs = trim(lhs);
            char *clean_op = trim(op);
            char *clean_rhs_if = trim(strtok(rhs, ")"));
            int addr1 = findVariable(clean_lhs, symbolTable, variableCount);
            int addr2 = findVariable(clean_rhs_if, symbolTable, variableCount);

            if (addr1 != -1)
            {
                sprintf(instructions[instructionCount].instruction, "LOAD");
                sprintf(instructions[instructionCount].operand1, "REG_A");
                sprintf(instructions[instructionCount].operand2, "%d", addr1);
            }
            else
            {
                sprintf(instructions[instructionCount].instruction, "MOV");
                sprintf(instructions[instructionCount].operand1, "REG_A");
                sprintf(instructions[instructionCount].operand2, "%s", clean_lhs);
            }
            instructionCount++;

            if (addr2 != -1)
            {
                sprintf(instructions[instructionCount].instruction, "LOAD");
                sprintf(instructions[instructionCount].operand1, "REG_B");
                sprintf(instructions[instructionCount].operand2, "%d", addr2);
            }
            else
            {
                sprintf(instructions[instructionCount].instruction, "MOV");
                sprintf(instructions[instructionCount].operand1, "REG_B");
                sprintf(instructions[instructionCount].operand2, "%s", clean_rhs_if);
            }
            instructionCount++;

            sprintf(instructions[instructionCount].instruction, "CMP");
            sprintf(instructions[instructionCount].operand1, "REG_A");
            sprintf(instructions[instructionCount].operand2, "REG_B");
            instructionCount++;

            // หลัง CMP แล้ว
            char label_buffer[20];
            generate_new_label(label_buffer);

            // Mapping ใหม่สำหรับ unsigned compare
            if (strcmp(clean_op, "==") == 0)
            {
                sprintf(instructions[instructionCount].instruction, "JNZ");
                sprintf(instructions[instructionCount].operand1, "%s", label_buffer);
                jump_fix_stack[++stack_ptr] = instructionCount;
                instructionCount++;
            }
            else if (strcmp(clean_op, "!=") == 0)
            {
                sprintf(instructions[instructionCount].instruction, "JZ");
                sprintf(instructions[instructionCount].operand1, "%s", label_buffer);
                jump_fix_stack[++stack_ptr] = instructionCount;
                instructionCount++;
            }
            else if (strcmp(clean_op, "<") == 0)
            {
                sprintf(instructions[instructionCount].instruction, "JC");
                sprintf(instructions[instructionCount].operand1, "%s", label_buffer);
                jump_fix_stack[++stack_ptr] = instructionCount;
                instructionCount++;
            }
            else if (strcmp(clean_op, ">=") == 0)
            {
                sprintf(instructions[instructionCount].instruction, "JNC");
                sprintf(instructions[instructionCount].operand1, "%s", label_buffer);
                jump_fix_stack[++stack_ptr] = instructionCount;
                instructionCount++;
            }
            else if (strcmp(clean_op, ">") == 0)
            {
                // not > → CARRY == 0 || ZERO == 1
                sprintf(instructions[instructionCount].instruction, "JNC");
                sprintf(instructions[instructionCount].operand1, "%s", label_buffer);
                jump_fix_stack[++stack_ptr] = instructionCount;
                instructionCount++;

                sprintf(instructions[instructionCount].instruction, "JZ");
                sprintf(instructions[instructionCount].operand1, "%s", label_buffer);
                jump_fix_stack[++stack_ptr] = instructionCount;
                instructionCount++;
            }
            else if (strcmp(clean_op, "<=") == 0)
            {
                // not <= → CARRY == 1 && ZERO == 0
                sprintf(instructions[instructionCount].instruction, "JC");
                sprintf(instructions[instructionCount].operand1, "%s", label_buffer);
                jump_fix_stack[++stack_ptr] = instructionCount;
                instructionCount++;

                sprintf(instructions[instructionCount].instruction, "JNZ");
                sprintf(instructions[instructionCount].operand1, "%s", label_buffer);
                jump_fix_stack[++stack_ptr] = instructionCount;
                instructionCount++;
            }

        }
        // 5. Closing brace for if
        else if (strcmp(trimmed_line, "}") == 0)
        {
            if (stack_ptr >= 0)
            {
                int jump_instruction_index = jump_fix_stack[stack_ptr--];
                char *label_to_set = instructions[jump_instruction_index].operand1;
                strcpy(instructions[instructionCount].label, label_to_set);
                strcpy(instructions[instructionCount].instruction, "");
                instructionCount++;
            }
        }
    }

    sprintf(instructions[instructionCount].instruction, "HLT");
    instructionCount++;
    *outNumInstructions = instructionCount;
    return instructions;
}

// ===================================================================================
//
// SECTION: Setup and Main Execution
//
// ===================================================================================
void clearSerialBuffer()
{
    if (hSerial != INVALID_HANDLE_VALUE)
    {
        PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
    }
}

BOOL WINAPI ConsoleHandler(DWORD dwCtrlType)
{
    if (dwCtrlType == CTRL_C_EVENT)
    {
        printf("\n[INFO] Ctrl+C detected. Shutting down.\n");
        if (hSerial != INVALID_HANDLE_VALUE)
        {
            clearSerialBuffer();
            CloseHandle(hSerial);
        }
        ExitProcess(0);
        return TRUE;
    }
    return FALSE;
}

HANDLE openAndSetupSerialPort()
{
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts = {0};

    printf("[DEBUG] Opening serial port: %s\n", COM_PORT);
    hSerial = CreateFileA(COM_PORT, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hSerial == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND)
            printf("[ERROR] Serial port %s not found.\n", COM_PORT);
        else if (err == ERROR_ACCESS_DENIED)
            printf("[ERROR] Serial port %s is in use by another program.\n", COM_PORT);
        else
            printf("[ERROR] Failed to open serial port %s. Error code: %lu\n", COM_PORT, err);
        return INVALID_HANDLE_VALUE;
    }

    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        printf("[ERROR] GetCommState failed.\n");
        goto cleanup;
    }
    dcbSerialParams.BaudRate = BAUD_RATE;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if (!SetCommState(hSerial, &dcbSerialParams))
    {
        printf("[ERROR] SetCommState failed.\n");
        goto cleanup;
    }

    timeouts.ReadIntervalTimeout = 100;
    timeouts.ReadTotalTimeoutConstant = READ_TIMEOUT_MS;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if (!SetCommTimeouts(hSerial, &timeouts))
    {
        printf("[ERROR] SetCommTimeouts failed.\n");
        goto cleanup;
    }

    return hSerial;

cleanup:
    CloseHandle(hSerial);
    return INVALID_HANDLE_VALUE;
}

bool executeArduinoCLI(const char *cliPath, const char *board, const char *port, const char *inoPath)
{
    char commandLine[1024];
    snprintf(commandLine, sizeof(commandLine), "\"%s\" compile --upload -b %s -p %s \"%s\"", cliPath, board, port, inoPath);
    printf("[INFO] Running Arduino CLI command...\n[DEBUG] %s\n", commandLine);

    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi;

    if (!CreateProcessA(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        printf("[ERROR] CreateProcess failed. Error code: %lu\n", GetLastError());
        return false;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exitCode != 0)
    {
        printf("[ERROR] Arduino code upload failed. Exit code: %lu\n", exitCode);
        return false;
    }
    printf("[INFO] Arduino code uploaded successfully.\n");
    return true;
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    const char *arduinoCliPath = "C:\\Users\\Administrator\\Desktop\\arduino-cli.exe";
    const char *boardType = "arduino:avr:uno";
    const char *inoFilePath = "C:\\Users\\Administrator\\Desktop\\ALU4B-Controller/ALU4B-Controller/ALU4B-Controller.ino";

    if (!executeArduinoCLI(arduinoCliPath, boardType, COM_PORT, inoFilePath))
    {
        printf("[FATAL] Could not upload to Arduino. Halting execution.\n");
        return 1;
    }

    printf("[INFO] Waiting %d ms for the board to initialize...\n", UPLOAD_WAIT_MS);
    Sleep(UPLOAD_WAIT_MS);

    hSerial = openAndSetupSerialPort();
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    printf("\n--- Starting Program Simulation ---\n");

    const char *highLevelProgram[] = {
        "int A = 25;",
        "int B = 12;",
        "int C = 1;",
      /*  "print(\"Initial A is\", A);",
        "print(\"Initial B is\", B);",
        "C = A - B;", // C = 25 - 12 = 13
        "print(\"C after A-B is\", C);",
        "",*/
        "if (C == 13) {",
        "   print(\"IF_BLOCK: C is 13!\", C);",
        "   C = C * 2;", // C = 13 * 2 = 26 (Simulated on PC)
        "}",
        /*"print(\"C after IF is\", C);",
        "",
        "if (C > 100) {",
        "   print(\"This should NOT print\", C);",
        "}",
        "print(\"Program finished. Final C:\", C);"*/};

    int numHighLevelLines = sizeof(highLevelProgram) / sizeof(const char *);

    int numGeneratedInstructions = 0;
    Instruction *program = parseAndGenerateInstructions(highLevelProgram, numHighLevelLines, &numGeneratedInstructions);

    if (program != NULL)
    {
        executeInstructions(program, numGeneratedInstructions);
        free(program);
    }

    printf("\n--- Program Execution Finished ---\n");

    if (hSerial != INVALID_HANDLE_VALUE)
    {
        clearSerialBuffer();
        CloseHandle(hSerial);
        printf("[DEBUG] Serial port closed successfully.\n");
    }

    return 0;
}