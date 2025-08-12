เห็นไหมว่าอะไรเกิดขึ้น PS C:\Users\Administrator\Desktop\ALU4B-Controller\mainCode_c> cd "c:\Users\Administrator\Desktop\ALU4B-Controller\mainCode_c\" ; if ($?) { gcc main.c -o main } ; if ($?) { .\main }

[INFO] กำลังรันคำสั่ง Arduino CLI...

[DEBUG] คำสั่ง: "C:\Users\Administrator\Desktop\arduino-cli.exe" co

mpile --upload -b arduino:avr:uno -p COM3 "C:\Users\Administrator\Desktop\ALU4B-Controller/ALU4B-Controller/ALU4B-Controller.ino"     

Sketch uses 5970 bytes (18%) of program storage space. Maximum is 32256 bytes.

Global variables use 212 bytes (10%) of dynamic memory, leaving 1836 bytes for local variables. Maximum is 2048 bytes.

[INFO] อัปโหลดโค้ด Arduino สำเร็จแล้ว

[INFO] กำลังรอให้บอร์ดพร้อมใช้งานเป็นเวลา 3000 มิลลิวินาที...

[DEBUG] กำลังเปิดพอร์ตซีเรียล: COM3



--- เริ่มต้นการทำงานของโปรแกรม Interpreter ---



Parsing: 'int ans = 100'



Parsing: 'int b = 20'



Parsing: 'ans = ans + b'



--- รันโค้ด Assembly ทั้งหมด ---

   DEF 0 100

   DEF 1 20

   LOAD REG_A 0

   LOAD REG_B 1

   ADD REG_A REG_B

   STORE REG_A 0

   HLT  4

[INFO] กำลังสร้างแผนที่ Label...

[DEBUG] พบ Label: '' ที่บรรทัด 2

[DEBUG] พบ Label: 'PL"' ที่บรรทัด 5

[DEBUG] พบ Label: 'L"' ที่บรรทัด 6



[PC:00] กำลังรันคำสั่ง: DEF 0 100

[INFO] DEFINE: กำหนดค่า 100 ให้กับ MEMORY[0]



[PC:01] กำลังรันคำสั่ง: DEF 1 20

[INFO] DEFINE: กำหนดค่า 20 ให้กับ MEMORY[1]



[PC:02] กำลังรันคำสั่ง: LOAD REG_A 0



[PC:03] กำลังรันคำสั่ง: LOAD REG_B 1

[INFO] LOAD 20 จาก MEMORY[1] เข้าสู่ REG_B



[PC:04] กำลังรันคำสั่ง: ADD REG_A REG_B

[INFO] กำลังประมวลผล REG_A + REG_B



[STEP 1] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 2] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 3] กำลังคำนวณบิต: A=0, B=1, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 1

"

[INFO] ได้รับข้อมูล 5 ไบต์: 1 0

[INFO] ALU half-add: halfSum=1 halfCarry=0 -> sum=1 carry=0        



[STEP 4] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 5] กำลังคำนวณบิต: A=0, B=1, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 1

"

[INFO] ได้รับข้อมูล 5 ไบต์: 1 0

[INFO] ALU half-add: halfSum=1 halfCarry=0 -> sum=1 carry=0        



[STEP 6] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 7] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 8] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 9] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 10] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 11] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 12] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 13] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 14] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 15] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 16] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 17] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 18] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 19] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 20] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 21] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 22] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 23] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 24] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 25] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 26] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 27] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 28] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 29] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 30] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 31] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 32] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        

[INFO] ผลลัพธ์ ADD: REG_A = 20, CARRY_FLAG = false, ZERO_FLAG = fal

se, SIGN_FLAG = false



[PC:05] กำลังรันคำสั่ง: STORE REG_A 0



[PC:06] กำลังรันคำสั่ง: HLT 4



[INFO] การทำงานเสร็จสมบูรณ์. กำลังส่งคืนค่าจาก Register...



--- ผลลัพธ์สุดท้ายจาก ALU ---

[INFO] ค่าใน REG_A หลังจากประมวลผล: 20

[INFO] ค่าใน REG_B หลังจากประมวลผล: 20

--- สิ้นสุดการแสดงผล ---



--- สิ้นสุดการรันโค้ด Assembly ---



--- ข้อมูลตัวแปรที่ถูกตั้งค่าใน Interpreter (อัปเดตจากบอร์ด Arduino

) ---

[INFO] MEM[0] (ans) = 20

[INFO] MEM[1] (b) = 20

--- สิ้นสุดการแสดงผลข้อมูล ---



--- สิ้นสุดการทำงานของโปรแกรม ---

[INFO] กำลังสร้างแผนที่ Label...



[PC:00] กำลังรันคำสั่ง: DEF 0 100

[INFO] DEFINE: กำหนดค่า 100 ให้กับ MEMORY[0]



[PC:01] กำลังรันคำสั่ง: DEF 1 20

[INFO] DEFINE: กำหนดค่า 20 ให้กับ MEMORY[1]



[PC:02] กำลังรันคำสั่ง: LOAD REG_A 0

[INFO] LOAD 100 จาก MEMORY[0] เข้าสู่ REG_A



[PC:03] กำลังรันคำสั่ง: LOAD REG_B 1

[INFO] LOAD 20 จาก MEMORY[1] เข้าสู่ REG_B



[PC:04] กำลังรันคำสั่ง: ADD REG_A REG_B

[INFO] กำลังประมวลผล REG_A + REG_B



[STEP 1] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 2] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 3] กำลังคำนวณบิต: A=1, B=1, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 1 1

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 1

[INFO] ALU half-add: halfSum=0 halfCarry=1 -> sum=0 carry=1        



[STEP 4] กำลังคำนวณบิต: A=0, B=0, Carry-in=1

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=1 carry=0        



[STEP 5] กำลังคำนวณบิต: A=0, B=1, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 1

"

[INFO] ได้รับข้อมูล 5 ไบต์: 1 0

[INFO] ALU half-add: halfSum=1 halfCarry=0 -> sum=1 carry=0        



[STEP 6] กำลังคำนวณบิต: A=1, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 1 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 1 0

[INFO] ALU half-add: halfSum=1 halfCarry=0 -> sum=1 carry=0        



[STEP 7] กำลังคำนวณบิต: A=1, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 1 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 1 0

[INFO] ALU half-add: halfSum=1 halfCarry=0 -> sum=1 carry=0        



[STEP 8] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 9] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 10] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 11] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 12] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 13] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 14] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 15] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 16] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 17] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 18] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 19] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 20] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 21] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 22] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 23] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 24] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 25] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 26] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 27] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 28] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 29] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 30] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 31] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        



[STEP 32] กำลังคำนวณบิต: A=0, B=0, Carry-in=0

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] กำลังเขียนข้อมูล 10 ไบต์: "001 0 0 0

"

[INFO] ได้รับข้อมูล 5 ไบต์: 0 0

[INFO] ALU half-add: halfSum=0 halfCarry=0 -> sum=0 carry=0        

[INFO] ผลลัพธ์ ADD: REG_A = 120, CARRY_FLAG = false, ZERO_FLAG = fa

lse, SIGN_FLAG = false



[PC:05] กำลังรันคำสั่ง: STORE REG_A 0

[INFO] STORE 120 จาก REG_A ไปยัง MEMORY[0]



[PC:06] กำลังรันคำสั่ง: PRINT ผลลัพธ์ REG_A



[PC:07] กำลังรันคำสั่ง: HLT

[INFO] คำสั่ง HLT. หยุดการทำงาน.



[INFO] การทำงานเสร็จสมบูรณ์. กำลังส่งคืนค่าจาก Register...

[DEBUG] กำลังล้างบัฟเฟอร์ของพอร์ตซีเรียล

[DEBUG] ปิดพอร์ตซีเรียลแล้ว เย่ๆ

PS C:\Users\Administrator\Desktop\ALU4B-Controller\mainCode_c> 



พอคำนวณเเบบเเอสเซมบลี้มันทำงานได้เฉยผลลัพเป๊ะ เเต่พอทำเเบบที่เป็นโค้ดระดับสูงมันใช้ไมไ่ด้  /*

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

        {"DEF", "0", "9999"},    // MEM[0] = 999

        {"DEF", "1", "3333"},    // MEM[1] = 333



        // โหลดค่าเข้าสู่ Register

        {"LOAD", "REG_A", "0"},  // REG_A = 999

        {"LOAD", "REG_B", "1"},  // REG_B = 333



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

            printf("   %s %s %s\n", assembly_program[i].instruction, assembly_program[i].operand1, assembly_program[i].operand2);

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



    Instruction program[] = {

        // กำหนดค่าเริ่มต้นให้กับ Memory

        {"DEF", "0", "100"}, // Memory[0] = 100

        {"DEF", "1", "20"},  // Memory[1] = 20



        // ทำการคำนวณ 100 + 20

        {"LOAD", "REG_A", "0"},    // REG_A = Memory[0] (100)

        {"LOAD", "REG_B", "1"},    // REG_B = Memory[1] (20)

        {"ADD", "REG_A", "REG_B"}, // REG_A = REG_A + REG_B (100 + 20)



        // เก็บผลลัพธ์ลงใน Memory[0] และแสดงผล

        {"STORE", "REG_A", "0"},     // Memory[0] = REG_A (120)

        {"PRINT", "ผลลัพธ์", "REG_A"}, // แสดงค่าใน REG_A



        // หยุดการทำงานของโปรแกรม

        {"HLT", "", ""},

    };



    int numInstructions = sizeof(program) / sizeof(Instruction);

    executeInstructions(program, numInstructions);



    clearSerialBuffer();

    CloseHandle(hSerial);

    printf("[DEBUG] ปิดพอร์ตซีเรียลแล้ว เย่ๆ\n");



    return 0;

}