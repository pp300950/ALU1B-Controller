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
    
    //เคลียร์ buffer ก่อนอ่านเพื่อป้องกันข้อมูลเก่า
    clearSerialBuffer();

    //ลูป อ่านข้อมูลตอบกลับ
    DWORD totalBytesRead = 0;
    do {
        if (totalBytesRead >= MAX_READ_BUFFER - 1) {
            printf("[WARNING] ข้อมูลที่รับมาถูกตัดเหลือ %d ไบต์ เนื่องจากบัฟเฟอร์เต็ม\n", MAX_READ_BUFFER - 1);
            break;
        }
        if (ReadFile(hSerial, readBuffer + totalBytesRead, MAX_READ_BUFFER - 1 - totalBytesRead, &bytesRead, NULL) && bytesRead > 0) {
            totalBytesRead += bytesRead;
        } else {
            DWORD err = GetLastError();
            if (err == ERROR_SUCCESS) { //no more data
                break;
            } else if (err != ERROR_IO_PENDING && err != ERROR_OPERATION_ABORTED) {
                printf("[ERROR] ReadFile ล้มเหลว รหัส: %lu\n", err);
                return false;
            }
            break;
        }
    } while (true);
    readBuffer[totalBytesRead] = '\0';
    if (totalBytesRead > 0) {
        printf("[INFO] ได้รับข้อมูล %lu ไบต์: %s\n", totalBytesRead, readBuffer);
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
    const char* dataToSend = "0110 1 0\n";
    sendAndReceiveData(dataToSend);

    //ปิดพอร์ต
    clearSerialBuffer();
    CloseHandle(hSerial);
    printf("[DEBUG] ปิดพอร์ตซีเรียลแล้ว เย่ๆ\n");

    return 0;
}