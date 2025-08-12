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

HANDLE hSerial = INVALID_HANDLE_VALUE;

// --- Global Registers and Memory (Hard-coded) ---
// เราจะใช้ตัวแปรเหล่านี้จำลองการเป็น Register และหน่วยความจำ
long long REG_A = 0;     // Accumulator Register
long long REG_B = 0;     // General Purpose Register
long long MEMORY[10];    // Simple Memory Array
bool CARRY_FLAG = false; // Carry Flag

// ฟังก์ชันสำหรับแปลงค่าเลขฐานสิบเป็นสตริงฐานสอง
char *decimalToBinary(long long decimal)
{
    if (decimal == 0)
    {
        char *binStr = (char *)malloc(2);
        strcpy(binStr, "0");
        return binStr;
    }

    int numBits = 0;
    long long temp = decimal;
    if (decimal < 0)
        temp = -decimal;
    while (temp > 0)
    {
        numBits++;
        temp >>= 1;
    }

    char *binaryString = (char *)malloc(numBits + 1);
    if (!binaryString)
        return NULL;
    binaryString[numBits] = '\0';

    long long value = (decimal < 0) ? -decimal : decimal;
    for (int i = numBits - 1; i >= 0; i--)
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
    for (int i = 0; i < length; i++)
    {
        if (binaryString[length - 1 - i] == '1')
        {
            decimalValue += pow(2, i);
        }
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

    size_t len1 = strlen(num1);
    size_t len2 = strlen(num2);
    size_t maxLen = (len1 > len2) ? len1 : len2;

    char *paddedNum1 = padBinaryString(num1, maxLen);
    char *paddedNum2 = padBinaryString(num2, maxLen);

    free(num1);
    free(num2);

    if (!paddedNum1 || !paddedNum2)
    {
        if (paddedNum1)
            free(paddedNum1);
        if (paddedNum2)
            free(paddedNum2);
        printf("[ERROR] Memory allocation failed\n");
        return 0;
    }

    char *result_bin = (char *)malloc(sizeof(char) * (maxLen + 2));
    if (!result_bin)
    {
        free(paddedNum1);
        free(paddedNum2);
        printf("[ERROR] Memory allocation failed\n");
        return 0;
    }
    result_bin[maxLen + 1] = '\0';

    int carry_in = (op_mode == 1) ? 1 : 0;
    int alu_result_sum = 0, alu_carry_sum = 0;

    for (int i = 0; i < (int)maxLen; i++)
    {
        int bitA = paddedNum1[maxLen - 1 - i] - '0';
        int bitB = paddedNum2[maxLen - 1 - i] - '0';

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
            free(paddedNum1);
            free(paddedNum2);
            free(result_bin);
            return 0;
        }

        int sum_with_carry = alu_result_sum ^ carry_in;
        int new_carry = (alu_carry_sum) | (alu_result_sum & carry_in);

        result_bin[maxLen - i] = sum_with_carry + '0';
        carry_in = new_carry;

        printf("[INFO] ALU half-add: halfSum=%d halfCarry=%d -> sum=%d carry=%d\n",
               alu_result_sum, alu_carry_sum, sum_with_carry, new_carry);
    }

    free(paddedNum1);
    free(paddedNum2);

    *final_carry = (carry_in == 1);
    result_bin[0] = carry_in + '0';

    long long finalDecimalResult = 0;

    if (op_mode == 1)
    {
        if (carry_in == 1)
        {
            *isNeg = false;
            char temp_result[maxLen + 1];
            strncpy(temp_result, result_bin + 1, maxLen);
            temp_result[maxLen] = '\0';
            finalDecimalResult = binaryToDecimal(temp_result);
        }
        else
        {
            *isNeg = true;
            char *magnitude = (char *)malloc(sizeof(char) * (maxLen + 1));
            for (size_t i = 1; i <= maxLen; i++)
            {
                magnitude[i - 1] = (result_bin[i] == '0') ? '1' : '0';
            }
            magnitude[maxLen] = '\0';
            int carry = 1;
            for (int i = (int)maxLen - 1; i >= 0; i--)
            {
                int bit = magnitude[i] - '0';
                int sum = bit ^ carry;
                carry = bit & carry;
                magnitude[i] = sum + '0';
            }
            finalDecimalResult = binaryToDecimal(magnitude);
            free(magnitude);
        }
    }
    else
    {
        *isNeg = false;
        char temp_result[maxLen + 1];
        strncpy(temp_result, result_bin + 1, maxLen);
        temp_result[maxLen] = '\0';
        finalDecimalResult = binaryToDecimal(temp_result);
    }

    free(result_bin);
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

    // Call the function here
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

    printf("\n--- เริ่มต้นการทำงานของโปรแกรมจำลอง Assembly ---\n");

    // ตัวอย่างโปรแกรมจำลอง
    /*  Instruction program[] = {
          // กำหนดค่าเริ่มต้นให้กับ Memory
           {"DEF", "0", "50"}, // Memory[0] = 50
           {"DEF", "1", "45"}, // Memory[1] = 45
           {"DEF", "2", "1"},  // Memory[2] = 1

           // ทำการคำนวณ (50 - 45) + 1
           {"LOAD", "REG_A", "0"}, // REG_A = Memory[0] (50)
           {"LOAD", "REG_B", "1"}, // REG_B = Memory[1] (45)
           {"SUB", "REG_A", "REG_B"},
           {"PRINT", "ผลลัพธ์ SUB", "REG_A"}, // เปลี่ยนให้ operand2 เป็นชื่อรีจิสเตอร์
           {"ADD", "REG_A", "REG_B"},
           {"PRINT", "ผลลัพธ์ ADD", "REG_A"}, // เปลี่ยนให้ operand2 เป็นชื่อรีจิสเตอร์
           {"HLT", "", ""}};
           */

    Instruction program[] = {
        // --- การกำหนดค่าเริ่มต้น ---
        {"DEF", "0", "5"},      // Memory[0] = 5
        {"DEF", "1", "3"},      // Memory[1] = 3
        {"DEF", "2", "10"},     // Memory[2] = 10
        {"LOAD", "REG_A", "0"}, // REG_A = 5
        {"LOAD", "REG_B", "1"}, // REG_B = 3

        // --- ทดสอบ ADD ---
        {"ADD", "REG_A", "REG_B"}, // REG_A = 5 + 3 = 8

        // --- ทดสอบ STORE ---
        {"STORE", "2", "REG_A"}, // Memory[2] = 8

        // --- ทดสอบ MOV ---
        {"MOV", "REG_C", "REG_B"}, // REG_C = REG_B (3)

        // --- ทดสอบ CMP และ JZ ---
        {"CMP", "REG_A", "REG_C"}, // 8 vs 3 → ไม่เท่ากัน → ZERO_FLAG = 0
        {"JZ", "EQUAL_LABEL", ""}, // ข้ามไปไม่ได้เพราะไม่เท่ากัน

        // --- ทดสอบ SUB ---
        {"SUB", "REG_A", "REG_B"}, // 8 - 3 = 5

        // --- ทดสอบ JNC (ไม่มี borrow → CARRY_FLAG = 0) ---
        {"JNC", "NO_CARRY_LABEL", ""},

        // --- Label เมื่อมี carry (จะไม่โดน jump ในรอบนี้) ---
        {"", "", "", "EQUAL_LABEL"},
        {"MOV", "REG_D", "REG_C"}, // แค่ตัวอย่าง (จะไม่ถูก execute ในรอบนี้)
        {"JMP", "AFTER_TESTS", ""},

        // --- Label สำหรับ JNC ---
        {"", "", "", "NO_CARRY_LABEL"},
        {"ADD", "REG_A", "REG_B"}, // 5 + 3 = 8 อีกครั้ง

        // --- ทดสอบ JNZ ---
        {"CMP", "REG_A", "REG_C"}, // 8 vs 3 → ไม่เท่ากัน → ZERO_FLAG = 0
        {"JNZ", "CONTINUE_LABEL", ""},

        {"MOV", "REG_D", "REG_A"}, // จะไม่ถูก execute เพราะ ZERO_FLAG != 1

        // --- Label สำหรับ JNZ ---
        {"", "", "", "CONTINUE_LABEL"},
        {"SUB", "REG_A", "REG_A"}, // REG_A = 0
        {"CMP", "REG_A", "REG_A"}, // 0 vs 0 → ZERO_FLAG = 1
        {"JZ", "ZERO_LABEL", ""},

        // --- ถ้าไม่ zero ---
        {"MOV", "REG_D", "REG_B"},

        // --- Label ZERO ---
        {"", "", "", "ZERO_LABEL"},
        {"MOV", "REG_D", "REG_A"}, // REG_D = 0

        // --- ทดสอบ JMP ตรง ๆ ---
        {"JMP", "END_LABEL", ""},

        {"", "", "", "AFTER_TESTS"},
        {"MOV", "REG_D", "REG_C"},

        // --- จุดจบโปรแกรม ---
        {"", "", "", "END_LABEL"},
        {"HLT", "", ""}};

    int numInstructions = sizeof(program) / sizeof(Instruction);
    executeInstructions(program, numInstructions);

    printf("\n--- สิ้นสุดการทำงานของโปรแกรม ---\n");

    clearSerialBuffer();
    CloseHandle(hSerial);
    printf("[DEBUG] ปิดพอร์ตซีเรียลแล้ว เย่ๆ\n");

    return 0;
}