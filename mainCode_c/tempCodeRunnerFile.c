
void executeInstructions(Instruction *instructions, int numInstructions)
{
    // Build a label map for faster jumps
    struct LabelMap
    {
        char label[20];
        int index;
    };
    struct LabelMap labelMap[MAX_INSTRUCTIONS];
    int labelCount = 0;
    for (int i = 0; i < numInstructions; i++)
    {
        if (strlen(instructions[i].label) > 0)
        {
            if (labelCount < MAX_INSTRUCTIONS)
            {
                strcpy(labelMap[labelCount].label, instructions[i].label);
                labelMap[labelCount].index = i;
                labelCount++;
            }
        }
    }

    int pc = 0; // Program Counter
    while (pc < numInstructions)
    {
        Instruction current = instructions[pc];
        bool shouldJump = false;

        // Skip blank lines that are just labels
        if (strlen(current.instruction) == 0 && strlen(current.label) > 0)
        {
            pc++;
            continue;
        }

        printf("\n[PC:%02d] Executing: %s", pc, current.instruction);
        if (strlen(current.operand1) > 0)
            printf(" %s", current.operand1);
        if (strlen(current.operand2) > 0)
            printf(", %s", current.operand2);
        printf("\n");

        // --- Data and Memory Operations ---
        if (strcmp(current.instruction, "DEF") == 0)
        {
            int mem_addr = atoi(current.operand1);
            long long value = atoll(current.operand2);
            if (mem_addr >= 0 && mem_addr < MAX_MEMORY)
                MEMORY[mem_addr] = value;
            printf("      [INFO] DEFINE: MEMORY[%d] = %lld\n", mem_addr, value);
        }
        else if (strcmp(current.instruction, "LOAD") == 0)
        {
            int mem_addr = atoi(current.operand2);
            if (mem_addr >= 0 && mem_addr < MAX_MEMORY)
            {
                setRegisterValue(current.operand1, MEMORY[mem_addr]);
                printf("      [INFO] LOAD: %s = MEMORY[%d] (Value: %lld)\n", current.operand1, mem_addr, getOperandValue(current.operand1));
            }
        }
        else if (strcmp(current.instruction, "STORE") == 0)
        {
            int mem_addr = atoi(current.operand1);
            long long val = getOperandValue(current.operand2);
            if (mem_addr >= 0 && mem_addr < MAX_MEMORY)
                MEMORY[mem_addr] = val;
            printf("      [INFO] STORE: MEMORY[%d] = %s (Value: %lld)\n", mem_addr, current.operand2, val);
        }
        else if (strcmp(current.instruction, "MOV") == 0)
        {
            long long value = getOperandValue(current.operand2);
            setRegisterValue(current.operand1, value);
            printf("      [INFO] MOV: %s = %lld\n", current.operand1, value);
        }

        // --- Hardware ALU Arithmetic Operations ---
        else if (strcmp(current.instruction, "ADD") == 0)
        {
            long long val1 = getOperandValue(current.operand1);
            long long val2 = getOperandValue(current.operand2);
            long long result = executeAluOperation(val1, val2, "001", 0, &CARRY_FLAG);
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            printf("      [INFO] HW_ADD: %s = %lld + %lld -> %lld. Flags: Z=%d S=%d C=%d\n", current.operand1, val1, val2, result, ZERO_FLAG, SIGN_FLAG, CARRY_FLAG);
        }
        else if (strcmp(current.instruction, "SUB") == 0)
        {
            long long val1 = getOperandValue(current.operand1);
            long long val2 = getOperandValue(current.operand2);
            long long result = executeAluOperation(val1, val2, "001", 1, &CARRY_FLAG);
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            printf("      [INFO] HW_SUB: %s = %lld - %lld -> %lld. Flags: Z=%d S=%d C=%d\n", current.operand1, val1, val2, result, ZERO_FLAG, SIGN_FLAG, CARRY_FLAG);
        }

        // --- Hardware ALU Logic Operations ---
        else if (strcmp(current.instruction, "AND") == 0)
        {
            long long val1 = getOperandValue(current.operand1);
            long long val2 = getOperandValue(current.operand2);
            long long result = executeAluOperation(val1, val2, "100", 0, &CARRY_FLAG); // Assuming MUX 100 is AND
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            printf("      [INFO] HW_AND: %s = %lld & %lld -> %lld. Flags: Z=%d S=%d\n", current.operand1, val1, val2, result, ZERO_FLAG, SIGN_FLAG);
        }
        else if (strcmp(current.instruction, "OR") == 0)
        {
            long long val1 = getOperandValue(current.operand1);
            long long val2 = getOperandValue(current.operand2);
            long long result = executeAluOperation(val1, val2, "101", 0, &CARRY_FLAG); // Assuming MUX 101 is OR
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            printf("      [INFO] HW_OR: %s = %lld | %lld -> %lld. Flags: Z=%d S=%d\n", current.operand1, val1, val2, result, ZERO_FLAG, SIGN_FLAG);
        }
        else if (strcmp(current.instruction, "XOR") == 0)
        {
            long long val1 = getOperandValue(current.operand1);
            long long val2 = getOperandValue(current.operand2);
            long long result = executeAluOperation(val1, val2, "110", 0, &CARRY_FLAG); // Assuming MUX 110 is XOR
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            printf("      [INFO] HW_XOR: %s = %lld ^ %lld -> %lld. Flags: Z=%d S=%d\n", current.operand1, val1, val2, result, ZERO_FLAG, SIGN_FLAG);
        }
        else if (strcmp(current.instruction, "NOT") == 0)
        {
            long long val = getOperandValue(current.operand1);
            long long result = executeAluOperation(0, val, "111", 0, &CARRY_FLAG); // MUX 111 is NOT, op1 is ignored
            setRegisterValue(current.operand1, result);
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            printf("      [INFO] HW_NOT: %s = ~%lld -> %lld. Flags: Z=%d S=%d\n", current.operand1, val, result, ZERO_FLAG, SIGN_FLAG);
        }

        // --- PC-Simulated Complex Operations ---
        else if (strcmp(current.instruction, "MUL") == 0)
        {
            printf("      [INFO] กำลังประมวลผล MUL ด้วยหลักการ Shift-and-Add บน ALU...\n");
            long long val1 = getOperandValue(current.operand1); // ตัวตั้งคูณ
            long long val2 = getOperandValue(current.operand2); // ตัวคูณ

            long long result = 0;

            // วนลูปตามจำนวนบิต (long long คือ 64 บิต)
            for (int i = 0; i < 64; i++)
            {
                // ตรวจสอบบิตที่ i ของตัวคูณ (val2)
                if ((val2 >> i) & 1)
                {
                    // ถ้าบิตเป็น 1, ให้บวกตัวตั้งคูณ (val1) ที่ Shift ไป i ครั้งเข้ากับผลลัพธ์
                    // ใช้ ALU ในการบวก
                    result = executeAluOperation(result, val1 << i, "001", 0, &CARRY_FLAG);
                }
                printf("      [STEP] i=%d, val2 bit=%lld, Current result=%lld\n", i, (val2 >> i) & 1, result);
            }

            // ตั้งค่า register ด้วยผลลัพธ์สุดท้าย
            setRegisterValue(current.operand1, result);

            // ตั้งค่า Flags ตามผลลัพธ์สุดท้าย
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);
            CARRY_FLAG = false; // Carry flag มักจะถูกรีเซ็ตหลังการคูณ

            printf("      [INFO] MUL: %s = %lld * %lld -> ผลลัพธ์ %lld\n", current.operand1, val1, val2, result);
            printf("      [INFO] Flags: Z=%d, S=%d, C=%d\n", ZERO_FLAG, SIGN_FLAG, CARRY_FLAG);
        }
        else if (strcmp(current.instruction, "DIV") == 0)
        {
            printf("      [INFO] กำลังประมวลผล DIV ด้วยหลักการ Shift-and-Subtract บน ALU...\n");
            long long val1 = getOperandValue(current.operand1);
            long long val2 = getOperandValue(current.operand2);
            if (val2 == 0)
            {
                printf("      [ERROR] Division by zero!\n");
                break;
            }

            long long remainder = 0;
            long long quotient = 0;

            for (int i = 63; i >= 0; i--)
            {
                // Shift remainder to the left by 1 bit
                remainder = (remainder << 1) | ((val1 >> i) & 1);

                // Use ALU to compare remainder with divisor (val2)
                // CMP is essentially a subtraction (remainder - val2)
                long long cmp_result = executeAluOperation(remainder, val2, "001", 1, &CARRY_FLAG);

                printf("      [STEP] i=%d, Current Remainder=%lld, Divisor=%lld, CARRY_FLAG=%d\n", i, remainder, val2, CARRY_FLAG);

                // If remainder is greater than or equal to divisor (no borrow)
                if (!CARRY_FLAG)
                {
                    // Use ALU to subtract
                    remainder = cmp_result;
                    // Use ALU to set the quotient bit
                    quotient = executeAluOperation(quotient, 1LL << i, "001", 0, &CARRY_FLAG);
                }
            }

            // Set the register with the final quotient
            setRegisterValue(current.operand1, quotient);

            // Set flags based on the final result
            ZERO_FLAG = (quotient == 0);
            SIGN_FLAG = (quotient < 0);
            CARRY_FLAG = false; // Carry flag is usually reset after division

            printf("      [INFO] DIV: %s = %lld / %lld -> ผลลัพธ์ %lld, เศษ %lld\n", current.operand1, val1, val2, quotient, remainder);
            printf("      [INFO] Flags: Z=%d, S=%d, C=%d\n", ZERO_FLAG, SIGN_FLAG, CARRY_FLAG);
        }

        // --- Control Flow and I/O ---
        else if (strcmp(current.instruction, "PRINT") == 0)
        {
            long long valueToPrint = getOperandValue(current.operand2);
            printf("\n>>> [OUTPUT] %s %lld <<<\n", current.operand1, valueToPrint);
        }
        else if (strcmp(current.instruction, "CMP") == 0)
        {
            long long op1_val = getOperandValue(current.operand1);
            long long op2_val = getOperandValue(current.operand2);

            // ใช้ ALU ในการลบแทนการใช้โอเปอเรเตอร์ '-' ของภาษา C
            // executeAluOperation ในโหมด SUB ("001", 1) จะทำการลบและตั้งค่า CARRY_FLAG
            long long result = executeAluOperation(op1_val, op2_val, "001", 1, &CARRY_FLAG);

            // ตั้งค่า ZERO_FLAG และ SIGN_FLAG ตามผลลัพธ์ที่ได้จาก ALU
            ZERO_FLAG = (result == 0);
            SIGN_FLAG = (result < 0);

            // CARRY_FLAG ถูกตั้งค่าโดย executeAluOperation อยู่แล้ว

            printf("      [INFO] CMP: %lld vs %lld. ผลลัพธ์จากการลบ=%lld. Flags: Z=%d, S=%d, C=%d\n", op1_val, op2_val, result, ZERO_FLAG, SIGN_FLAG, CARRY_FLAG);
        }
        else if (strcmp(current.instruction, "JMP") == 0 || strcmp(current.instruction, "JZ") == 0 || strcmp(current.instruction, "JNZ") == 0 || strcmp(current.instruction, "JS") == 0 || strcmp(current.instruction, "JNS") == 0 || strcmp(current.instruction, "JC") == 0 || strcmp(current.instruction, "JNC") == 0)
        {
            bool do_the_jump =
                (strcmp(current.instruction, "JMP") == 0) ||
                (strcmp(current.instruction, "JZ") == 0 && ZERO_FLAG) ||   // Jump if Zero (Equal)
                (strcmp(current.instruction, "JNZ") == 0 && !ZERO_FLAG) || // Jump if Not Zero (Not Equal)
                (strcmp(current.instruction, "JS") == 0 && SIGN_FLAG) ||   // Jump if Sign (Negative)
                (strcmp(current.instruction, "JNS") == 0 && !SIGN_FLAG) || // Jump if Not Sign (Positive)
                (strcmp(current.instruction, "JC") == 0 && CARRY_FLAG) ||  // Jump if Carry (Less than)
                (strcmp(current.instruction, "JNC") == 0 && !CARRY_FLAG);  // Jump if No Carry (Greater or Equal)

            if (do_the_jump)
            {
                for (int i = 0; i < labelCount; i++)
                {
                    if (strcmp(labelMap[i].label, current.operand1) == 0)
                    {
                        pc = labelMap[i].index;
                        shouldJump = true;
                        printf("      [INFO] JUMP to '%s' (PC -> %d)\n", current.operand1, pc);
                        break;
                    }
                }
            }
            else
            {
                printf("      [INFO] JUMP condition false. No jump.\n");
            }
        }
        else if (strcmp(current.instruction, "HLT") == 0)
        {
            printf("      [INFO] HLT. Program terminated.\n");
            break;
        }

        if (!shouldJump)
        {
            pc++;
        }
    }
}
