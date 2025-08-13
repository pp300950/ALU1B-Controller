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
// ฟังก์ชันหลักสำหรับจำลองการคำนวณ (ปรับปรุงสำหรับ 8 บิต)
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

    // *** ปรับปรุง: ใช้ NUM_BITS (8) แทนการคำนวณความยาวแบบ Dynamic ***
    char *num1 = decimalToBinary(dec1);
    char *num2 = decimalToBinary(dec2);

    if (!num1 || !num2)
    {
        if (num1) free(num1);
        if (num2) free(num2);
        printf("[ERROR] Memory allocation failed\n");
        return 0;
    }

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

        int sum_with_carry = alu_result_sum ^ carry_in;
        int new_carry = (alu_carry_sum) | (alu_result_sum & carry_in);

        result_bin[NUM_BITS - 1 - i] = sum_with_carry + '0';
        carry_in = new_carry;

        printf("[INFO] ALU half-add: halfSum=%d halfCarry=%d -> sum=%d carry=%d\n",
               alu_result_sum, alu_carry_sum, sum_with_carry, new_carry);
    }

    free(num1);
    free(num2);

    *final_carry = (carry_in == 1);
    
    long long finalDecimalResult = binaryToDecimal(result_bin);
    free(result_bin);

    // *** ปรับปรุง: ตรรกะการตรวจสอบค่าลบและ Carry Flag ***
    if (op_mode == 1) // สำหรับการลบ (Subtraction)
    {
        if (carry_in == 1) { // ถ้ามี borrow (carry-in เป็น 1) แสดงว่าผลลัพธ์เป็นบวก
            *isNeg = false;
        } else { // ถ้าไม่มี borrow แสดงว่าผลลัพธ์เป็นลบ
            *isNeg = true;
        }
    }
    else // สำหรับการบวก (Addition)
    {
        *isNeg = (finalDecimalResult > 127); // ตรวจสอบ Signed 8-bit (MSB=1 คือติดลบ)
        if (*isNeg) {
            // Logic สำหรับการแปลง 2's complement เป็นค่าติดลบที่ถูกต้อง
            finalDecimalResult = (signed char)finalDecimalResult;
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
void executeInstructions(Instruction *instructions, int numInstructions)
{
    // สร้างแผนที่ Label -> Index เพื่อเพิ่มความเร็วในการ Jump
    struct LabelMap
    {
        char label[20];
        int index;
    };
    struct LabelMap labelMap[numInstructions];
    int labelCount = 0;

    printf("[INFO] กำลังสร้างแผนที่ Label...\n");
    for (int i = 0; i < numInstructions; i++)
    {
        if (strlen(instructions[i].label) > 0)
        {
            if (labelCount < numInstructions)
            {
                strcpy(labelMap[labelCount].label, instructions[i].label);
                labelMap[labelCount].index = i;
                printf("[DEBUG] พบ Label: '%s' ที่บรรทัด %d\n", labelMap[labelCount].label, labelMap[labelCount].index);
                labelCount++;
            }
        }
    }

    int pc = 0; // Program Counter
    while (pc < numInstructions)
    {
        Instruction current = instructions[pc];
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
        // --- เพิ่มใหม่: เพิ่มการจัดการคำสั่ง PRINT ---
        else if (strcmp(current.instruction, "PRINT") == 0)
        {
            long long valueToPrint = getOperandValue(current.operand2);
            // operand1 คือข้อความที่ต้องการให้แสดงผล, operand2 คือ register ที่จะแสดงค่า
            printf("\n>>> [OUTPUT] %s: %lld <<<\n", current.operand1, valueToPrint);
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
}

// --- เพิ่มใหม่: ส่วนของคอมไพเลอร์ภาษาขั้นสูง ---

// โครงสร้างสำหรับเก็บข้อมูลตัวแปร (Symbol Table)
typedef struct {
    char name[50];
    int mem_addr;
} Variable;

// ฟังก์ชันสำหรับค้นหาตัวแปรใน Symbol Table
int findVariable(const char* name, Variable* table, int count) {
    for (int i = 0; i < count; i++) {
        if (strcmp(name, table[i].name) == 0) {
            return table[i].mem_addr;
        }
    }
    return -1; // ไม่พบ
}

// ฟังก์ชันหลักของ Compiler: แปลงโค้ดขั้นสูงเป็นชุดคำสั่ง Assembly
Instruction* parseAndGenerateInstructions(const char** highLevelCode, int numLines, int* outNumInstructions) {
    // ประมาณการว่า 1 บรรทัดโค้ดขั้นสูงจะสร้างได้สูงสุด 5 คำสั่ง Assembly
    Instruction* generatedInstructions = (Instruction*)malloc(sizeof(Instruction) * numLines * 5);
    if (!generatedInstructions) {
        printf("[COMPILER ERROR] ไม่สามารถจองหน่วยความจำสำหรับคำสั่งได้\n");
        return NULL;
    }

    Variable symbolTable[50]; // รองรับตัวแปรได้ 50 ตัว
    int variableCount = 0;
    int nextMemAddr = 0;
    int instructionCount = 0;

    printf("\n--- เริ่มการแปลภาษา (Compiling) ---\n");

    for (int i = 0; i < numLines; i++) {
        const char* line = highLevelCode[i];
        char varName[50];
        int value;

        // รูปแบบที่ 1: การประกาศตัวแปร int var = value;
        if (sscanf(line, "int %s = %d;", varName, &value) == 2) {
            printf("[COMPILER] กำลังประมวลผล: ประกาศตัวแปร '%s' = %d\n", varName, value);
            
            // ตรวจสอบว่ามีตัวแปรชื่อนี้แล้วหรือยัง
            if (findVariable(varName, symbolTable, variableCount) != -1) {
                printf("[COMPILER ERROR] ตัวแปร '%s' ถูกประกาศไปแล้ว\n", varName);
                free(generatedInstructions);
                return NULL;
            }

            // เพิ่มตัวแปรใหม่ลงใน Symbol Table
            strcpy(symbolTable[variableCount].name, varName);
            symbolTable[variableCount].mem_addr = nextMemAddr;
            variableCount++;

            // สร้างคำสั่ง DEF <mem_addr> <value>
            Instruction inst;
            strcpy(inst.instruction, "DEF");
            sprintf(inst.operand1, "%d", nextMemAddr); // แปลง int เป็น string
            sprintf(inst.operand2, "%d", value);
            strcpy(inst.label, "");
            generatedInstructions[instructionCount++] = inst;
            
            nextMemAddr++;
            continue;
        }

        // รูปแบบที่ 2: การสั่งพิมพ์ print(var);
        if (sscanf(line, "print(%[^)]);", varName) == 1) {
             printf("[COMPILER] กำลังประมวลผล: พิมพ์ค่าตัวแปร '%s'\n", varName);

            int mem_addr = findVariable(varName, symbolTable, variableCount);
            if (mem_addr == -1) {
                printf("[COMPILER ERROR] ไม่พบตัวแปรชื่อ '%s'\n", varName);
                free(generatedInstructions);
                return NULL;
            }

            // 1. สร้างคำสั่ง LOAD REG_A, <mem_addr>
            Instruction load_inst;
            strcpy(load_inst.instruction, "LOAD");
            strcpy(load_inst.operand1, "REG_A");
            sprintf(load_inst.operand2, "%d", mem_addr);
            strcpy(load_inst.label, "");
            generatedInstructions[instructionCount++] = load_inst;

            // 2. สร้างคำสั่ง PRINT "Message", REG_A
            Instruction print_inst;
            strcpy(print_inst.instruction, "PRINT");
            sprintf(print_inst.operand1, "Value of %s", varName); // สร้างข้อความแสดงผล
            strcpy(print_inst.operand2, "REG_A");
            strcpy(print_inst.label, "");
            generatedInstructions[instructionCount++] = print_inst;

            continue;
        }
        
        printf("[COMPILER WARNING] ไม่รู้จักรูปแบบคำสั่ง: %s\n", line);
    }
    
    // เพิ่มคำสั่ง HLT (Halt) เพื่อจบการทำงาน
    Instruction hlt_inst;
    strcpy(hlt_inst.instruction, "HLT");
    strcpy(hlt_inst.operand1, "");
    strcpy(hlt_inst.operand2, "");
    strcpy(hlt_inst.label, "");
    generatedInstructions[instructionCount++] = hlt_inst;

    printf("--- การแปลภาษาเสร็จสิ้น! สร้างได้ %d คำสั่ง ---\n", instructionCount);
    *outNumInstructions = instructionCount;
    return generatedInstructions;
}


// -----------------------------------------------------------
// Main
// -----------------------------------------------------------
int main()
{
    SetConsoleOutputCP(CP_UTF8);

    const char *arduinoCliPath = "C:\\Users\\Administrator\\Desktop\\arduino-cli.exe";
    const char *boardType = "arduino:avr:uno";
    const char *inoFilePath = "C:\\Users\\Administrator\\Desktop\\ALU4B-Controller/ALU4B-Controller/ALU4B-Controller.ino";

    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    // ปิดการทำงานส่วนนี้ไปก่อนเพื่อทดสอบ Compiler
    /*
    if (!executeArduinoCLI(arduinoCliPath, boardType, COM_PORT, inoFilePath))
    {
        return 1;
    }

    printf("[INFO] กำลังรอให้บอร์ดพร้อมใช้งานเป็นเวลา %d มิลลิวินาที...\n", UPLOAD_WAIT_MS);
    Sleep(UPLOAD_WAIT_MS);

    hSerial = openAndSetupSerialPort();
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        return 1;
    }
    */

    printf("\n--- เริ่มต้นการทำงานของโปรแกรมจำลอง ---\n");

    // --- เพิ่มใหม่: เขียนโปรแกรมด้วยภาษาขั้นสูง ---
    const char* highLevelProgram[] = {
        "int Num = 50;",
        "int VarX = 123;",
        "print(Num);",
        "print(VarX);"
    };
    int numHighLevelLines = sizeof(highLevelProgram) / sizeof(const char*);
    
    // --- เพิ่มใหม่: เรียกใช้ Compiler ---
    int numGeneratedInstructions = 0;
    Instruction* program = parseAndGenerateInstructions(highLevelProgram, numHighLevelLines, &numGeneratedInstructions);

    if (program != NULL) {
        // --- เพิ่มใหม่: รันโปรแกรมที่ได้จากการ Compile ---
        executeInstructions(program, numGeneratedInstructions);
        free(program); // คืนหน่วยความจำที่จองไว้
    }


    printf("\n--- สิ้นสุดการทำงานของโปรแกรม ---\n");
    
    // ปิดการเชื่อมต่อพอร์ตอนุกรมถ้ามีการเปิด
    if (hSerial != INVALID_HANDLE_VALUE) {
        clearSerialBuffer();
        CloseHandle(hSerial);
        printf("[DEBUG] ปิดพอร์ตซีเรียลแล้ว เย่ๆ\n");
    }

    return 0;
}
