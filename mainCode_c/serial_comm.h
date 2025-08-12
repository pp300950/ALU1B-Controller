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

    // ตั้งค่า timeouts สำหรับการอ่านแบบวนลูป
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

// ปรับ sendAndReceiveData: เคลียร์ buffer ก่อนเขียน, อย่าเคลียร์หลังเขียน
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

    // เคลียร์ข้อมูลเก่าใน buffer ก่อนส่ง (อย่าเคลียร์หลังส่ง)
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

    // ให้บอร์ดมีเวลาตอบ (ปรับค่าได้ตามความเร็วของบอร์ด)
    Sleep(5);

    DWORD totalBytesRead = 0;
    // อ่านจนไม่มีข้อมูลเข้ามา (หรือเต็ม buffer)
    do
    {
        if (totalBytesRead >= MAX_READ_BUFFER - 1)
            break;
        if (ReadFile(hSerial, readBuffer + totalBytesRead, MAX_READ_BUFFER - 1 - totalBytesRead, &bytesRead, NULL) && bytesRead > 0)
        {
            totalBytesRead += bytesRead;
            // ถ้าเจอ newline อาจหยุดอ่าน (option)
            if (strchr(readBuffer, '\n'))
                break;
        }
        else
        {
            // ไม่มีข้อมูลเพิ่ม
            break;
        }
    } while (true);

    readBuffer[totalBytesRead] = '\0';
    if (totalBytesRead > 0)
    {
        // ตัด newline/CR ออกก่อน sscanf
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

char *binaryOp(const char *num1, const char *num2, const char *op, bool *isNeg)
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
        return NULL;
    }

    size_t len1 = strlen(num1);
    size_t len2 = strlen(num2);
    size_t maxLen = (len1 > len2) ? len1 : len2;

    char *paddedNum1 = padBinaryString(num1, maxLen);
    char *paddedNum2 = padBinaryString(num2, maxLen);

    if (!paddedNum1 || !paddedNum2)
    {
        if (paddedNum1)
            free(paddedNum1);
        if (paddedNum2)
            free(paddedNum2);
        printf("[ERROR] Memory allocation failed\n");
        return NULL;
    }

    // result length = carry(1) + bits(maxLen) + null
    char *result = (char *)malloc(sizeof(char) * (maxLen + 2)); // Allocate size for potential overflow bit
    if (!result)
    {
        free(paddedNum1);
        free(paddedNum2);
        printf("[ERROR] Memory allocation failed\n");
        return NULL;
    }
    result[maxLen + 1] = '\0';

    int carry_in = (op_mode == 1) ? 1 : 0;
    int alu_result_sum = 0, alu_carry_sum = 0;
    // ถ้าคุณต้องการยังเรียก ALU invert จริง ๆ ให้แก้ที่นี่ แต่ตอนนี้ใช้ invert ในซอฟต์แวร์
    for (int i = 0; i < (int)maxLen; i++)
    {
        int bitA = paddedNum1[maxLen - 1 - i] - '0';
        int bitB = paddedNum2[maxLen - 1 - i] - '0';

        printf("\n[STEP %d] กำลังคำนวณบิต: A=%d, B=%d, Carry-in=%d\n", i + 1, bitA, bitB, carry_in);

        int useB = bitB;
        if (op_mode == 1)
        {
            // invert B ในซอฟต์แวร์ (flip bit) แทนเรียก ALU invert
            useB = 1 - bitB;
            printf("[INFO] ทำการ invert B ในซอฟต์แวร์: B_inv=%d\n", useB);
        }

        // เรียก ALU เพื่อทำ half-adder ของ A กับ useB
        char command_add[32];
        snprintf(command_add, sizeof(command_add), "001 0 %d %d\n", bitA, useB);
        if (!sendAndReceiveData(command_add, &alu_result_sum, &alu_carry_sum))
        {
            printf("[ERROR] Failed to perform operation for bit %d\n", i);
            free(paddedNum1);
            free(paddedNum2);
            free(result);
            return NULL;
        }

        // full-adder: sum = (a^b)^cin, carry_out = (a&b) | ((a^b)&cin)
        int sum_with_carry = alu_result_sum ^ carry_in;
        int new_carry = (alu_carry_sum) | (alu_result_sum & carry_in);

        result[maxLen - i] = sum_with_carry + '0';
        carry_in = new_carry;

        printf("[INFO] ALU half-add: halfSum=%d halfCarry=%d -> sum=%d carry=%d\n",
               alu_result_sum, alu_carry_sum, sum_with_carry, new_carry);
    }

    free(paddedNum1);
    free(paddedNum2);

    // เก็บ carry-out ที่ตำแหน่ง 0
    result[0] = carry_in + '0';

    char *finalResult = NULL;
    if (op_mode == 1)
    { // ถ้าเป็นคำสั่ง SUB
        if (carry_in == 1)
        { // A >= B (ผลลัพธ์เป็นบวก)
            *isNeg = false;
            // ตัดบิต carry_out ที่เกินมาทิ้ง
            // ผลลัพธ์ที่ถูกต้องคือ result[1..maxLen]
            finalResult = (char *)malloc(sizeof(char) * (maxLen + 1));
            strncpy(finalResult, result + 1, maxLen);
            finalResult[maxLen] = '\0';
        }
        else
        { // A < B (ผลลัพธ์เป็นลบ)
            *isNeg = true;
            // ทำ two's complement ของผลลัพธ์เพื่อได้ค่า magnitude
            char *magnitude = (char *)malloc(sizeof(char) * (maxLen + 1));
            // Invert bits
            for (size_t i = 1; i <= maxLen; i++)
            {
                magnitude[i - 1] = (result[i] == '0') ? '1' : '0';
            }
            magnitude[maxLen] = '\0';
            // Add 1
            int carry = 1;
            for (int i = (int)maxLen - 1; i >= 0; i--)
            {
                int bit = magnitude[i] - '0';
                int sum = bit ^ carry;
                carry = bit & carry;
                magnitude[i] = sum + '0';
            }
            finalResult = magnitude;
        }
    }
    else
    { // ถ้าเป็นคำสั่ง ADD
        *isNeg = false;
        // ตัดบิต carry_out ทิ้งหากไม่ต้องการให้ผลลัพธ์เกินขนาด
        // หรือเก็บไว้หากต้องการให้รองรับ overflow
        finalResult = _strdup(result);
    }

    free(result); // Free the temporary result

    return finalResult;
}

// เรียก arduino-cli
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

    char op[10];
    long long dec1, dec2;

    printf("โปรดป้อนคำสั่ง (ADD/SUB) ตามด้วยเลขฐานสิบสองตัว (เช่น ADD 23 15):\n");
    if (scanf("%s %lld %lld", op, &dec1, &dec2) != 3)
    {
        printf("[ERROR] รูปแบบอินพุตไม่ถูกต้อง\n");
        CloseHandle(hSerial);
        return 1;
    }

    char *num1 = decimalToBinary(dec1);
    char *num2 = decimalToBinary(dec2);

    if (!num1 || !num2)
    {
        printf("[ERROR] Memory allocation failed\n");
        if (num1)
            free(num1);
        if (num2)
            free(num2);
        CloseHandle(hSerial);
        return 1;
    }

    bool isNegative = false;
    char *result = binaryOp(num1, num2, op, &isNegative);

    if (result != NULL)
    {
        printf("\n--- ผลลัพธ์สุดท้าย ---\n");
        printf("คำสั่ง: %s\n", op);

        long long decResult = binaryToDecimal(result);

        printf("  %s (ฐาน 2) = %lld (ฐาน 10)\n", num1, dec1);

        if (strcmp(op, "SUB") == 0)
        {
            printf("- %s (ฐาน 2) = %lld (ฐาน 10)\n", num2, dec2);
        }
        else
        {
            printf("+ %s (ฐาน 2) = %lld (ฐาน 10)\n", num2, dec2);
        }

        printf("---------------------\n");

        if (isNegative)
        {
            printf("  -%s (ฐาน 2) = -%lld (ฐาน 10)\n", result, decResult);
        }
        else
        {
            printf("  %s (ฐาน 2) = %lld (ฐาน 10)\n", result, decResult);
        }

        free(result);
        free(num1);
        free(num2);
    }
    else
    {
        printf("[ERROR] ไม่สามารถคำนวณได้\n");
        if (num1)
            free(num1);
        if (num2)
            free(num2);
    }

    clearSerialBuffer();
    CloseHandle(hSerial);
    printf("[DEBUG] ปิดพอร์ตซีเรียลแล้ว เย่ๆ\n");

    return 0;
}