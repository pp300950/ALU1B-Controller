#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ฟังก์ชันสำหรับแปลงเลขฐานสองสตริงเป็นเลขทศนิยม
int binToDec(char *bin) {
    int dec = 0;
    int power = 1;
    for (int i = strlen(bin) - 1; i >= 0; i--) {
        if (bin[i] == '1') {
            dec += power;
        }
        power *= 2;
    }
    return dec;
}

// ฟังก์ชันสำหรับแปลงเลขทศนิยมเป็นเลขฐานสองสตริง 4 บิต
void decToBin(int dec, char *bin) {
    for (int i = 3; i >= 0; i--) {
        bin[i] = (dec % 2) + '0';
        dec /= 2;
    }
    bin[4] = '\0'; // เพิ่ม null terminator
}

// ฟังก์ชันสำหรับหา Two's Complement ของเลขฐานสอง 4 บิต
void twosComplement(char *bin) {
    char temp[5];
    strcpy(temp, bin);

    // 1. Invert bits (One's Complement)
    for (int i = 0; i < 4; i++) {
        if (temp[i] == '0') {
            temp[i] = '1';
        } else {
            temp[i] = '0';
        }
    }

    // 2. Add 1
    int carry = 1;
    for (int i = 3; i >= 0; i--) {
        if (temp[i] == '1' && carry == 1) {
            temp[i] = '0';
        } else if (temp[i] == '0' && carry == 1) {
            temp[i] = '1';
            carry = 0;
        } else {
            // No change, no carry
        }
    }
    strcpy(bin, temp);
}

// ฟังก์ชันสำหรับ ALU 4 บิต
void alu_4bit(char X, char Y1, char Y2, char Y3, char *A_bin, char *B_bin) {
    int A_dec = binToDec(A_bin); // แปลง A จากเลขฐานสองเป็นทศนิยม
    int B_dec = binToDec(B_bin); // แปลง B จากเลขฐานสองเป็นทศนิยม
    int result_dec;
    char result_bin[5]; // สำหรับเก็บผลลัพธ์ 4 บิต
    int is_negative = 0; // แฟล็กสำหรับระบุว่าผลลัพธ์ติดลบหรือไม่ (0 = ไม่ติดลบ, 1 = ติดลบ)

    // ตรวจสอบ X เพื่อกำหนดว่า A เป็นค่าติดลบหรือไม่ (Two's Complement)
    if (X == '1') {
        twosComplement(A_bin); // ทำ Two's Complement ให้กับ A
        A_dec = binToDec(A_bin); // อัปเดตค่า A_dec หลังจากทำ Two's Complement
        A_dec = -A_dec; // เปลี่ยนค่า A ให้เป็นติดลบในทางทศนิยม
    }

    // สร้างโหมดการทำงานจาก Y1 Y2 Y3
    char mode[4];
    mode[0] = Y1;
    mode[1] = Y2;
    mode[2] = Y3;
    mode[3] = '\0';

    printf("Input A (decimal): %d\n", A_dec);
    printf("Input B (decimal): %d\n", B_dec);

    printf("Mode: %s\n", mode);

    if (strcmp(mode, "000") == 0) { // Add (A + B)
        result_dec = A_dec + B_dec;
        printf("Operation: A + B\n");
    } else if (strcmp(mode, "001") == 0) { // Subtract (A - B)
        // การลบ A - B สามารถทำได้โดย A + (-B)
        // โดยการหา Two's Complement ของ B และนำไปบวกกับ A
        char B_twos_comp[5];
        strcpy(B_twos_comp, B_bin); // คัดลอก B_bin เพื่อไม่ให้กระทบค่าเดิม
        twosComplement(B_twos_comp);
        int B_twos_comp_dec = binToDec(B_twos_comp);
        
        result_dec = A_dec + B_twos_comp_dec;
        printf("Operation: A - B\n");
    } else if (strcmp(mode, "010") == 0) { // AND (A & B)
        result_dec = A_dec & B_dec; // ทำ Bitwise AND
        printf("Operation: A & B\n");
    } else if (strcmp(mode, "011") == 0) { // OR (A | B)
        result_dec = A_dec | B_dec; // ทำ Bitwise OR
        printf("Operation: A | B\n");
    } else if (strcmp(mode, "100") == 0) { // XOR (A ^ B)
        result_dec = A_dec ^ B_dec; // ทำ Bitwise XOR
        printf("Operation: A ^ B\n");
    } else if (strcmp(mode, "101") == 0) { // NOT A (~A)
        // สำหรับ NOT A เราจะทำ Bitwise NOT บนไบนารี A โดยตรง
        // เนื่องจากผลลัพธ์อาจจะเป็นค่าลบในระบบ Two's Complement
        for (int i = 0; i < 4; i++) {
            if (A_bin[i] == '0') {
                result_bin[i] = '1';
            } else {
                result_bin[i] = '0';
            }
        }
        result_bin[4] = '\0';
        result_dec = binToDec(result_bin); // แปลงกลับเป็นทศนิยมเพื่อแสดง
        printf("Operation: ~A\n");
    } else if (strcmp(mode, "110") == 0) { // Compare Equal (A == B)
        if (A_dec == B_dec) {
            result_dec = 0; // ถ้าเท่ากัน, ผลลัพธ์ 0000
            strcpy(result_bin, "0000"); // กำหนดผลลัพธ์เป็น "0000"
            printf("Operation: A == B (True)\n");
        } else {
            // ถ้าไม่เท่ากัน, ผลลัพธ์อาจจะไม่ใช่ 0000
            // เพื่อให้สอดคล้องกับ Zero Flag, อาจจะส่งค่าที่ไม่ใช่ 0000 ออกมา
            // ในที่นี้จะใช้ A - B และดูผลลัพธ์
            char B_twos_comp[5];
            strcpy(B_twos_comp, B_bin);
            twosComplement(B_twos_comp);
            int B_twos_comp_dec = binToDec(B_twos_comp);
            result_dec = A_dec + B_twos_comp_dec;
            printf("Operation: A == B (False)\n");
        }
    } else {
        printf("Invalid mode selected.\n");
        return;
    }

    // การจัดการผลลัพธ์ที่เป็นลบและแปลงเป็น Two's Complement สำหรับแสดงผล
    if (result_dec < 0) {
        is_negative = 1; // ตั้งค่าแฟล็กว่าผลลัพธ์ติดลบ
        result_dec = abs(result_dec); // ทำให้เป็นค่าบวกก่อนแปลงเป็นฐานสอง
        // แปลงค่าบวกกลับเป็นฐานสอง แล้วค่อยทำ Two's Complement
        decToBin(result_dec, result_bin);
        twosComplement(result_bin); // ทำ Two's Complement เพื่อให้ได้ค่าลบในรูปฐานสอง
    } else {
        // ตรวจสอบ overflow สำหรับเลขบวก 4 บิต (ค่าสูงสุด 15)
        if (result_dec > 15 && (strcmp(mode, "000") == 0 || strcmp(mode, "001") == 0)) {
            // สำหรับการบวกและลบ ถ้าเกิน 4 บิต อาจจะเกิด overflow
            // เราจะใช้แค่ 4 บิตล่างสุดของผลลัพธ์
            result_dec = result_dec % 16;
        }
        decToBin(result_dec, result_bin);
    }
    
    // พิมพ์ผลลัพธ์
    printf("Output: %d%s\n", is_negative, result_bin);
}

int main() {
    char X;
    char Y1, Y2, Y3;
    char A_bin[5]; // สำหรับเก็บ A 4 บิต + null terminator
    char B_bin[5]; // สำหรับเก็บ B 4 บิต + null terminator

    printf("Enter input (X Y1 Y2 Y3 A1 A2 A3 A4 B1 B2 B3 B4): ");
    scanf(" %c %c %c %c %s %s", &X, &Y1, &Y2, &Y3, A_bin, B_bin);

    // ตรวจสอบความถูกต้องของอินพุต A และ B (ต้องเป็น 4 บิต)
    if (strlen(A_bin) != 4 || strlen(B_bin) != 4) {
        printf("Error: A and B must be 4-bit binary numbers.\n");
        return 1;
    }

    // ตรวจสอบว่าอินพุตเป็น '0' หรือ '1' เท่านั้น
    for (int i = 0; i < 4; i++) {
        if (!((A_bin[i] == '0' || A_bin[i] == '1') && (B_bin[i] == '0' || B_bin[i] == '1'))) {
            printf("Error: A and B must contain only '0' or '1'.\n");
            return 1;
        }
    }

    // เรียกใช้ฟังก์ชัน ALU
    alu_4bit(X, Y1, Y2, Y3, A_bin, B_bin);

    return 0;
}
