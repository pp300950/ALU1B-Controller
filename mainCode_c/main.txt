#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <locale.h>

// ===================================================================================
// 1. DEFINITIONS & GLOBAL VARIABLES
// ===================================================================================

#define COM_PORT "COM3"
#define BAUD_RATE CBR_9600
#define UPLOAD_WAIT_MS 3000
#define READ_TIMEOUT_MS 1000
#define MAX_READ_BUFFER 256
#define MAX_VARS 100
#define MAX_LABELS 50
#define MAX_LINES 200
#define MAX_BINARY_LEN 128 // ความยาวสูงสุดของเลขฐานสอง

HANDLE hSerial = INVALID_HANDLE_VALUE;
typedef struct
{
    char name[32];
    char value[MAX_BINARY_LEN];
} Variable;
typedef struct
{
    char name[32];
    int line_number;
} Label;
static Variable vars[MAX_VARS];
static int varCount = 0;
static Label labels[MAX_LABELS];
static int labelCount = 0;
static int comparison_flag = 0;

// ===================================================================================
// 2. FUNCTION PROTOTYPES
// ===================================================================================

// --- Serial & System ---
void clearSerialBuffer();
BOOL WINAPI ConsoleHandler(DWORD dwCtrlType);
HANDLE openAndSetupSerialPort();
int sendAndReceiveData(const char *dataToSend);
bool executeArduinoCLI(const char *cliPath, const char *board, const char *port, const char *inoPath);

// --- Scripting Engine ---
const char *getVar(const char *name);
void setVar(const char *name, const char *value);
const char *resolve_argument(const char *arg);
int getLabelLine(const char *name);

// --- *** HARDWARE-BACKED ALU FUNCTIONS (REIMPLEMENTED) *** ---
int alu_execute_bit(const char *muxCode, int subAdd, int aBit, int bBit);
void alu_add_binary(const char *binA, const char *binB, char *result, size_t maxLen);
void alu_sub_binary(const char *binA, const char *binB, char *result, size_t maxLen);
void alu_not_binary(const char *binIn, char *result, size_t maxLen);
int alu_compare_binary(const char *binA, const char *binB);
int alu_process_instruction(const char *instruction);
void runScript(FILE *f);

// --- Helpers ---
long binToLong(const char *bin);
void pad_binary_strings(const char *inA, const char *inB, char *outA, char *outB, int len);
void strreverse(char *str);

// ===================================================================================
// 3. MAIN FUNCTION
// ===================================================================================
int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    const char *arduinoCliPath = "C:\\Users\\Administrator\\Desktop\\arduino-cli.exe";
    const char *boardType = "arduino:avr:uno";
    // *** สำคัญ: ต้องอัปโหลดโค้ด .ino ที่แก้ไขแล้ว ***
    const char *inoFilePath = "C:\\Path\\To\\Your\\ALU_Controller_Simple.ino"; // <--- แก้ไข path นี้

    if (!executeArduinoCLI(arduinoCliPath, boardType, COM_PORT, inoFilePath))
    {
        printf("\nกด Enter เพื่อปิดโปรแกรม...");
        getchar();
        return 1;
    }

    printf("[INFO] กำลังรอให้บอร์ดพร้อมใช้งานเป็นเวลา %d มิลลิวินาที...\n", UPLOAD_WAIT_MS);
    Sleep(UPLOAD_WAIT_MS);

    hSerial = openAndSetupSerialPort();
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        printf("\nกด Enter เพื่อปิดโปรแกรม...");
        getchar();
        return 1;
    }

    const char *script_filename = "script.txt";
    FILE *f = fopen(script_filename, "w");
    if (f)
    {
        fprintf(f, "# Script ตัวอย่าง\n");
        fprintf(f, "VAR N 1010     # N = 10\n");
        fprintf(f, "VAR X 1        # X = 1\n");
        fprintf(f, "VAR ONE 1      # ตัวแปรสำหรับเลข 1\n");
        fprintf(f, "LOOP:\n");
        fprintf(f, "PRINT X\n");
        fprintf(f, "ADD X ONE      # X = X + 1\n");
        fprintf(f, "CMP X N        # เปรียบเทียบ X กับ N\n");
        fprintf(f, "JLE LOOP       # ถ้า X <= N ให้วนลูป\n");
        fprintf(f, "PRINT X        # พิมพ์ผลลัพธ์สุดท้าย\n");
        fclose(f);
    }

    f = fopen(script_filename, "r");
    if (!f)
    {
        perror("Could not open script file");
        if (hSerial != INVALID_HANDLE_VALUE)
            CloseHandle(hSerial);
        return 1;
    }

    printf("\n---------- STARTING SCRIPT EXECUTION ----------\n");
    runScript(f);
    printf("----------- SCRIPT EXECUTION FINISHED ----------\n\n");
    fclose(f);

    clearSerialBuffer();
    CloseHandle(hSerial);
    printf("[DEBUG] ปิดพอร์ตซีเรียลแล้ว\n");

    return 0;
}

// ===================================================================================
// 4. FUNCTION IMPLEMENTATIONS
// ===================================================================================

// --- Serial & System Functions (No Changes) ---
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
        printf("\n[INFO] ตรวจพบ Ctrl+C. กำลังปิดโปรแกรม...\n");
        if (hSerial != INVALID_HANDLE_VALUE)
        {
            clearSerialBuffer();
            CloseHandle(hSerial);
        }
        ExitProcess(0);
    }
    return FALSE;
}
HANDLE openAndSetupSerialPort()
{
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts = {0};
    printf("[DEBUG] กำลังเปิดพอร์ตซีเรียล: %s\n", COM_PORT);
    hSerial = CreateFileA(COM_PORT, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "[ERROR] ไม่สามารถเปิดพอร์ตซีเรียล %s (รหัส: %lu)\n", COM_PORT, GetLastError());
        return INVALID_HANDLE_VALUE;
    }
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        fprintf(stderr, "[ERROR] GetCommState ล้มเหลว\n");
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }
    dcbSerialParams.BaudRate = BAUD_RATE;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if (!SetCommState(hSerial, &dcbSerialParams))
    {
        fprintf(stderr, "[ERROR] SetCommState ล้มเหลว\n");
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }
    timeouts.ReadIntervalTimeout = 100;
    timeouts.ReadTotalTimeoutConstant = READ_TIMEOUT_MS;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if (!SetCommTimeouts(hSerial, &timeouts))
    {
        fprintf(stderr, "[ERROR] SetCommTimeouts ล้มเหลว\n");
        CloseHandle(hSerial);
        return INVALID_HANDLE_VALUE;
    }
    return hSerial;
}
int sendAndReceiveData(const char *dataToSend)
{
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "[ERROR] Handle ซีเรียลไม่ถูกต้อง\n");
        return -1;
    }
    DWORD bytesWritten = 0;
    DWORD bytesRead = 0;
    char readBuffer[MAX_READ_BUFFER] = {0};
    clearSerialBuffer();
    if (!WriteFile(hSerial, dataToSend, strlen(dataToSend), &bytesWritten, NULL))
    {
        fprintf(stderr, "[ERROR] WriteFile ล้มเหลว (รหัส: %lu)\n", GetLastError());
        return -1;
    }
    if (!ReadFile(hSerial, readBuffer, MAX_READ_BUFFER - 1, &bytesRead, NULL))
    {
    }
    if (bytesRead > 0)
    {
        readBuffer[bytesRead] = '\0';
        return atoi(readBuffer);
    }
    else
    {
        fprintf(stderr, "[WARN] ไม่ได้รับข้อมูลตอบกลับ (Timeout)\n");
        return -1;
    }
}
bool executeArduinoCLI(const char *cliPath, const char *board, const char *port, const char *inoPath)
{
    char commandLine[1024];
    snprintf(commandLine, sizeof(commandLine), "\"%s\" compile --upload -b %s -p %s \"%s\"", cliPath, board, port, inoPath);
    printf("[INFO] กำลังรันคำสั่ง Arduino CLI...\n");
    printf("[DEBUG] คำสั่ง: %s\n", commandLine);
    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    if (!CreateProcessA(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        fprintf(stderr, "[ERROR] CreateProcess ล้มเหลว (รหัส: %lu)\n", GetLastError());
        return false;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    if (exitCode != 0)
    {
        fprintf(stderr, "[ERROR] อัปโหลดโค้ด Arduino ล้มเหลว (รหัสออก: %lu)\n", exitCode);
        return false;
    }
    printf("[INFO] อัปโหลดโค้ด Arduino สำเร็จแล้ว\n");
    return true;
}

// --- Scripting Engine Base Functions (No Changes)---
const char *resolve_argument(const char *arg)
{
    for (int i = 0; i < varCount; i++)
    {
        if (strcmp(vars[i].name, arg) == 0)
        {
            return vars[i].value;
        }
    }
    return arg;
}
const char *getVar(const char *name)
{
    for (int i = 0; i < varCount; i++)
    {
        if (strcmp(vars[i].name, name) == 0)
            return vars[i].value;
    }
    fprintf(stderr, "[WARN] Variable '%s' not found. Returning '0'.\n", name);
    return "0";
}
void setVar(const char *name, const char *value)
{
    for (int i = 0; i < varCount; i++)
    {
        if (strcmp(vars[i].name, name) == 0)
        {
            strncpy(vars[i].value, value, sizeof(vars[i].value) - 1);
            vars[i].value[sizeof(vars[i].value) - 1] = '\0';
            return;
        }
    }
    if (varCount < MAX_VARS)
    {
        strncpy(vars[varCount].name, name, sizeof(vars[varCount].name) - 1);
        vars[varCount].name[sizeof(vars[varCount].name) - 1] = '\0';
        strncpy(vars[varCount].value, value, sizeof(vars[varCount].value) - 1);
        vars[varCount].value[sizeof(vars[varCount].value) - 1] = '\0';
        varCount++;
    }
    else
    {
        fprintf(stderr, "[ERROR] Maximum variable limit reached.\n");
    }
}
int getLabelLine(const char *name)
{
    for (int i = 0; i < labelCount; i++)
    {
        if (strcmp(labels[i].name, name) == 0)
            return labels[i].line_number;
    }
    return -1;
}

// --- Helper Functions ---
long binToLong(const char *bin) { return strtol(bin, NULL, 2); }
void strreverse(char *begin)
{
    char *end = begin + strlen(begin) - 1;
    char aux;
    while (end > begin)
    {
        aux = *end;
        *end-- = *begin;
        *begin++ = aux;
    }
}
void pad_binary_strings(const char *inA, const char *inB, char *outA, char *outB, int len)
{
    int lenA = strlen(inA);
    int lenB = strlen(inB);
    int maxLen = (lenA > lenB) ? lenA : lenB;
    // Pad A
    for (int i = 0; i < maxLen - lenA; i++)
        outA[i] = '0';
    strcpy(outA + (maxLen - lenA), inA);
    // Pad B
    for (int i = 0; i < maxLen - lenB; i++)
        outB[i] = '0';
    strcpy(outB + (maxLen - lenB), inB);
}

// =============================================================================
// --- *** HARDWARE-BACKED ALU FUNCTION IMPLEMENTATIONS *** ---
// =============================================================================

// ติดต่อ ALU 1-bit บน Arduino
int alu_execute_bit(const char *muxCode, int subAdd, int aBit, int bBit)
{
    char command[32];
    snprintf(command, sizeof(command), "%s %d %d %d\n", muxCode, subAdd, aBit, bBit);
    int result = sendAndReceiveData(command);
    if (result < 0)
    {
        fprintf(stderr, "[ERROR] Failed to execute bit operation on hardware.\n");
        // ในกรณีที่เกิดข้อผิดพลาด ให้หยุดทำงานเพื่อป้องกันการคำนวณที่ผิดพลาด
        printf("กด Enter เพื่อปิดโปรแกรม...");
        getchar();
        exit(1);
    }
    return result;
}

// จำลองการบวกแบบหลายบิต (Ripple-Carry Adder) โดยใช้ Arduino เป็น Logic Gate
void alu_add_binary(const char *binA, const char *binB, char *result, size_t maxLen)
{
    char paddedA[MAX_BINARY_LEN], paddedB[MAX_BINARY_LEN];
    pad_binary_strings(binA, binB, paddedA, paddedB, MAX_BINARY_LEN);

    int len = strlen(paddedA);
    int carry = 0;
    char temp_result[MAX_BINARY_LEN] = {0};
    int res_idx = 0;

    // ประมวลผลจาก LSB ไป MSB (ขวาสุดไปซ้ายสุด)
    for (int i = len - 1; i >= 0; i--)
    {
        int bitA = paddedA[i] - '0';
        int bitB = paddedB[i] - '0';

        // Full-Adder Logic:
        // Sum = A XOR B XOR Carry_in
        // Carry_out = (A AND B) OR (Carry_in AND (A XOR B))

        // คำนวณ Sum
        int axorb = alu_execute_bit("001", 0, bitA, bitB); // MUX "001" คือ XOR ใน ALU ของเรา
        int sum_bit = alu_execute_bit("001", 0, axorb, carry);

        // คำนวณ Carry_out สำหรับรอบถัดไป
        int aandb = alu_execute_bit("000", 0, bitA, bitB); // MUX "000" คือ AND
        int cin_and_axorb = alu_execute_bit("000", 0, carry, axorb);
        carry = alu_execute_bit("110", 0, aandb, cin_and_axorb); // MUX "110" คือ OR

        temp_result[res_idx++] = sum_bit + '0';
    }

    // หากมีตัวทดสุดท้าย
    if (carry > 0)
    {
        temp_result[res_idx++] = '1';
    }
    temp_result[res_idx] = '\0';

    // กลับด้านสตริง (เพราะเราคำนวณจาก LSB)
    strreverse(temp_result);
    strncpy(result, temp_result, maxLen - 1);
    result[maxLen - 1] = '\0';
}

// กลับค่าทุกบิต (NOT) โดยใช้ Arduino
void alu_not_binary(const char *binIn, char *result, size_t maxLen)
{
    size_t len = strlen(binIn);
    if (len >= maxLen)
        len = maxLen - 1;
    for (size_t i = 0; i < len; i++)
    {
        int bit = binIn[i] - '0';
        // MUX "111" คือ NOT A
        result[i] = alu_execute_bit("111", 0, bit, 0) + '0';
    }
    result[len] = '\0';
}

// การลบแบบ 2's Complement: A - B = A + (NOT B) + 1
void alu_sub_binary(const char *binA, const char *binB, char *result, size_t maxLen)
{
    char notB[MAX_BINARY_LEN];
    char one[] = "1";
    char temp[MAX_BINARY_LEN];

    // 1. หา NOT B
    alu_not_binary(binB, notB, sizeof(notB));

    // 2. บวก 1 เข้าไปที่ NOT B
    // ===================================================================================
    // 4. FUNCTION IMPLEMENTATIONS (เวอร์ชันแก้ไข - เรียกใช้ Arduino)
    // ===================================================================================

    // --- Serial Communication & System Function Implementations ---
    // ... (ส่วนนี้เหมือนเดิมทั้งหมด ไม่มีการเปลี่ยนแปลง)...
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
            printf("\n[INFO] ตรวจพบ Ctrl+C. กำลังปิดโปรแกรม...\n");
            if (hSerial != INVALID_HANDLE_VALUE)
            {
                clearSerialBuffer();
                CloseHandle(hSerial);
            }
            ExitProcess(0);
        }
        return FALSE;
    }
    HANDLE openAndSetupSerialPort()
    {
        DCB dcbSerialParams = {0};
        COMMTIMEOUTS timeouts = {0};
        printf("[DEBUG] กำลังเปิดพอร์ตซีเรียล: %s\n", COM_PORT);
        hSerial = CreateFileA(COM_PORT, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hSerial == INVALID_HANDLE_VALUE)
        {
            fprintf(stderr, "[ERROR] ไม่สามารถเปิดพอร์ตซีเรียล %s (รหัส: %lu)\n", COM_PORT, GetLastError());
            return INVALID_HANDLE_VALUE;
        }
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
        if (!GetCommState(hSerial, &dcbSerialParams))
        {
            fprintf(stderr, "[ERROR] GetCommState ล้มเหลว\n");
            CloseHandle(hSerial);
            return INVALID_HANDLE_VALUE;
        }
        dcbSerialParams.BaudRate = BAUD_RATE;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;
        if (!SetCommState(hSerial, &dcbSerialParams))
        {
            fprintf(stderr, "[ERROR] SetCommState ล้มเหลว\n");
            CloseHandle(hSerial);
            return INVALID_HANDLE_VALUE;
        }
        timeouts.ReadIntervalTimeout = 100;
        timeouts.ReadTotalTimeoutConstant = READ_TIMEOUT_MS;
        timeouts.ReadTotalTimeoutMultiplier = 0;
        timeouts.WriteTotalTimeoutConstant = 50;
        timeouts.WriteTotalTimeoutMultiplier = 10;
        if (!SetCommTimeouts(hSerial, &timeouts))
        {
            fprintf(stderr, "[ERROR] SetCommTimeouts ล้มเหลว\n");
            CloseHandle(hSerial);
            return INVALID_HANDLE_VALUE;
        }
        return hSerial;
    }
    bool executeArduinoCLI(const char *cliPath, const char *board, const char *port, const char *inoPath)
    {
        char commandLine[1024];
        snprintf(commandLine, sizeof(commandLine), "\"%s\" compile --upload -b %s -p %s \"%s\"", cliPath, board, port, inoPath);
        printf("[INFO] กำลังรันคำสั่ง Arduino CLI...\n");
        printf("[DEBUG] คำสั่ง: %s\n", commandLine);
        STARTUPINFOA si = {sizeof(si)};
        PROCESS_INFORMATION pi;
        if (!CreateProcessA(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            fprintf(stderr, "[ERROR] CreateProcess ล้มเหลว (รหัส: %lu)\n", GetLastError());
            return false;
        }
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        if (exitCode != 0)
        {
            fprintf(stderr, "[ERROR] อัปโหลดโค้ด Arduino ล้มเหลว (รหัสออก: %lu)\n", exitCode);
            return false;
        }
        printf("[INFO] อัปโหลดโค้ด Arduino สำเร็จแล้ว\n");
        return true;
    }

    // *** [แก้ไข] ฟังก์ชันส่ง/รับข้อมูล ให้รับค่า Result และ Carry กลับมา ***
    int sendAndReceiveData(const char *dataToSend, int *resultBit, int *carryBit)
    {
        if (hSerial == INVALID_HANDLE_VALUE)
        {
            fprintf(stderr, "[ERROR] Handle ซีเรียลไม่ถูกต้อง\n");
            return 0; // Failure
        }
        DWORD bytesWritten = 0;
        DWORD bytesRead = 0;
        char readBuffer[MAX_READ_BUFFER] = {0};
        char command[64];
        snprintf(command, sizeof(command), "%s\n", dataToSend);

        clearSerialBuffer();

        if (!WriteFile(hSerial, command, strlen(command), &bytesWritten, NULL))
        {
            fprintf(stderr, "[ERROR] WriteFile ล้มเหลว (รหัส: %lu)\n", GetLastError());
            return 0; // Failure
        }

        if (!ReadFile(hSerial, readBuffer, MAX_READ_BUFFER - 1, &bytesRead, NULL))
        {
            // ReadFile อาจจะคืนค่า FALSE ถึงแม้จะอ่านได้ (เช่นเมื่อ timeout) ให้เช็ค bytesRead แทน
        }

        if (bytesRead > 0)
        {
            readBuffer[bytesRead] = '\0';
            // Arduino ส่งค่ากลับมาในรูปแบบ "Result Carry"
            if (sscanf(readBuffer, "%d %d", resultBit, carryBit) == 2)
            {
                return 1; // Success
            }
        }

        fprintf(stderr, "[WARN] ไม่ได้รับข้อมูลตอบกลับที่ถูกต้องจาก Arduino (ได้รับ: '%s')\n", readBuffer);
        *resultBit = 0;
        *carryBit = 0;
        return 0; // Failure
    }

    // --- Scripting Engine Function Implementations ---

    const char *resolve_argument(const char *arg)
    {
        for (int i = 0; i < varCount; i++)
        {
            if (strcmp(vars[i].name, arg) == 0)
            {
                return vars[i].value;
            }
        }
        return arg;
    }

    const char *getVar(const char *name)
    {
        for (int i = 0; i < varCount; i++)
        {
            if (strcmp(vars[i].name, name) == 0)
                return vars[i].value;
        }
        fprintf(stderr, "[WARN] Variable '%s' not found. Returning '0'.\n", name);
        return "0";
    }

    void setVar(const char *name, const char *value)
    {
        for (int i = 0; i < varCount; i++)
        {
            if (strcmp(vars[i].name, name) == 0)
            {
                strncpy(vars[i].value, value, sizeof(vars[i].value) - 1);
                vars[i].value[sizeof(vars[i].value) - 1] = '\0';
                return;
            }
        }
        if (varCount < MAX_VARS)
        {
            strncpy(vars[varCount].name, name, sizeof(vars[varCount].name) - 1);
            vars[varCount].name[sizeof(vars[varCount].name) - 1] = '\0';
            strncpy(vars[varCount].value, value, sizeof(vars[varCount].value) - 1);
            vars[varCount].value[sizeof(vars[varCount].value) - 1] = '\0';
            varCount++;
        }
        else
        {
            fprintf(stderr, "[ERROR] Maximum variable limit reached.\n");
        }
    }

    int getLabelLine(const char *name)
    {
        for (int i = 0; i < labelCount; i++)
        {
            if (strcmp(labels[i].name, name) == 0)
                return labels[i].line_number;
        }
        return -1;
    }

    // --- [ฟังก์ชันใหม่] Helper สำหรับกลับด้านสตริง ---
    char *strrev(char *str)
    {
        char *p1, *p2;
        if (!str || !*str)
            return str;
        for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
        {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
        }
        return str;
    }

    // --- [ฟังก์ชันใหม่] ALU ที่เรียกใช้ HARDWARE ---

    void alu_add_sub_binary(const char *binA, const char *binB, char *result, size_t maxLen, int is_sub)
    {
        char command[64];
        int lenA = strlen(binA);
        int lenB = strlen(binB);
        int max_len = (lenA > lenB) ? lenA : lenB;

        char paddedA[128], paddedB[128];
        // Pad A
        memset(paddedA, '0', max_len - lenA);
        strcpy(paddedA + (max_len - lenA), binA);
        // Pad B
        memset(paddedB, '0', max_len - lenB);
        strcpy(paddedB + (max_len - lenB), binB);

        int carry = is_sub; // สำหรับการลบ (2's complement) carry เริ่มต้นคือ 1
        char temp_result[128] = "";
        int result_idx = 0;

        // ทำ 2's Complement สำหรับ B ถ้าเป็นการลบ
        if (is_sub)
        {
            for (int i = 0; i < max_len; i++)
            {
                paddedB[i] = (paddedB[i] == '0' ? '1' : '0');
            }
        }

        for (int i = max_len - 1; i >= 0; i--)
        {
            int bitA = paddedA[i] - '0';
            int bitB = paddedB[i] - '0';
            int sum, c_out1, c_out2;

            // Step 1: Add A + B
            snprintf(command, sizeof(command), "001 0 %d %d", bitA, bitB);
            sendAndReceiveData(command, &sum, &c_out1);

            // Step 2: Add result + carry_in
            snprintf(command, sizeof(command), "001 0 %d %d", sum, carry);
            sendAndReceiveData(command, &sum, &c_out2);

            temp_result[result_idx++] = sum + '0';
            carry = c_out1 || c_out2; // Carry out คือ carry จากขั้นตอนใดขั้นตอนหนึ่ง
        }

        if (carry && !is_sub)
        { // ถ้าเป็นการบวกและมีทดสุดท้าย
            temp_result[result_idx++] = '1';
        }
        temp_result[result_idx] = '\0';

        strncpy(result, strrev(temp_result), maxLen - 1);
        result[maxLen - 1] = '\0';
    }

    void alu_add_binary(const char *binA, const char *binB, char *result, size_t maxLen)
    {
        alu_add_sub_binary(binA, binB, result, maxLen, 0); // is_sub = 0
    }

    void alu_sub_binary(const char *binA, const char *binB, char *result, size_t maxLen)
    {
        alu_add_sub_binary(binA, binB, result, maxLen, 1); // is_sub = 1
    }

    void alu_not_binary(const char *binIn, char *result, size_t maxLen)
    {
        size_t len = strlen(binIn);
        if (len >= maxLen)
            len = maxLen - 1;
        char command[64];
        int result_bit, carry_bit;

        for (size_t i = 0; i < len; i++)
        {
            snprintf(command, sizeof(command), "111 0 %d 0", binIn[i] - '0');
            sendAndReceiveData(command, &result_bit, &carry_bit);
            result[i] = result_bit + '0';
        }
        result[len] = '\0';
    }

    long binToLong(const char *bin) { return strtol(bin, NULL, 2); }

    int alu_compare_binary(const char *binA, const char *binB)
    {
        long numA = binToLong(binA);
        long numB = binToLong(binB);
        if (numA > numB)
            return 1;
        if (numA < numB)
            return -1;
        return 0;
    }

    // MUL และ DIV จะใช้การบวกลบซ้ำๆ (อาจช้าแต่ทำงานได้ถูกต้องกับฮาร์ดแวร์)
    void alu_mul_binary(const char *binA, const char *binB, char *result, size_t maxLen)
    {
        long numB = binToLong(binB);
        char current_sum[128] = "0";
        char temp_a[128];

        for (long i = 0; i < numB; i++)
        {
            strncpy(temp_a, getVar(resolve_argument(binA)), sizeof(temp_a) - 1);
            alu_add_binary(current_sum, temp_a, current_sum, sizeof(current_sum));
        }
        strncpy(result, current_sum, maxLen - 1);
        result[maxLen - 1] = '\0';
    }

    void alu_div_binary(const char *binA, const char *binB, char *result, size_t maxLen)
    {
        long numB = binToLong(binB);
        if (numB == 0)
        {
            fprintf(stderr, "[ERROR] Division by zero.\n");
            strcpy(result, "0");
            return;
        }
        char current_val[128];
        strncpy(current_val, binA, sizeof(current_val) - 1);
        long count = 0;

        while (alu_compare_binary(current_val, binB) >= 0)
        {
            alu_sub_binary(current_val, binB, current_val, sizeof(current_val));
            count++;
        }
        // ใช้ ltoa สำหรับแปลง count กลับเป็น binary เพราะไม่มีฟังก์ชัน hardware สำหรับงานนี้
        ltoa(count, result, 2);
    }

    // --- Instruction Dispatcher (ไม่เปลี่ยนแปลงมาก) ---
    // โค้ดที่แก้ไขแล้ว (ส่วนของฟังก์ชัน alu_process_instruction)
    int alu_process_instruction(const char *instruction)
    {
        char op[16], arg1[64], arg2[64];
        int params = sscanf(instruction, "%15s %63s %63s", op, arg1, arg2);

        if (params < 1 || instruction[0] == '#')
            return -1;
        if (strchr(instruction, ':') != NULL)
            return -1;

        if (strcmp(op, "VAR") == 0 || strcmp(op, "SET") == 0)
        {
            if (params < 3)
                return -1;
            setVar(arg1, arg2);
            printf("Set %s = %s\n", arg1, getVar(arg1));
        }
        else if (strcmp(op, "ADD") == 0 || strcmp(op, "SUB") == 0 || strcmp(op, "MUL") == 0 || strcmp(op, "DIV") == 0 || strcmp(op, "CMP") == 0)
        {
            if (params < 3)
                return -1;
            const char *val1 = resolve_argument(arg1);
            const char *val2 = resolve_argument(arg2);
            char result[128];

            if (strcmp(op, "ADD") == 0)
            {
                alu_add_binary(val1, val2, result, sizeof(result));
                setVar(arg1, result);
                printf("%s = %s\n", arg1, getVar(arg1));
            }
            else if (strcmp(op, "SUB") == 0)
            {
                alu_sub_binary(val1, val2, result, sizeof(result));
                setVar(arg1, result);
                printf("%s = %s\n", arg1, getVar(arg1));
            }
            else if (strcmp(op, "MUL") == 0)
            {
                alu_mul_binary(val1, val2, result, sizeof(result));
                setVar(arg1, result);
                printf("%s = %s\n", arg1, getVar(arg1));
            }
            else if (strcmp(op, "DIV") == 0)
            {
                alu_div_binary(val1, val2, result, sizeof(result));
                setVar(arg1, result);
                printf("%s = %s\n", arg1, getVar(arg1));
            }
            else if (strcmp(op, "CMP") == 0)
            {
                comparison_flag = alu_compare_binary(val1, val2);
                if (comparison_flag > 0)
                    printf("CMP: %s > %s\n", arg1, arg2);
                else if (comparison_flag < 0)
                    printf("CMP: %s < %s\n", arg1, arg2);
                else
                    printf("CMP: %s == %s\n", arg1, arg2);
            }
        }
        else if (strcmp(op, "PRINT") == 0)
        {
            if (params < 2)
                return -1;
            const char *val = getVar(arg1);
            printf("PRINT %s = %s (dec: %ld)\n", arg1, val, binToLong(val));
        }
        else if (strcmp(op, "JLE") == 0)
        {
            if (params < 2)
                return -1;
            if (comparison_flag <= 0)
            {
                int line = getLabelLine(arg1);
                if (line != -1)
                {
                    printf("Jumping to %s (line %d)\n", arg1, line);
                    return line;
                }
                else
                {
                    fprintf(stderr, "[ERROR] Label '%s' not found.\n", arg1);
                }
            }
        }
        else if (strcmp(op, "SUB") == 0)
        {
            alu_sub_binary(val1, val2, result, sizeof(result));
            setVar(arg1, result);
            printf("%s = %s\n", arg1, getVar(arg1));
        }
        else if (strcmp(op, "CMP") == 0)
        {
            comparison_flag = alu_compare_binary(val1, val2);
            if (comparison_flag > 0)
                printf("CMP: %s > %s\n", arg1, arg2);
            else if (comparison_flag < 0)
                printf("CMP: %s < %s\n", arg1, arg2);
            else
                printf("CMP: %s == %s\n", arg1, arg2);
        }
    }
    else if (strcmp(op, "PRINT") == 0)
    {
        if (params < 2)
            return -1;
        const char *val = getVar(arg1);
        printf("PRINT %s = %s (dec: %ld)\n", arg1, val, binToLong(val));
    }
    else if (strcmp(op, "JLE") == 0)
    {
        if (params < 2)
            return -1;
        if (comparison_flag <= 0)
        { // Jump if Less than or Equal
            int line = getLabelLine(arg1);
            if (line != -1)
            {
                printf("Jumping to %s (line %d)\n", arg1, line);
                return line;
            }
            else
            {
                fprintf(stderr, "[ERROR] Label '%s' not found.\n", arg1);
            }
        }
    }
    else
    {
        fprintf(stderr, "[ERROR] Unknown instruction: %s\n", op);
    }
    return -1; // -1 หมายถึงไปบรรทัดถัดไป
}

void runScript(FILE *f)
{
    char lines[MAX_LINES][256];
    int lineCount = 0;

    while (fgets(lines[lineCount], sizeof(lines[0]), f) && lineCount < MAX_LINES)
    {
        lines[lineCount][strcspn(lines[lineCount], "\r\n")] = 0;
        char temp_line[256];
        strcpy(temp_line, lines[lineCount]);
        char *colon = strchr(temp_line, ':');
        if (colon != NULL)
        {
            *colon = '\0';
            if (labelCount < MAX_LABELS)
            {
                strcpy(labels[labelCount].name, temp_line);
                labels[labelCount].line_number = lineCount;
                labelCount++;
            }
        }
        lineCount++;
    }

    int pc = 0;
    while (pc < lineCount)
    {
        char *line = lines[pc];
        if (line[0] == '\0' || line[0] == '#')
        {
            pc++;
            continue;
        }
        printf("[%02d] > %s\n", pc, line);
        int jump_to = alu_process_instruction(line);
        if (jump_to != -1)
        {
            pc = jump_to;
        }
        else
        {
            pc++;
        }
    }
}

// *** ต้องขึ้นบรรทัดใหม่เพื่อแยกฟังก์ชันออกจากกัน ***

char *ltoa(long value, char *buffer, int base)
{
    if (base < 2 || base > 36)
    {
        *buffer = '\0';
        return buffer;
    }
    char *ptr = buffer, *ptr1 = buffer, tmp_char;
    long tmp_value;

    do
    {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + (tmp_value - value * base)];
    } while (value);

    if (tmp_value < 0)
        *ptr++ = '-';
    *ptr-- = '\0';

    while (ptr1 < ptr)
    {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
    return buffer;
}

// ... runScript (No Changes) ...
void runScript(FILE *f)
{
    char lines[MAX_LINES][256];
    int lineCount = 0;
    while (fgets(lines[lineCount], sizeof(lines[0]), f) && lineCount < MAX_LINES)
    {
        lines[lineCount][strcspn(lines[lineCount], "\r\n")] = 0;
        char temp_line[256];
        strcpy(temp_line, lines[lineCount]);
        char *colon = strchr(temp_line, ':');
        if (colon != NULL)
        {
            *colon = '\0';
            if (labelCount < MAX_LABELS)
            {
                strcpy(labels[labelCount].name, temp_line);
                labels[labelCount].line_number = lineCount;
                labelCount++;
            }
        }
        lineCount++;
    }
    int pc = 0;
    while (pc < lineCount)
    {
        char *line = lines[pc];
        if (line[0] == '\0' || line[0] == '#')
        {
            pc++;
            continue;
        }
        printf("[%02d] > %s\n", pc, line);
        int jump_to = alu_process_instruction(line);
        if (jump_to != -1)
        {
            pc = jump_to;
        }
        else
        {
            pc++;
        }
    }
}
// ส่งคำสั่งไป Arduino แล้วอ่านผลกลับ (resultBit, carryBit)
int sendAndReceiveData(const char *dataToSend, int *resultBit, int *carryBit) {
    if (hSerial == INVALID_HANDLE_VALUE) return 0;
    DWORD bytesWritten = 0, bytesRead = 0;
    char readBuffer[MAX_READ_BUFFER] = {0};
    char command[64];
    snprintf(command, sizeof(command), "%s\n", dataToSend);

    clearSerialBuffer();
    WriteFile(hSerial, command, strlen(command), &bytesWritten, NULL);
    ReadFile(hSerial, readBuffer, MAX_READ_BUFFER - 1, &bytesRead, NULL);

    if (bytesRead > 0) {
        readBuffer[bytesRead] = '\0';
        if (sscanf(readBuffer, "%d %d", resultBit, carryBit) == 2) {
            return 1; // success
        }
    }
    return 0; // fail
}

// บวกหรือลบด้วย Arduino เท่านั้น
void alu_add_sub_binary(const char *binA, const char *binB, char *result, size_t maxLen, int is_sub) {
    int lenA = strlen(binA), lenB = strlen(binB);
    int max_len = (lenA > lenB) ? lenA : lenB;
    char paddedA[128], paddedB[128];

    // padding
    memset(paddedA, '0', max_len - lenA);
    strcpy(paddedA + (max_len - lenA), binA);
    memset(paddedB, '0', max_len - lenB);
    strcpy(paddedB + (max_len - lenB), binB);

    // ถ้าลบ → ทำ NOT B และตั้ง carry เริ่มเป็น 1
    int carry = is_sub ? 1 : 0;
    if (is_sub) {
        for (int i = 0; i < max_len; i++)
            paddedB[i] = (paddedB[i] == '0') ? '1' : '0';
    }

    char temp[128] = "";
    int idx = 0;
    for (int i = max_len - 1; i >= 0; i--) {
        int sum, c1, c2;

        // step1: A XOR B
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "001 0 %d %d", paddedA[i]-'0', paddedB[i]-'0');
        sendAndReceiveData(cmd, &sum, &c1);

        // step2: (A XOR B) XOR carry
        snprintf(cmd, sizeof(cmd), "001 0 %d %d", sum, carry);
        sendAndReceiveData(cmd, &sum, &c2);

        temp[idx++] = sum + '0';
        carry = c1 || c2;
    }

    if (carry && !is_sub) temp[idx++] = '1';
    temp[idx] = '\0';

    // reverse
    for (int i = 0; i < idx/2; i++) {
        char t = temp[i];
        temp[i] = temp[idx-1-i];
        temp[idx-1-i] = t;
    }
    strncpy(result, temp, maxLen);
}

void alu_add_binary(const char *a, const char *b, char *res, size_t maxLen) {
    alu_add_sub_binary(a, b, res, maxLen, 0);
}

void alu_sub_binary(const char *a, const char *b, char *res, size_t maxLen) {
    alu_add_sub_binary(a, b, res, maxLen, 1);
}

void alu_not_binary(const char *in, char *out, size_t maxLen) {
    int result_bit, carry_bit;
    size_t len = strlen(in);
    if (len >= maxLen) len = maxLen-1;
    for (size_t i = 0; i < len; i++) {
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "111 0 %d 0", in[i]-'0');
        sendAndReceiveData(cmd, &result_bit, &carry_bit);
        out[i] = result_bit + '0';
    }
    out[len] = '\0';
}

int alu_compare_binary(const char *a, const char *b) {
    // ให้ Arduino ส่ง flag กลับ เช่น "GT", "LT", "EQ"
    // แต่ถ้าฮาร์ดแวร์ไม่มี → ใช้การลบแล้วดูบิต sign/carry
    char diff[128];
    alu_sub_binary(a, b, diff, sizeof(diff));
    // สมมติว่าฮาร์ดแวร์ส่งผลลัพธ์บวก/ลบ → ตีความที่นี่
    // (ตรงนี้แล้วแต่โปรโตคอล Arduino)
    return strcmp(diff, "0") == 0 ? 0 : (diff[0]=='1' ? 1 : -1);
}
