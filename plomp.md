จากผลลัพนี้
--- Starting Program Simulation ---

--- Compiler: Stage 1 - Parsing and Generating Instructions ---[DEBUG] Init part: 'i = 1'
[DEBUG] Condition part: 'i <= 12'
[DEBUG] Update part: 'i++'

[COMPILER_LOG] --- Entering FOR block ---
[COMPILER_LOG] Registered loop START label 'L0' at PC address 5[COMPILER_LOG] Pushed FOR block. Current block_stack_ptr = 0
[COMPILER_LOG] Generated loop EXIT label 'L1' for the conditional jump.

[PC:00] Executing: PRINT --- Multiplication Table for 5 ---    

>>> [OUTPUT] --- Multiplication Table for 5 --- <<<

[PC:01] Executing: DEF 0, 0
      [INFO] DEFINE: MEMORY[0] = 0

[PC:02] Executing: DEF 1, 0
      [INFO] DEFINE: MEMORY[1] = 0

[PC:03] Executing: MOV REG_A, 1
      [INFO] MOV: REG_A = 1

[PC:04] Executing: STORE 0, REG_A
      [INFO] STORE: MEMORY[0] = REG_A (Value: 1)

[PC:05] Executing: LOAD REG_A, 0
      [INFO] LOAD: REG_A = MEMORY[0] (Value: 1)

[PC:06] Executing: MOV REG_B, 12
      [INFO] MOV: REG_B = 12

[PC:07] Executing: CMP REG_A, REG_B
      [INFO] CMP: เริ่มประมวลผล 1 เปรียบเทียบกับ 12
      [DEBUG] ALU(32-bit) Input: op1=12, op2=0, sub=0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 1 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 111 0 1 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 111 0 0 0 0
[SERIAL RX] <- 1 0
      [DEBUG] ALU Raw 32-bit Result: 0xFFFFFFF3
      [DEBUG] CMP Step 1 (NOT): ~12 (op2_val) -> -13
      [DEBUG] ALU(32-bit) Input: op1=-13, op2=1, sub=0
[SERIAL TX] -> 001 0 1 1 0
[SERIAL RX] <- 0 1
[SERIAL TX] -> 001 0 1 0 1
[SERIAL RX] <- 0 1
[SERIAL TX] -> 001 0 0 0 1
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
      [DEBUG] ALU Raw 32-bit Result: 0xFFFFFFF4
      [DEBUG] CMP Step 2 (ADD 1): -13 + 1 -> -12 (Two's Complement)
      [DEBUG] ALU(32-bit) Input: op1=1, op2=-12, sub=0
[SERIAL TX] -> 001 0 1 0 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
      [DEBUG] ALU Raw 32-bit Result: 0xFFFFFFF5
      [DEBUG] CMP Step 3 (ADD): 1 + -12 (Two's Complement) -> -11
      [INFO] CMP Finished: 1 vs 12. ผลลัพธ์จากการลบ (เพื่อ Flag
s เท่านั้น)=-11. Flags: Z=0, S=1, C=1, O=0

[PC:08] Executing: JGT L1
      [INFO] JUMP condition false. No jump.

[PC:09] Executing: MOV REG_A, 5
      [INFO] MOV: REG_A = 5

[PC:10] Executing: LOAD REG_B, 0
      [INFO] LOAD: REG_B = MEMORY[0] (Value: 1)

[PC:11] Executing: MUL REG_A, REG_B
      [INFO] กำลังประมวลผล MUL ด้วยหลักการ Shift-and-Add บน ALU
...
      [DEBUG] ALU(32-bit) Input: op1=0, op2=5, sub=0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 1 0
[SERIAL RX] <- 1 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
[SERIAL TX] -> 001 0 0 0 0
[SERIAL RX] <- 0 0
      [DEBUG] ALU Raw 32-bit Result: 0x00000005
      [STEP] i=0, val2 bit=1, Current result=5
      [STEP] i=1, val2 bit=0, Current result=5
      [STEP] i=2, val2 bit=0, Current result=5
      [STEP] i=3, val2 bit=0, Current result=5
      [STEP] i=4, val2 bit=0, Current result=5
      [STEP] i=5, val2 bit=0, Current result=5
      [STEP] i=6, val2 bit=0, Current result=5
      [STEP] i=7, val2 bit=0, Current result=5
      [STEP] i=8, val2 bit=0, Current result=5
      [STEP] i=9, val2 bit=0, Current result=5
      [STEP] i=10, val2 bit=0, Current result=5
      [STEP] i=11, val2 bit=0, Current result=5
      [STEP] i=12, val2 bit=0, Current result=5
      [STEP] i=13, val2 bit=0, Current result=5
      [STEP] i=14, val2 bit=0, Current result=5
      [STEP] i=15, val2 bit=0, Current result=5
      [STEP] i=16, val2 bit=0, Current result=5
      [STEP] i=17, val2 bit=0, Current result=5
      [STEP] i=18, val2 bit=0, Current result=5
      [STEP] i=19, val2 bit=0, Current result=5
      [STEP] i=20, val2 bit=0, Current result=5
      [STEP] i=21, val2 bit=0, Current result=5
      [STEP] i=22, val2 bit=0, Current result=5
      [STEP] i=23, val2 bit=0, Current result=5
      [STEP] i=24, val2 bit=0, Current result=5
      [STEP] i=25, val2 bit=0, Current result=5
      [STEP] i=26, val2 bit=0, Current result=5
      [STEP] i=27, val2 bit=0, Current result=5
      [STEP] i=28, val2 bit=0, Current result=5
      [STEP] i=29, val2 bit=0, Current result=5
      [STEP] i=30, val2 bit=0, Current result=5
      [STEP] i=31, val2 bit=0, Current result=5
      [INFO] MUL: REG_A = 5 * 1 -> ผลลัพธ์ 5
      [INFO] Flags: Z=0, S=0, C=0

[PC:12] Executing: STORE 1, REG_A
      [INFO] STORE: MEMORY[1] = REG_A (Value: 5)

[PC:13] Executing: LOAD REG_A, 1
      [INFO] LOAD: REG_A = MEMORY[1] (Value: 5)

[PC:14] Executing: PRINT REG_A

>>> [OUTPUT] 5 <<<

[PC:15] Executing: PRINT #### จบการทำงาน ####

>>> [OUTPUT] #### จบการทำงาน #### <<<

[PC:16] Executing: HLT

--- Program Execution Finished ---
[DEBUG] Serial port closed successfully.
PS C:\Users\Administrator\Desktop\ALU4B-Controller\Use> 

มันผิดพลาดที่ส่วนไหน 
        else if (strncmp(trimmed_line, "for(", 4) == 0)
        {
            // --- ส่วนของการ Parse (เหมือนเดิม) ---
            const char *for_content = trimmed_line + 4;
            const char *semicolon1 = strchr(for_content, ';');
            const char *semicolon2 = strchr(semicolon1 + 1, ';');
            const char *paren_end = strchr(semicolon2 + 1, ')');
            char init[100], cond[100], update[100];
            strncpy(init, for_content, semicolon1 - for_content);
            init[semicolon1 - for_content] = '\0';
            strncpy(cond, semicolon1 + 1, semicolon2 - (semicolon1 + 1));
            cond[semicolon2 - (semicolon1 + 1)] = '\0';
            strncpy(update, semicolon2 + 1, paren_end - (semicolon2 + 1));
            update[paren_end - (semicolon2 + 1)] = '\0';

            printf("[DEBUG] Init part: '%s'\n", trim(init));
            printf("[DEBUG] Condition part: '%s'\n", trim(cond));
            printf("[DEBUG] Update part: '%s'\n", trim(update));

            // --- Part 1: Initialization (เหมือนเดิม) ---
            char initVar[50], initValStr[50];
            if (sscanf(trim(init), "%s = %s", initVar, initValStr) == 2)
            {
                int dest_addr = findVariable(initVar, symbolTable, variableCount);
                if (dest_addr != -1)
                {
                    sprintf(instructions[instructionCount].instruction, "MOV");
                    sprintf(instructions[instructionCount].operand1, "REG_A");
                    sprintf(instructions[instructionCount].operand2, "%s", initValStr);
                    instructionCount++;
                    sprintf(instructions[instructionCount].instruction, "STORE");
                    sprintf(instructions[instructionCount].operand1, "%d", dest_addr);
                    sprintf(instructions[instructionCount].operand2, "REG_A");
                    instructionCount++;
                }
                else
                {
                    printf("ERROR: Variable '%s' not declared.\n", initVar);
                    return NULL;
                }
            }

            // --- Part 2: Condition Check (Start of loop) ---
            
            // 💡 --- LOGGING & FIX POINT --- 💡
            printf("\n[COMPILER_LOG] --- Entering FOR block ---\n");
            
            // สร้างและบันทึก Label สำหรับจุดเริ่มต้นของลูป
            char loop_start_label[20];
            generate_new_label(loop_start_label);
            addLabel(loop_start_label, instructionCount);
            
            // เพิ่ม Log เพื่อยืนยันการบันทึก Label
            printf("[COMPILER_LOG] Registered loop START label '%s' at PC address %d\n", loop_start_label, instructionCount);

            // จัดการข้อมูล loop (Push stacks)
            block_type_stack[++block_stack_ptr] = BLOCK_FOR;
            for_stack_ptr++;
            for_loop_start_stack[for_stack_ptr] = instructionCount;
            strcpy(for_update_statement_stack[for_stack_ptr], trim(update));

            // เพิ่ม Log เพื่อดูสถานะของ Stack
            printf("[COMPILER_LOG] Pushed FOR block. Current block_stack_ptr = %d\n", block_stack_ptr);

            // ... ส่วนที่เหลือของโค้ดเหมือนเดิม ...
            char condVar[50], condOp[10], condVal[50];
            sscanf(trim(cond), "%s %s %s", condVar, condOp, condVal);

            int addr1 = findVariable(trim(condVar), symbolTable, variableCount);
            if (addr1 == -1) { return NULL; }
            sprintf(instructions[instructionCount].instruction, "LOAD");
            sprintf(instructions[instructionCount].operand1, "REG_A");
            sprintf(instructions[instructionCount].operand2, "%d", addr1);
            instructionCount++;

            int addr2 = findVariable(trim(condVal), symbolTable, variableCount);
            if (addr2 != -1) {
                sprintf(instructions[instructionCount].instruction, "LOAD");
                sprintf(instructions[instructionCount].operand1, "REG_B");
                sprintf(instructions[instructionCount].operand2, "%d", addr2);
            } else {
                sprintf(instructions[instructionCount].instruction, "MOV");
                sprintf(instructions[instructionCount].operand1, "REG_B");
                sprintf(instructions[instructionCount].operand2, "%s", trim(condVal));
            }
            instructionCount++;

            sprintf(instructions[instructionCount].instruction, "CMP");
            sprintf(instructions[instructionCount].operand1, "REG_A");
            sprintf(instructions[instructionCount].operand2, "REG_B");
            instructionCount++;

            // --- Part 3: Generate Conditional Jump to Exit Loop (เหมือนเดิม) ---
            char jump_instruction[10];
            int jump_generated = 1;
            if (strcmp(condOp, "==") == 0) strcpy(jump_instruction, "JNE");
            else if (strcmp(condOp, "!=") == 0) strcpy(jump_instruction, "JE");
            else if (strcmp(condOp, "<") == 0) strcpy(jump_instruction, "JGE");
            else if (strcmp(condOp, "<=") == 0) strcpy(jump_instruction, "JGT");
            else if (strcmp(condOp, ">") == 0) strcpy(jump_instruction, "JLE");
            else if (strcmp(condOp, ">=") == 0) strcpy(jump_instruction, "JLT");
            else {
                printf("ERROR: Unsupported operator '%s' in for loop condition.\n", condOp);
                jump_generated = 0;
            }

            if (jump_generated)
            {
                char exit_label[20];
                generate_new_label(exit_label);
                
                // เพิ่ม Log เพื่อดู Label ที่จะใช้กระโดดออก
                printf("[COMPILER_LOG] Generated loop EXIT label '%s' for the conditional jump.\n", exit_label);

                sprintf(instructions[instructionCount].instruction, "%s", jump_instruction);
                sprintf(instructions[instructionCount].operand1, "%s", exit_label);

                jump_fix_stack[++stack_ptr] = instructionCount;
                instructionCount++;
            }
            
            // ❌❌❌ ลบบรรทัด `block_type_stack[++block_stack_ptr] = BLOCK_FOR;` ที่ซ้ำซ้อนจากตรงนี้ ❌❌❌
        }

        else if (strcmp(trimmed_line, "}") == 0)
        {
            if (block_stack_ptr > 0 && block_type_stack[block_stack_ptr] == BLOCK_FOR)
            {
                // --------------------------- FIX 1: เพิ่มส่วนที่ขาดไป ---------------------------
                // --- Part 1: Generate code for the update statement (e.g., i++) ---
                // ดึงคำสั่ง update ที่เก็บไว้ใน stack ออกมา
                char update_statement[100];
                strcpy(update_statement, for_update_statement_stack[for_stack_ptr]);

                // ทำการ Parse คำสั่ง update แบบง่าย (รองรับ var++)
                char update_var[50];
                if (sscanf(update_statement, "%[a-zA-Z0-9_]++", update_var) == 1)
                {
                    int var_addr = findVariable(update_var, symbolTable, variableCount);
                    if (var_addr != -1)
                    {
                        // สร้าง Assembly สำหรับ i = i + 1
                        // LOAD REG_A, [addr_of_i]
                        sprintf(instructions[instructionCount].instruction, "LOAD");
                        sprintf(instructions[instructionCount].operand1, "REG_A");
                        sprintf(instructions[instructionCount].operand2, "%d", var_addr);
                        instructionCount++;

                        // MOV REG_B, 1 (หรือจะใช้ ADD REG_A, 1 เลยก็ได้ถ้า simulator รองรับ)
                        sprintf(instructions[instructionCount].instruction, "MOV");
                        sprintf(instructions[instructionCount].operand1, "REG_B");
                        sprintf(instructions[instructionCount].operand2, "1");
                        instructionCount++;

                        // ADD REG_A, REG_B
                        sprintf(instructions[instructionCount].instruction, "ADD");
                        sprintf(instructions[instructionCount].operand1, "REG_A");
                        sprintf(instructions[instructionCount].operand2, "REG_B");
                        instructionCount++;

                        // STORE [addr_of_i], REG_A
                        sprintf(instructions[instructionCount].instruction, "STORE");
                        sprintf(instructions[instructionCount].operand1, "%d", var_addr);
                        sprintf(instructions[instructionCount].operand2, "REG_A");
                        instructionCount++;
                    }
                }
                // (สามารถเพิ่ม logic สำหรับ i--, i=i+N, etc. ได้ที่นี่)
                // ---------------------------------------------------------------------------------

                // --- Part 2: Jump back to the start of the loop (เหมือนเดิม) ---
                int loop_start_addr = for_loop_start_stack[for_stack_ptr];
                char loop_start_label[20] = "";
                bool found_label = false;
                for (int k = 0; k < labelCount; k++)
                {
                    if (labelMap[k].index == loop_start_addr)
                    {
                        strcpy(loop_start_label, labelMap[k].label);
                        found_label = true;
                        break;
                    }
                }

                if (found_label)
                {
                    sprintf(instructions[instructionCount].instruction, "JMP");
                    sprintf(instructions[instructionCount].operand1, "%s", loop_start_label);
                    instructionCount++;
                }
                else
                {
                    printf("[ERROR] Compiler bug: Could not find start label for address %d\n", loop_start_addr);
                }

                // --------------------------- FIX 2: แก้ไขการจัดการ Label ---------------------------
                // --- Part 3: Fix (Backpatch) the conditional jump out of the loop ---
                int jump_out_pc = jump_fix_stack[stack_ptr];

                // ดึงชื่อ Label ที่ถูกต้องจากคำสั่ง JUMP ที่รอการแก้ไขอยู่
                // ไม่ใช่การสร้างชื่อใหม่ หรือเดาจาก address
                char *exit_label_name = instructions[jump_out_pc].operand1;

                // เพิ่ม Label ที่ถูกต้อง (ที่ JGT กำลังจะกระโดดไป) ลงใน Map
                // โดยให้ชี้มาที่ตำแหน่งโค้ด "หลัง" ลูป ซึ่งก็คือ address ปัจจุบัน
                addLabel(exit_label_name, instructionCount);
                // ---------------------------------------------------------------------------------

                // --- Pop stacks ---
                stack_ptr--;
                for_stack_ptr--;
                block_stack_ptr--;
            }
            // (เพิ่ม else if สำหรับ block อื่นๆ เช่น if, while)
        }

        }
        else if (strcmp(current.instruction, "CMP") == 0)
        {
            long long op1_val = getOperandValue(current.operand1);
            long long op2_val = getOperandValue(current.operand2);

            printf("      [INFO] CMP: เริ่มประมวลผล %lld เปรียบเทียบกับ %lld\n", op1_val, op2_val);

            // ขั้นตอนที่ 1: Invert op2_val (~op2_val) โดยใช้ executeAluOperation
            bool temp_carry_not = false;
            long long val2_invert = executeAluOperation(op2_val, 0, "111", 0, &temp_carry_not);
            printf("      [DEBUG] CMP Step 1 (NOT): ~%lld (op2_val) -> %lld\n", op2_val, val2_invert);

            // ขั้นตอนที่ 2: Add 1 to inverted op2_val (Two's Complement)
            bool temp_carry_add1 = false;
            long long negated_op2_val = executeAluOperation(val2_invert, 1, "001", 0, &temp_carry_add1);
            printf("      [DEBUG] CMP Step 2 (ADD 1): %lld + 1 -> %lld (Two's Complement)\n", val2_invert, negated_op2_val);

            // ขั้นตอนที่ 3: Add op1_val and negated_op2_val
            bool carry_flag_temp = false;
            long long final_result_for_flags = executeAluOperation(op1_val, negated_op2_val, "001", 0, &carry_flag_temp);
            CARRY_FLAG = !carry_flag_temp; // Invert carry for subtract (borrow)
            printf("      [DEBUG] CMP Step 3 (ADD): %lld + %lld (Two's Complement) -> %lld\n", op1_val, negated_op2_val, final_result_for_flags);

            // ตั้งค่า Flags (Zero, Sign, Overflow)
            ZERO_FLAG = (final_result_for_flags == 0);
            SIGN_FLAG = (final_result_for_flags < 0);

            // คำนวณ Overflow Flag: OV = (Sign_of_op1 == Sign_of_op2) && (Sign_of_op1 != Sign_of_result)
            // สำหรับการลบ (A-B), B ถูกทำ Two's Complement, ดังนั้นเราต้องดูเครื่องหมายของ A และ -B
            bool sign_op1 = (op1_val >> (NUM_BITS - 1)) & 1;
            bool sign_op2 = (op2_val >> (NUM_BITS - 1)) & 1;
            bool sign_result = (final_result_for_flags >> (NUM_BITS - 1)) & 1;

            // A - B = A + (-B) -> Overflow เกิดขึ้นเมื่อ A และ -B มีเครื่องหมายเหมือนกัน และผลลัพธ์มีเครื่องหมายต่างออกไป
            OVERFLOW_FLAG = (sign_op1 != sign_op2) && (sign_op1 != sign_result);

            // แสดงผลลัพธ์และ Flags สุดท้ายของ CMP
            printf("      [INFO] CMP Finished: %lld vs %lld. ผลลัพธ์จากการลบ (เพื่อ Flags เท่านั้น)=%lld. Flags: Z=%d, S=%d, C=%d, O=%d\n",
                   op1_val, op2_val, final_result_for_flags, ZERO_FLAG, SIGN_FLAG, CARRY_FLAG, OVERFLOW_FLAG);
        }
        else if (strcmp(current.instruction, "JMP") == 0 ||
                 strcmp(current.instruction, "JZ") == 0 || strcmp(current.instruction, "JE") == 0 ||
                 strcmp(current.instruction, "JNZ") == 0 || strcmp(current.instruction, "JNE") == 0 ||
                 strcmp(current.instruction, "JC") == 0 || strcmp(current.instruction, "JNC") == 0 ||
                 strcmp(current.instruction, "JGT") == 0 || strcmp(current.instruction, "JLT") == 0 ||
                 strcmp(current.instruction, "JGE") == 0 || strcmp(current.instruction, "JLE") == 0)
        {
            bool do_the_jump =
                (strcmp(current.instruction, "JMP") == 0) ||

                // Unsigned jumps
                ((strcmp(current.instruction, "JZ") == 0 || strcmp(current.instruction, "JE") == 0) && ZERO_FLAG) ||    // ZF=1
                ((strcmp(current.instruction, "JNZ") == 0 || strcmp(current.instruction, "JNE") == 0) && !ZERO_FLAG) || // ZF=0
                (strcmp(current.instruction, "JC") == 0 && CARRY_FLAG) ||                                               // CF=1
                (strcmp(current.instruction, "JNC") == 0 && !CARRY_FLAG) ||                                             // CF=0

                // Signed jumps (ใช้ ZERO_FLAG, SIGN_FLAG, OVERFLOW_FLAG ตาม x86)
                (strcmp(current.instruction, "JG") == 0 && !ZERO_FLAG && (SIGN_FLAG == OVERFLOW_FLAG)) ||   // >  : ZF=0 และ SF=OF
                (strcmp(current.instruction, "JGE") == 0 && (SIGN_FLAG == OVERFLOW_FLAG)) ||                // >= : SF=OF
                (strcmp(current.instruction, "JL") == 0 && (SIGN_FLAG != OVERFLOW_FLAG)) ||                 // <  : SF≠OF
                (strcmp(current.instruction, "JLE") == 0 && (ZERO_FLAG || (SIGN_FLAG != OVERFLOW_FLAG))) || // <= : ZF=1 หรือ SF≠OF
                (strcmp(current.instruction, "JO") == 0 && OVERFLOW_FLAG) ||                                // Overflow
                (strcmp(current.instruction, "JNO") == 0 && !OVERFLOW_FLAG);                                // No Overflow

            if (do_the_jump)
            {
                bool label_found = false;
                for (int i = 0; i < labelCount; i++)
                {
                    if (strcmp(labelMap[i].label, current.operand1) == 0)
                    {
                        pc = labelMap[i].index;
                        shouldJump = true;
                        label_found = true;
                        printf("      [INFO] JUMP to '%s' (PC -> %d)\n", current.operand1, pc);
                        break;
                    }
                }
                if (!label_found)
                {
                    printf("      [ERROR] Label '%s' not found for JUMP!\n", current.operand1);
                }
            }
            else
            {
                printf("      [INFO] JUMP condition false. No jump.\n");
            }
        }

        if (!shouldJump)
        {
            pc++;
        }

#define NUM_BITS 32
long long executeAluOperation(long long op1, long long op2, const char *muxCode, int subAddFlag, bool *final_carry)
{
    unsigned long long result_raw = 0;
    int carry_in = (subAddFlag == 1) ? 1 : 0;

    printf("      [DEBUG] ALU(%d-bit) Input: op1=%lld, op2=%lld, sub=%d\n", NUM_BITS, op1, op2, subAddFlag);

    // วนลูปเพื่อประมวลผลทุกบิตโดยไม่มีเงื่อนไข
    for (int i = 0; i < NUM_BITS; i++)
    {
        int bitA = (op1 >> i) & 1;
        int bitB = (op2 >> i) & 1;
        int alu_result_bit = 0, alu_carry_out = 0;

        // --- ลบ IF/ELSE BLOCK ที่เป็นปัญหาออก ---
        // ส่งคำสั่งไปยัง ALU สำหรับทุกๆ บิตเสมอ
        char command[32];
        snprintf(command, sizeof(command), "%s %d %d %d %d\n", muxCode, subAddFlag, bitA, bitB, carry_in);

        if (!sendAndReceiveData(command, &alu_result_bit, &alu_carry_out))
        {
            printf("[FATAL] Hardware communication failed at bit %d. Aborting.\n", i);
            return 0; // Error
        }

        // สร้างผลลัพธ์จากบิตที่ได้กลับมา
        if (alu_result_bit)
        {
            result_raw |= (1ULL << i);
        }
        carry_in = alu_carry_out;
    }

    *final_carry = (carry_in == 1);

    printf("      [DEBUG] ALU Raw %d-bit Result: 0x%08llX\n", NUM_BITS, result_raw);

    long long final_result;

    // การจัดการ Sign Extension สำหรับเลขจำนวนเต็มแบบ 32 บิต
    if (result_raw & (1ULL << (NUM_BITS - 1)))
    {
        final_result = (long long)(result_raw | (0xFFFFFFFFULL << NUM_BITS));
    }
    else
    {
        final_result = (long long)result_raw;
    }

    return final_result;
}