#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h> // เพิ่มเข้ามาเพื่อใช้ strtol, ltoa

#include "serial_comm.h"

// -------------------- 1. โครงสร้างข้อมูลและตัวแปร Global --------------------

// Struct สำหรับเก็บตัวแปร (ชื่อและค่าที่เป็นเลขฐานสอง)
typedef struct {
    char name[32];
    char value[64]; // เก็บค่าเป็น binary string
} Variable;

// Struct สำหรับเก็บตำแหน่งของ Label ในสคริปต์
typedef struct {
    char name[32];
    int line_number;
} Label;

#define MAX_VARS 100
#define MAX_LABELS 50
#define MAX_LINES 200

static Variable vars[MAX_VARS];
static int varCount = 0;
static Label labels[MAX_LABELS];
static int labelCount = 0;

// Global flag ที่จะถูกตั้งค่าโดยคำสั่ง CMP
// 0: A == B, 1: A > B, -1: A < B
static int comparison_flag = 0;

// -------------------- 2. Function Prototypes (ประกาศฟังก์ชัน) --------------------

// Variable functions
const char* getVar(const char *name);
void setVar(const char *name, const char *value);

// ALU bit-level execute (สมมติว่าคุยกับ Hardware จริง)
static int alu_execute_bit(const char *muxCode, int subAdd, int aBit, int bBit);

// ALU multi-bit operations (ฟังก์ชันจำลองที่คำนวณใน C)
void alu_add_binary(const char *binA, const char *binB, char *result, size_t maxLen);
void alu_sub_binary(const char *binA, const char *binB, char *result, size_t maxLen);
void alu_mul_binary(const char *binA, const char *binB, char *result, size_t maxLen);
void alu_div_binary(const char *binA, const char *binB, char *result, size_t maxLen);
void alu_not_binary(const char *binIn, char *result, size_t maxLen);
int alu_compare_binary(const char *binA, const char *binB);

// Instruction processing
int alu_process_instruction(const char *instruction);
void runScript(FILE *f);

// Helper function (ไม่มีใน C มาตรฐาน)
char* ltoa(long value, char* buffer, int base);


// -------------------- 3. Variable & Label Functions Implementation --------------------

// ค้นหาและคืนค่าของตัวแปร
const char* getVar(const char *name) {
    for (int i = 0; i < varCount; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            return vars[i].value;
        }
    }
    // ถ้าไม่เจอตัวแปร อาจจะคืนค่า "0" หรือแสดง error
    fprintf(stderr, "[WARN] Variable '%s' not found. Returning '0'.\n", name);
    return "0";
}

// ตั้งค่าหรือสร้างตัวแปรใหม่
void setVar(const char *name, const char *value) {
    for (int i = 0; i < varCount; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            strncpy(vars[i].value, value, sizeof(vars[i].value) - 1);
            vars[i].value[sizeof(vars[i].value) - 1] = '\0';
            return;
        }
    }
    // ถ้าไม่เจอ ให้สร้างใหม่
    if (varCount < MAX_VARS) {
        strncpy(vars[varCount].name, name, sizeof(vars[varCount].name) - 1);
        vars[varCount].name[sizeof(vars[varCount].name) - 1] = '\0';
        strncpy(vars[varCount].value, value, sizeof(vars[varCount].value) - 1);
        vars[varCount].value[sizeof(vars[varCount].value) - 1] = '\0';
        varCount++;
    } else {
        fprintf(stderr, "[ERROR] Maximum variable limit reached.\n");
    }
}

// ค้นหาบรรทัดของ Label
int getLabelLine(const char *name) {
    for (int i = 0; i < labelCount; i++) {
        if (strcmp(labels[i].name, name) == 0) {
            return labels[i].line_number;
        }
    }
    return -1; // ไม่เจอ
}


// -------------------- 4. ALU Bit-Level Execute --------------------

// ฟังก์ชันสำหรับส่งคำสั่ง 1-bit ไปยัง Arduino (นี่คือเวอร์ชันที่ควรใช้จริง)
static int alu_execute_bit(const char *muxCode, int subAdd, int aBit, int bBit) {
    char command[32];
    snprintf(command, sizeof(command), "%s %d %d %d\n", muxCode, subAdd, aBit, bBit);
    
    // ในโค้ดจริง ฟังก์ชันนี้จะส่งข้อมูลและรอรับผลลัพธ์จาก Arduino
    int result = sendAndReceiveData(command);

    if (result < 0) {
        fprintf(stderr, "[ERROR] Failed to send or receive data from hardware.\n");
        return 0; // คืนค่า default เมื่อเกิดข้อผิดพลาด
    }
    return result;
}


// -------------------- 5. ALU Multi-Bit Operations (Mock Implementations) --------------------

// ฟังก์ชันแปลงเลขฐานสองเป็น long
long binToLong(const char *bin) {
    return strtol(bin, NULL, 2);
}

// ฟังก์ชันบวก (เวอร์ชันจำลอง)
void alu_add_binary(const char *binA, const char *binB, char *result, size_t maxLen) {
    long numA = binToLong(binA);
    long numB = binToLong(binB);
    ltoa(numA + numB, result, 2);
}

// ฟังก์ชันลบ (เวอร์ชันจำลอง)
void alu_sub_binary(const char *binA, const char *binB, char *result, size_t maxLen) {
    long numA = binToLong(binA);
    long numB = binToLong(binB);
    ltoa(numA - numB, result, 2);
}

// ฟังก์ชันคูณ (เวอร์ชันจำลอง)
void alu_mul_binary(const char *binA, const char *binB, char *result, size_t maxLen) {
    long numA = binToLong(binA);
    long numB = binToLong(binB);
    ltoa(numA * numB, result, 2);
}

// ฟังก์ชันหาร (เวอร์ชันจำลอง)
void alu_div_binary(const char *binA, const char *binB, char *result, size_t maxLen) {
    long numA = binToLong(binA);
    long numB = binToLong(binB);
    if (numB == 0) {
        fprintf(stderr, "[ERROR] Division by zero.\n");
        strcpy(result, "0");
        return;
    }
    ltoa(numA / numB, result, 2);
}

// ฟังก์ชัน NOT (แก้ไขให้ทำงานจริงโดยไม่พึ่ง ALU)
void alu_not_binary(const char *binIn, char *result, size_t maxLen) {
    size_t len = strlen(binIn);
    if (len >= maxLen) len = maxLen - 1;

    for (size_t i = 0; i < len; i++) {
        result[i] = (binIn[i] == '0' ? '1' : '0');
    }
    result[len] = '\0';
}

// ฟังก์ชันเปรียบเทียบ (ทำงานบน C ได้โดยตรง)
int alu_compare_binary(const char *binA, const char *binB) {
    long numA = binToLong(binA);
    long numB = binToLong(binB);

    if (numA > numB) return 1;
    if (numA < numB) return -1;
    return 0;
}


// -------------------- 6. Instruction Dispatcher --------------------

// ประมวลผลคำสั่ง 1 บรรทัด และคืนค่าบรรทัดที่จะ "กระโดด" ไป หากมีการ jump
// คืนค่า -1 หากทำงานบรรทัดถัดไปตามปกติ
int alu_process_instruction(const char *instruction) {
    char op[16], arg1[64], arg2[64];
    int params = sscanf(instruction, "%15s %63s %63s", op, arg1, arg2);

    if (params < 1) return -1; // บรรทัดว่าง

    // ตรวจสอบ Label ก่อน
    char* label_check = strchr(instruction, ':');
    if (label_check != NULL) {
        // นี่คือบรรทัดที่เป็น Label (เช่น LOOP:) ไม่ต้องทำอะไร
        return -1;
    }

    if (strcmp(op, "VAR") == 0 || strcmp(op, "SET") == 0) {
        if (params < 3) {
            fprintf(stderr, "[ERROR] '%s' requires 2 arguments.\n", op);
            return -1;
        }
        setVar(arg1, arg2);
        printf("Set %s = %s\n", arg1, arg2);
    }
    else if (strcmp(op, "ADD") == 0) {
        if (params < 3) { fprintf(stderr, "[ERROR] ADD requires 2 arguments.\n"); return -1; }
        const char *val1 = getVar(arg1);
        const char *val2 = getVar(arg2);
        char result[128];
        alu_add_binary(val1, val2, result, sizeof(result));
        setVar(arg1, result); // ผลลัพธ์เก็บในตัวแปรแรก
        printf("%s = %s\n", arg1, result);
    }
    else if (strcmp(op, "SUB") == 0) {
        if (params < 3) { fprintf(stderr, "[ERROR] SUB requires 2 arguments.\n"); return -1; }
        const char *val1 = getVar(arg1);
        const char *val2 = getVar(arg2);
        char result[128];
        alu_sub_binary(val1, val2, result, sizeof(result));
        setVar(arg1, result);
        printf("%s = %s\n", arg1, result);
    }
    else if (strcmp(op, "MUL") == 0) {
        if (params < 3) { fprintf(stderr, "[ERROR] MUL requires 2 arguments.\n"); return -1; }
        const char *val1 = getVar(arg1);
        const char *val2 = getVar(arg2);
        char result[128];
        alu_mul_binary(val1, val2, result, sizeof(result));
        setVar(arg1, result);
        printf("%s = %s\n", arg1, result);
    }
     else if (strcmp(op, "DIV") == 0) {
        if (params < 3) { fprintf(stderr, "[ERROR] DIV requires 2 arguments.\n"); return -1; }
        const char *val1 = getVar(arg1);
        const char *val2 = getVar(arg2);
        char result[128];
        alu_div_binary(val1, val2, result, sizeof(result));
        setVar(arg1, result);
        printf("%s = %s\n", arg1, result);
    }
    else if (strcmp(op, "NOT") == 0) {
        if (params < 2) { fprintf(stderr, "[ERROR] NOT requires 1 argument.\n"); return -1; }
        const char *val1 = getVar(arg1);
        char result[128];
        alu_not_binary(val1, result, sizeof(result));
        setVar(arg1, result);
        printf("NOT %s -> %s\n", arg1, result);
    }
     else if (strcmp(op, "CMP") == 0) {
        if (params < 3) { fprintf(stderr, "[ERROR] CMP requires 2 arguments.\n"); return -1; }
        const char *val1 = getVar(arg1);
        const char *val2 = getVar(arg2);
        comparison_flag = alu_compare_binary(val1, val2);
        if (comparison_flag > 0) printf("CMP: %s > %s\n", arg1, arg2);
        else if (comparison_flag < 0) printf("CMP: %s < %s\n", arg1, arg2);
        else printf("CMP: %s == %s\n", arg1, arg2);
    }
    else if (strcmp(op, "PRINT") == 0) {
         if (params < 2) { fprintf(stderr, "[ERROR] PRINT requires 1 argument.\n"); return -1; }
         printf("PRINT %s = %s\n", arg1, getVar(arg1));
    }
    else if (strcmp(op, "JLE") == 0) { // Jump if Less than or Equal
        if (params < 2) { fprintf(stderr, "[ERROR] JLE requires a label.\n"); return -1; }
        if (comparison_flag <= 0) { // ถ้า A <= B
            int line = getLabelLine(arg1);
            if (line != -1) {
                printf("Jumping to %s (line %d)\n", arg1, line);
                return line; // คืนค่าบรรทัดที่จะไป
            } else {
                fprintf(stderr, "[ERROR] Label '%s' not found.\n", arg1);
            }
        }
    }
    else {
        fprintf(stderr, "[ERROR] Unknown instruction: %s\n", op);
    }

    return -1; // ทำงานบรรทัดถัดไป
}

// -------------------- 7. Script Runner --------------------

void runScript(FILE *f) {
    char lines[MAX_LINES][256];
    int lineCount = 0;
    
    // 1. อ่านสคริปต์ทั้งหมดและค้นหา Labels
    while (fgets(lines[lineCount], sizeof(lines[0]), f) && lineCount < MAX_LINES) {
        // ลบ newline character
        lines[lineCount][strcspn(lines[lineCount], "\r\n")] = 0;

        char temp_line[256];
        strcpy(temp_line, lines[lineCount]);

        char *colon = strchr(temp_line, ':');
        if (colon != NULL) {
            *colon = '\0'; // ตัดเอาเฉพาะชื่อ label
            if (labelCount < MAX_LABELS) {
                strcpy(labels[labelCount].name, temp_line);
                labels[labelCount].line_number = lineCount;
                labelCount++;
            }
        }
        lineCount++;
    }

    // 2. ประมวลผลสคริปต์ทีละบรรทัด
    int pc = 0; // Program Counter
    while (pc < lineCount) {
        char *line = lines[pc];
        if (line[0] == '#' || strlen(line) < 2) {
            pc++;
            continue;
        }

        printf("[%d] > %s\n", pc, line);
        int jump_to = alu_process_instruction(line);

        if (jump_to != -1) {
            pc = jump_to; // กระโดดไปบรรทัดที่กำหนด
        } else {
            pc++; // ไปบรรทัดถัดไป
        }
    }
}

// -------------------- 8. Main Function --------------------

int main() {
    // สร้างไฟล์ script ตัวอย่าง
    const char *script_filename = "script.txt";
    FILE *f = fopen(script_filename, "w");
    if (f) {
        fprintf(f, "# Example: Multiplication Table (2 * 1 to 2 * 5)\n");
        fprintf(f, "VAR N 101       # N = 5 (loop until X > N)\n");
        fprintf(f, "VAR M 10        # M = 2 (Multiplier)\n");
        fprintf(f, "VAR X 1         # X = 1 (Counter)\n");
        fprintf(f, "VAR RESULT 0    # Result accumulator\n");
        fprintf(f, "\n");
        fprintf(f, "LOOP:\n");
        fprintf(f, "MUL RESULT M X # Not really, we will set RESULT = M * X for this mock\n");
        fprintf(f, "SET RESULT 0 # clear\n");
        fprintf(f, "ADD RESULT M # res = M\n");
        fprintf(f, "PRINT RESULT\n");
        fprintf(f, "PRINT X\n");
        fprintf(f, "\n");
        fprintf(f, "ADD X 1         # X = X + 1\n");
        fprintf(f, "CMP X N         # Compare X with N\n");
        fprintf(f, "JLE LOOP        # If X <= N, jump to LOOP\n");
        fprintf(f, "\n");
        fprintf(f, "PRINT RESULT    # Final result should be M*N = 10\n");
        fclose(f);
    }

    // เปิดและรันสคริปต์
    f = fopen(script_filename, "r");
    if (!f) {
        perror("Could not open script file");
        return 1;
    }

    runScript(f);
    fclose(f);

    return 0;
}


// -------------------- Helper Function Implementation --------------------

/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */
char* ltoa(long value, char* buffer, int base) {
    // check that the base if valid
    if (base < 2 || base > 36) { *buffer = '\0'; return buffer; }

    char* ptr = buffer, *ptr1 = buffer, tmp_char;
    long tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return buffer;
}
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <locale.h>
#define COM_PORT "COM3"
#define BAUD_RATE CBR_9600
#define UPLOAD_WAIT_MS 3000
#define READ_TIMEOUT_MS 1000
#define MAX_READ_BUFFER 256

HANDLE hSerial = INVALID_HANDLE_VALUE;

//เคลียร์ buffer ของ serial port
void clearSerialBuffer() {
    if (hSerial != INVALID_HANDLE_VALUE) {
        printf("[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล\n");
        PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
    }
}

//Signal handlerสำหรับ cleanup
BOOL WINAPI ConsoleHandler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT) {
        printf("\n[INFO] ตรวจพบ Ctrl+C. กำลังปิดโปรแกรม...\n");
        if (hSerial != INVALID_HANDLE_VALUE) {
            clearSerialBuffer();
            CloseHandle(hSerial);
        }
        ExitProcess(0);
        return TRUE;
    }
    return FALSE;
}

// ฟังก์ชันสำหรับเปิดพอร์ตและตั้งค่า
HANDLE openAndSetupSerialPort() {
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts = {0};

    printf("[DEBUG] กำลังเปิดพอร์ตซีเรียล: %s\n", COM_PORT);

    int attempts = 0;
    while (attempts < 5) {
        hSerial = CreateFileA(COM_PORT, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hSerial != INVALID_HANDLE_VALUE) {
            break;
        }

        DWORD err = GetLastError();
        printf("[WARNING] เปิดพอร์ตไม่ได้ (ครั้งที่ %d). รหัสข้อผิดพลาด: %lu. กำลังลองใหม่ใน 1 วินาที...\n", attempts + 1, err);
        Sleep(1000);
        attempts++;
    }

    if (hSerial == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND) {
            printf("[ERROR] ไม่พบพอร์ตซีเรียล %s\n", COM_PORT);
        } else if (err == ERROR_ACCESS_DENIED) {
            printf("[ERROR] พอร์ตซีเรียล %s กำลังถูกใช้งานโดยโปรแกรมอื่น (เช่น Serial Monitor)\n", COM_PORT);
        } else {
            printf("[ERROR] ไม่สามารถเปิดพอร์ตซีเรียล %s ได้ รหัสข้อผิดพลาด: %lu\n", COM_PORT, err);
        }
        return INVALID_HANDLE_VALUE;
    }
    
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        printf("[ERROR] GetCommState ล้มเหลว\n");
        goto cleanup;
    }
    dcbSerialParams.BaudRate = BAUD_RATE;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if (!SetCommState(hSerial, &dcbSerialParams)) {
        printf("[ERROR] SetCommState ล้มเหลว\n");
        goto cleanup;
    }

    //ตั้งค่า timeouts สำหรับการอ่านแบบวนลูป
    timeouts.ReadIntervalTimeout = 100;
    timeouts.ReadTotalTimeoutConstant = READ_TIMEOUT_MS;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if (!SetCommTimeouts(hSerial, &timeouts)) {
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
// ฟังก์ชันสำหรับส่งและรับข้อมูล
bool sendAndReceiveData(const char* dataToSend) {
    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("[ERROR] Handle ซีเรียลไม่ถูกต้อง\n");
        return false;
    }

    DWORD bytesWritten = 0;
    DWORD bytesRead = 0;
    char readBuffer[MAX_READ_BUFFER] = {0};

    // ตรวจสอบข้อมูลก่อนส่ง
    size_t len = strlen(dataToSend);
    if (len == 0 || dataToSend[len - 1] != '\n') {
        printf("[ERROR] ข้อมูลที่จะส่งต้องลงท้ายด้วยอักขระขึ้นบรรทัดใหม่ (\\n)\n");
        return false;
    }

    // ล้าง buffer ก่อนส่งเพื่อเคลียร์ข้อมูลเก่า
    clearSerialBuffer();

    // เขียนข้อมูลจนครบ
    DWORD dataSize = (DWORD)len;
    DWORD totalWritten = 0;
    printf("[DEBUG] กำลังเขียนข้อมูล %lu ไบต์: \"%s\"\n", dataSize, dataToSend);
    while (totalWritten < dataSize) {
        if (!WriteFile(hSerial, dataToSend + totalWritten, dataSize - totalWritten, &bytesWritten, NULL)) {
            printf("[ERROR] WriteFile ล้มเหลว รหัสข้อผิดพลาด: %lu\n", GetLastError());
            return false;
        }
        totalWritten += bytesWritten;
    }
    if (totalWritten != dataSize) {
        printf("[WARNING] ส่งข้อมูลไปเพียง %lu จากทั้งหมด %lu ไบต์\n", totalWritten, dataSize);
    } else {
        printf("[INFO] เขียนข้อมูลสำเร็จ %lu ไบต์\n", totalWritten);
    }

    // รอให้ Arduino ประมวลผลและส่งข้อมูลกลับ
    Sleep(100); // 100ms ปรับได้ตามความเร็วของโค้ด Arduino

    // อ่านข้อมูลตอบกลับทั้งหมด
    DWORD totalBytesRead = 0;
    do {
        if (totalBytesRead >= MAX_READ_BUFFER - 1) {
            printf("[WARNING] ข้อมูลที่รับมาถูกตัดเหลือ %d ไบต์ เนื่องจากบัฟเฟอร์เต็ม\n", MAX_READ_BUFFER - 1);
            break;
        }
        if (ReadFile(hSerial, readBuffer + totalBytesRead, MAX_READ_BUFFER - 1 - totalBytesRead, &bytesRead, NULL) && bytesRead > 0) {
            totalBytesRead += bytesRead;
        } else {
            break; // ไม่มีข้อมูลเพิ่ม
        }
    } while (true);

    readBuffer[totalBytesRead] = '\0';

    if (totalBytesRead > 0) {
        printf("[INFO] ได้รับข้อมูล %lu ไบต์:\n%s", totalBytesRead, readBuffer);
        return true;
    } else {
        printf("[DEBUG] ไม่ได้รับข้อมูลหรืออ่านข้อมูลหมดเวลาแล้ว\n");
        return false;
    }
}

//เรียกarduino-cli
bool executeArduinoCLI(const char* cliPath, const char* board, const char* port, const char* inoPath) {
    char commandLine[1024];
    snprintf(commandLine, sizeof(commandLine),
             "\"%s\" compile --upload -b %s -p %s \"%s\"",
             cliPath, board, port, inoPath);

    printf("[INFO] กำลังรันคำสั่ง Arduino CLI...\n");
    printf("[DEBUG] คำสั่ง: %s\n", commandLine);

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (!CreateProcessA(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        printf("[ERROR] CreateProcess ล้มเหลว รหัสข้อผิดพลาด: %lu\n", GetLastError());
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exitCode != 0) {
        printf("[ERROR] อัปโหลดโค้ด Arduino ล้มเหลว รหัสออก: %lu\n", exitCode);
        return false;
    }

    printf("[INFO] อัปโหลดโค้ด Arduino สำเร็จแล้ว\n");
    return true;
}

int main() {
    //ปรินต์ภาษาไทย
    SetConsoleOutputCP(CP_UTF8);
    
    //path และพารามิเตอร์
    const char* arduinoCliPath = "C:\\Users\\Administrator\\Desktop\\arduino-cli.exe";
    const char* boardType = "arduino:avr:uno";
    const char* inoFilePath = "C:\\Users\\Administrator\\Desktop\\ALU4B-Controller/ALU4B-Controller/ALU4B-Controller.ino";

    //ตั้งค่า signal handler
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    //อัปโหลดโค้ด Arduino
    if (!executeArduinoCLI(arduinoCliPath, boardType, COM_PORT, inoFilePath)) {
        return 1;
    }

    printf("[INFO] กำลังรอให้บอร์ดพร้อมใช้งานเป็นเวลา %d มิลลิวินาที...\n", UPLOAD_WAIT_MS);
    Sleep(UPLOAD_WAIT_MS);

    //เปิดพอร์ต
    hSerial = openAndSetupSerialPort();
    if (hSerial == INVALID_HANDLE_VALUE) {
        return 1;
    }

    //ส่งข้อมูลไปบอร์ด
    const char* dataToSend = "011 0 1 0\n";
    sendAndReceiveData(dataToSend);

    //ปิดพอร์ต
    clearSerialBuffer();
    CloseHandle(hSerial);
    printf("[DEBUG] ปิดพอร์ตซีเรียลแล้ว เย่ๆ\n");

    return 0;
}


// serial_comm.c
int sendAndReceiveData(const char *command) {
    // ส่งคำสั่งไป Arduino
    // รอรับค่ากลับ เช่น "1\n"
    char buffer[16];
    if (!serial_write(command)) return -1;
    if (!serial_read(buffer, sizeof(buffer))) return -1;
    return atoi(buffer); // แปลงเป็น int
}
