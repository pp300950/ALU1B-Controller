#ifndef ALU_SIMULATOR_H
#define ALU_SIMULATOR_H

#include <windows.h>
#include <stdbool.h>

// --- โครงสร้างข้อมูลสำหรับคำสั่ง Assembly ---
typedef struct {
    char instruction[10];
    char operand1[50];
    char operand2[50];
    char label[20];
} Instruction;


// --- Global Registers and Memory (ให้ไฟล์อื่นเข้าถึงได้) ---
extern long long REG_A;
extern long long REG_B;
extern long long MEMORY[10];
extern bool CARRY_FLAG;
extern bool ZERO_FLAG;
extern bool SIGN_FLAG;


// --- ประกาศฟังก์ชันหลักๆ ที่จะเรียกใช้จากภายนอก ---

// ฟังก์ชันสำหรับตั้งค่าและเชื่อมต่อ Serial Port
bool setup_environment(const char* cliPath, const char* board, const char* port, const char* inoPath);

// ฟังก์ชันสำหรับรันชุดคำสั่ง Assembly
void executeInstructions(Instruction* instructions, int numInstructions);

// ฟังก์ชันสำหรับปิดการเชื่อมต่อ
void cleanup_environment();

#endif // ALU_SIMULATOR_H