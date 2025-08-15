เวลารัน
คอมไพล์โค้ดก่อน >> เเล้วค่อยอัปโหลด

คำสั่งคอมไพล์ สำหรับ UNO
C:\Users\Administrator\Desktop\arduino-cli.exe compile -b arduino:avr:uno C:\Users\Administrator\Desktop\ALU4B-Controller/ALU4B-Controller/ALU4B-Controller.ino

คำสั่งอัปโหลด สำหรับ UNO
C:\Users\Administrator\Desktop\arduino-cli.exe upload -b arduino:avr:uno -p COM6 C:\Users\Administrator\Desktop\ALU4B-Controller/ALU4B-Controller/ALU4B-Controller.ino

คำสั่งรวม UNO
C:\Users\Administrator\Desktop\arduino-cli.exe compile --upload -b arduino:avr:uno -p COM6 C:\Users\Administrator\Desktop\ALU4B-Controller/ALU4B-Controller/ALU4B-Controller.ino

---

# คู่มือการใช้งาน 1-Bit ALU Interactive Controller

คู่มือนี้อธิบายวิธีการใช้งานโปรแกรมควบคุม ALU (หน่วยคำนวณและตรรกะ) ขนาด 1 บิต ผ่าน Serial Monitor ของ Arduino ผู้ใช้สามารถป้อนคำสั่งเพื่อเลือกฟังก์ชันการคำนวณต่างๆ และดูผลลัพธ์ได้ทันที

---

## ⚙️ สิ่งที่ต้องมี

- Arduino Board ที่อัปโหลดโค้ด `ALU_Interactive_Controller.ino` เรียบร้อยแล้ว
- วงจร 1-Bit ALU ที่เชื่อมต่อกับขาของ Arduino อย่างถูกต้อง
- โปรแกรม Arduino IDE หรือโปรแกรม Serial Monitor อื่นๆ เพื่อใช้ส่งคำสั่ง

---

## 🚀 วิธีการใช้งาน

1.  **เชื่อมต่อ Arduino** เข้ากับคอมพิวเตอร์ผ่านสาย USB
2.  **เปิดโปรแกรม Arduino IDE** และไปที่เมนู `Tools` > `Serial Monitor` (หรือกด `Ctrl+Shift+M`)
3.  **ตั้งค่า Baud Rate** ที่มุมขวาล่างของหน้าต่าง Serial Monitor ให้เป็น **`9600`**
4.  **พิมพ์คำสั่ง** ลงในช่องป้อนข้อมูลตามรูปแบบที่กำหนด แล้วกด `Enter` หรือ `Send`

    

---

## ⌨️ รูปแบบคำสั่ง

คำสั่งที่ใช้ควบคุม ALU จะต้องอยู่ในรูปแบบ `MuxCode Sub/Add A B` โดยแต่ละส่วนคั่นด้วยการเว้นวรรค

`MuxCode Sub/Add A B`

-   **`MuxCode`**: เลขฐานสอง **3 บิต** สำหรับเลือกฟังก์ชันที่ต้องการ (ดูตารางด้านล่าง)
-   **`Sub/Add`**: เลข **1 บิต** (`0` หรือ `1`) ใช้เพื่อเลือกระหว่างการ **บวก** หรือ **ลบ** จะมีผลเฉพาะเมื่อ `MuxCode` เป็น `001` เท่านั้น สำหรับฟังก์ชันอื่นให้ใส่เป็น `0`
-   **`A`**: ค่าอินพุตตัวที่หนึ่ง (`0` หรือ `1`)
-   **`B`**: ค่าอินพุตตัวที่สอง (`0` หรือ `1`)

---

## 🧮 รายการฟังก์ชันและ MuxCode

ใช้ตารางนี้เพื่ออ้างอิง `MuxCode` สำหรับการคำนวณที่ต้องการ

| MuxCode | ชื่อฟังก์ชัน | คำอธิบายการทำงาน |
| :---: | :--- | :--- |
| `000` | **OR** | ทำการ OR ระหว่าง A และ B (A \| B) |
| `001` | **ADD / SUB** | - `Sub/Add=0` : บวก (A + B) <br>- `Sub/Add=1` : ลบ (A - B) |
| `010` | **CARRY OUT** | แสดงผลเฉพาะค่าตัวทด (Carry) จากการบวก A + B |
| `100` | **AND** | ทำการ AND ระหว่าง A และ B (A & B) |
| `101` | **XOR** | ทำการ XOR ระหว่าง A และ B (A ^ B) |
| `111` | **NOT** | กลับค่าของอินพุต A (~A) โดยจะไม่สนใจค่าอินพุต B |

---

## ✨ ตัวอย่างการใช้งาน

### ตัวอย่างที่ 1: การ OR (1 OR 0)

-   **คำสั่ง:** `000 0 1 0`
-   **ผลลัพธ์ที่แสดง:**
    ```
    ----------------------------------------
    Input MuxCode: 000
    Input Sub/Add Pin: 0
    Input A: 1
    Input B: 0
    ---
    Digital Output (Result): 1
    Digital Output (Negative Flag): 0
    ----------------------------------------
    ```

### ตัวอย่างที่ 2: การบวก (1 + 1)

-   **คำสั่ง:** `001 0 1 1`
-   **ผลลัพธ์ที่แสดง:**
    ```
    ----------------------------------------
    Input MuxCode: 001
    Input Sub/Add Pin: 0
    Input A: 1
    Input B: 1
    ---
    Digital Output (Result): 0
    Digital Output (Negative Flag): 0
    Digital Output (Carry Flag): 1
    ----------------------------------------
    ```
    *สังเกตว่าผลลัพธ์ (Result) คือ `0` และมีค่าตัวทด (Carry Flag) เป็น `1`*

### ตัวอย่างที่ 3: การลบ (0 - 1)

-   **คำสั่ง:** `001 1 0 1`
-   **ผลลัพธ์ที่แสดง:**
    ```
    ----------------------------------------
    Input MuxCode: 001
    Input Sub/Add Pin: 1
    Input A: 0
    Input B: 1
    ---
    Digital Output (Result): 1
    Digital Output (Negative Flag): 1
    Digital Output (Carry Flag): 0
    ----------------------------------------
    ```
    *สังเกตว่าค่าติดลบ (Negative Flag) เป็น `1` เพื่อแสดงว่าผลลัพธ์ติดลบ*

### ตัวอย่างที่ 4: การ NOT (NOT 0)

-   **คำสั่ง:** `111 0 0 0`
-   **ผลลัพธ์ที่แสดง:**
    ```
    ----------------------------------------
    Input MuxCode: 111
    Input Sub/Add Pin: 0
    Input A: 0
    Input B: 0
    ---
    Digital Output (Result): 1
    Digital Output (Negative Flag): 0
    ----------------------------------------
    ```
    *สังเกตว่า B เป็น 0 แต่ไม่ถูกนำมาคำนวณ และผลลัพธ์คือค่าที่กลับกันของ A*

---

## 🔧 การแก้ไขปัญหาเบื้องต้น

-   **ขึ้นข้อความ "Error: Invalid input format..."**:
    -   ตรวจสอบว่าพิมพ์คำสั่งครบ 4 ส่วน และคั่นด้วยการเว้นวรรค
    -   ตัวอย่างที่ผิด: `001011`, `100 1 1`
-   **ไม่แสดงผลอะไรเลย**:
    -   ตรวจสอบว่าตั้งค่า Baud Rate เป็น `9600`
    -   ตรวจสอบว่าเลือก COM Port ถูกต้อง
-   **ผลลัพธ์ไม่ถูกต้อง**:
    -   ตรวจสอบการเดินสายไฟระหว่าง Arduino และวงจร ALU อีกครั้ง

---

## 📜 คู่มือ Assembly จำลอง (serial_comm.c)
 **🛠 ส่วนประกอบหลัก**
- REG_A, REG_B — Register เก็บค่าชั่วคราว
- MEMORY — หน่วยความจำหลัก
- CARRY_FLAG — สถานะการบวก/ลบ (true = มี Carry/Borrow)

 **📋 คำสั่ง Assembly**

 **คำสั่งที่รองรับในปัจจุบัน**
 *การจัดการข้อมูล:*
- DEF: กำหนดค่าตัวเลขให้กับหน่วยความจำ (Memory)
- LOAD: โหลดค่าจากหน่วยความจำเข้าสู่รีจิสเตอร์ (Register)
- STORE: เก็บค่าจากรีจิสเตอร์ลงในหน่วยความจำ
- MOV: ย้ายค่าจากรีจิสเตอร์หนึ่งไปอีกรีจิสเตอร์หนึ่ง หรือย้ายค่าคงที่เข้าสู่รีจิสเตอร์

 *การคำนวณและตรรกะ:*
- ADD: คำสั่งบวกเลข (ใช้ ALU)
- SUB: คำสั่งลบเลข (ใช้ ALU)
- CMP: เปรียบเทียบค่าสองค่าและตั้งค่าแฟล็ก (Zero, Sign, Carry)

 *การควบคุมโปรแกรม:*
- JMP: กระโดดไปยังตำแหน่งที่ระบุ (Label) โดยไม่มีเงื่อนไข
- JNC: กระโดดเมื่อไม่มีการทด (Carry Flag เป็น false)
- JZ: กระโดดเมื่อผลลัพธ์เป็นศูนย์ (Zero Flag เป็น true)
- JNZ: กระโดดเมื่อผลลัพธ์ไม่เป็นศูนย์ (Zero Flag เป็น false)
- HLT: หยุดการทำงานของโปรแกรม

---



คำสั่งรัน
gcc fix_Sub.c cpu_logic.c -o fix_Sub
