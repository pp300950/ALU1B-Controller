#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define COM_PORT "COM3"
#define BAUD_RATE CBR_9600
#define UPLOAD_WAIT_MS 3000
#define READ_TIMEOUT_MS 1000
#define MAX_READ_BUFFER 256

HANDLE hSerial = INVALID_HANDLE_VALUE; // Global handle for proper cleanup

// ฟังก์ชันสำหรับเคลียร์ buffer ของ serial port
void clearSerialBuffer() {
    if (hSerial != INVALID_HANDLE_VALUE) {
        printf("[DEBUG] Clearing serial port buffers.\n");
        PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
    }
}

// Signal handler สำหรับการ cleanup
BOOL WINAPI ConsoleHandler(DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT) {
        printf("\n[INFO] Ctrl+C detected. Cleaning up and exiting...\n");
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

    printf("[DEBUG] Opening serial port: %s\n", COM_PORT);

    int attempts = 0;
    while (attempts < 5) {
        hSerial = CreateFileA(COM_PORT, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hSerial != INVALID_HANDLE_VALUE) {
            break;
        }

        DWORD err = GetLastError();
        printf("[WARNING] Failed to open port (attempt %d). Error: %lu. Retrying in 1s...\n", attempts + 1, err);
        Sleep(1000);
        attempts++;
    }

    if (hSerial == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND) {
            printf("[ERROR] Serial port %s not found.\n", COM_PORT);
        } else if (err == ERROR_ACCESS_DENIED) {
            printf("[ERROR] Serial port %s is already in use by another application (e.g., Serial Monitor).\n", COM_PORT);
        } else {
            printf("[ERROR] Could not open serial port %s. Error code: %lu\n", COM_PORT, err);
        }
        return INVALID_HANDLE_VALUE;
    }
    
    // ตั้งค่าพารามิเตอร์ Serial
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

    // ตั้งค่า timeouts ให้เหมาะสมสำหรับการอ่านแบบวนลูป
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
    clearSerialBuffer();
    CloseHandle(hSerial);
    hSerial = INVALID_HANDLE_VALUE;
    return INVALID_HANDLE_VALUE;
}

// ฟังก์ชันสำหรับส่งและรับข้อมูล
bool sendAndReceiveData(const char* dataToSend) {
    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("[ERROR] Invalid serial handle.\n");
        return false;
    }

    DWORD bytesWritten = 0;
    DWORD bytesRead = 0;
    char readBuffer[MAX_READ_BUFFER] = {0};
    
    // ตรวจสอบข้อมูลก่อนส่ง
    size_t len = strlen(dataToSend);
    if (len == 0 || dataToSend[len - 1] != '\n') {
        printf("[ERROR] Data to send must end with a newline character (\\n).\n");
        return false;
    }

    // เขียนข้อมูลจนครบ
    DWORD dataSize = (DWORD)len;
    DWORD totalWritten = 0;
    printf("[DEBUG] Writing %lu bytes: \"%s\"\n", dataSize, dataToSend);
    while (totalWritten < dataSize) {
        if (!WriteFile(hSerial, dataToSend + totalWritten, dataSize - totalWritten, &bytesWritten, NULL)) {
            printf("[ERROR] WriteFile failed. Error code: %lu\n", GetLastError());
            return false;
        }
        totalWritten += bytesWritten;
    }
    if (totalWritten != dataSize) {
        printf("[WARNING] Only %lu of %lu bytes were sent.\n", totalWritten, dataSize);
    } else {
        printf("[INFO] Successfully wrote %lu bytes.\n", totalWritten);
    }
    
    // เคลียร์ buffer ก่อนอ่านเพื่อป้องกันข้อมูลเก่า
    clearSerialBuffer();

    // อ่านข้อมูลตอบกลับแบบวนลูป
    DWORD totalBytesRead = 0;
    do {
        if (totalBytesRead >= MAX_READ_BUFFER - 1) {
            printf("[WARNING] Response truncated to %d bytes due to buffer limit.\n", MAX_READ_BUFFER - 1);
            break;
        }
        if (ReadFile(hSerial, readBuffer + totalBytesRead, MAX_READ_BUFFER - 1 - totalBytesRead, &bytesRead, NULL) && bytesRead > 0) {
            totalBytesRead += bytesRead;
        } else {
            DWORD err = GetLastError();
            if (err == ERROR_SUCCESS) { // no more data
                break;
            } else if (err != ERROR_IO_PENDING && err != ERROR_OPERATION_ABORTED) {
                printf("[ERROR] ReadFile failed. Code: %lu\n", err);
                return false;
            }
            break;
        }
    } while (true);
    readBuffer[totalBytesRead] = '\0';
    if (totalBytesRead > 0) {
        printf("[INFO] Received %lu bytes: %s\n", totalBytesRead, readBuffer);
        return true;
    } else {
        printf("[DEBUG] No data received or read timed out.\n");
        return false;
    }
}

// ฟังก์ชันสำหรับเรียกใช้ arduino-cli
bool executeArduinoCLI(const char* cliPath, const char* board, const char* port, const char* inoPath) {
    char commandLine[1024];
    snprintf(commandLine, sizeof(commandLine),
             "\"%s\" compile --upload -b %s -p %s \"%s\"",
             cliPath, board, port, inoPath);

    printf("[INFO] Executing Arduino CLI command...\n");
    printf("[DEBUG] Command: %s\n", commandLine);

    STARTUPINFOA si = { sizeof(si) };
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
        printf("[ERROR] Failed to upload Arduino code. Exit code: %lu\n", exitCode);
        return false;
    }

    printf("[INFO] Arduino code uploaded successfully.\n");
    return true;
}

int main() {
    // กำหนด path และพารามิเตอร์
    const char* arduinoCliPath = "C:\\Users\\Administrator\\Desktop\\arduino-cli.exe";
    const char* boardType = "arduino:avr:uno";
    const char* inoFilePath = "C:\\Users\\Administrator\\Desktop\\ALU4B-Controller/ALU4B-Controller/ALU4B-Controller.ino";

    // ตั้งค่า signal handler
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    // อัปโหลดโค้ด Arduino
    if (!executeArduinoCLI(arduinoCliPath, boardType, COM_PORT, inoFilePath)) {
        return 1;
    }

    printf("[INFO] Waiting for %d ms for board to be ready...\n", UPLOAD_WAIT_MS);
    Sleep(UPLOAD_WAIT_MS);

    // เปิดพอร์ต
    hSerial = openAndSetupSerialPort();
    if (hSerial == INVALID_HANDLE_VALUE) {
        return 1;
    }

    // ส่งข้อมูลไปยัง Arduino
    const char* dataToSend = "0110 1 0\n";
    sendAndReceiveData(dataToSend);

    // ปิดพอร์ตอย่างปลอดภัย
    clearSerialBuffer();
    CloseHandle(hSerial);
    printf("[DEBUG] Serial port closed.\n");

    return 0;
}