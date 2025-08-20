// alu_control.c
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
//#include "serial_comm.h" 

// ฟังก์ชันภายใน: ส่งคำสั่ง ALU ไปยัง Arduino
// muxCode = "000" ถึง "111" (string)
// subAdd  = 0 (บวก / logic) หรือ 1 (ลบ)
// aBit, bBit = 0 หรือ 1
// return: ผลลัพธ์บิตที่ได้จาก Arduino
static int alu_execute_bit(const char *muxCode, int subAdd, int aBit, int bBit) {
    char command[16];
    snprintf(command, sizeof(command), "%s %d %d %d\n", muxCode, subAdd, aBit, bBit);
    if (!sendAndReceiveData(command)) {
        fprintf(stderr, "[ERROR] ส่งข้อมูลไป Arduino ไม่สำเร็จ\n");
        return 0;
    }
    // ในตัวอย่างนี้ assume ว่า sendAndReceiveData() จะแสดงค่าผลลัพธ์
    // ถ้าต้องการให้ return ค่าจริง ต้องแก้ serial_comm.c ให้ส่งค่ากลับมา
    // ตอนนี้ขอสมมติว่าผลลัพธ์อ่านจาก global ตัวแปร หรือ mock
    return 0; // TODO: ดึงค่าจาก serial_comm.c ถ้ามี
}

// ฟังก์ชันบวกเลขไบนารีแบบหลายบิต (Little Endian: bit0 = LSB)
void alu_add_binary(const char *binA, const char *binB, char *result, size_t maxLen) {
    size_t lenA = strlen(binA);
    size_t lenB = strlen(binB);
    size_t maxBits = (lenA > lenB ? lenA : lenB);

    int carry = 0;
    size_t pos = 0;

    for (size_t i = 0; i < maxBits && pos < maxLen - 1; i++) {
        int aBit = (i < lenA) ? (binA[lenA - 1 - i] - '0') : 0;
        int bBit = (i < lenB) ? (binB[lenB - 1 - i] - '0') : 0;

        // ส่งไป ALU เพื่อทำ ADD: สมมติ MUX = 001 คือ ADD
        int sumBit = alu_execute_bit("001", 0, aBit, bBit ^ carry);
        // หมายเหตุ: จริง ๆ ต้องจัดการ carry-out ด้วยจาก Arduino
        // ตอนนี้เป็น mock

        result[pos++] = sumBit + '0';
    }

    result[pos] = '\0';
    // กลับลำดับ string (เพราะตอนบันทึกเป็น LSB-first)
    for (size_t i = 0; i < pos / 2; i++) {
        char tmp = result[i];
        result[i] = result[pos - 1 - i];
        result[pos - 1 - i] = tmp;
    }
}

// ฟังก์ชัน NOT ทุกบิต
void alu_not_binary(const char *binIn, char *result, size_t maxLen) {
    size_t len = strlen(binIn);
    if (len >= maxLen) len = maxLen - 1;

    for (size_t i = 0; i < len; i++) {
        int bit = binIn[i] - '0';
        // MUX = 111 คือ NOT
        int notBit = alu_execute_bit("111", 0, bit, 0);
        result[i] = notBit + '0';
    }
    result[len] = '\0';
}

// ฟังก์ชันเทียบ <, ==, >
int alu_compare_binary(const char *binA, const char *binB) {
    size_t lenA = strlen(binA);
    size_t lenB = strlen(binB);
    size_t maxBits = (lenA > lenB ? lenA : lenB);

    // แปลงให้เท่ากันโดยเติมศูนย์ข้างหน้า
    char aPadded[64], bPadded[64];
    snprintf(aPadded, sizeof(aPadded), "%0*.*s", (int)maxBits, (int)lenA, binA);
    snprintf(bPadded, sizeof(bPadded), "%0*.*s", (int)maxBits, (int)lenB, binB);

    for (size_t i = 0; i < maxBits; i++) {
        int aBit = aPadded[i] - '0';
        int bBit = bPadded[i] - '0';
        if (aBit > bBit) return 1;  // A > B
        if (aBit < bBit) return -1; // A < B
    }
    return 0; // เท่ากัน
}

// ตัวอย่าง dispatcher แปลงคำสั่ง Assembly → เรียกฟังก์ชันที่เกี่ยวข้อง
void alu_process_instruction(const char *instruction) {
    char op[16], arg1[64], arg2[64];
    if (sscanf(instruction, "%15s %63s %63s", op, arg1, arg2) < 2) {
        printf("[ERROR] รูปแบบคำสั่งไม่ถูกต้อง\n");
        return;
    }

    if (strcmp(op, "ADD") == 0) {
        char result[128];
        alu_add_binary(arg1, arg2, result, sizeof(result));
        printf("ADD result: %s\n", result);
    }
    else if (strcmp(op, "NOT") == 0) {
        char result[128];
        alu_not_binary(arg1, result, sizeof(result));
        printf("NOT result: %s\n", result);
    }
    else if (strcmp(op, "CMP") == 0) {
        int cmp = alu_compare_binary(arg1, arg2);
        if (cmp > 0) printf("A > B\n");
        else if (cmp < 0) printf("A < B\n");
        else printf("A == B\n");
    }
    else {
        printf("[ERROR] ไม่รู้จักคำสั่ง %s\n", op);
    }
}
