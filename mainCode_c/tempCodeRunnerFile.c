/*
    ในไฟล์ serial_comm.h รองรับคำสั่งดังนี้

    ## 📜 คู่มือ Assembly จำลอง (serial_comm.c)
    - 🛠 ส่วนประกอบหลัก
    - REG_A, REG_B — Register เก็บค่าชั่วคราว
    - MEMORY — หน่วยความจำหลัก
    - CARRY_FLAG — สถานะการบวก/ลบ (true = มี Carry/Borrow)

    - 📋 คำสั่ง Assembly

    - คำสั่งที่รองรับในปัจจุบัน
    - การจัดการข้อมูล:
    - DEF: กำหนดค่าตัวเลขให้กับหน่วยความจำ (Memory)
    - LOAD: โหลดค่าจากหน่วยความจำเข้าสู่รีจิสเตอร์ (Register)
    - STORE: เก็บค่าจากรีจิสเตอร์ลงในหน่วยความจำ
    - MOV: ย้ายค่าจากรีจิสเตอร์หนึ่งไปอีกรีจิสเตอร์หนึ่ง หรือย้ายค่าคงที่เข้าสู่รีจิสเตอร์

    - การคำนวณและตรรกะ:
    - ADD: คำสั่งบวกเลข (ใช้ ALU)
    - SUB: คำสั่งลบเลข (ใช้ ALU)
    - CMP: เปรียบเทียบค่าสองค่าและตั้งค่าแฟล็ก (Zero, Sign, Carry)

    - การควบคุมโปรแกรม:
    - JMP: กระโดดไปยังตำแหน่งที่ระบุ (Label) โดยไม่มีเงื่อนไข
    - JNC: กระโดดเมื่อไม่มีการทด (Carry Flag เป็น false)
    - JZ: กระโดดเมื่อผลลัพธ์เป็นศูนย์ (Zero Flag เป็น true)
    - JNZ: กระโดดเมื่อผลลัพธ์ไม่เป็นศูนย์ (Zero Flag เป็น false)
    - HLT: หยุดการทำงานของโปรแกรม

    ตัวอย่างการใช้คำสั่ง
    Instruction program[] = {
        // กำหนดค่าเริ่มต้นใน Memory
        {"DEF", "0", "9999"},    // MEM[0] = 999
        {"DEF", "1", "3333"},    // MEM[1] = 333

        // โหลดค่าเข้าสู่ Register
        {"LOAD", "REG_A", "0"},  // REG_A = 999
        {"LOAD", "REG_B", "1"},  // REG_B = 333

        // ทำการลบ
        {"SUB", "REG_A", "REG_B"}, // REG_A = 999 - 333

        // สิ้นสุดโปรแกรม
        {"HLT", "", ""},
    };
    ---

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "serial_comm.h"

// สร้างหน่วยความจำสำหรับตัวแปรระดับสูง
typedef struct
{
    char name[32];
    int value;
    bool is_defined;
} Variable;

Variable high_level_memory[16];

// ฟังก์ชันสำหรับค้นหาหรือสร้างตัวแปร
int get_or_create_variable(const char *name)
{
    for (int i = 0; i < 16; ++i)
    {
        if (high_level_memory[i].is_defined && strcmp(high_level_memory[i].name, name) == 0)
        {
            return i;
        }
        if (!high_level_memory[i].is_defined)
        {
            strcpy(high_level_memory[i].name, name);
            high_level_memory[i].is_defined = true;
            return i;
        }
    }
    return -1;
}

// เพิ่มฟังก์ชันสำหรับแยก Operand ออกจากกัน (ไม่ถูกใช้งานในโค้ดนี้ แต่เก็บไว้เผื่อขยาย)
void split_operands(const char *expression, char *op1, char *op, char *op2)
{
    sscanf(expression, "%s %s %s", op1, op, op2);
}

// ฟังก์ชันสำหรับแปลงและประมวลผลคำสั่ง
void parse_and_execute(const char *high_level_code)
{
    char code_copy[1024];
    strcpy(code_copy, high_level_code);

    char *token;
    char *rest = code_copy;

    // สร้างรายการคำสั่ง Assembly ทั้งหมด
    Instruction assembly_program[100];
    int prog_idx = 0;

    token = strtok_r(rest, ";", &rest);

    while (token != NULL)
    {
        while (*token == ' ' || *token == '\t' || *token == '\n')
        {
            token++;
        }
        size_t len = strlen(token);
        while (len > 0 && (token[len - 1] == ' ' || token[len - 1] == '\t' || token[len - 1] == '\n'))
        {
            token[--len] = '\0';
        }

        printf("\nParsing: '%s'\n", token);

        if (strstr(token, "int") == token)
        {
            char var_name[32];
            int var_value;
            if (sscanf(token, "int %s = %d", var_name, &var_value) == 2)
            {
                int mem_address = get_or_create_variable(var_name);
                if (mem_address != -1)
                {
                    high_level_memory[mem_address].value = var_value;
                    sprintf(assembly_program[prog_idx].instruction, "DEF");
                    sprintf(assembly_program[prog_idx].operand1, "%d", mem_address);
                    sprintf(assembly_program[prog_idx++].operand2, "%d", var_value);
                }
            }
        }
        else
        {
            char var_name[32];
            char operand1[32], op[4], operand2[32];
            int assign_idx = strchr(token, '=') - token;
            if (assign_idx > 0)
            {
                char expression[100];
                strncpy(var_name, token, assign_idx);
                var_name[assign_idx] = '\0';

                int i = 0;
                while (var_name[i] == ' ' || var_name[i] == '\t')
                {
                    i++;
                }
                int j = strlen(var_name) - 1;
                while (j >= 0 && (var_name[j] == ' ' || var_name[j] == '\t'))
                {
                    j--;
                }
                var_name[j + 1] = '\0';
                memmove(var_name, var_name + i, j - i + 2);

                strcpy(expression, token + assign_idx + 1);

                if (sscanf(expression, "%s %s %s", operand1, op, operand2) == 3)
                {
                    int mem_addr_ans = get_or_create_variable(var_name);
                    int mem_addr_op1 = get_or_create_variable(operand1);

                    sprintf(assembly_program[prog_idx].instruction, "LOAD");
                    sprintf(assembly_program[prog_idx].operand1, "REG_A");
                    sprintf(assembly_program[prog_idx++].operand2, "%d", mem_addr_op1);

                    if (atoi(operand2) != 0 || (strlen(operand2) == 1 && operand2[0] == '0'))
                    {
                        sprintf(assembly_program[prog_idx].instruction, "MOV");
                        sprintf(assembly_program[prog_idx].operand1, "REG_B");
                        sprintf(assembly_program[prog_idx++].operand2, "%s", operand2);
                    }
                    else
                    {
                        int mem_addr_op2 = get_or_create_variable(operand2);
                        sprintf(assembly_program[prog_idx].instruction, "LOAD");
                        sprintf(assembly_program[prog_idx].operand1, "REG_B");
                        sprintf(assembly_program[prog_idx++].operand2, "%d", mem_addr_op2);
                    }

                    if (strcmp(op, "+") == 0)
                    {
                        sprintf(assembly_program[prog_idx].instruction, "ADD");
                    }
                    else if (strcmp(op, "-") == 0)
                    {
                        sprintf(assembly_program[prog_idx].instruction, "SUB");
                    }
                    sprintf(assembly_program[prog_idx].operand1, "REG_A");
                    sprintf(assembly_program[prog_idx++].operand2, "REG_B");

                    sprintf(assembly_program[prog_idx].instruction, "STORE");
                    sprintf(assembly_program[prog_idx].operand1, "REG_A");
                    sprintf(assembly_program[prog_idx++].operand2, "%d", mem_addr_ans);
                }
            }
        }
        token = strtok_r(NULL, ";", &rest);
    }

    // --- รันและแสดงผลลัพธ์เพียงครั้งเดียว ---
    printf("\n--- รันโค้ด Assembly ทั้งหมด ---\n");
    if (prog_idx > 0)
    {
        sprintf(assembly_program[prog_idx].instruction, "HLT");
        prog_idx++;

        for (int i = 0; i < prog_idx; i++)
        {
            printf("   %s %s %s\n", assembly_program[i].instruction, assembly_program[i].operand1, assembly_program[i].operand2);
        }

        // รันและแสดงผลลัพธ์
        AluRegisters final_registers = executeInstructions(assembly_program, prog_idx);

        // อัปเดตค่าใน high_level_memory ด้วยผลลัพธ์ที่ถูกต้องจาก REG_A
        // โดย MEM[0] เป็นที่อยู่ของตัวแปร 'ans'
        high_level_memory[0].value = final_registers.reg_a;
        
        // แก้ไขส่วนนี้: แสดงผลค่าจาก REG_A และ REG_B โดยตรง
        printf("\n--- ผลลัพธ์สุดท้ายจาก ALU ---\n");
        printf("[INFO] ค่าใน REG_A หลังจากประมวลผล: %lld\n", final_registers.reg_a);
        printf("[INFO] ค่าใน REG_B หลังจากประมวลผล: %lld\n", final_registers.reg_b);
        printf("--- สิ้นสุดการแสดงผล ---\n");
    }
    printf("\n--- สิ้นสุดการรันโค้ด Assembly ---\n");

    printf("\n--- ข้อมูลตัวแปรที่ถูกตั้งค่าใน Interpreter (อัปเดตจากบอร์ด Arduino) ---\n");
    // อัปเดตค่าใน high_level_memory จากบอร์ด Arduino
    for (int i = 0; i < 16; i++)
    {
        if (high_level_memory[i].is_defined)
        {
            // โค้ดเดิมที่พยายามโหลดค่ากลับมา แต่ตอนนี้ไม่จำเป็นแล้ว
            // เพราะเราสามารถแสดงผลจาก REG_A และ REG_B ได้โดยตรง
            // ดังนั้นผมจึงคอมเมนต์ส่วนนี้ไว้เพื่อแสดงให้เห็นว่าไม่จำเป็นต้องมีอีก
            /*
            Instruction load_prog[] = {
                {"LOAD", "REG_A", ""},
                {"HLT", "", ""}};
            sprintf(load_prog[0].operand2, "%d", i);

            AluRegisters result_from_load = executeInstructions(load_prog, 2);
            high_level_memory[i].value = result_from_load.reg_a;
            */

            // แก้ไขส่วนนี้: แสดงผลลัพธ์จาก high_level_memory ที่อัปเดตจาก STORE คำสั่งสุดท้าย
            printf("[INFO] MEM[%d] (%s) = %d\n", i, high_level_memory[i].name, high_level_memory[i].value);
        }
    }
    printf("--- สิ้นสุดการแสดงผลข้อมูล ---\n");
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

    printf("\n--- เริ่มต้นการทำงานของโปรแกรม Interpreter ---\n");

    for (int i = 0; i < 16; ++i)
    {
        high_level_memory[i].is_defined = false;
    }

    const char *user_code =
        "int ans = 100;"
        "int b = 20;"
        "ans = ans + b;";

    parse_and_execute(user_code);

    printf("\n--- สิ้นสุดการทำงานของโปรแกรม ---\n");

    clearSerialBuffer();
    CloseHandle(hSerial);
    printf("[DEBUG] ปิดพอร์ตซีเรียลแล้ว เย่ๆ\n");

    return 0;
}