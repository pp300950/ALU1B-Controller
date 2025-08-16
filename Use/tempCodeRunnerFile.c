else if (strcmp(current.instruction, "CMP") == 0)
        {
            long long op1_val = getOperandValue(current.operand1);
            long long op2_val = getOperandValue(current.operand2);
            bool temp_carry_not = false;

            printf("       [INFO] CMP: เริ่มประมวลผล %lld เปรียบเทียบกับ %lld\n", op1_val, op2_val);

            // ขั้นตอนที่ 1: Invert op2_val (~op2_val)
            long long val2_invert = executeAluOperation(op2_val, 0, "111", 0, &temp_carry_not);
            printf("       [DEBUG] CMP Step 1 (NOT): ~%lld (op2_val) -> %lld\n", op2_val, val2_invert);

            // ขั้นตอนที่ 2: Add 1 to inverted op2_val (Two's Complement)
            long long negated_op2_val = executeAluOperation(1, val2_invert, "001", 0, &CARRY_FLAG);
            //long long negated_op2_val = val2_invert + 1;
            printf("       [DEBUG] CMP Step 2 (ADD 1): %lld + 1 -> %lld (Two's Complement)\n", val2_invert, negated_op2_val);

            // ขั้นตอนที่ 3: Add op1_val and negated_op2_val
            // CARRY_FLAG จะถูกตั้งค่าจากการบวกนี้
            bool carry_flag_temp = false;
            long long final_result_for_flags = executeAluOperation(op1_val, negated_op2_val, "001", 0, &carry_flag_temp);
            CARRY_FLAG = !carry_flag_temp; // Invert carry for subtract (borrow)
            printf("       [DEBUG] CMP Step 3 (ADD): %lld + %lld (Two's Complement) -> %lld\n", op1_val, negated_op2_val, final_result_for_flags);

            // ตั้งค่า ZERO_FLAG และ SIGN_FLAG ตามผลลัพธ์สุดท้าย
            ZERO_FLAG = (final_result_for_flags == 0);

            // แปลงผลลัพธ์เป็น signed 32-bit ก่อนคำนวณ sign
            int signed_result = (int)final_result_for_flags;
            SIGN_FLAG = (signed_result < 0);

            // แสดงผลลัพธ์และ Flags สุดท้ายของ CMP
            printf("       [INFO] CMP Finished: %lld vs %lld. ผลลัพธ์จากการลบ (เพื่อ Flags เท่านั้น)=%lld. Flags: Z=%d, S=%d, C=%d\n",
                   op1_val, op2_val, final_result_for_flags, ZERO_FLAG, SIGN_FLAG, CARRY_FLAG);
        }