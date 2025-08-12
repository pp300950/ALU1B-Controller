// alu_control.h
#ifndef ALU_CONTROL_H
#define ALU_CONTROL_H

// ประกาศตัวแปรและโครงสร้าง
extern long long REG_A;
extern long long REG_B;
extern long long MEMORY[10];

typedef struct {
    char instruction[10];
    char operand1[50];
    char operand2[50];
    char label[20];
} Instruction;

// ประกาศ prototypes ของฟังก์ชัน
long long getRegisterValue(const char *regName);
void setRegisterValue(const char *regName, long long value);
void executeInstructions(Instruction *instructions, int numInstructions);

#endif