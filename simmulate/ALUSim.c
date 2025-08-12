#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include <locale.h>

// ===================================================================================
// ส่วนของโค้ดจำลอง ALU (ALU Simulator)
// ===================================================================================
void simulate_alu(int mux_2, int mux_1, int mux_0, int sub_add_pin, int a_input, int b_input, int *result_output, int *carry_output) {
    int A = a_input;
    int B = b_input;
    
    // โหมด invert (111) จะใช้ A เป็น B
    if (mux_2 == 1 && mux_1 == 1 && mux_0 == 1) {
        A = b_input;
        B = 0; // ค่า B ไม่สำคัญสำหรับโหมดนี้
    }

    int Cin = (sub_add_pin == 1) ? 1 : 0;

    // คำนวณแบบ Half-adder
    int halfSum = A ^ B;
    int halfCarry = A & B;

    // คำนวณแบบ Full-adder
    int fullSum = halfSum ^ Cin;
    int fullCarry = halfCarry | (halfSum & Cin);
    
    // กำหนดค่าผลลัพธ์ตามโหมด
    if (mux_2 == 1 && mux_1 == 1 && mux_0 == 1) {
        *result_output = 1 - A; // Not A (invert B จาก input)
        *carry_output = 0;
    } else {
        *result_output = fullSum;
        *carry_output = fullCarry;
    }
}

// ฟังก์ชันจำลองการส่งและรับข้อมูล (แทนที่ sendAndReceiveData เดิม)
bool simulateAndReceiveData(const char *dataToSend, int *resultOutput, int *carryOutput) {
    char muxCodeStr[4];
    int subAddPin, aInput, bInput;

    printf("[DEBUG] กำลังประมวลผลข้อมูลจำลอง: \"%s\"\n", dataToSend);

    int items = sscanf(dataToSend, "%3s %d %d %d", muxCodeStr, &subAddPin, &aInput, &bInput);

    if (items == 4) {
        int mux_2 = muxCodeStr[0] - '0';
        int mux_1 = muxCodeStr[1] - '0';
        int mux_0 = muxCodeStr[2] - '0';

        // เรียกใช้ฟังก์ชันจำลอง ALU
        simulate_alu(mux_2, mux_1, mux_0, subAddPin, aInput, bInput, resultOutput, carryOutput);
        
        printf("[INFO] ได้รับข้อมูลจำลอง: %d %d\n", *resultOutput, *carryOutput);
        return true;
    } else {
        printf("[ERROR] รูปแบบข้อมูลที่ส่งไม่ถูกต้อง: \"%s\"\n", dataToSend);
        return false;
    }
}

// ===================================================================================
// โค้ดหลักของโปรแกรม (ตัดส่วน Serial Port ออกแล้ว)
// ===================================================================================

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
    
    char *result = (char *)malloc(sizeof(char) * (maxLen + 2));
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
    
    for (int i = 0; i < (int)maxLen; i++)
    {
        int bitA = paddedNum1[maxLen - 1 - i] - '0';
        int bitB = paddedNum2[maxLen - 1 - i] - '0';
        
        printf("\n[STEP %d] กำลังคำนวณบิต: A=%d, B=%d, Carry-in=%d\n", i + 1, bitA, bitB, carry_in);
        
        int useB = bitB;
        if (op_mode == 1)
        {
            // invert B ในซอฟต์แวร์
            useB = 1 - bitB;
            printf("[INFO] ทำการ invert B ในซอฟต์แวร์: B_inv=%d\n", useB);
        }
        
        // เรียก ALU เพื่อทำ half-adder ของ A กับ useB
        char command_add[32];
        snprintf(command_add, sizeof(command_add), "001 0 %d %d\n", bitA, useB);
        
        // *** เปลี่ยนมาเรียกใช้ฟังก์ชันจำลองแทน ***
        if (!simulateAndReceiveData(command_add, &alu_result_sum, &alu_carry_sum))
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
    {
        if (carry_in == 1)
        {
            *isNeg = false;
            finalResult = (char *)malloc(sizeof(char) * (maxLen + 1));
            strncpy(finalResult, result + 1, maxLen);
            finalResult[maxLen] = '\0';
        }
        else
        {
            *isNeg = true;
            char *magnitude = (char *)malloc(sizeof(char) * (maxLen + 1));
            for (size_t i = 1; i <= maxLen; i++)
            {
                magnitude[i - 1] = (result[i] == '0') ? '1' : '0';
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
            finalResult = magnitude;
        }
    }
    else
    {
        *isNeg = false;
        finalResult = _strdup(result);
    }
    
    free(result);
    
    return finalResult;
}

int main()
{
    SetConsoleOutputCP(CP_UTF8);

    // *** ตัดส่วนที่เกี่ยวข้องกับการอัปโหลดโค้ดและ Serial Port ออก ***
    // const char *arduinoCliPath = "...";
    // const char *boardType = "...";
    // const char *inoFilePath = "...";
    // SetConsoleCtrlHandler(ConsoleHandler, TRUE);
    // if (!executeArduinoCLI(arduinoCliPath, boardType, COM_PORT, inoFilePath)) { return 1; }
    // printf("[INFO] กำลังรอให้บอร์ดพร้อมใช้งานเป็นเวลา %d มิลลิวินาที...\n", UPLOAD_WAIT_MS);
    // Sleep(UPLOAD_WAIT_MS);
    // hSerial = openAndSetupSerialPort();
    // if (hSerial == INVALID_HANDLE_VALUE) { return 1; }
    
    char op[10];
    long long dec1, dec2;
    
    printf("โปรดป้อนคำสั่ง (ADD/SUB) ตามด้วยเลขฐานสิบสองตัว (เช่น ADD 23 15):\n");
    if (scanf("%s %lld %lld", op, &dec1, &dec2) != 3)
    {
        printf("[ERROR] รูปแบบอินพุตไม่ถูกต้อง\n");
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
        return 1;
    }
    
    bool isNegative = false;
    char *result = binaryOp(num1, num2, op, &isNegative);
    
    if (result != NULL)
    {
        printf("\n--- ผลลัพธ์สุดท้าย ---\n");
        printf("คำสั่ง: %s\n", op);
        
        long long decResult = binaryToDecimal(result);
        
        printf("  %s (ฐาน 2) = %lld (ฐาน 10)\n", num1, dec1);
        
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
            printf("  -%s (ฐาน 2) = -%lld (ฐาน 10)\n", result, decResult);
        }
        else
        {
            printf("  %s (ฐาน 2) = %lld (ฐาน 10)\n", result, decResult);
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
    
    // *** ตัดการปิด Serial Port ออก ***
    // clearSerialBuffer();
    // CloseHandle(hSerial);
    // printf("[DEBUG] ปิดพอร์ตซีเรียลแล้ว เย่ๆ\n");
    
    return 0;
}