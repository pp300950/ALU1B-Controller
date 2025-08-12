// นี่คือไฟล์ C:\Users\Administrator\Desktop\ALU4B-Controller\mainCode_c\serial_comm.h

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include <locale.h>

#define COM_PORT "COM3"
#define BAUD_RATE CBR_9600
#define UPLOAD_WAIT_MS 3000
#define READ_TIMEOUT_MS 1000
#define MAX_READ_BUFFER 256
#define NUM_BITS 32

HANDLE hSerial = INVALID_HANDLE_VALUE;

// --- Global Registers and Memory (Hard-coded) ---
// เราจะใช้ตัวแปรเหล่านี้จำลองการเป็น Register และหน่วยความจำ
long long REG_A = 0;     // Accumulator Register
long long REG_B = 0;     // General Purpose Register
long long MEMORY[10];    // Simple Memory Array
bool CARRY_FLAG = false; // Carry Flag

char *decimalToBinary(long long decimal)
{
    // โค้ดเดิมจะคำนวณจำนวนบิตตามค่าที่ป้อนเข้ามา
    // โค้ดที่ปรับปรุงจะกำหนดให้เป็น 8 บิตเสมอ
    char *binaryString = (char *)malloc(NUM_BITS + 1);
    if (!binaryString)
        return NULL;
    binaryString[NUM_BITS] = '\0';

    long long value = decimal;
    for (int i = NUM_BITS - 1; i >= 0; i--)
    {
        binaryString[i] = (value & 1) ? '1' : '0';
        value >>= 1;
    }
    return binaryString;
}

// ฟังก์ชันสำหรับแปลงเลขฐานสองสตริงเป็นค่าเลขฐานสิบ
long long binaryToDecimal(const char *binaryString)
{
    long long decimalValue = 0;
    int length = strlen(binaryString);
    long long powerOfTwo = 1;
    for (int i = length - 1; i >= 0; i--)
    {
        if (binaryString[i] == '1')
        {
            decimalValue += powerOfTwo;
        }
        powerOfTwo *= 2;
    }
    return decimalValue;
}

// ฟังก์ชันสำหรับเติม 0 ด้านหน้าสตริงฐานสอง
char *padBinaryString(const char *binaryString, size_t desiredLength)
{
    size_t currentLength = strlen(binaryString);
    if (currentLength >= desiredLength)
    {
        return _strdup(binaryString);
    }

    char *paddedString = (char *)malloc(desiredLength + 1);
    if (!paddedString)
        return NULL;

    size_t padCount = desiredLength - currentLength;
    for (size_t i = 0; i < padCount; i++)
    {
        paddedString[i] = '0';
    }
    strcpy(paddedString + padCount, binaryString);
    paddedString[desiredLength] = '\0';
    return paddedString;
}

// เคลียร์ buffer ของ serial port
void clearSerialBuffer()
{
    if (hSerial != INVALID_HANDLE_VALUE)
    {
        printf("[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล\n");
        PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
    }
}

// Signal handler สำหรับ cleanup
BOOL WINAPI ConsoleHandler(DWORD dwCtrlType)
{
    if (dwCtrlType == CTRL_C_EVENT)
    {
        printf("\n[INFO] ตรวจพบ Ctrl+C. กำลังปิดโปรแกรม...\n");
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

// ฟังก์ชันสำหรับเปิดพอร์ตและตั้งค่า
HANDLE openAndSetupSerialPort()
{
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts = {0};

    printf("[DEBUG] กำลังเปิดพอร์ตซีเรียล: %s\n", COM_PORT);

    int attempts = 0;
    while (attempts < 5)
    {
        hSerial = CreateFileA(COM_PORT, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hSerial != INVALID_HANDLE_VALUE)
        {
            break;
        }

        DWORD err = GetLastError();
        printf("[WARNING] เปิดพอร์ตไม่ได้ (ครั้งที่ %d). รหัสข้อผิดพลาด: %lu. กำลังลองใหม่ใน 1 วินาที...\n", attempts + 1, err);
        Sleep(1000);
        attempts++;
    }

    if (hSerial == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND)
        {
            printf("[ERROR] ไม่พบพอร์ตซีเรียล %s\n", COM_PORT);
        }
        else if (err == ERROR_ACCESS_DENIED)
        {
            printf("[ERROR] พอร์ตซีเรียล %s กำลังถูกใช้งานโดยโปรแกรมอื่น (เช่น Serial Monitor)\n", COM_PORT);
        }
        else
        {
            printf("[ERROR] ไม่สามารถเปิดพอร์ตซีเรียล %s ได้ รหัสข้อผิดพลาด: %lu\n", COM_PORT, err);
        }
        return INVALID_HANDLE_VALUE;
    }

    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        printf("[ERROR] GetCommState ล้มเหลว\n");
        goto cleanup;
    }
    dcbSerialParams.BaudRate = BAUD_RATE;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if (!SetCommState(hSerial, &dcbSerialParams))
    {
        printf("[ERROR] SetCommState ล้มเหลว\n");
        goto cleanup;
    }

    timeouts.ReadIntervalTimeout = 100;
    timeouts.ReadTotalTimeoutConstant = READ_TIMEOUT_MS;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if (!SetCommTimeouts(hSerial, &timeouts))
    {
        printf("[ERROR] SetCommTimeouts ล้มเหลว\n");
        goto cleanup;
    }

    return hSerial;

cleanup:
    clearSerialBuffer();
    CloseHandle(hSerial);
    hSerial = INVALID_HANDLE_VALUE;
    return INVALID_HANDLE_VALUE;
}

bool sendAndReceiveData(const char *dataToSend, int *resultOutput, int *carryOutput)
{
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        printf("[ERROR] Handle ซีเรียลไม่ถูกต้อง\n");
        return false;
    }

    DWORD bytesWritten = 0;
    DWORD bytesRead = 0;
    char readBuffer[MAX_READ_BUFFER] = {0};

    size_t len = strlen(dataToSend);
    if (len == 0 || dataToSend[len - 1] != '\n')
    {
        printf("[ERROR] ข้อมูลที่จะส่งต้องลงท้ายด้วยอักขระขึ้นบรรทัดใหม่ (\\n)\n");
        return false;
    }

    clearSerialBuffer();

    DWORD dataSize = (DWORD)len;
    DWORD totalWritten = 0;
    printf("[DEBUG] กำลังเขียนข้อมูล %lu ไบต์: \"%s\"\n", dataSize, dataToSend);
    while (totalWritten < dataSize)
    {
        if (!WriteFile(hSerial, dataToSend + totalWritten, dataSize - totalWritten, &bytesWritten, NULL))
        {
            printf("[ERROR] WriteFile ล้มเหลว รหัสข้อผิดพลาด: %lu\n", GetLastError());
            return false;
        }
        totalWritten += bytesWritten;
    }

    Sleep(5);

    DWORD totalBytesRead = 0;
    do
    {
        if (totalBytesRead >= MAX_READ_BUFFER - 1)
            break;
        if (ReadFile(hSerial, readBuffer + totalBytesRead, MAX_READ_BUFFER - 1 - totalBytesRead, &bytesRead, NULL) && bytesRead > 0)
        {
            totalBytesRead += bytesRead;
            if (strchr(readBuffer, '\n'))
                break;
        }
        else
        {
            break;
        }
    } while (true);

    readBuffer[totalBytesRead] = '\0';
    if (totalBytesRead > 0)
    {
        for (int i = (int)totalBytesRead - 1; i >= 0; --i)
        {
            if (readBuffer[i] == '\n' || readBuffer[i] == '\r')
                readBuffer[i] = '\0';
            else
                break;
        }
        printf("[INFO] ได้รับข้อมูล %lu ไบต์: %s\n", totalBytesRead, readBuffer);
        if (sscanf(readBuffer, "%d %d", resultOutput, carryOutput) == 2)
        {
            return true;
        }
        else
        {
            printf("[ERROR] รูปแบบข้อมูลที่ได้รับไม่ถูกต้อง: \"%s\"\n", readBuffer);
            return false;
        }
    }
    else
    {
        printf("[DEBUG] ไม่ได้รับข้อมูลหรืออ่านข้อมูลหมดเวลาแล้ว\n");
        return false;
    }
}
/// ฟังก์ชันหลักสำหรับจำลองการคำนวณ (ปรับปรุงสำหรับ 8 บิต)
long long binaryOp(long long dec1, long long dec2, const char *op, bool *isNeg, bool *final_carry)
{
    int op_mode = 0;
    if (strcmp(op, "ADD") == 0)
    {
        op_mode = 0;
    }
    else if (strcmp(op, "SUB") == 0)
    {
        op_mode = 1;
    }
    else
    {
        printf("[ERROR] คำสั่งไม่ถูกต้อง\n");
        return 0;
    }

    // *** ใช้ NUM_BITS (32) ตามที่กำหนดไว้ ***
    char *num1 = decimalToBinary(dec1);
    char *num2 = decimalToBinary(dec2);

    if (!num1 || !num2)
    {
        if (num1)
            free(num1);
        if (num2)
            free(num2);
        printf("[ERROR] Memory allocation failed\n");
        return 0;
    }

    // ไม่จำเป็นต้องใช้ result_bin ในการคำนวณแล้ว แต่ใช้เพื่อแสดงผล
    char *result_bin = (char *)malloc(sizeof(char) * (NUM_BITS + 1));
    if (!result_bin)
    {
        free(num1);
        free(num2);
        printf("[ERROR] Memory allocation failed\n");
        return 0;
    }
    result_bin[NUM_BITS] = '\0';

    int carry_in = (op_mode == 1) ? 1 : 0;
    int alu_result_sum = 0, alu_carry_sum = 0;
    long long finalDecimalResult = 0; // เพิ่มตัวแปรนี้เพื่อเก็บผลลัพธ์สุดท้าย

    for (int i = 0; i < NUM_BITS; i++)
    {
        int bitA = num1[NUM_BITS - 1 - i] - '0';
        int bitB = num2[NUM_BITS - 1 - i] - '0';

        printf("\n[STEP %d] กำลังคำนวณบิต: A=%d, B=%d, Carry-in=%d\n", i + 1, bitA, bitB, carry_in);

        int useB = bitB;
        if (op_mode == 1)
        {
            useB = 1 - bitB;
            printf("[INFO] ทำการ invert B ในซอฟต์แวร์: B_inv=%d\n", useB);
        }

        char command_add[32];
        snprintf(command_add, sizeof(command_add), "001 0 %d %d\n", bitA, useB);
        if (!sendAndReceiveData(command_add, &alu_result_sum, &alu_carry_sum))
        {
            printf("[ERROR] Failed to perform operation for bit %d\n", i);
            free(num1);
            free(num2);
            free(result_bin);
            return 0;
        }

        // ตรรกะ Full Adder ที่ถูกต้อง
        int sum_with_carry = alu_result_sum ^ carry_in;
        int new_carry = (alu_carry_sum) | (alu_result_sum & carry_in);

        // เก็บผลลัพธ์บิตลงในสตริงเพื่อแสดงผล (ไม่ใช่เพื่อการคำนวณ)
        result_bin[NUM_BITS - 1 - i] = sum_with_carry + '0';
        
        // **นี่คือส่วนสำคัญที่แก้ไข: การรวมผลลัพธ์บิต**
        // หากบิตผลลัพธ์เป็น 1 ให้บวกค่าของบิตนั้นๆ เข้าไปใน finalDecimalResult
        if (sum_with_carry == 1) {
            finalDecimalResult = finalDecimalResult | (1LL << i);
        }

        carry_in = new_carry;

        printf("[INFO] ALU half-add: halfSum=%d halfCarry=%d -> sum=%d carry=%d\n",
                alu_result_sum, alu_carry_sum, sum_with_carry, new_carry);
    }

    free(num1);
    free(num2);
    free(result_bin);

    *final_carry = (carry_in == 1);
    
    // *** ปรับปรุง: ตรรกะการตรวจสอบค่าลบและ Carry Flag ***
    if (op_mode == 1) // สำหรับการลบ (Subtraction)
    {
        if (carry_in == 1)
        {
            *isNeg = false;
        }
        else
        {
            *isNeg = true;
        }
    }
    else // สำหรับการบวก (Addition)
    {
        // ตรวจสอบ MSB สำหรับ Signed 32-bit
        *isNeg = (finalDecimalResult >> (NUM_BITS - 1)) & 1;
        if (*isNeg)
        {
            // ทำการแปลง 2's complement เป็นค่าติดลบที่ถูกต้อง
            finalDecimalResult = (signed long long)finalDecimalResult;
        }
    }

    return finalDecimalResult;
}

// -----------------------------------------------------------
// ฟังก์ชันสำหรับเรียก Arduino CLI (ต้องอยู่ในไฟล์เดียวกัน)
// -----------------------------------------------------------
bool executeArduinoCLI(const char *cliPath, const char *board, const char *port, const char *inoPath)
{
    char commandLine[1024];
    snprintf(commandLine, sizeof(commandLine),
             "\"%s\" compile --upload -b %s -p %s \"%s\"",
             cliPath, board, port, inoPath);

    printf("[INFO] กำลังรันคำสั่ง Arduino CLI...\n");
    printf("[DEBUG] คำสั่ง: %s\n", commandLine);

    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi;

    if (!CreateProcessA(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        printf("[ERROR] CreateProcess ล้มเหลว รหัสข้อผิดพลาด: %lu\n", GetLastError());
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exitCode != 0)
    {
        printf("[ERROR] อัปโหลดโค้ด Arduino ล้มเหลว รหัสออก: %lu\n", exitCode);
        return false;
    }

    printf("[INFO] อัปโหลดโค้ด Arduino สำเร็จแล้ว\n");
    return true;
}

// -----------------------------------------------------------
// คำสั่งจำลองภาษา Assembly
// -----------------------------------------------------------
typedef struct
{
    char instruction[10];
    char operand1[50];
    char operand2[50];
    char label[20];
} Instruction;

// Global Flags
bool ZERO_FLAG = false;
bool SIGN_FLAG = false; // สำหรับบอกว่าผลลัพธ์เป็นค่าติดลบหรือไม่

// เพิ่มฟังก์ชัน helper เพื่อแปลงชื่อรีจิสเตอร์เป็นค่าตัวแปร
long long getRegisterValue(const char *regName)
{
    if (strcmp(regName, "REG_A") == 0)
    {
        return REG_A;
    }
    if (strcmp(regName, "REG_B") == 0)
    {
        return REG_B;
    }
    return 0;
}

// เพิ่มฟังก์ชัน helper เพื่อตั้งค่ารีจิสเตอร์
void setRegisterValue(const char *regName, long long value)
{
    if (strcmp(regName, "REG_A") == 0)
    {
        REG_A = value;
    }
    if (strcmp(regName, "REG_B") == 0)
    {
        REG_B = value;
    }
}

long long logicOp(long long op1, long long op2, const char *op, bool *zeroFlag, bool *signFlag)
{
    long long result = 0;
    if (strcmp(op, "AND") == 0)
    {
        result = op1 & op2;
    }
    else if (strcmp(op, "OR") == 0)
    {
        result = op1 | op2;
    }
    else if (strcmp(op, "XOR") == 0)
    {
        result = op1 ^ op2;
    }
    else if (strcmp(op, "NOT") == 0) // NOT เป็น unary op
    {
        result = ~op1;
    }

    *zeroFlag = (result == 0);
    *signFlag = (result < 0);
    return result;
}

long long shiftOp(long long value, int shiftCount, const char *op, bool *carryFlag, bool *zeroFlag, bool *signFlag)
{
    long long result = 0;
    if (strcmp(op, "SHL") == 0)
    {
        *carryFlag = (value >> (64 - shiftCount)) & 1;
        result = value << shiftCount;
    }
    else if (strcmp(op, "SHR") == 0)
    {
        *carryFlag = (value >> (shiftCount - 1)) & 1;
        result = value >> shiftCount;
    }
    *zeroFlag = (result == 0);
    *signFlag = (result < 0);
    return result;
}

#define STACK_SIZE 1024
long long STACK[STACK_SIZE];
int sp = -1; // Stack Pointer
// เพิ่มฟังก์ชัน PUSH และ POP
void push(long long value)
{
    if (sp < STACK_SIZE - 1)
    {
        STACK[++sp] = value;
        printf("[STACK] PUSH: %lld at stack[%d]\n", value, sp);
    }
    else
    {
        printf("[ERROR] Stack overflow!\n");
    }
}
long long pop()
{
    if (sp >= 0)
    {
        printf("[STACK] POP: %lld from stack[%d]\n", STACK[sp], sp);
        return STACK[sp--];
    }
    else
    {
        printf("[ERROR] Stack underflow!\n");
        return 0;
    }
}

long long getOperandValue(const char *operand)
{
    if (strcmp(operand, "REG_A") == 0)
    {
        return REG_A;
    }
    if (strcmp(operand, "REG_B") == 0)
    {
        return REG_B;
    }
    // ถ้าไม่ใช่ Register ให้ลองแปลงเป็นตัวเลข
    return atoll(operand);
}

typedef struct
{
    long long reg_a; // เปลี่ยนเป็น long long
    long long reg_b; // เปลี่ยนเป็น long long
    bool carry_flag;
    bool zero_flag;
    bool sign_flag;
} AluRegisters;

// แก้ไข prototype ของฟังก์ชันให้คืนค่าเป็น struct AluRegisters
AluRegisters executeInstructions(Instruction program[], int num_instructions)
{
    // สร้างแผนที่ Label -> Index เพื่อเพิ่มความเร็วในการ Jump
    struct LabelMap
    {
        char label[20];
        int index;
    };
    struct LabelMap labelMap[num_instructions];
    
    int labelCount = 0;

    printf("[INFO] กำลังสร้างแผนที่ Label...\n");
    for (int i = 0; i < num_instructions; i++)
    {
        if (strlen(program[i].label) > 0)
        {
            if (labelCount < num_instructions)
            {
                strcpy(labelMap[labelCount].label, program[i].label);
                labelMap[labelCount].index = i;
                printf("[DEBUG] พบ Label: '%s' ที่บรรทัด %d\n", labelMap[labelCount].label, labelMap[labelCount].index);
                labelCount++;
            }
        }
    }

    int pc = 0; // Program Counter
    while (pc < num_instructions)
    {
        Instruction current = program[pc];
        bool shouldJump = false;

        printf("\n[PC:%02d] กำลังรันคำสั่ง: %s", pc, current.instruction);
        if (strlen(current.operand1) > 0)
            printf(" %s", current.operand1);
        if (strlen(current.operand2) > 0)
            printf(" %s", current.operand2);
        printf("\n");

        if (strlen(current.label) > 0)
        {
            // Do nothing, just a label
        }
        else if (strcmp(current.instruction, "DEF") == 0)
        {
            int mem_addr = atoi(current.operand1);
            long long value = atoll(current.operand2);
            if (mem_addr >= 0 && mem_addr < 10)
            {
                MEMORY[mem_addr] = value;
                printf("[INFO] DEFINE: กำหนดค่า %lld ให้กับ MEMORY[%d]\n", value, mem_addr);
            }
            else
            {
                printf("[ERROR] Memory Out-of-Bounds: ไม่สามารถเข้าถึง MEMORY[%d] ได้\n", mem_addr);
            }
        }
        else if (strcmp(current.instruction, "LOAD") == 0)
        {
            int mem_addr = atoi(current.operand2);
            if (mem_addr >= 0 && mem_addr < 10)
            {
                if (strcmp(current.operand1, "REG_A") == 0)
                {
                    REG_A = MEMORY[mem_addr];
                    printf("[INFO] LOAD %lld จาก MEMORY[%d] เข้าสู่ REG_A\n", REG_A, mem_addr);
                }
                else if (strcmp(current.operand1, "REG_B") == 0)
                {
                    REG_B = MEMORY[mem_addr];
                    printf("[INFO] LOAD %lld จาก MEMORY[%d] เข้าสู่ REG_B\n", REG_B, mem_addr);
                }
            }
            else
            {
                printf("[ERROR] Memory Out-of-Bounds: ไม่สามารถเข้าถึง MEMORY[%d] ได้\n", mem_addr);
            }
        }
        else if (strcmp(current.instruction, "STORE") == 0)
        {
            int mem_addr = atoi(current.operand2);
            if (mem_addr >= 0 && mem_addr < 10)
            {
                if (strcmp(current.operand1, "REG_A") == 0)
                {
                    MEMORY[mem_addr] = REG_A;
                    printf("[INFO] STORE %lld จาก REG_A ไปยัง MEMORY[%d]\n", REG_A, mem_addr);
                }
                else if (strcmp(current.operand1, "REG_B") == 0)
                {
                    MEMORY[mem_addr] = REG_B;
                    printf("[INFO] STORE %lld จาก REG_B ไปยัง MEMORY[%d]\n", REG_B, mem_addr);
                }
            }
            else
            {
                printf("[ERROR] Memory Out-of-Bounds: ไม่สามารถเข้าถึง MEMORY[%d] ได้\n", mem_addr);
            }
        }
        else if (strcmp(current.instruction, "MOV") == 0)
        {
            long long value = getOperandValue(current.operand2);
            setRegisterValue(current.operand1, value);
            printf("[INFO] MOV: ย้ายค่า %lld ไปยัง %s\n", value, current.operand1);
        }
        else if (strcmp(current.instruction, "ADD") == 0)
        {
            long long op1_val = getRegisterValue(current.operand1);
            long long op2_val = getOperandValue(current.operand2);
            printf("[INFO] กำลังประมวลผล %s + %s\n", current.operand1, current.operand2);
            bool isNeg = false;
            long long result = binaryOp(op1_val, op2_val, "ADD", &isNeg, &CARRY_FLAG);
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            printf("[INFO] ผลลัพธ์ ADD: %s = %lld, CARRY_FLAG = %s, ZERO_FLAG = %s, SIGN_FLAG = %s\n",
                   current.operand1, result, CARRY_FLAG ? "true" : "false", ZERO_FLAG ? "true" : "false", SIGN_FLAG ? "true" : "false");
        }
        else if (strcmp(current.instruction, "SUB") == 0)
        {
            long long op1_val = getRegisterValue(current.operand1);
            long long op2_val = getOperandValue(current.operand2);
            printf("[INFO] กำลังประมวลผล %s - %s\n", current.operand1, current.operand2);
            bool isNeg = false;
            long long result = binaryOp(op1_val, op2_val, "SUB", &isNeg, &CARRY_FLAG);
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = isNeg;
            printf("[INFO] ผลลัพธ์ SUB: %s = %lld, CARRY_FLAG = %s, ZERO_FLAG = %s, SIGN_FLAG = %s\n",
                   current.operand1, result, CARRY_FLAG ? "true" : "false", ZERO_FLAG ? "true" : "false", SIGN_FLAG ? "true" : "false");
        }
        else if (strcmp(current.instruction, "CMP") == 0)
        {
            long long op1_val = getOperandValue(current.operand1);
            long long op2_val = getOperandValue(current.operand2);
            long long result = op1_val - op2_val;
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            CARRY_FLAG = (op1_val < op2_val);
            printf("[INFO] CMP: %lld กับ %lld. Z=%s, S=%s, C=%s\n",
                   op1_val, op2_val, ZERO_FLAG ? "true" : "false", SIGN_FLAG ? "true" : "false", CARRY_FLAG ? "true" : "false");
        }
        else if (strcmp(current.instruction, "JMP") == 0)
        {
            char *label = current.operand1;
            for (int i = 0; i < labelCount; i++)
            {
                if (strcmp(labelMap[i].label, label) == 0)
                {
                    pc = labelMap[i].index;
                    shouldJump = true;
                    printf("[INFO] คำสั่ง JMP. กระโดดไปที่ Label '%s' (บรรทัด %d)\n", label, pc);
                    break;
                }
            }
        }
        else if (strcmp(current.instruction, "JNC") == 0)
        {
            if (!CARRY_FLAG)
            {
                char *label = current.operand1;
                for (int i = 0; i < labelCount; i++)
                {
                    if (strcmp(labelMap[i].label, label) == 0)
                    {
                        pc = labelMap[i].index;
                        shouldJump = true;
                        printf("[INFO] คำสั่ง JNC. กระโดดไปที่ Label '%s' (บรรทัด %d)\n", label, pc);
                        break;
                    }
                }
            }
            else
            {
                printf("[INFO] คำสั่ง JNC. CARRY_FLAG = true. ไม่มีการกระโดด.\n");
            }
        }
        else if (strcmp(current.instruction, "JZ") == 0)
        {
            if (ZERO_FLAG)
            {
                char *label = current.operand1;
                for (int i = 0; i < labelCount; i++)
                {
                    if (strcmp(labelMap[i].label, label) == 0)
                    {
                        pc = labelMap[i].index;
                        shouldJump = true;
                        printf("[INFO] คำสั่ง JZ. กระโดดไปที่ Label '%s' (บรรทัด %d)\n", label, pc);
                        break;
                    }
                }
            }
            else
            {
                printf("[INFO] คำสั่ง JZ. ZERO_FLAG = false. ไม่มีการกระโดด.\n");
            }
        }
        else if (strcmp(current.instruction, "JNZ") == 0)
        {
            if (!ZERO_FLAG)
            {
                char *label = current.operand1;
                for (int i = 0; i < labelCount; i++)
                {
                    if (strcmp(labelMap[i].label, label) == 0)
                    {
                        pc = labelMap[i].index;
                        shouldJump = true;
                        printf("[INFO] คำสั่ง JNZ. กระโดดไปที่ Label '%s' (บรรทัด %d)\n", label, pc);
                        break;
                    }
                }
            }
            else
            {
                printf("[INFO] คำสั่ง JNZ. ZERO_FLAG = true. ไม่มีการกระโดด.\n");
            }
        }
        else if (strcmp(current.instruction, "HLT") == 0)
        {
            printf("[INFO] คำสั่ง HLT. หยุดการทำงาน.\n");
            break;
        }

        if (!shouldJump)
        {
            pc++;
        }
    }

    // โค้ดที่ต้องเพิ่มและแก้ไข
    printf("\n[INFO] การทำงานเสร็จสมบูรณ์. กำลังส่งคืนค่าจาก Register...\n");

    AluRegisters result = {
        .reg_a = REG_A,
        .reg_b = REG_B};

    return result; // คืนค่า struct ที่มีทั้ง REG_A และ REG_B
}
// -----------------------------------------------------------
// Main
// -----------------------------------------------------------
// >>>> โค้ดไปอยู่ที่ main.c