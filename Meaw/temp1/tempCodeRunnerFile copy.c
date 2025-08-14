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

// Global serial handle
HANDLE hSerial = INVALID_HANDLE_VALUE;

// Struct for instructions
typedef struct
{
    char opcode[16];
    char operand1[32];
    char operand2[32];
} Instruction;

// ===================================================================================
// Clear serial buffer
// ===================================================================================
void clearSerialBuffer()
{
    if (hSerial != INVALID_HANDLE_VALUE)
    {
        PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
    }
}

// ===================================================================================
// Ctrl+C handler
// ===================================================================================
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

// ===================================================================================
// Open and configure serial port
// ===================================================================================
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

    timeouts.ReadIntervalTimeout = 100;
    timeouts.ReadTotalTimeoutConstant = READ_TIMEOUT_MS;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
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

// ===================================================================================
// Execute Arduino CLI
// ===================================================================================
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

// ===================================================================================
// Execute instructions
// ===================================================================================
void executeInstructions(Instruction *program, int numInstructions)
{
    for (int i = 0; i < numInstructions; i++)
    {
        printf("[DEBUG] Executing: %s %s %s\n",
               program[i].opcode,
               program[i].operand1,
               program[i].operand2);

        // TODO: Send command to Arduino via Serial
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "%s %s %s\n",
                 program[i].opcode,
                 program[i].operand1,
                 program[i].operand2);

        DWORD bytesWritten;
        WriteFile(hSerial, buffer, strlen(buffer), &bytesWritten, NULL);
        Sleep(50); // allow Arduino time to process
    }
}

// ส่งคำสั่ง ALU 1 บรรทัด และอ่านผลลัพธ์ "<result> <carry>\n"
bool sendALUCommand(const char *mux3, int subAdd, int a, int b, int carryIn,
                    int *outResult, int *outCarry)
{
    if (!mux3 || strlen(mux3) != 3)
    {
        printf("[ERROR] mux3 ต้องยาว 3 ตัวอักษร เช่น \"001\"\n");
        return false;
    }
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        printf("[ERROR] Serial ยังไม่เปิด\n");
        return false;
    }

    // เตรียมสตริงคำสั่งตามรูปแบบที่ Arduino รอรับ
    char line[64];
    // ใส่ \n ปิดท้ายบรรทัด เสมอ!
    _snprintf(line, sizeof(line), "%s %d %d %d %d\n",
              mux3, subAdd, a, b, carryIn);

    // ล้างบัฟเฟอร์เก่า ๆ กัน echo/เศษข้อมูลค้าง
    clearSerialBuffer();

    // เขียนไปยังพอร์ต
    DWORD written = 0;
    BOOL ok = WriteFile(hSerial, line, (DWORD)strlen(line), &written, NULL);
    if (!ok || written != strlen(line))
    {
        printf("[ERROR] เขียนพอร์ตล้มเหลว\n");
        return false;
    }

    // รออ่านบรรทัดตอบกลับ เช่น "1 0\n"
    char resp[64] = {0};
    DWORD totalRead = 0;
    DWORD bytesRead = 0;

    // อ่านทีละก้อนจนกว่าจะเจอ '\n' หรือเต็มบัฟเฟอร์
    for (;;)
    {
        ok = ReadFile(hSerial, resp + totalRead, sizeof(resp) - 1 - totalRead, &bytesRead, NULL);
        if (!ok)
        {
            printf("[ERROR] อ่านพอร์ตล้มเหลว\n");
            return false;
        }
        if (bytesRead == 0)
        {
            // timeout ตาม READ_TIMEOUT_MS — จะวนอ่านต่อได้อีกเล็กน้อยก็ได้
            // แต่เพื่อความเรียบง่าย ออกจากลูปถ้าไม่มีอะไรเข้า
            break;
        }
        totalRead += bytesRead;
        resp[totalRead] = '\0';
        if (strchr(resp, '\n'))
            break; // ได้บรรทัดครบแล้ว
        if (totalRead >= sizeof(resp) - 2)
            break; // กันบัฟเฟอร์ล้น
    }

    // แกะค่า result และ carry จากสตริงที่ได้
    int r = -1, c = -1;
    if (sscanf(resp, "%d %d", &r, &c) == 2)
    {
        if (outResult)
            *outResult = r;
        if (outCarry)
            *outCarry = c;
        return true;
    }
    else
    {
        printf("[ERROR] ตีความผลลัพธ์ไม่ได้: \"%s\"\n", resp);
        return false;
    }
}

/**
 * quickTestOneOp
 * ----------------
 * หน้าที่:
 *   - ส่งคำสั่งควบคุม ALU (1 บรรทัด) ไปที่ Arduino ผ่าน serial
 *   - รออ่านผลลัพธ์ที่ Arduino ตอบกลับ แล้วพิมพ์ผลลัพธ์บนคอนโซล
 *
 * โปรโตคอลระหว่าง PC <-> Arduino:
 *   ส่ง: "<mux3> <subAdd> <A> <B> <carryIn>\n"
 *        - mux3   : สตริงยาว 3 ตัวอักษรของ '0'/'1' (เช่น "001") ใช้เลือกช่อง MUX
 *        - subAdd : 0 = โหมดลอจิก, 1 = โหมดบวก/ลบ (ตามที่คุณออกแบบ)
 *        - A,B    : บิตอินพุตของ ALU (0 หรือ 1)
 *        - carryIn: บิตพกเข้าของฟูลแอดเดอร์ (0 หรือ 1)
 *   รับ: "<result> <carry>\n"
 *        - result : บิตผลลัพธ์จาก ALU (0/1)
 *        - carry  : บิตพกออกของฟูลแอดเดอร์ (0/1)
 *
 * เงื่อนไขก่อนเรียกใช้:
 *   - พอร์ตอนุกรม (hSerial) ต้องถูกเปิดและตั้งค่าแล้ว (openAndSetupSerialPort)
 *   - มีฟังก์ชัน sendALUCommand(...) ที่ทำงานส่ง/รับจริงผ่านพอร์ต และคืนค่า true/false
 *
 * ขั้นตอนทำงานของฟังก์ชันนี้:
 *   1) สร้างตัวแปร result, carry เป็นค่าเริ่มต้น -1 (เผื่อดีบักว่าอ่านไม่สำเร็จ)
 *   2) เรียก sendALUCommand(...) เพื่อส่งคำสั่งไป Arduino และอ่าน "<result> <carry>\n" กลับมา
 *   3) ถ้าสำเร็จ: พิมพ์บรรทัด [OK] พร้อม echo คำสั่งและค่าที่อ่านได้
 *      ถ้าล้มเหลว: พิมพ์บรรทัด [FAIL] พร้อม echo คำสั่ง
 *
 * หมายเหตุ:
 *   - การบล็อกรออ่าน/timeout จัดการอยู่ใน sendALUCommand(...)
 *   - ข้อผิดพลาดที่พบบ่อยคือ: ลืมใส่ '\n' ตอนส่งคำสั่ง, หรือ hSerial ยังไม่เปิด
 */
void quickTestOneOp(const char *mux3, int subAdd, int a, int b, int carryIn)
{
    // ค่าเริ่มต้นเป็น -1 เพื่อให้เห็นชัดว่าถ้าอ่านไม่สำเร็จจะไม่ใช่ 0/1
    int result = -1, carry = -1;

    // เรียกทำงานจริง: ส่งสตริง "mux3 subAdd A B carryIn\n" และรออ่าน "result carry\n"
    if (sendALUCommand(mux3, subAdd, a, b, carryIn, &result, &carry))
    {
        // กรณีสำเร็จ: แสดงทั้งคำสั่งที่ส่งและผลที่อ่านได้
        // ตัวอย่างเอาต์พุต:
        // [OK] CMD="001 1 0 1 0" -> result=1 carry=0
        printf("[OK] CMD=\"%s %d %d %d %d\" -> result=%d carry=%d\n",
               mux3, subAdd, a, b, carryIn, result, carry);
    }
    else
    {
        // กรณีล้มเหลว: แสดงเฉพาะคำสั่ง (รายละเอียดสาเหตุให้ดูจาก sendALUCommand ที่พิมพ์ [ERROR])
        printf("[FAIL] CMD=\"%s %d %d %d %d\"\n",
               mux3, subAdd, a, b, carryIn);
    }
}


// ทดสอบ ALU ครบทุกค่า mux3, subAdd, a, b, carryIn
// ทดสอบเฉพาะ MuxCode ที่อยู่ในตาราง
void runSelectedALUTests()
{
    const char *muxList[] = {"000", "001", "010", "100", "101", "111"};
    int bitList[] = {0, 1};

    for (int mi = 0; mi < 6; mi++) // 6 ค่าของ mux3
    {
        const char *mux = muxList[mi];

        // subAdd มีผลเฉพาะใน "001" (ADD/SUB) เท่านั้น
        int subAddOptions = (strcmp(mux, "001") == 0) ? 2 : 1;

        for (int si = 0; si < subAddOptions; si++) // 0 หรือ 1
        {
            int subAdd = (strcmp(mux, "001") == 0) ? si : 0;

            for (int ai = 0; ai < 2; ai++) // A = 0,1
            {
                for (int bi = 0; bi < 2; bi++) // B = 0,1
                {
                    for (int ci = 0; ci < 2; ci++) // carryIn = 0,1
                    {
                        // สำหรับ "111" (NOT) ไม่สนใจ B ให้ fix B=0 เพื่อลดเคส
                        if (strcmp(mux, "111") == 0 && bi > 0)
                            continue;

                        quickTestOneOp(mux, subAdd, bitList[ai], bitList[bi], bitList[ci]);
                        Sleep(20); // หน่วงนิดเพื่อให้ Arduino ทำงานทัน
                    }
                }
            }
        }
    }
}


// ===================================================================================
// Main function
// ===================================================================================
int main()
{
    SetConsoleOutputCP(CP_UTF8);

    const char *arduinoCliPath = "C:\\Users\\Administrator\\Desktop\\arduino-cli.exe";
    const char *boardType = "arduino:avr:uno";
    const char *inoFilePath = "C:\\Users\\Administrator\\Desktop\\ALU4B-Controller/ALU4B-Controller/ALU4B-Controller.ino";

    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

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

    runSelectedALUTests();

    printf("\n--- สิ้นสุดการทำงานของโปรแกรม ---\n");

    clearSerialBuffer();
    CloseHandle(hSerial);
    printf("[DEBUG] ปิดพอร์ตซีเรียลแล้ว เย่ๆ\n");

    return 0;
}
