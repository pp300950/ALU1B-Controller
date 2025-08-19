
### 🔹 คำสั่งคอมไพล์ สำหรับ UNO
```powershell
C:\Users\Administrator\Desktop\arduino-cli.exe compile -b arduino:avr:uno C:\Users\Administrator\Desktop\ALU4B-Controller/ALU4B-Controller/ALU4B-Controller.ino
```

```
###🔹 คำสั่งอัปโหลด สำหรับ UNO
C:\Users\Administrator\Desktop\arduino-cli.exe upload -b arduino:avr:uno -p COM6 C:\Users\Administrator\Desktop\ALU4B-Controller/ALU4B-Controller/ALU4B-Controller.ino
```
```
###🔹 คำสั่งรวม (คอมไพล์ + อัปโหลด) สำหรับ UNO
C:\Users\Administrator\Desktop\arduino-cli.exe compile --upload -b arduino:avr:uno -p COM6 C:\Users\Administrator\Desktop\ALU4B-Controller/ALU4B-Controller/ALU4B-Controller.ino
```
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

# **คู่มือการใช้งาน High-Level CPU Simulator และ Hardware ALU Controller**
*เอกสารนี้อธิบายการทำงานและวิธีใช้งานโปรแกรมจำลอง CPU ซึ่งทำหน้าที่คอมไพล์ภาษาระดับสูง (High-Level Language) ที่มีโครงสร้างคล้ายภาษา C ไปเป็นชุดคำสั่ง Assembly จำลอง จากนั้นจะทำการประมวลผลชุดคำสั่งเหล่านั้น โดยอาศัยการคำนวณทางคณิตศาสตร์และตรรกะจากวงจร ALU (Arithmetic Logic Unit) ภายนอกที่เชื่อมต่อกับบอร์ด Arduino ผ่านพอร์ตอนุกรม (Serial Port)*

---

## **📝 ภาษาระดับสูงที่รองรับ (High-Level Language)**
*คุณสามารถเขียนโปรแกรมโดยใช้ไวยากรณ์ง่ายๆ ที่คล้ายภาษา C มั๊กๆ ดังนี้*

### # 1. **การประกาศตัวแปร (Variable Declaration)**  
ใช้ `int` เพื่อประกาศตัวแปร สามารถกำหนดค่าเริ่มต้นได้ทันที ตัวแปรทั้งหมดจะถูกเก็บไว้ในหน่วยความจำจำลอง (MEMORY)

**ไวยากรณ์**
```
int <variable_name>;
int <variable_name> = <initial_value>;
// ประกาศตัวแปร score โดยไม่มีค่าเริ่มต้น (จะมีค่าเป็น 0)
int score;

// ประกาศตัวแปร players และกำหนดค่าเริ่มต้นเป็น 4
int players = 4;
```

---

# 2. การกำหนดค่า (Assignment)
ใช้เครื่องหมาย = เพื่อกำหนดค่าให้กับตัวแปรที่ประกาศไว้แล้ว สามารถใช้ร่วมกับการคำนวณพื้นฐานได้
```
<variable_name> = <value | variable | expression>;
int x = 10;
int y;
int z;

// กำหนดค่าจากตัวเลข
y = 25;

// กำหนดค่าจากตัวแปรอื่น
z = x;

// กำหนดค่าจากผลลัพธ์การคำนวณ
x = y + z;       // รองรับ: +, -, *, /
```

---

# 3. การแสดงผล (Printing)
ใช้ print() เพื่อแสดงข้อความหรือค่าของตัวแปรออกทางหน้าจอคอนโซล
```
print("<message>");
print(<variable_name>);
print("<message>", <variable_name>);

int final_score = 100;

// แสดงข้อความอย่างเดียว
print("Game Over!");

// แสดงค่าของตัวแปรอย่างเดียว
print(final_score);

// แสดงข้อความและค่าของตัวแปร
print("Your score is: ", final_score);
```

---

# 4. คำสั่งเงื่อนไข (Conditional Statements)
ใช้ if เพื่อสร้างเงื่อนไขในการทำงาน รองรับตัวเปรียบเทียบ ==, !=, <, >, <=, >=
```
if (<variable> <operator> <value | variable>) {
    // โค้ดที่จะทำงานเมื่อเงื่อนไขเป็นจริง
}
หมายเหตุ: ปัจจุบันยังไม่รองรับ else หรือ else if

int age = 18;
if (age >= 18) {
    print("You are an adult.");
}
if (age < 18) {
    print("You are a minor.");
}
```

---

# 5. คำสั่งวนซ้ำ (For Loop)
ใช้ for เพื่อทำงานซ้ำๆ ตามเงื่อนไขที่กำหนด
```
for (<initialization>; <condition>; <update>) {
    // โค้ดที่จะทำงานในแต่ละรอบ
}

int i;
int fact = 1;

// คำนวณ 3! (3 * 2 * 1)
for (i = 1; i <= 3; i++) {
    fact = fact * i;
}
print("Factorial of 3 is: ", fact);
```

---

# ⚙️ ชุดคำสั่ง Assembly จำลอง

โค้ดภาษาระดับสูงจะถูกแปลงเป็นชุดคำสั่งเหล่านี้ก่อนการประมวลผล

| คำสั่ง | ตัวอย่าง | คำอธิบาย |
|--------|-----------|-----------|
| DEF  | `DEF 0, 100` | กำหนดค่า MEMORY[0] = 100 |
| LOAD | `LOAD REG_A, 0` | โหลดค่าจาก MEMORY[0] ไปยัง REG_A |
| STORE| `STORE 0, REG_A` | เก็บค่า REG_A ลง MEMORY[0] |
| MOV  | `MOV REG_A, 50` | ย้ายค่าคงที่ 50 ไปยัง REG_A |
| ADD  | `ADD REG_A, REG_B` | บวกค่า REG_A ด้วย REG_B (ใช้ Hardware ALU) |
| SUB  | `SUB REG_A, REG_B` | ลบค่า REG_A ด้วย REG_B (ใช้ Hardware ALU) |
| MUL  | `MUL REG_A, REG_B` | คูณ (Shift-and-Add) |
| DIV  | `DIV REG_A, REG_B` | หาร (Shift-and-Subtract) |
| AND  | `AND REG_A, REG_B` | AND (ใช้ Hardware ALU) |
| OR   | `OR REG_A, REG_B` | OR (ใช้ Hardware ALU) |
| XOR  | `XOR REG_A, REG_B` | XOR (ใช้ Hardware ALU) |
| NOT  | `NOT REG_A` | NOT (ใช้ Hardware ALU) |
| CMP  | `CMP REG_A, REG_B` | เปรียบเทียบและตั้งค่า Flags (Z,S,C,O) |
| PRINT| `PRINT "Value:", REG_A` | แสดงข้อความและค่าจากรีจิสเตอร์ |
| JMP  | `JMP LOOP_START` | กระโดดแบบไม่มีเงื่อนไข |
| JZ   | `JZ END_IF` | กระโดดถ้า Zero Flag = 1 |
| JNZ  | `JNZ LOOP` | กระโดดถ้า Zero Flag = 0 |
| JC   | `JC CARRY_FOUND` | กระโดดถ้า Carry = 1 |
| JNC  | `JNC NO_CARRY` | กระโดดถ้า Carry = 0 |
| JG   | `JG GREATER_LABEL` | กระโดดถ้ามากกว่า (Signed) |
| JGE  | `JGE GREATER_EQ_LABEL` | กระโดดถ้ามากกว่าหรือเท่ากับ (Signed) |
| JLT  | `JLT LESS_LABEL` | กระโดดถ้าน้อยกว่า (Signed) |
| JLE  | `JLE LESS_EQ_LABEL` | กระโดดถ้าน้อยกว่าหรือเท่ากับ (Signed) |
| JO   | `JO OVERFLOW_ERROR` | กระโดดถ้า Overflow = 1 |
| JNO  | `JNO NORMAL_OPERATION` | กระโดดถ้า Overflow = 0 |
| HLT  | `HLT` | หยุดการทำงาน |

---

## 🔌 การทำงานร่วมกับ Hardware ALU
โปรแกรมนี้ไม่ได้จำลองการคำนวณทั้งหมด แต่จะส่งผ่านการคำนวณระดับบิตไปยังวงจร ALU ภายนอกผ่าน Serial Port โดยใช้รูปแบบคำสั่ง

| MuxCode | ฟังก์ชัน | คำอธิบาย |
|---------|----------|-----------|
| `001` | ADD/SUB | บวก (subAddFlag=0) หรือ ลบ (subAddFlag=1) |
| `100` | AND     | A & B |
| `101` | OR      | A \| B |
| `110` | XOR     | A ^ B |
| `111` | NOT     | ~B |
