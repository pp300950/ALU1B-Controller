#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include <stdint.h>
#include <locale.h>
#include <time.h>

#define COM_PORT "COM3"
#define BAUD_RATE CBR_115200
#define UPLOAD_WAIT_MS 1000
#define READ_TIMEOUT_MS 200
#define MAX_READ_BUFFER 256

#define MAX_MEMORY 50
#define MAX_INSTRUCTIONS 2048
#define MAX_VARIABLES 50
#define JUMP_STACK_SIZE 20
#define STACK_SIZE 1024

// --- Global Handles and Flags ---
HANDLE hSerial = INVALID_HANDLE_VALUE;
bool ZERO_FLAG = false;
bool SIGN_FLAG = false;
bool CARRY_FLAG = false;
bool OVERFLOW_FLAG = false; // <-- เพิ่มตัวนี้

// --- Simulated CPU Components ---
long long REG_A = 0;
long long REG_B = 0;
long long MEMORY[MAX_MEMORY];
long long STACK[STACK_SIZE];
int sp = -1;                            // Stack Pointer
long long BINARY_INSTRUCTION_COUNT = 0; // จำนวนคำสั่งที่รันไปแล้ว

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
        // ไม่มี printf ในเวอร์ชันที่เน้นความเร็วสูงสุดเพื่อลด I/O
        return false;
    }

    DWORD bytesWritten = 0;
    DWORD bytesRead = 0;
    char readBuffer[MAX_READ_BUFFER] = {0};
    size_t len = strlen(dataToSend);

    // ตรวจสอบความถูกต้องของ Input, ยังคงจำเป็น
    if (len == 0 || dataToSend[len - 1] != '\n')
    {
        return false;
    }

    // เขียนข้อมูลไปยัง Serial Port
    if (!WriteFile(hSerial, dataToSend, (DWORD)len, &bytesWritten, NULL))
    {
        return false;
    }

    // อ่านการตอบกลับจาก Serial Port ทันที
    // ไม่จำเป็นต้อง Sleep เพราะ ReadFile เป็น Blocking call
    // มันจะรอจนกว่าจะมีข้อมูลเข้ามา (หรือจนกว่าจะ Timeout)
    if (ReadFile(hSerial, readBuffer, MAX_READ_BUFFER - 1, &bytesRead, NULL) && bytesRead > 0)
    {
        // ใช้ strcspn เพื่อหาตำแหน่งของ \r หรือ \n ตัวแรกและตัด string ทันที
        // ซึ่งเร็วกว่าการวน loop จากด้านหลัง
        readBuffer[strcspn(readBuffer, "\r\n")] = 0;

        if (sscanf(readBuffer, "%d %d", resultOutput, carryOutput) == 2)
        {
            return true;
        }
    }

    // หากมีข้อผิดพลาดใดๆ เกิดขึ้น จะไปที่นี่
    return false;
}

int highestBit(long long x)
{
    if (x == 0)
        return 0;
    int bit = 63;
    while (bit > 0 && ((x >> bit) & 1) == 0)
    {
        bit--;
    }
    return bit;
}

#define NUM_BITS 32

long long executeAluOperation(long long op1, long long op2, const char *muxCode, int subAddFlag, bool *final_carry)
{
    unsigned long long result_raw = 0;
    int carry_in = subAddFlag;

    int limit; // <-- ย้ายการประกาศตัวแปร limit มาไว้ตรงนี้

    // *** นี่คือส่วนที่แก้ไข ***
    // ถ้าเป็นการกลับบิต (NOT) หรือการลบโดยตรง (ที่อาจเพิ่มในอนาคต)
    // เราจำเป็นต้องประมวลผลให้ครบทุกบิตเพื่อความถูกต้องของ Two's Complement
    if (strcmp(muxCode, "111") == 0 || subAddFlag == 1)
    {
        limit = NUM_BITS;
    }
    else
    {
        // หากเป็นคำสั่งอื่น ให้ใช้ตรรกะเดิมเพื่อความเร็ว
        int max_bit = highestBit(op1);
        int max_bit2 = highestBit(op2);
        limit = (max_bit > max_bit2 ? max_bit : max_bit2) + 2; // +2 เผื่อ carry
    }

    for (int i = 0; i < limit; i++)
    {
        int bitA = (op1 >> i) & 1;
        int bitB = (op2 >> i) & 1;
        int alu_result_bit = 0, alu_carry_out = 0;

        char command[32];
        // snprintf(command, sizeof(command), "%s %d %d %d %d\n", muxCode, subAddFlag, bitA, bitB, carry_in);
        snprintf(command, sizeof(command), "%s %d %d %d %d\n", muxCode, subAddFlag, bitA, bitB, carry_in);

        if (!sendAndReceiveData(command, &alu_result_bit, &alu_carry_out))
        {
            printf("[FATAL] Hardware communication failed at bit %d. Aborting.\n", i);
            return 0; // Error
        }

        BINARY_INSTRUCTION_COUNT++;

        if (alu_result_bit)
        {
            result_raw |= (1ULL << i);
        }
        carry_in = alu_carry_out;
    }

    *final_carry = (carry_in == 1);

    long long final_result;
    if (result_raw & (1ULL << (NUM_BITS - 1)))
    {
        final_result = (long long)(int32_t)result_raw;
    }
    else
    {
        final_result = (long long)result_raw;
    }

    return final_result;
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

// ประกาศโครงสร้าง Label
typedef struct
{
    char label[50];
    int index;
} LabelEntry;

// ประกาศตัวแปร Global ที่ใช้ในการจัดการ Label
#define MAX_LABELS 100
LabelEntry labelMap[MAX_LABELS];
int labelCount = 0;

// ประกาศ Prototype ของฟังก์ชัน
void addLabel(const char *label, int index);
void generate_new_label(char *label_name);

// โค้ดของฟังก์ชัน addLabel
void addLabel(const char *label, int index)
{
    if (labelCount < MAX_LABELS)
    {
        strcpy(labelMap[labelCount].label, label);
        labelMap[labelCount].index = index;
        labelCount++;
    }
    else
    {
        // Handle error: too many labels
    }
}

// Helper to set a register's value by name
void setRegisterValue(const char *regName, long long value)
{
    if (strcmp(regName, "REG_A") == 0)
        REG_A = value;
    if (strcmp(regName, "REG_B") == 0)
        REG_B = value;
}

// เพิ่มฟังก์ชันนี้ไว้ในไฟล์ my_alu_simulator.c
char *longToBinary(long long n)
{
    static char binaryString[65]; // 64 bits + null terminator
    binaryString[64] = '\0';
    for (int i = 63; i >= 0; i--)
    {
        binaryString[63 - i] = ((n >> i) & 1) ? '1' : '0';
    }
    return binaryString;
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

        /*printf("\n[PC:%02d] Executing: %s", pc, current.instruction);
        if (strlen(current.operand1) > 0)
            printf(" %s", current.operand1);
        if (strlen(current.operand2) > 0)
            printf(", %s", current.operand2);
        printf("\n");
*/
        // --- Data and Memory Operations ---
        if (strcmp(current.instruction, "DEF") == 0)
        {
            int mem_addr = atoi(current.operand1);
            long long value = atoll(current.operand2);
            if (mem_addr >= 0 && mem_addr < MAX_MEMORY)
                MEMORY[mem_addr] = value;
            /*printf("      [INFO] DEFINE: MEMORY[%d] = %lld\n", mem_addr, value);
             */
        }
        else if (strcmp(current.instruction, "LOAD") == 0)
        {
            int mem_addr = atoi(current.operand2);
            if (mem_addr >= 0 && mem_addr < MAX_MEMORY)
            {
                setRegisterValue(current.operand1, MEMORY[mem_addr]);
                /*printf("      [INFO] LOAD: %s = MEMORY[%d] (Value: %lld)\n", current.operand1, mem_addr, getOperandValue(current.operand1));
                 */
            }
        }
        else if (strcmp(current.instruction, "STORE") == 0)
        {
            int mem_addr = atoi(current.operand1);
            long long val = getOperandValue(current.operand2);
            if (mem_addr >= 0 && mem_addr < MAX_MEMORY)
                MEMORY[mem_addr] = val;
            /*printf("      [INFO] STORE: MEMORY[%d] = %s (Value: %lld)\n", mem_addr, current.operand2, val);
             */
        }

        else if (strcmp(current.instruction, "MOV") == 0)
        {
            long long value = getOperandValue(current.operand2);
            setRegisterValue(current.operand1, value);
            /*
            printf("      [INFO] MOV: %s = %lld\n", current.operand1, value);
            */
        }

        else if (strcmp(current.instruction, "ADD") == 0)
        {
            long long val1 = getOperandValue(current.operand1);
            long long val2 = getOperandValue(current.operand2);
            long long result = executeAluOperation(val1, val2, "001", 0, &CARRY_FLAG);
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            /*
            printf("      [INFO] HW_ADD: %s = %lld + %lld -> %lld. Flags: Z=%d S=%d C=%d\n", current.operand1, val1, val2, result, ZERO_FLAG, SIGN_FLAG, CARRY_FLAG);
            */
        }

        else if (strcmp(current.instruction, "SUB") == 0)
        {
            long long val1 = getOperandValue(current.operand1);
            long long val2 = getOperandValue(current.operand2);

            // --- DEBUG START ---
            //    printf("\n [DEBUG_SUB] --- เริ่มคำสั่ง SUB ---\n");
            //    printf(" [DEBUG_SUB] ค่าเริ่มต้น: val1 = %lld (%s), val2 = %lld (%s)\n", val1, longToBinary(val1), val2, longToBinary(val2));
            // --- DEBUG END ---

            // ขั้นตอนที่ 1: กลับบิตของตัวถูกลบ (val2) โดยใช้ ~
            long long val2_invert = ~val2;

            // --- DEBUG STEP 1 ---
            //    printf(" [DEBUG_SUB] ขั้นตอนที่ 1 (NOT): กลับบิตของ val2 (%lld) -> ได้ผลลัพธ์ val2_invert = %lld (%s)\n", val2, val2_invert, longToBinary(val2_invert));
            // --- DEBUG END ---

            // ขั้นตอนที่ 2: บวก 1 เข้าไปในค่าที่กลับบิต เพื่อทำ Two's Complement
            long long negated_val2 = val2_invert + 1;

            // --- DEBUG STEP 2 ---
            // printf(" [DEBUG_SUB] ขั้นตอนที่ 2 (ADD 1): ทำ Two's Complement ของ val2 -> ได้ผลลัพธ์ negated_val2 = %lld (%s)\n", negated_val2, longToBinary(negated_val2));
            // --- DEBUG END ---

            // ขั้นตอนที่ 3: บวกตัวตั้ง (val1) กับค่า negated_val2
            // ใช้การบวกแบบปกติของ C เพื่อหลีกเลี่ยงข้อผิดพลาดใน executeAluOperation
            long long result = val1 + negated_val2;
            long long carry_result_check = val1 + negated_val2;

            // --- DEBUG STEP 3 ---
            //  printf(" [DEBUG_SUB] ขั้นตอนที่ 3 (ADD): นำ val1 (%lld) + negated_val2 (%lld) -> ได้ผลลัพธ์สุดท้าย = %lld (%s)\n", val1, negated_val2, result, longToBinary(result));
            // --- DEBUG END ---

            // ตั้งค่า Flags ด้วยตัวเอง
            // ผลลัพธ์จากการลบ 5-5 คือ 0
            // CARRY_FLAG สำหรับการลบคือ A >= B ซึ่งในกรณีนี้คือ 5 >= 5 (จริง)
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            CARRY_FLAG = (val1 >= val2);

            setRegisterValue(current.operand1, result);

            // --- DEBUG FINAL ---
            //  printf(" [DEBUG_SUB] อัปเดต Flags: ZERO=%d, SIGN=%d, CARRY=%d\n", ZERO_FLAG, SIGN_FLAG, CARRY_FLAG);
            // printf(" [DEBUG_SUB] --- จบคำสั่ง SUB ---\n");
            // --- DEBUG END ---
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
            /*printf("      [INFO] HW_AND: %s = %lld & %lld -> %lld. Flags: Z=%d S=%d\n", current.operand1, val1, val2, result, ZERO_FLAG, SIGN_FLAG);
             */
        }
        else if (strcmp(current.instruction, "OR") == 0)
        {
            long long val1 = getOperandValue(current.operand1);
            long long val2 = getOperandValue(current.operand2);
            long long result = executeAluOperation(val1, val2, "101", 0, &CARRY_FLAG); // Assuming MUX 101 is OR
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            /*printf("      [INFO] HW_OR: %s = %lld | %lld -> %lld. Flags: Z=%d S=%d\n", current.operand1, val1, val2, result, ZERO_FLAG, SIGN_FLAG);
             */
        }
        else if (strcmp(current.instruction, "XOR") == 0)
        {
            long long val1 = getOperandValue(current.operand1);
            long long val2 = getOperandValue(current.operand2);
            long long result = executeAluOperation(val1, val2, "110", 0, &CARRY_FLAG); // Assuming MUX 110 is XOR
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            /*printf("      [INFO] HW_XOR: %s = %lld ^ %lld -> %lld. Flags: Z=%d S=%d\n", current.operand1, val1, val2, result, ZERO_FLAG, SIGN_FLAG);
             */
        }
        else if (strcmp(current.instruction, "NOT") == 0)
        {
            long long val = getOperandValue(current.operand1);
            long long result = executeAluOperation(0, val, "111", 0, &CARRY_FLAG); // MUX 111 is NOT, op1 is ignored
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            /*
            printf("      [INFO] HW_NOT: %s = ~%lld -> %lld. Flags: Z=%d S=%d\n", current.operand1, val, result, ZERO_FLAG, SIGN_FLAG);
            */
        }

        else if (strcmp(current.instruction, "MUL") == 0)
        {
            /*
            printf("      [INFO] กำลังประมวลผล MUL ด้วยหลักการ Shift-and-Add บน ALU...\n");
            */
            long long val1 = getOperandValue(current.operand1); // ตัวตั้งคูณ
            long long val2 = getOperandValue(current.operand2); // ตัวคูณ

            long long result = 0;

            for (int i = 0; i < 64; i++)
            {
                // ตรวจสอบบิตที่ i ของตัวคูณ (val2)
                if ((val2 >> i) & 1)
                {
                    result = executeAluOperation(result, val1 << i, "001", 0, &CARRY_FLAG);
                }
                /*
                printf("      [STEP] i=%d, val2 bit=%lld, Current result=%lld\n", i, (val2 >> i) & 1, result);
                */
            }

            // ตั้งค่า register ด้วยผลลัพธ์สุดท้าย
            setRegisterValue(current.operand1, result);

            // ตั้งค่า Flags ตามผลลัพธ์สุดท้าย
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            CARRY_FLAG = false; // Carry flag มักจะถูกรีเซ็ตหลังการคูณ

            /*printf("      [INFO] MUL: %s = %lld * %lld -> ผลลัพธ์ %lld\n", current.operand1, val1, val2, result);
            printf("      [INFO] Flags: Z=%d, S=%d, C=%d\n", ZERO_FLAG, SIGN_FLAG, CARRY_FLAG);
            */
        }
        else if (strcmp(current.instruction, "DIV_FAST") == 0)
        {
            long long val1 = getOperandValue(current.operand1); // ตัวตั้ง
            long long val2 = getOperandValue(current.operand2); // ตัวหาร

            // **สำคัญมาก:** ป้องกันการหารด้วยศูนย์
            if (val2 == 0)
            {
                printf("      [ERROR] Runtime Error: Division by zero!\n");
                break; // หยุดการทำงานทันที
            }

            // ใช้ตัวหารของ PC คำนวณทันที!
            long long result = val1 / val2;

            // อัปเดตค่าใน Register และตั้งค่า Flags
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
        }
        else if (strcmp(current.instruction, "DIV") == 0)
        {
            long long val1 = getOperandValue(current.operand1); // Dividend
            long long val2 = getOperandValue(current.operand2); // Divisor
            if (val2 == 0)
            {
                printf("      [ERROR] Division by zero!\n");
                break;
            }

            // --- จัดการเครื่องหมาย ---
            bool result_is_negative = (val1 < 0) ^ (val2 < 0);
            long long abs_val1 = (val1 < 0) ? -val1 : val1;
            long long abs_val2 = (val2 < 0) ? -val2 : val2;

            long long remainder = abs_val1;
            long long quotient = 0;

            // หาบิตสูงสุดที่ divisor << shift <= dividend
            int shift = 0;
            while ((abs_val2 << (shift + 1)) > 0 && (abs_val2 << (shift + 1)) <= remainder)
            {
                shift++;
            }

            // วนจาก shift ลงมา
            for (int i = shift; i >= 0; i--)
            {
                long long divisor_shifted = abs_val2 << i;

                if (remainder >= divisor_shifted)
                {
                    // ถ้าลบได้ → เรียก ALU ลบจริง
                    bool temp_carry;
                    long long divisor_invert = executeAluOperation(divisor_shifted, 0, "111", 0, &temp_carry);
                    long long negated_divisor = executeAluOperation(divisor_invert, 1, "001", 0, &temp_carry);

                    remainder = executeAluOperation(remainder, negated_divisor, "001", 0, &temp_carry);
                    quotient |= (1LL << i);

                    // Early Exit → ถ้า remainder = 0 แล้วก็จบเลย
                    if (remainder == 0)
                        break;
                }
                else
                {
                    // ไม่จำเป็นต้องส่งไป ALU ถ้า remainder < divisor_shifted
                    continue;
                }
            }

            if (result_is_negative)
                quotient = -quotient;

            setRegisterValue(current.operand1, quotient);
            ZERO_FLAG = (quotient == 0);
            SIGN_FLAG = (quotient < 0);
        }

        else if (strcmp(current.instruction, "MOD") == 0)
        {
            long long val1 = getOperandValue(current.operand1); // ตัวตั้ง
            long long val2 = getOperandValue(current.operand2); // ตัวหาร

            if (val2 == 0)
            {
                printf("      [ERROR] Runtime Error: Division by zero in MOD!\n");
                break; // หยุดการทำงานทันที
            }

            // ใช้ตัวดำเนินการ % ของ C เพื่อคำนวณเศษโดยตรง
            long long result = val1 % val2;

            // อัปเดตค่าใน Register และตั้งค่า Flags
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
        }

        else if (strcmp(current.instruction, "PRINT") == 0)
        {
            // ตรวจสอบว่า operand1 เป็น register หรือไม่
            if (strcmp(current.operand1, "REG_A") == 0 ||
                strcmp(current.operand1, "REG_B") == 0 ||
                strncmp(current.operand1, "MEM[", 4) == 0)
            {
                // ถ้าใช่ ให้พิมพ์ค่าของมันออกมา
                long long valueToPrint = getOperandValue(current.operand1);
                printf("%lld", valueToPrint);
            }
            else
            {
                // ถ้าไม่ใช่ ให้ถือว่าเป็นข้อความธรรมดา
                // จัดการกับ Escape Characters เช่น \n (ขึ้นบรรทัดใหม่)
                const char *str = current.operand1;
                for (int i = 0; str[i] != '\0'; i++)
                {
                    if (str[i] == '\\' && str[i + 1] == 'n')
                    {
                        printf("\n");
                        i++; // ข้ามตัว n ไป
                    }
                    else
                    {
                        printf("%c", str[i]);
                    }
                }
            }
        }

        else if (strcmp(current.instruction, "CMP_FAST") == 0)
        {
            long long op1_val = getOperandValue(current.operand1);
            long long op2_val = getOperandValue(current.operand2);

            // คำนวณผลต่างด้วยความเร็วของ PC!
            long long result = op1_val - op2_val;

            // ตั้งค่า Flags โดยตรง ไม่ต้องเรียก Arduino
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            // การตั้ง Carry Flag สำหรับการลบคือ op1 < op2 (เกิดการยืม)
            CARRY_FLAG = ((unsigned long long)op1_val < (unsigned long long)op2_val);

            // คำนวณ Overflow Flag แบบง่าย
            bool sign1 = (op1_val < 0);
            bool sign2 = (op2_val < 0);
            bool sign_res = (result < 0);
            OVERFLOW_FLAG = (sign1 != sign2) && (sign1 != sign_res);
        }
        else if (strcmp(current.instruction, "INC_MEM") == 0)
        {
            int mem_addr = atoi(current.operand1);
            if (mem_addr >= 0 && mem_addr < MAX_MEMORY)
            {
                // เพิ่มค่าใน Memory โดยตรง เร็วกว่าเดิมมาก!
                MEMORY[mem_addr]++;
            }
        }

        else if (strcmp(current.instruction, "CMP") == 0)
        {
            long long op1_val = getOperandValue(current.operand1);
            long long op2_val = getOperandValue(current.operand2);
            /*
                        printf("      [INFO] CMP: เริ่มประมวลผล %lld เปรียบเทียบกับ %lld\n", op1_val, op2_val);
            */
            // ขั้นตอนที่ 1: Invert op2_val (~op2_val) โดยใช้ executeAluOperation
            bool temp_carry_not = false;
            long long val2_invert = executeAluOperation(op2_val, 0, "111", 0, &temp_carry_not);

            /*printf("      [DEBUG] CMP Step 1 (NOT): ~%lld (op2_val) -> %lld\n", op2_val, val2_invert);
             */
            // ขั้นตอนที่ 2: Add 1 to inverted op2_val (Two's Complement)
            bool temp_carry_add1 = false;
            long long negated_op2_val = executeAluOperation(val2_invert, 1, "001", 0, &temp_carry_add1);
            /*printf("      [DEBUG] CMP Step 2 (ADD 1): %lld + 1 -> %lld (Two's Complement)\n", val2_invert, negated_op2_val);
             */
            // ขั้นตอนที่ 3: Add op1_val and negated_op2_val
            bool carry_flag_temp = false;
            long long final_result_for_flags = executeAluOperation(op1_val, negated_op2_val, "001", 0, &carry_flag_temp);
            CARRY_FLAG = !carry_flag_temp; // Invert carry for subtract (borrow)
            /*printf("      [DEBUG] CMP Step 3 (ADD): %lld + %lld (Two's Complement) -> %lld\n", op1_val, negated_op2_val, final_result_for_flags);
             */
            // คำนวณ Overflow Flag: OV = (Sign_of_op1 == Sign_of_op2) && (Sign_of_op1 != Sign_of_result)
            // สำหรับการลบ (A-B), B ถูกทำ Two's Complement, ดังนั้นเราต้องดูเครื่องหมายของ A และ -B
            bool sign_op1 = (op1_val >> (NUM_BITS - 1)) & 1;
            bool sign_op2 = (op2_val >> (NUM_BITS - 1)) & 1;
            bool sign_result = (final_result_for_flags >> (NUM_BITS - 1)) & 1;

            ZERO_FLAG = (final_result_for_flags == 0);
            SIGN_FLAG = (final_result_for_flags < 0);
            CARRY_FLAG = !carry_flag_temp;

            // --- แก้ไขตรรกะการคำนวณ OVERFLOW_FLAG ---
            // Overflow จะเกิดเมื่อ: (บวก + บวก = ลบ) หรือ (ลบ + ลบ = บวก)
            bool sign1 = (op1_val < 0);
            bool sign2 = (negated_op2_val < 0);
            bool sign_res = (final_result_for_flags < 0);
            OVERFLOW_FLAG = (sign1 == sign2) && (sign1 != sign_res);
            // --- จบส่วนที่แก้ไข ---

            // แสดงผลลัพธ์และ Flags สุดท้ายของ CMP
            /*printf("      [INFO] CMP Finished: %lld vs %lld. ผลลัพธ์จากการลบ (เพื่อ Flags เท่านั้น)=%lld. Flags: Z=%d, S=%d, C=%d, O=%d\n",
                   op1_val, op2_val, final_result_for_flags, ZERO_FLAG, SIGN_FLAG, CARRY_FLAG, OVERFLOW_FLAG);
                   */
        }
        else if (strcmp(current.instruction, "JMP") == 0 ||
                 strcmp(current.instruction, "JZ") == 0 || strcmp(current.instruction, "JE") == 0 ||
                 strcmp(current.instruction, "JNZ") == 0 || strcmp(current.instruction, "JNE") == 0 ||
                 strcmp(current.instruction, "JC") == 0 || strcmp(current.instruction, "JNC") == 0 ||
                 strcmp(current.instruction, "JG") == 0 || strcmp(current.instruction, "JGE") == 0 ||
                 strcmp(current.instruction, "JL") == 0 || strcmp(current.instruction, "JLE") == 0 ||
                 strcmp(current.instruction, "JO") == 0 || strcmp(current.instruction, "JNO") == 0)

        {
            bool do_the_jump =
                (strcmp(current.instruction, "JMP") == 0) ||

                // Unsigned jumps
                ((strcmp(current.instruction, "JZ") == 0 || strcmp(current.instruction, "JE") == 0) && ZERO_FLAG) ||    // ZF=1
                ((strcmp(current.instruction, "JNZ") == 0 || strcmp(current.instruction, "JNE") == 0) && !ZERO_FLAG) || // ZF=0
                (strcmp(current.instruction, "JC") == 0 && CARRY_FLAG) ||                                               // CF=1
                (strcmp(current.instruction, "JNC") == 0 && !CARRY_FLAG) ||                                             // CF=0

                // Signed jumps (ใช้ ZERO_FLAG, SIGN_FLAG, OVERFLOW_FLAG ตามหลักการ)
                (strcmp(current.instruction, "JG") == 0 && !ZERO_FLAG && (SIGN_FLAG == OVERFLOW_FLAG)) ||   // Jump if Greater (A > B)
                (strcmp(current.instruction, "JGE") == 0 && (SIGN_FLAG == OVERFLOW_FLAG)) ||                // Jump if Greater or Equal (A >= B)
                (strcmp(current.instruction, "JLT") == 0 && (SIGN_FLAG != OVERFLOW_FLAG)) ||                // Jump if Less (A < B)
                (strcmp(current.instruction, "JLE") == 0 && (ZERO_FLAG || (SIGN_FLAG != OVERFLOW_FLAG))) || // Jump if Less or Equal (A <= B)
                (strcmp(current.instruction, "JO") == 0 && OVERFLOW_FLAG) ||                                // Jump on Overflow
                (strcmp(current.instruction, "JNO") == 0 && !OVERFLOW_FLAG);                                // Jump on No Overflow

            if (do_the_jump)
            {

                bool label_found = false;
                for (int i = 0; i < labelCount; i++)
                {
                    if (strcmp(labelMap[i].label, current.operand1) == 0)
                    {
                        pc = labelMap[i].index;
                        shouldJump = true;
                        label_found = true;
                        /*printf("      [INFO] JUMP to '%s' (PC -> %d)\n", current.operand1, pc);
                         */
                        break;
                    }
                }
                if (!label_found)
                {
                    printf("      [ERROR] Label '%s' not found for JUMP!\n", current.operand1);
                }
            }
            else if (strcmp(current.instruction, "JL") == 0)
            {
                return (SIGN_FLAG != OVERFLOW_FLAG);
            }

            else
            {
                /*printf("      [INFO] JUMP condition false. No jump.\n");
                 */
            }
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

// ฟังก์ชันช่วยแปลง expression เป็น sequence ของ ALU instructions
int parseExpression(const char *expr, Instruction *instructions, int instructionCount,
                    Variable *symbolTable, int variableCount)
{
    char expr_copy[256];
    strcpy(expr_copy, expr);

    char *token = strtok(expr_copy, " ");
    int first = 1;
    char op[5];

    while (token != NULL)
    {
        if (first)
        {
            // operand ตัวแรก -> โหลดเข้า REG_A
            int addr = findVariable(token, symbolTable, variableCount);
            if (addr != -1)
            {
                sprintf(instructions[instructionCount].instruction, "LOAD");
                sprintf(instructions[instructionCount].operand1, "REG_A");
                sprintf(instructions[instructionCount].operand2, "%d", addr);
            }
            else
            {
                sprintf(instructions[instructionCount].instruction, "MOV");
                sprintf(instructions[instructionCount].operand1, "REG_A");
                sprintf(instructions[instructionCount].operand2, "%s", token);
            }
            instructionCount++;
            first = 0;
        }
        else
        {
            if (strcmp(token, "+") == 0 || strcmp(token, "-") == 0 ||
                strcmp(token, "*") == 0 || strcmp(token, "/") == 0 || strcmp(token, "%") == 0)
            {
                // token นี้คือ operator
                strcpy(op, token);
            }
            else
            {
                // token นี้คือ operand
                int addr = findVariable(token, symbolTable, variableCount);
                if (addr != -1)
                {
                    sprintf(instructions[instructionCount].instruction, "LOAD");
                    sprintf(instructions[instructionCount].operand1, "REG_B");
                    sprintf(instructions[instructionCount].operand2, "%d", addr);
                }
                else
                {
                    sprintf(instructions[instructionCount].instruction, "MOV");
                    sprintf(instructions[instructionCount].operand1, "REG_B");
                    sprintf(instructions[instructionCount].operand2, "%s", token);
                }
                instructionCount++;

                // apply operator (REG_A op REG_B)
                if (strcmp(op, "+") == 0)
                    sprintf(instructions[instructionCount].instruction, "ADD");
                else if (strcmp(op, "-") == 0)
                    sprintf(instructions[instructionCount].instruction, "SUB");
                else if (strcmp(op, "*") == 0)
                    sprintf(instructions[instructionCount].instruction, "MUL");
                /*else if (strcmp(op, "/") == 0)
                    sprintf(instructions[instructionCount].instruction, "DIV");*/
                else if (strcmp(op, "/") == 0)
                    sprintf(instructions[instructionCount].instruction, "DIV_FAST");
                else if (strcmp(op, "%") == 0) // <--- เพิ่มบรรทัดนี้
                    sprintf(instructions[instructionCount].instruction, "MOD");

                sprintf(instructions[instructionCount].operand1, "REG_A");
                sprintf(instructions[instructionCount].operand2, "REG_B");
                instructionCount++;
            }
        }
        token = strtok(NULL, " ");
    }

    return instructionCount;
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

    // Stacks สำหรับจัดการ nested for-loops
    int for_loop_start_stack[JUMP_STACK_SIZE];
    char for_update_statement_stack[JUMP_STACK_SIZE][100];
    int for_stack_ptr = -1;

    // Stack สำหรับจัดการประเภทของ block ที่กำลังทำงานอยู่ (if/for)
    enum BlockType
    {
        BLOCK_IF,
        BLOCK_FOR,
        BLOCK_ELSE
    };
    enum BlockType block_type_stack[JUMP_STACK_SIZE];
    int block_stack_ptr = -1;

    // Stack สำหรับ JMP ข้าม else (เมื่อ if เป็นจริง)
    int else_jump_fix_stack[JUMP_STACK_SIZE];
    int else_stack_ptr = -1;
    // Stack สำหรับเก็บจุดที่ต้องกระโดดออกจาก loop (หลังจากจบ loop)
    int loop_exit_jump_stack[JUMP_STACK_SIZE];
    int loop_stack_ptr = -1;

    // printf("\n--- Compiler: Stage 1 - Parsing and Generating Instructions ---\n");

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
        // 4. For loop: for(i = 1; i <= 12; i++)

        else if (strncmp(trimmed_line, "for(", 4) == 0)
        {
            const char *for_content = trimmed_line + 4;
            const char *semicolon1 = strchr(for_content, ';');
            const char *semicolon2 = strchr(semicolon1 + 1, ';');
            const char *paren_end = strchr(semicolon2 + 1, ')');
            char init[100], cond[100], update[100];
            strncpy(init, for_content, semicolon1 - for_content);
            init[semicolon1 - for_content] = '\0';
            strncpy(cond, semicolon1 + 1, semicolon2 - (semicolon1 + 1));
            cond[semicolon2 - (semicolon1 + 1)] = '\0';
            strncpy(update, semicolon2 + 1, paren_end - (semicolon2 + 1));
            update[paren_end - (semicolon2 + 1)] = '\0';
            /*
                        printf("[DEBUG] Init part: '%s'\n", trim(init));
                        printf("[DEBUG] Condition part: '%s'\n", trim(cond));
                        printf("[DEBUG] Update part: '%s'\n", trim(update));
            */
            // --- Part 1: Initialization (เหมือนเดิม) ---
            char initVar[50], initValStr[50];
            if (sscanf(trim(init), "%s = %s", initVar, initValStr) == 2)
            {
                int dest_addr = findVariable(initVar, symbolTable, variableCount);
                if (dest_addr != -1)
                {
                    sprintf(instructions[instructionCount].instruction, "MOV");
                    sprintf(instructions[instructionCount].operand1, "REG_A");
                    sprintf(instructions[instructionCount].operand2, "%s", initValStr);
                    instructionCount++;
                    sprintf(instructions[instructionCount].instruction, "STORE");
                    sprintf(instructions[instructionCount].operand1, "%d", dest_addr);
                    sprintf(instructions[instructionCount].operand2, "REG_A");
                    instructionCount++;
                }
                else
                {
                    printf("ERROR: Variable '%s' not declared.\n", initVar);
                    return NULL;
                }
            }

            // --- Part 2: Condition Check (Start of loop) ---
            // printf("\n[COMPILER_LOG] --- Entering FOR block ---\n");

            // สร้างและบันทึก Label สำหรับจุดเริ่มต้นของลูป
            char loop_start_label[20];
            generate_new_label(loop_start_label);
            strcpy(instructions[instructionCount].label, loop_start_label);
            strcpy(instructions[instructionCount].instruction, ""); // no-op ที่มีแต่ label
            instructionCount++;

            addLabel(loop_start_label, instructionCount);

            // เพิ่ม Log เพื่อยืนยันการบันทึก Label
            // printf("[COMPILER_LOG] Registered loop START label '%s' at PC address %d\n", loop_start_label, instructionCount);

            // จัดการข้อมูล loop (Push stacks)
            block_type_stack[++block_stack_ptr] = BLOCK_FOR;
            for_stack_ptr++;
            for_loop_start_stack[for_stack_ptr] = instructionCount;
            strcpy(for_update_statement_stack[for_stack_ptr], trim(update));

            // เพิ่ม Log เพื่อดูสถานะของ Stack
            // printf("[COMPILER_LOG] Pushed FOR block. Current block_stack_ptr = %d\n", block_stack_ptr);

            // ... ส่วนที่เหลือของโค้ดเหมือนเดิม ...
            char condVar[50], condOp[10], condVal[50];
            sscanf(trim(cond), "%s %s %s", condVar, condOp, condVal);

            int addr1 = findVariable(trim(condVar), symbolTable, variableCount);
            if (addr1 == -1)
            {
                return NULL;
            }
            sprintf(instructions[instructionCount].instruction, "LOAD");
            sprintf(instructions[instructionCount].operand1, "REG_A");
            sprintf(instructions[instructionCount].operand2, "%d", addr1);
            instructionCount++;

            int addr2 = findVariable(trim(condVal), symbolTable, variableCount);
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
                sprintf(instructions[instructionCount].operand2, "%s", trim(condVal));
            }
            instructionCount++;

            sprintf(instructions[instructionCount].instruction, "CMP_FAST");

            sprintf(instructions[instructionCount].operand1, "REG_A");
            sprintf(instructions[instructionCount].operand2, "REG_B");
            instructionCount++;

            // --- Part 3: Generate Conditional Jump to Exit Loop (เหมือนเดิม) ---
            char jump_instruction[10];

            int jump_generated = 1;
            if (strcmp(condOp, "==") == 0)
                strcpy(jump_instruction, "JNE"); // jump if not equal
            else if (strcmp(condOp, "!=") == 0)
                strcpy(jump_instruction, "JE"); // jump if equal
            else if (strcmp(condOp, "<") == 0)
                strcpy(jump_instruction, "JGE"); // jump if >=
            else if (strcmp(condOp, "<=") == 0)
                strcpy(jump_instruction, "JG"); // jump if greater than
            else if (strcmp(condOp, ">") == 0)
                strcpy(jump_instruction, "JLE"); // jump if <=
            else if (strcmp(condOp, ">=") == 0)
                strcpy(jump_instruction, "JL"); // jump if <

            else
            {
                printf("ERROR: Unsupported operator '%s' in for loop condition.\n", condOp);
                jump_generated = 0;
            }

            if (jump_generated)
            {
                char exit_label[20];
                generate_new_label(exit_label);

                // printf("[COMPILER_LOG] Generated loop EXIT label '%s' for the conditional jump.\n", exit_label);

                sprintf(instructions[instructionCount].instruction, "%s", jump_instruction);
                sprintf(instructions[instructionCount].operand1, "%s", exit_label);

                // เก็บตำแหน่ง jump ไว้ใน jump_fix_stack
                jump_fix_stack[++stack_ptr] = instructionCount;
                instructionCount++;
            }
        }
        else if (strcmp(trimmed_line, "break") == 0)
        {
            // ตรวจสอบว่าเราอยู่ใน loop หรือไม่
            if (loop_stack_ptr >= 0)
            {
                // ดึงตำแหน่งของคำสั่ง JUMP ที่ใช้ للخروجจาก loop ปัจจุบัน
                int exit_jump_idx = loop_exit_jump_stack[loop_stack_ptr];

                // ดึงชื่อ Label ปลายทางจากคำสั่ง JUMP นั้น
                char *exit_label_name = instructions[exit_jump_idx].operand1;

                // สร้างคำสั่ง JMP เพื่อกระโดดไปยัง Label ปลายทางทันที
                sprintf(instructions[instructionCount].instruction, "JMP");
                sprintf(instructions[instructionCount].operand1, "%s", exit_label_name);
                instructionCount++;
            }
            else
            {
                // กรณีใช้ break นอก loop
                printf("ERROR: 'break' statement not within a loop.\n");
                free(instructions);
                return NULL;
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

            // 🔹 ใช้ parser ใหม่
            instructionCount = parseExpression(clean_rhs, instructions, instructionCount,
                                               symbolTable, variableCount);

            // STORE ผลลัพธ์กลับเข้า memory
            sprintf(instructions[instructionCount].instruction, "STORE");
            sprintf(instructions[instructionCount].operand1, "%d", dest_addr);
            sprintf(instructions[instructionCount].operand2, "REG_A");
            instructionCount++;
        }

        // 3. Print: รองรับการพิมพ์แบบยืดหยุ่น
        else if (strncmp(trimmed_line, "print(", 6) == 0)
        {
            // หาตำแหน่งของวงเล็บเปิดและปิด
            char *start = strchr(trimmed_line, '(') + 1;
            char *end = strrchr(trimmed_line, ')');

            if (start && end)
            {
                // คัดลอกเฉพาะส่วนที่อยู่ข้างในวงเล็บ
                char args[256];
                strncpy(args, start, end - start);
                args[end - start] = '\0';

                // ใช้ strtok เพื่อแยก argument ที่คั่นด้วย ','
                char *token = strtok(args, ",");
                while (token != NULL)
                {
                    char *trimmed_token = trim(token);

                    // กรณีที่ 1: Token เป็นข้อความ (อยู่ในเครื่องหมายคำพูด)
                    if (trimmed_token[0] == '"' && trimmed_token[strlen(trimmed_token) - 1] == '"')
                    {
                        // ตัดเครื่องหมายคำพูดออก
                        trimmed_token[strlen(trimmed_token) - 1] = '\0';
                        char *message = trimmed_token + 1;

                        // สร้างคำสั่ง PRINT สำหรับข้อความ
                        sprintf(instructions[instructionCount].instruction, "PRINT");
                        sprintf(instructions[instructionCount].operand1, "%s", message);
                        strcpy(instructions[instructionCount].operand2, "");
                        instructionCount++;
                    }
                    // กรณีที่ 2: Token เป็นตัวแปร
                    else
                    {
                        int mem_addr = findVariable(trimmed_token, symbolTable, variableCount);
                        if (mem_addr != -1)
                        {
                            // สร้างคำสั่ง LOAD และ PRINT สำหรับตัวแปร
                            sprintf(instructions[instructionCount].instruction, "LOAD");
                            sprintf(instructions[instructionCount].operand1, "REG_A");
                            sprintf(instructions[instructionCount].operand2, "%d", mem_addr);
                            instructionCount++;

                            sprintf(instructions[instructionCount].instruction, "PRINT");
                            sprintf(instructions[instructionCount].operand1, "REG_A");
                            strcpy(instructions[instructionCount].operand2, "");
                            instructionCount++;
                        }
                        else
                        {
                            printf("ERROR: Undeclared variable '%s' in print statement.\n", trimmed_token);
                            free(instructions);
                            return NULL;
                        }
                    }
                    token = strtok(NULL, ",");
                }
            }
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
            block_type_stack[++block_stack_ptr] = BLOCK_IF;
        }
        // 5. Else If statement: } else if (A < 10) {
        else if (strncmp(trimmed_line, "} else if", 9) == 0)
        {
            // Step 1: สร้าง JMP เพื่อข้าม block else นี้ ถ้า if ก่อนหน้าเป็นจริง
            sprintf(instructions[instructionCount].instruction, "JMP");
            char end_if_label[20];
            generate_new_label(end_if_label); // สร้าง Label สำหรับจุดสิ้นสุด
            sprintf(instructions[instructionCount].operand1, "%s", end_if_label);
            else_jump_fix_stack[++else_stack_ptr] = instructionCount; // push index ของ JMP นี้ไว้แก้
            instructionCount++;

            // Step 2: แก้ไข Jump ของ if block ก่อนหน้า ให้ชี้มาที่นี่
            int prev_if_jump_idx = jump_fix_stack[stack_ptr--];
            char *else_label = instructions[prev_if_jump_idx].operand1; // ดึง Label ที่จะใช้
            strcpy(instructions[instructionCount].label, else_label);   // กำหนด Label ให้ instruction *ว่าง* ถัดไป
            strcpy(instructions[instructionCount].instruction, "");     // สร้าง instruction ว่างๆ ที่มีแต่ label
            instructionCount++;

            // Step 3: Parse เงื่อนไขของ else if นี้ (เหมือน if ปกติ)
            sscanf(trimmed_line, "} else if (%s %s %s)", lhs, op, rhs);

            char *clean_lhs = trim(lhs);
            char *clean_op = trim(op);
            char *clean_rhs_if = trim(strtok(rhs, ")"));

            jump_fix_stack[++stack_ptr] = instructionCount; // push jump ใหม่
            instructionCount++;

            block_type_stack[block_stack_ptr] = BLOCK_ELSE; // เปลี่ยน block type เป็น else
        }
        // 6. Else statement: } else {
        else if (strncmp(trimmed_line, "} else", 6) == 0)
        {
            // Step 1: สร้าง JMP เพื่อข้าม block else นี้
            sprintf(instructions[instructionCount].instruction, "JMP");
            else_jump_fix_stack[++else_stack_ptr] = instructionCount;
            instructionCount++;

            // Step 2: แก้ไข Jump ของ if/else if block ก่อนหน้า ให้ชี้มาที่นี่
            int prev_if_jump_idx = jump_fix_stack[stack_ptr--];
            sprintf(instructions[prev_if_jump_idx].operand1, "%d", instructionCount);

            block_type_stack[block_stack_ptr] = BLOCK_ELSE; // เปลี่ยน block type
        }

        else if (strcmp(trimmed_line, "}") == 0)
        {
            if (block_stack_ptr < 0)
            {
                continue;
            }

            enum BlockType current_block = block_type_stack[block_stack_ptr--];

            if (current_block == BLOCK_FOR)
            {
                // --- Update i++ ---
                char update_statement[100];
                strcpy(update_statement, for_update_statement_stack[for_stack_ptr]);

                char update_var[50];
                if (sscanf(update_statement, "%[a-zA-Z0-9_]++", update_var) == 1)
                {
                    int var_addr = findVariable(update_var, symbolTable, variableCount);
                    if (var_addr != -1)
                    {
                        sprintf(instructions[instructionCount].instruction, "INC_MEM");
                        sprintf(instructions[instructionCount].operand1, "%d", var_addr);
                        instructionCount++;
                    }
                }

                // --- JMP back to loop start ---
                int loop_start_addr = for_loop_start_stack[for_stack_ptr];
                char loop_start_label[20] = "";
                for (int k = 0; k < labelCount; k++)
                {
                    if (labelMap[k].index == loop_start_addr)
                    {
                        strcpy(loop_start_label, labelMap[k].label);
                        break;
                    }
                }

                sprintf(instructions[instructionCount].instruction, "JMP");
                sprintf(instructions[instructionCount].operand1, "%s", loop_start_label);
                instructionCount++;

                // --- Place EXIT label *here* (after loop ends) ---
                int jump_out_pc = jump_fix_stack[stack_ptr--];
                char *exit_label_name = instructions[jump_out_pc].operand1;

                addLabel(exit_label_name, instructionCount);
                strcpy(instructions[instructionCount].label, exit_label_name);
                strcpy(instructions[instructionCount].instruction, "");
                instructionCount++;

                for_stack_ptr--;
            }
            else if (current_block == BLOCK_IF)
            {
                int jump_idx = jump_fix_stack[stack_ptr--];
                char *label_to_set = instructions[jump_idx].operand1;
                strcpy(instructions[instructionCount].label, label_to_set);
                strcpy(instructions[instructionCount].instruction, "");
                instructionCount++;
            }
            else if (current_block == BLOCK_ELSE)
            {
                int else_jump_idx = else_jump_fix_stack[else_stack_ptr--];
                char *label_to_set = instructions[else_jump_idx].operand1;
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

    /*
        timeouts.ReadIntervalTimeout = 100;
        timeouts.ReadTotalTimeoutConstant = READ_TIMEOUT_MS;
        timeouts.ReadTotalTimeoutMultiplier = 0;
        timeouts.WriteTotalTimeoutConstant = 50;
        timeouts.WriteTotalTimeoutMultiplier = 10;

        --------------------------------------------------
        เวลาประมวลผล: 66.347000 seconds
        --------------------------------------------------
    */

    timeouts.ReadIntervalTimeout = 0;        // <--- ปิดการใช้งาน Interval Timeout (สำคัญที่สุด)
    timeouts.ReadTotalTimeoutConstant = 50;  // <--- ลดเวลารอรวมทั้งหมดเหลือ 50ms (ถ้า Arduino ค้าง)
    timeouts.ReadTotalTimeoutMultiplier = 0; // <--- ไม่ใช้ตัวคูณ
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 0;

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



int main() // ไม่ต้องรับ arguments แล้ว
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    clock_t start_time, end_time;
    double cpu_time_used;

    // ส่วนของการเรียกใช้ Arduino และการตั้งค่า Serial Port ยังคงเดิม
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

   const char *highLevelProgram[] = {
        "int a = 48;",
        "int b = 18;",
        "int minVal = 18;",
        "int i = 0;",
        "int gcd = 0;",
        "",
        "for(i = minVal; i >= 1; i--) {",
        "    int r1 = a % i;",
        "    int r2 = b % i;",
        "    if (r1 == 0) {",
        "        if (r2 == 0) {",
        "            gcd = i;",
        "            break;",
        "        }",
        "    }",
        "}",
        "",
        "print(\"GCD is: \", gcd, \"\\n\");"
    };

    // คำนวณจำนวนบรรทัดของโค้ดโดยอัตโนมัติ
    int numHighLevelLines = sizeof(highLevelProgram) / sizeof(highLevelProgram[0]);
    // --- จบส่วนใส่โค้ด ---

    // เริ่มจับเวลา
    start_time = clock();

    int numGeneratedInstructions = 0;
    Instruction *program = parseAndGenerateInstructions((const char **)highLevelProgram, numHighLevelLines, &numGeneratedInstructions);

    if (program != NULL)
    {
        executeInstructions(program, numGeneratedInstructions);
        free(program);
    }

    // ไม่ต้องคืนหน่วยความจำของ highLevelProgram แล้ว เพราะเป็น static array

    // สิ้นสุดการจับเวลา
    end_time = clock();
    cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    if (hSerial != INVALID_HANDLE_VALUE)
    {
        clearSerialBuffer();
        CloseHandle(hSerial);
        printf("\n[DEBUG] Serial port closed successfully.\n");
    }

    printf("\n--------------------------------------------------\n");
    printf("จำนวนคำสั่งไบนารี ทั้งหมด: %lld\n", BINARY_INSTRUCTION_COUNT);
    printf("เวลาประมวลผล: %f seconds\n", cpu_time_used);
    printf("--------------------------------------------------\n");

    return 0;
}