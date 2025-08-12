#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// ฟังก์ชันจำลองการทำงานของ ALU 1 บิต
void simulate_alu(int mux_2, int mux_1, int mux_0, int sub_add_pin, int a_input, int b_input, int *result_output, int *carry_output) {
    // กำหนดค่า A และ B สำหรับการคำนวณ
    int A = a_input;
    int B = b_input;

    // จำลองโหมด invert (111)
    if (mux_2 == 1 && mux_1 == 1 && mux_0 == 1) {
        A = b_input;
        B = 0; // ค่า B ไม่สำคัญสำหรับโหมดนี้
    }

    // กำหนดค่า Carry-in
    int Cin = (sub_add_pin == 1) ? 1 : 0;

    // คำนวณตามโหมดการทำงานของ ALU (จำลองจาก pin outputs)
    int halfSum = A ^ B; // A XOR B
    int halfCarry = A & B; // A AND B

    int fullSum = halfSum ^ Cin;
    int fullCarry = halfCarry | (halfSum & Cin);

    // กำหนดค่าผลลัพธ์
    *result_output = fullSum;
    *carry_output = fullCarry;

    // โหมด invert (111) จะส่งผลลัพธ์เป็น Not A
    if (mux_2 == 1 && mux_1 == 1 && mux_0 == 1) {
        *result_output = 1 - A; // Not A
    } else {
        *result_output = fullSum;
    }
}

int main() {
    char input[256];
    printf("ALU Simulator is running. Enter commands (e.g., 001 0 1 0):\n");

    while (fgets(input, sizeof(input), stdin)) {
        char muxCodeStr[4];
        int subAddPin, aInput, bInput;

        // แยกข้อมูลจากบรรทัดอินพุต
        int items = sscanf(input, "%3s %d %d %d", muxCodeStr, &subAddPin, &aInput, &bInput);

        if (items == 4) {
            int mux_2 = muxCodeStr[0] - '0';
            int mux_1 = muxCodeStr[1] - '0';
            int mux_0 = muxCodeStr[2] - '0';

            int resultOutput, carryOutput;
            
            // เรียกฟังก์ชันจำลองการทำงานของ ALU
            simulate_alu(mux_2, mux_1, mux_0, subAddPin, aInput, bInput, &resultOutput, &carryOutput);

            // แสดงผลลัพธ์
            printf("%d %d\n", resultOutput, carryOutput);
        } else {
            printf("Invalid input format. Please use 'XXX D D D' format.\n");
        }
    }
    
    return 0;
}