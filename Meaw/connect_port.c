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
#define NUM_BITS 32
#define READ_TIMEOUT_MS 500
#define UPLOAD_WAIT_MS 3000

// --- โครงสร้างและตัวแปรสำหรับ Interpreter ---
#define MAX_VARS 10

// Struct สำหรับเก็บตัวแปร (เช่น 'x', 'y')
typedef struct {
    char name[32];
    int value;
} Variable;

// Array สำหรับเก็บตัวแปรทั้งหมดที่ผู้ใช้สร้าง
Variable variables[MAX_VARS];
int variable_count = 0;

// Global serial handle
HANDLE hSerial = INVALID_HANDLE_VALUE;

// ===================================================================================
// ฟังก์ชันพื้นฐาน (เหมือนเดิม)
// ===================================================================================

void clearSerialBuffer() {
    if (hSerial != INVALID_HANDLE_VALUE) {
        PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
    }
}

BOOL WINAPI ConsoleHandler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT) {
        printf("\n[INFO] Ctrl+C detected. Shutting down.\n");
        if (hSerial != INVALID_HANDLE_VALUE) {
            clearSerialBuffer();
            CloseHandle(hSerial);
        }
        ExitProcess(0);
        return TRUE;
    }
    return FALSE;
}

HANDLE openAndSetupSerialPort() {
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts = {0};

    printf("[DEBUG] Opening serial port: %s\n", COM_PORT);
    hSerial = CreateFileA(COM_PORT, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hSerial == INVALID_HANDLE_VALUE) {
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
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        printf("[ERROR] GetCommState failed.\n");
        goto cleanup;
    }
    dcbSerialParams.BaudRate = BAUD_RATE;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if (!SetCommState(hSerial, &dcbSerialParams)) {
        printf("[ERROR] SetCommState failed.\n");
        goto cleanup;
    }

    timeouts.ReadIntervalTimeout = 100;
    timeouts.ReadTotalTimeoutConstant = READ_TIMEOUT_MS;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if (!SetCommTimeouts(hSerial, &timeouts)) {
        printf("[ERROR] SetCommTimeouts failed.\n");
        goto cleanup;
    }

    return hSerial;

cleanup:
    CloseHandle(hSerial);
    return INVALID_HANDLE_VALUE;
}

bool executeArduinoCLI(const char *cliPath, const char *board, const char *port, const char *inoPath) {
    char commandLine[1024];
    snprintf(commandLine, sizeof(commandLine), "\"%s\" compile --upload -b %s -p %s \"%s\"", cliPath, board, port, inoPath);
    printf("[INFO] Running Arduino CLI command...\n[DEBUG] %s\n", commandLine);

    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi;

    if (!CreateProcessA(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        printf("[ERROR] CreateProcess failed. Error code: %lu\n", GetLastError());
        return false;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exitCode != 0) {
        printf("[ERROR] Arduino code upload failed. Exit code: %lu\n", exitCode);
        return false;
    }
    printf("[INFO] Arduino code uploaded successfully.\n");
    return true;
}

bool sendALUCommand(const char* mux3, int subAdd, int a, int b, int carryIn, int* outResult, int* outCarry) {
    if (!mux3 || strlen(mux3) != 3) {
        printf("[ERROR] mux3 must be 3 characters long, e.g., \"001\"\n");
        return false;
    }
    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("[ERROR] Serial port is not open.\n");
        return false;
    }

    char line[64];
    _snprintf(line, sizeof(line), "%s %d %d %d %d\n", mux3, subAdd, a, b, carryIn);

    clearSerialBuffer();

    DWORD written = 0;
    BOOL ok = WriteFile(hSerial, line, (DWORD)strlen(line), &written, NULL);
    if (!ok || written != strlen(line)) {
        printf("[ERROR] Failed to write to serial port.\n");
        return false;
    }

    char resp[64] = {0};
    DWORD totalRead = 0;
    DWORD bytesRead = 0;

    for (;;) {
        ok = ReadFile(hSerial, resp + totalRead, sizeof(resp) - 1 - totalRead, &bytesRead, NULL);
        if (!ok) {
            printf("[ERROR] Failed to read from serial port.\n");
            return false;
        }
        if (bytesRead == 0) {
            break;
        }
        totalRead += bytesRead;
        resp[totalRead] = '\0';
        if (strchr(resp, '\n'))
            break;
        if (totalRead >= sizeof(resp) - 2)
            break;
    }

    int r = -1, c = -1;
    if (sscanf(resp, "%d %d", &r, &c) == 2) {
        if (outResult) *outResult = r;
        if (outCarry) *outCarry = c;
        return true;
    } else {
        printf("[ERROR] Could not parse response: \"%s\"\n", resp);
        return false;
    }
}


// ===================================================================================
// ฟังก์ชันใหม่สำหรับ INTERPRETER
// ===================================================================================

// ฟังก์ชันสำหรับสั่ง ALU บวกเลข 32-bit สองจำนวน
int executeALU_ADD(int a, int b) {
    printf("--- Executing 32-bit ADD on ALU: %d + %d ---\n", a, b);
    int finalResult = 0;
    int carryIn = 0; // การบวกปกติ เริ่มต้น carry = 0

    for (int i = 0; i < NUM_BITS; ++i) {
        int a_bit = (a >> i) & 1;
        int b_bit = (b >> i) & 1;
        int result_bit = -1;
        int carry_out = -1;

        printf("  [L%d] bit%d: ", i, i);
        if (sendALUCommand("001", 0, a_bit, b_bit, carryIn, &result_bit, &carry_out)) {
             printf("[OK] CMD=\"001 0 %d %d %d\" -> result=%d carry=%d\n",
                   a_bit, b_bit, carryIn, result_bit, carry_out);
        } else {
            printf("[FAIL] CMD=\"001 0 %d %d %d\"\n", a_bit, b_bit, carryIn);
            return -1; // Error
        }

        if (result_bit == 1) {
            finalResult |= (1 << i);
        }
        carryIn = carry_out;
    }
    printf("--- ALU ADD finished. Result: %d ---\n", finalResult);
    return finalResult;
}

// ค้นหาตัวแปรจากชื่อ
Variable* find_variable(const char* name) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return &variables[i];
        }
    }
    return NULL;
}

// สร้างหรืออัปเดตค่าตัวแปร
void set_variable(const char* name, int value) {
    Variable* var = find_variable(name);
    if (var) {
        var->value = value;
    } else {
        if (variable_count < MAX_VARS) {
            strcpy(variables[variable_count].name, name);
            variables[variable_count].value = value;
            variable_count++;
        } else {
            printf("[ERROR] Maximum number of variables reached.\n");
        }
    }
}

// ตรวจสอบว่าเป็นตัวเลขหรือไม่
bool is_number(const char* s) {
    if (s == NULL || *s == '\0') return false;
    while (*s) {
        if (!isdigit(*s)) return false;
        s++;
    }
    return true;
}

// หาค่าของ operand (อาจจะเป็นตัวเลข หรือชื่อตัวแปร)
int get_operand_value(const char* operand_str) {
    if (is_number(operand_str)) {
        return atoi(operand_str);
    } else {
        Variable* var = find_variable(operand_str);
        if (var) {
            return var->value;
        } else {
            printf("[ERROR] Variable '%s' not found.\n", operand_str);
            return 0; // Return 0 on error
        }
    }
}

// ฟังก์ชันหลักสำหรับตีความและประมวลผลคำสั่ง
void process_command(char* line) {
    char var_name[32], op1_str[32], op2_str[32], op_char;

    // รูปแบบ 1: print x
    if (sscanf(line, "print %s", var_name) == 1) {
        Variable* var = find_variable(var_name);
        if (var) {
            printf(">> %s = %d\n", var->name, var->value);
        } else {
            printf("[ERROR] Variable '%s' not found.\n", var_name);
        }
    }
    // รูปแบบ 2: y = x + z  (ตัวแปร = ตัวแปร + ตัวแปร)
    else if (sscanf(line, "%s = %s + %s", var_name, op1_str, op2_str) == 3) {
        int val1 = get_operand_value(op1_str);
        int val2 = get_operand_value(op2_str);
        int result = executeALU_ADD(val1, val2);
        set_variable(var_name, result);
        printf(">> %s set to %d\n", var_name, result);
    }
    // รูปแบบ 3: int x = 5 (การประกาศและกำหนดค่าเริ่มต้น)
    else if (sscanf(line, "int %s = %d", var_name, &op1_str) == 2) {
         set_variable(var_name, atoi(op1_str));
         printf(">> Variable '%s' declared and set to %d.\n", var_name, atoi(op1_str));
    }
    // รูปแบบ 4: x = 10 (การกำหนดค่า)
    else if (sscanf(line, "%s = %d", var_name, &op1_str) == 2) {
         set_variable(var_name, atoi(op1_str));
         printf(">> %s set to %d.\n", var_name, atoi(op1_str));
    }
    else {
        printf("[ERROR] Invalid command format.\n");
    }
}

// ===================================================================================
// Main function (INTERPRETER LOOP)
// ===================================================================================
int main() {
    SetConsoleOutputCP(CP_UTF8);

    const char *arduinoCliPath = "C:\\Users\\Administrator\\Desktop\\arduino-cli.exe";
    const char *boardType = "arduino:avr:uno";
    const char *inoFilePath = "C:\\Users\\Administrator\\Desktop\\ALU4B-Controller/ALU4B-Controller/ALU4B-Controller.ino";

    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    if (!executeArduinoCLI(arduinoCliPath, boardType, COM_PORT, inoFilePath)) {
        return 1;
    }

    printf("[INFO] Waiting %d ms for board to be ready...\n", UPLOAD_WAIT_MS);
    Sleep(UPLOAD_WAIT_MS);

    hSerial = openAndSetupSerialPort();
    if (hSerial == INVALID_HANDLE_VALUE) {
        return 1;
    }

    printf("\n--- ALU Interpreter Ready ---\n");
    printf("Enter commands (e.g., 'int x = 5', 'y = x + 10', 'print y', 'exit')\n");

    char input_line[256];
    while (1) {
        printf("> ");
        if (fgets(input_line, sizeof(input_line), stdin) == NULL) continue;

        // Remove trailing newline character
        input_line[strcspn(input_line, "\r\n")] = 0;

        if (strcmp(input_line, "exit") == 0) {
            break;
        }
        
        if(strlen(input_line) > 0) {
            process_command(input_line);
        }
    }

    printf("\n--- Exiting Program ---\n");

    clearSerialBuffer();
    CloseHandle(hSerial);
    printf("[DEBUG] Serial port closed.\n");

    return 0;
}