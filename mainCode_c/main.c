#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include <locale.h>
#include "serial_comm2.h" // เรียกใช้ไลบรารีที่เราสร้าง

#define UPLOAD_WAIT_MS 3000
#define READ_TIMEOUT_MS 1000
#define MAX_READ_BUFFER 256
#define NUM_BITS 8

// --- ระบบจัดการตัวแปรและคำสั่งแบบไดนามิก ---

#define COM_PORT "COM3"
#define BAUD_RATE CBR_9600
#define MAX_VARS 10         // จำกัดจำนวนตัวแปร (เท่ากับขนาด MEMORY)
#define MAX_INSTRUCTIONS 100 // จำกัดจำนวนคำสั่งสูงสุด

// ตารางสัญลักษณ์ (Symbol Table) เพื่อจับคู่ชื่อตัวแปรกับตำแหน่งในหน่วยความจำ
typedef struct {
    char name[50];
    int address; // ตำแหน่งใน MEMORY array
} Variable;

Variable var_table[MAX_VARS];
int var_count = 0;

// Array สำหรับเก็บคำสั่ง Assembly ที่จะสร้างขึ้น
Instruction generated_program[MAX_INSTRUCTIONS];
int instruction_count = 0;

// ฟังก์ชันสำหรับค้นหาตัวแปรในตาราง
Variable* find_variable(const char* name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(var_table[i].name, name) == 0) {
            return &var_table[i];
        }
    }
    return NULL;
}

// --- ฟังก์ชัน API ระดับสูง ---

// เทียบเท่ากับ: int var_name = initial_value;
Variable* declare_variable(const char* name, long long initial_value) {
    if (var_count >= MAX_VARS) {
        printf("[COMPILER-ERROR] Too many variables declared.\n");
        return NULL;
    }
    if (find_variable(name) != NULL) {
        printf("[COMPILER-ERROR] Variable '%s' already declared.\n", name);
        return NULL;
    }

    // เพิ่มตัวแปรใหม่เข้าตาราง
    int address = var_count;
    strcpy(var_table[var_count].name, name);
    var_table[var_count].address = address;
    var_count++;
    
    // สร้างคำสั่ง Assembly: DEF <address> <value>
    Instruction instr;
    strcpy(instr.instruction, "DEF");
    sprintf(instr.operand1, "%d", address);
    sprintf(instr.operand2, "%lld", initial_value);
    instr.label[0] = '\0';
    generated_program[instruction_count++] = instr;

    printf("[HIGH-LEVEL] Declared variable '%s' with value %lld at MEMORY[%d]\n", name, initial_value, address);
    return &var_table[var_count - 1];
}

// เทียบเท่ากับ: dest_var = dest_var + value_or_var;
void add_to_variable(const char* dest_var_name, const char* operand_str) {
    Variable* dest_var = find_variable(dest_var_name);
    if (!dest_var) {
        printf("[COMPILER-ERROR] Undeclared variable '%s'\n", dest_var_name);
        return;
    }

    char addr_str[10];
    sprintf(addr_str, "%d", dest_var->address);

    // 1. โหลดค่าปัจจุบันของตัวแปรปลายทางเข้า REG_A
    strcpy(generated_program[instruction_count].instruction, "LOAD");
    strcpy(generated_program[instruction_count].operand1, "REG_A");
    strcpy(generated_program[instruction_count].operand2, addr_str);
    instruction_count++;

    // 2. ทำการบวก
    strcpy(generated_program[instruction_count].instruction, "ADD");
    strcpy(generated_program[instruction_count].operand1, "REG_A");
    strcpy(generated_program[instruction_count].operand2, operand_str); // operand อาจเป็นตัวเลข "1" หรือ "REG_B"
    instruction_count++;
    
    // 3. เก็บผลลัพธ์กลับไปยังหน่วยความจำ
    strcpy(generated_program[instruction_count].instruction, "STORE");
    strcpy(generated_program[instruction_count].operand1, "REG_A");
    strcpy(generated_program[instruction_count].operand2, addr_str);
    instruction_count++;

    printf("[HIGH-LEVEL] Generated ADD operation for '%s'\n", dest_var_name);
}

// ฟังก์ชันสำหรับพิมพ์ค่าของตัวแปรหลังโปรแกรมทำงานเสร็จ
void print_variable_value(const char* message, const char* var_name) {
    Variable* var = find_variable(var_name);
    if(var) {
        printf("\n[OUTPUT] %s%lld\n", message, MEMORY[var->address]);
    } else {
        printf("\n[OUTPUT] Error: Could not find variable '%s' to print.\n", var_name);
    }
}


int main() {
    // --- ตั้งค่า Environment ของ Arduino (เหมือนเดิม) ---
    const char *arduinoCliPath = "C:\\Users\\Administrator\\Desktop\\arduino-cli.exe";
    const char *boardType = "arduino:avr:uno";
    const char *inoFilePath = "C:\\Users\\Administrator\\Desktop\\ALU4B-Controller/ALU4B-Controller/ALU4B-Controller.ino";

    if (!setup_environment(arduinoCliPath, boardType, COM_PORT, inoFilePath)) {
        return 1;
    }

    // --- ส่วนของการเขียนโค้ดระดับสูง ---
    printf("--- Generating Assembly from High-Level Code ---\n");
    
    // เทียบเท่า: int Num = 5;
    declare_variable("Num", 5);

    // เทียบเท่า: Num = Num + 1;
    add_to_variable("Num", "1"); // operand2 คือค่าที่ต้องการบวก

    // (ตัวอย่างเพิ่มเติม)
    // เทียบเท่า: int AnotherNum = 10;
    // declare_variable("AnotherNum", 10);
    // เทียบเท่า: Num = Num + AnotherNum; (ยังไม่รองรับ ต้องใช้ REG_B ช่วย)
    
    // --- จบโปรแกรมและรัน ---
    strcpy(generated_program[instruction_count++].instruction, "HLT"); // เพิ่มคำสั่ง HLT

    // รันโปรแกรมที่สร้างขึ้นทั้งหมด
    executeInstructions(generated_program, instruction_count);

    // --- แสดงผลลัพธ์ ---
    // เทียบเท่า: print("ผลลัพธ์คือ: ", Num)
    print_variable_value("ผลลัพธ์คือ: ", "Num");

    // คืนค่าทรัพยากร
    cleanup_environment();

    return 0;
}