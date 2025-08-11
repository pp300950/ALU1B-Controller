#include <windows.h>
#include <stdio.h>
#include <stdlib.h> // สำหรับใช้ฟังก์ชัน system()

void sendSerialData(const char* dataToSend) {
    HANDLE hSerial;
    DCB dcbSerialParams = {0};
    COMMTIMEOUTS timeouts = {0};
    DWORD bytesWritten;
    
    // 1. เปิด Serial Port (COM3)
    LPCSTR portName = "COM3";
    hSerial = CreateFileA(portName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hSerial == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            printf("Error: COM3 port not found.\n");
        } else {
            printf("Error: Could not open COM3 port.\n");
        }
        return;
    }

    // 2. ตั้งค่าการสื่อสาร
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        printf("Error: GetCommState failed.\n");
        CloseHandle(hSerial);
        return;
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        printf("Error: SetCommState failed.\n");
        CloseHandle(hSerial);
        return;
    }
    
    // 3. ตั้งค่า timeouts
    timeouts.WriteTotalTimeoutConstant = 50;
    if (!SetCommTimeouts(hSerial, &timeouts)) {
        printf("Error: SetCommTimeouts failed.\n");
        CloseHandle(hSerial);
        return;
    }

    // 4. เขียนข้อมูลลงใน Serial Port
    DWORD dataSize = strlen(dataToSend);
    if (!WriteFile(hSerial, dataToSend, dataSize, &bytesWritten, NULL)) {
        printf("Error: WriteFile failed.\n");
        CloseHandle(hSerial);
        return;
    }
    
    printf("Successfully wrote %ld bytes.\n", bytesWritten);
    
    // 5. ปิด Serial Port
    CloseHandle(hSerial);
}

int main() {
    // โค้ดส่วนที่ 1: อัปโหลดโค้ด Arduino ลงบอร์ด
    const char *uploadCommand = "C:\\Users\\Administrator\\Desktop\\arduino-cli.exe compile --upload -b arduino:avr:uno -p COM6 C:\\Users\\Administrator\\Desktop\\ALU4B-Controller/ALU4B-Controller/ALU4B-Controller.ino";
    
    printf("Executing Arduino CLI command...\n");
    int result = system(uploadCommand);
    
    if (result != 0) {
        printf("Error: Failed to upload Arduino code. Exiting.\n");
        return 1; // จบโปรแกรมหากอัปโหลดไม่สำเร็จ
    }
    
    printf("Arduino code uploaded successfully. Waiting for 3 seconds...\n");
    Sleep(3000); // หน่วงเวลา 3 วินาทีเพื่อให้บอร์ดรีบูตและพร้อมใช้งาน

    // โค้ดส่วนที่ 2: ส่งข้อมูล Serial ไปให้ Arduino
    const char* dataToSend = "0110 1 0\n"; // ตัวอย่าง: NOT (011), บวก (0), A=1, B=0
    printf("Sending data to COM3: %s", dataToSend);
    sendSerialData(dataToSend);

    return 0;
}